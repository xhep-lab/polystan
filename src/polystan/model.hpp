#ifndef POLYSTAN_MODEL_HPP_
#define POLYSTAN_MODEL_HPP_

#include <filesystem>
#include <random>
#include <regex>
#include <string>
#include <vector>

#include "polystan/read.hpp"
#include "polystan/json.hpp"
#include "polystan/version.hpp"
#include "polystan/mpi.hpp"

#include "bridgestan/src/bridgestan.h"
#include "polychord/interfaces.hpp"

namespace polystan {

double logit(double x) { return std::log(x / (1 - x)); }

bool unit_hypercube(bs_model* model) {
  const double tol = 1e-5;

  const int ndim = bs_param_num(model, 0, 0);
  double theta_unc[ndim];
  double theta[ndim];

  std::random_device r;
  std::default_random_engine e(r());
  std::uniform_real_distribution<double> u(0, 1);

  for (int i = 0; i < ndim; i++) {
    theta[i] = u(e);
  }

  char* err;
  const int err_code
      = bs_param_constrain(model, 0, 0, theta_unc, theta, nullptr, &err);

  if (err_code != 0) {
    throw std::runtime_error("Error in bs_param_constrain: "
                             + std::string(err));
  }

  for (int i = 0; i < ndim; i++) {
    const double diff = std::abs(logit(theta[i]) - theta_unc[i]);

    if (diff > tol) {
      return false;
    }
  }

  return true;
}

bs_model* make_bs_model(std::string data_file_name, unsigned int seed) {
  char* err;

  bs_model* model = bs_model_construct(data_file_name.c_str(), seed, &err);

  if (!model && err) {
    std::string err_msg = "Error in bs_model_construct: " + std::string(err);
    if (data_file_name.empty()) {
      err_msg += "\nDid your model require a data file to be specified?";
    }
    throw std::runtime_error(err_msg);
  }

  if (!unit_hypercube(model)) {
    throw std::runtime_error(
        "Parameters are not defined on unit hypercube; expect e.g. "
        "real<lower=0, upper=1>");
  }

  return model;
}

bs_rng* make_bs_rng(bs_model* model, unsigned int seed) {
  if (bs_param_num(model, 0, 1) == bs_param_num(model, 0, 0)) {
    return nullptr;
  }

  char* err;
  bs_rng* rng = bs_rng_construct(seed, &err);

  if (!rng && err) {
    throw std::runtime_error("Error in bs_rng_construct: " + std::string(err));
  }

  return rng;
}

double loglike(bs_model* model, bs_rng* rng, double* theta, int ndim,
               double* phi, int nderived) {
  int err_code = 0;
  char* err;

  double theta_unc[ndim];
  err_code = bs_param_unconstrain(model, theta, theta_unc, &err);

  if (err_code != 0) {
    throw std::runtime_error("Error in bs_param_unconstrain: "
                             + std::string(err));
  }

  double theta_phi[ndim + nderived];
  err_code = bs_param_constrain(model, 1, rng != nullptr, theta_unc, theta_phi,
                                rng, &err);

  if (err_code != 0) {
    throw std::runtime_error("Error in bs_param_constrain: "
                             + std::string(err));
  }

  for (int i = 0; i < nderived; i++) {
    phi[i] = theta_phi[i + ndim];
  }

  double loglike;
  err_code = bs_log_density(model, 0, 0, theta_unc, &loglike, &err);

  if (err_code != 0) {
    throw std::runtime_error("Error in bs_log_density: " + std::string(err));
  }

  return loglike;
}

class Model {
 public:
  Model(std::string data_file_name, unsigned int seed, Settings settings)
      : seed(seed),
        data_file_name(data_file_name),
        model(make_bs_model(data_file_name, seed)),
        rng(make_bs_rng(model, seed)),
        settings(settings) {
    fix_settings();
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

  void write(std::string json_file_name) const {
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
      samples_.add(names_, samples());
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
    return read::param_names(bs_param_names(model, 1, 1));
  }

  std::vector<std::string> param_names() const {
    std::vector<std::string> names_ = names();
    return std::vector<std::string>(names_.begin(), names_.begin() + ndims());
  }

  std::vector<std::string> derived_names() const {
    std::vector<std::string> names_ = names();
    return std::vector<std::string>(names_.begin() + ndims(), names_.end());
  }

  int ndims() const { return bs_param_num(model, 0, 0); }

  int nderived() const {
    return bs_param_num(model, 1, 1) - bs_param_num(model, 0, 0);
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
