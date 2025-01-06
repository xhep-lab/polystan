#ifndef POLYSTAN_MODEL_HPP_
#define POLYSTAN_MODEL_HPP_

#include <filesystem>
#include <limits>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "polystan/read.hpp"
#include "polystan/json.hpp"
#include "polystan/read_err.hpp"
#include "polystan/version.hpp"
#include "polystan/metadata.hpp"
#include "polystan/mpi.hpp"

#include "bridgestan/src/bridgestan.h"
#include "polychord/interfaces.hpp"

namespace polystan {

std::optional<std::string> unconstrain_err(bs_model* model,
                                           std::vector<double> theta) {
  std::vector<double> theta_unc(theta.size());
  char* err;
  const int err_code
      = bs_param_unconstrain(model, theta.data(), theta_unc.data(), &err);
  return err_code == 0 ? std::nullopt
                       : std::optional<std::string>(add_to_err(err));
}

bs_model* make_bs_model(const std::string& data_file_name, unsigned int seed) {
  char* err;

  bs_model* model = bs_model_construct(data_file_name.c_str(), seed, &err);

  if ((model == nullptr) && (err != nullptr)) {
    std::string err_msg = add_to_err(err);
    if (data_file_name.empty()) {
      err_msg += "\nDid your model require a data file to be specified e.g. by data --file=your-data-file.json?\n";
    }
    throw std::runtime_error(err_msg);
  }

  return model;
}

bs_rng* make_bs_rng(bs_model* model, unsigned int seed) {
  if (bs_param_num(model, false, true) == bs_param_num(model, false, false)) {
    return nullptr;
  }

  char* err;
  bs_rng* rng = bs_rng_construct(seed, &err);

  if ((rng == nullptr) && (err != nullptr)) {
    throw std::runtime_error(add_to_err(err));
  }

  return rng;
}

double loglike(bs_model* model, bs_rng* rng, double* theta, int ndim,
               double* phi, int nderived) {
  int err_code = 0;
  char* err;

  // compute unconstrained parameters

  std::vector<double> theta_unc(ndim);
  err_code = bs_param_unconstrain(model, theta, theta_unc.data(), &err);

  if (err_code != 0) {
    throw std::runtime_error(add_to_err(err));
  }

  // constrain parameters to compute derived

  std::vector<double> theta_phi(ndim + nderived);
  err_code = bs_param_constrain(model, true, rng != nullptr, theta_unc.data(),
                                theta_phi.data(), rng, &err);

  if (err_code != 0) {
    throw std::runtime_error(add_to_err(err));
  }

  for (int i = 0; i < nderived; i++) {
    phi[i] = theta_phi[i + ndim];
  }

  // compute density - stan works on unconstrained space

  double loglike;
  err_code
      = bs_log_density(model, false, false, theta_unc.data(), &loglike, &err);

  if (err_code != 0) {
    throw std::runtime_error(add_to_err(err));
  }

  return loglike;
}

class Model {
 public:
  Model(const std::string& data_file_name, unsigned int seed, Settings settings)
      : seed(seed),
        data_file_name(data_file_name),
        model(make_bs_model(data_file_name, seed)),
        rng(make_bs_rng(model, seed)),
        settings(std::move(settings)) {
    check_unit_hypercube();
    fix_settings();
  }

  void check_unit_hypercube() const {
    const std::string msg
        = "\nParameters are not defined on unit hypercube; "
          "expect e.g. real<lower=0, upper=1>\n";

    const std::vector<double> zeros(ndims(), 0.);
    const auto zeros_err = unconstrain_err(model, zeros);
    if (zeros_err.has_value()) {
      throw std::runtime_error(zeros_err.value() + msg);
    }

    for (int i = 0; i < ndims(); i++) {
      std::vector<double> one(ndims(), 0.);
      one[i] = 1.;
      const auto one_err = unconstrain_err(model, one);
      if (one_err.has_value()) {
        throw std::runtime_error(one_err.value() + msg);
      }

      std::vector<double> gt_one(ndims(), 0.5);
      gt_one[i] = 1. + std::numeric_limits<double>::round_error();
      const auto gt_err = unconstrain_err(model, gt_one);
      if (!gt_err.has_value()) {
        throw std::runtime_error("> 1 was not out of bounds for parameter "
                                 + param_names()[i] + msg);
      }

      std::vector<double> lt_zero(ndims(), 0.5);
      lt_zero[i] = -std::numeric_limits<double>::round_error();
      const auto lt_err = unconstrain_err(model, lt_zero);
      if (!lt_err.has_value()) {
        throw std::runtime_error("< 0 was not out of bounds for parameter "
                                 + param_names()[i] + msg);
      }
    }
  }

