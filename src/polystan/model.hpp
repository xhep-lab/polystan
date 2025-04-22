#ifndef POLYSTAN_MODEL_HPP_
#define POLYSTAN_MODEL_HPP_

#include <ctime>
#include <filesystem>
#include <limits>
#include <optional>
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
#include "polystan/test.hpp"

#include "bridgestan/src/bridgestan.h"
#include "polychord/interfaces.hpp"

namespace polystan {

std::optional<std::string> unconstrain_err(const bs_model* model,
                                           const std::vector<double>& theta) {
  double* theta_unc = new double[theta.size()];
  char* err;
  const int err_code
      = bs_param_unconstrain(model, theta.data(), theta_unc, &err);
  delete[] theta_unc;
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

bs_rng* make_bs_rng(const bs_model* model, unsigned int seed) {
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

double loglike(const bs_model* model, bs_rng* rng, double* theta, int ndim,
               double* phi, int nderived) {
  int err_code = 0;
  char* err;

  // compute unconstrained parameters

  double* theta_unc = new double[ndim];
  err_code = bs_param_unconstrain(model, theta, theta_unc, &err);

  if (err_code != 0) {
    throw std::runtime_error(add_to_err(err));
  }

  // constrain parameters to compute derived

  if (nderived > 0) {
    double* theta_phi = new double[ndim + nderived];
    err_code = bs_param_constrain(model, true, rng != nullptr, theta_unc,
                                  theta_phi, rng, &err);

    if (err_code != 0) {
      throw std::runtime_error(add_to_err(err));
    }

    for (int i = 0; i < nderived; i++) {
      phi[i] = theta_phi[i + ndim];
    }

    delete[] theta_phi;
  }

  // compute density - stan works on unconstrained space

  double loglike;
  err_code = bs_log_density(model, false, false, theta_unc, &loglike, &err);

  delete[] theta_unc;

  if (err_code != 0) {
    throw std::runtime_error(add_to_err(err));
  }

  return loglike;
}

class Model {
 public:
  Model(const std::string& data_file_name, unsigned int seed,
        const Settings& settings, bool no_derived)
      : seed(seed),
        _no_derived(no_derived),
        data_file_name(data_file_name),
        model(make_bs_model(data_file_name, seed)),
        rng(make_bs_rng(model, seed)),
        settings(settings) {
    check_unit_hypercube();
    fix_settings();
  }

  void check_unit_hypercube() const {
    const std::string msg
        = "\nParameters are not defined on unit hypercube; "
          "expect e.g. real<lower=0, upper=1>\n";
    const int ndims_ = ndims();
    const std::vector<double> zeros(ndims_, 0.);
    const auto zeros_err = unconstrain_err(model, zeros);
    if (zeros_err.has_value()) {
      throw std::runtime_error(zeros_err.value() + msg);
    }

    const std::vector<double> ones(ndims_, 1.);
    const auto ones_err = unconstrain_err(model, ones);
    if (ones_err.has_value()) {
      throw std::runtime_error(ones_err.value() + msg);
    }

    for (int i = 0; i < ndims_; i++) {
      std::vector<double> gt_one(ndims_, 0.5);
      gt_one[i] = 1. + std::numeric_limits<double>::round_error();
      const auto gt_err = unconstrain_err(model, gt_one);
      if (!gt_err.has_value()) {
        throw std::runtime_error("> 1 was not out of bounds for parameter "
                                 + param_names()[i] + msg);
      }

      std::vector<double> lt_zero(ndims_, 0.5);
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

    static const bs_model* model_(model);
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

  void write(const std::string& json_file_name,
             const std::string& toml_file_name) const {
    // polystan metadata

    json::Object polystan;

    polystan.add("stan file name", stan_file_name);
    polystan.add("stan data file", data_file_name);
    polystan.add("polystan toml file", toml_file_name);
    polystan.add("stan build info", stan_build_info());
    polystan.add("seed", seed);

    // polychord metadata

    json::Object polychord;

    polychord.add("nlive", settings.nlive);
    polychord.add("num_repeats", settings.num_repeats);
    polychord.add("precision_criterion", settings.precision_criterion);
    polychord.add("seed", settings.seed);

    // metadata format

    json::Object metadata;

    const std::time_t raw = std::time(nullptr);
    std::string now(std::ctime(&raw));
    now.erase(std::remove(now.begin(), now.end(), '\n'), now.cend());

    metadata.add("name", name());
    metadata.add("created_at", now);
    metadata.add("inference_library", "PolyChord");
    metadata.add("inference_library_version", polychord_version);
    metadata.add("creation_library", "PolyStan");
    metadata.add("creation_library_version", version);
    metadata.add("creation_library_language", "C++");
    metadata.add("polystan", polystan);
    metadata.add("polychord", polychord);

    // effective sample size

    json::Object ess_entry;
    const auto ess_ = ess();

    if (ess_.has_value()) {
      ess_entry.add("metadata", "Kish estimate of effective sample size");
      ess_entry.add("n", ess_.value());
    } else {
      ess_entry.set("did not write weighted samples file");
    }

    // test

    json::Object test;
    const auto p_value_ = p_value();

    if (p_value_.has_value()) {
      test.add(
          "metadata",
          "This is a test of uniformity of insertion indexes of live points");
      test.add("p-value", p_value_.value());
      test.add("batch size / n_live", batch);
    } else {
      test.set("did not write dead points file");
    }

    // evidence

    json::Object evidence_entry;
    const auto evidence_ = evidence();

    if (evidence_.has_value()) {
      const auto [logz, err] = evidence_.value();
      evidence_entry.add("metadata",
                         "The evidence is log-normally distributed");
      evidence_entry.add("log evidence", logz);
      evidence_entry.add("error log evidence", err);
    } else {
      evidence_entry.set("did not write stats file");
    }

    // add samples stats data

    json::Object sample_stats;
    sample_stats.add("test", test);
    sample_stats.add("ess", ess_entry);
    sample_stats.add("evidence", evidence_entry);

    // samples

    auto names_ = names();
    names_.insert(names_.begin(), "log-likelihood");

    json::Object posterior_entry;
    const auto posterior_samples_ = posterior_samples();

    if (posterior_samples_.has_value()) {
      posterior_entry.add(names_, posterior_samples_.value());
    } else {
      posterior_entry.set("did not write equally weighted posterior points");
    }

    json::Object prior_entry;
    const auto prior_samples_ = prior_samples();

    if (prior_samples_.has_value()) {
      prior_entry.add(names_, prior_samples_.value());
    } else {
      prior_entry.set("did not write equally weighted prior points");
    }

    // write to disk

    json::Object document;
    document.copy("posterior_attrs", metadata);
    document.copy("prior_attrs", metadata);
    document.add("sample_stats_attrs", metadata);
    document.add("sample_stats", sample_stats);
    document.add("posterior", posterior_entry);
    document.add("prior", prior_entry);
    document.write(json_file_name);
  }

  const std::string data_file_name;
  const std::string stan_file_name{polystan::stan_file_name};

  std::string stan_build_info() const {
    std::string build_info = bs_model_info(model);
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
    if (no_derived()) {
      return 0;
    }
    return bs_param_num(model, true, true) - bs_param_num(model, false, false);
  }

  std::string name() const { return bs_name(model); }

  std::string basename() const {
    return std::filesystem::weakly_canonical(settings.base_dir)
           / settings.file_root;
  }

  std::optional<std::vector<std::vector<double>>> posterior_samples() const {
    if (!settings.equals) {
      return std::nullopt;
    }
    return read::samples(basename() + "_equal_weights.txt");
  }

  std::optional<std::vector<std::vector<double>>> prior_samples() const {
    if (!settings.write_prior) {
      return std::nullopt;
    }
    return read::samples(basename() + "_prior.txt");
  }

  std::optional<std::array<double, 2>> evidence() const {
    if (!settings.write_stats) {
      return std::nullopt;
    }
    return read::evidence(basename() + ".stats");
  }

  std::optional<double> p_value() const {
    if (!settings.write_dead) {
      return std::nullopt;
    }
    return test::insertion_index_p_value(basename() + "_dead-birth.txt",
                                         settings.nlive, batch);
  }

  std::optional<double> ess() const {
    if (!settings.posteriors) {
      return std::nullopt;
    }
    return test::ess(basename() + ".txt");
  }

  bool no_derived() const {
    return _no_derived
           || (!settings.write_prior && !settings.write_live
               && !settings.write_resume && !settings.write_dead
               && !settings.posteriors && !settings.equals
               && !settings.write_stats);
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

  const bs_model* model;
  bs_rng* rng;
  Settings settings;
  const unsigned int seed;
  const int batch = 1;
  const bool _no_derived;
};

}  // end namespace polystan

#endif  // POLYSTAN_MODEL_HPP_