  void run() const {
    std::filesystem::create_directory(settings.base_dir);

    static bs_model* model_(model);
    static bs_rng* rng_(rng);

    const auto this_loglike
        = [](double* theta, int ndim, double* phi, int nderived) {
            return loglike(model_, rng_, theta, ndim, phi, nderived);
          };

#ifdef USE_MPI
    run_polychord(this_loglike, settings, mpi::get_comm());
#else
    run_polychord(this_loglike, settings);
#endif
  }

  void write(const std::string& json_file_name) const {
    // polystan metadata

    json::Object polystan;

    polystan.add("polystan version", version);
    polystan.add("stan file name", stan_file_name);
    polystan.add("stan data file", data_file_name);
    polystan.add("stan build info", stan_build_info());
    polystan.add("seed", seed);

    // polychord metadata

    json::Object polychord;

    polychord.add("polychord version", polychord_version);
    polychord.add("nlive", settings.nlive);
    polychord.add("num_repeats", settings.num_repeats);
    polychord.add("precision_criterion", settings.precision_criterion);
    polychord.add("seed", settings.seed);

    // evidence

    json::Object evidence_;

    if (settings.write_stats) {
      const auto [logz, error_logz] = evidence();

      evidence_.add("metadata", "The evidence is log-normally distributed");
      evidence_.add("log evidence", logz);
      evidence_.add("error log evidence", error_logz);
    } else {
      evidence_.set("did not write stats file");
    }

    // samples

    json::Object samples_;

    if (settings.equals) {
      auto names_ = names();
      names_.insert(names_.begin(), "log likelihood");

      samples_.add("metadata", "These samples are equally weighted");
      json::Object data;
      data.add(names_, samples());
      samples_.add("data", data);
    } else {
      samples_.set("did not write equally weighted points");
    }

    // write to disk

    json::Object document;
    document.add("polystan", polystan);
    document.add("polychord", polychord);
    document.add("evidence", evidence_);
    document.add("samples", samples_);
    document.write(json_file_name);
  }

  const std::string data_file_name;
  const std::string stan_file_name{polystan::stan_file_name};

  std::string stan_build_info() const {
    std::string build_info = bs_model_info(model);
    build_info = std::regex_replace(build_info, std::regex("\n"), ", ");
    build_info = std::regex_replace(build_info, std::regex("\t"), "");
    return build_info;
  }

  std::vector<std::string> names() const {
    return read::param_names(bs_param_names(model, true, true));
  }

  std::vector<std::string> param_names() const {
    std::vector<std::string> names_ = names();
    return std::vector<std::string>(names_.begin(), names_.begin() + ndims());
  }

  std::vector<std::string> derived_names() const {
    std::vector<std::string> names_ = names();
    return std::vector<std::string>(names_.begin() + ndims(), names_.end());
  }

  int ndims() const { return bs_param_num(model, false, false); }

  int nderived() const {
    return bs_param_num(model, true, true) - bs_param_num(model, false, false);
  }

  std::string basename() const {
    return std::filesystem::weakly_canonical(settings.base_dir)
           / settings.file_root;
  }

  std::vector<std::vector<double>> samples() const {
    return read::samples(basename() + "_equal_weights.txt");
  }

  std::array<double, 2> evidence() const {
    return read::evidence(basename() + ".stats");
  }

 private:
  void fix_settings() {
    settings.nDims = ndims();
    settings.nDerived = nderived();

    const Settings default_zero(0, 0);
    const Settings fixed(settings.nDims, settings.nDerived);

    if (settings.grade_dims == default_zero.grade_dims) {
      settings.grade_dims = fixed.grade_dims;
    }

    if (settings.num_repeats == default_zero.num_repeats) {
      settings.num_repeats = fixed.num_repeats;
    }
  }

  bs_model* model;
  bs_rng* rng;
  Settings settings;
  unsigned int seed;
};

}  // end namespace polystan

#endif  // POLYSTAN_MODEL_HPP_
