#ifndef POLYSTAN_POLYCHORD_CLI_HPP_
#define POLYSTAN_POLYCHORD_CLI_HPP_

#include <iostream>

#include "CLI11/CLI11.hpp"
#include "polychord/interfaces.hpp"

namespace polystan {

std::string bool2str(bool b) {
  std::stringstream converter;
  converter << std::boolalpha << b;
  return converter.str();
}

void AddFlag(CLI::App* app, std::string flag, bool& var, std::string help) {
  app->add_flag(flag, var, help)->default_val(var)->default_str(bool2str(var));
}

void AddPolyChord(CLI::App* app, Settings* settings) {
  app->add_option("--nlive", settings->nlive,
                  "The number of live points. Increasing nlive increases the "
                  "accuracy of posteriors and evidences, and proportionally "
                  "increases runtime ~ O(nlive).")
      ->check(CLI::PositiveNumber);

  app->add_option(
         "--num-repeats", settings->num_repeats,
         "The number of slice slice-sampling steps to generate a new point. "
         "Increasing num_repeats increases the reliability of the algorithm. "
         "Typically, for reliable evidences need num_repeats ~ O(5*nDims) and "
         "for reliable posteriors need num_repeats ~ O(nDims). "
         "If 0, defaults to 5 * nDims.")
      ->check(CLI::NonNegativeNumber);

  app->add_option(
      "--nprior", settings->nprior,
      "The number of prior samples to draw before starting compression. "
      "If -1, defaults to nLive.");

  app->add_option(
      "--nfail", settings->nfail,
      "The number of failed spawns before stopping nested sampling. "
      "If -1, defaults to nLive.");

  AddFlag(app, "--cluster,!--no-cluster", settings->do_clustering,
          "Whether or not to use clustering at run time.");

  app->add_option("--feedback", settings->feedback,
                  "How much command line feedback to give.")
      ->check(CLI::Range(0, 3));

  app->add_option("--precision_criterion", settings->precision_criterion,
                  "Termination criterion. Nested sampling terminates when the "
                  "evidence contained in the live points is "
                  "precision_criterion fraction of the total evidence.")
      ->check(CLI::PositiveNumber);

  app->add_option("--logzero", settings->logzero,
                  "The loglikelihood value at which PolyChord considers points "
                  "'unphysical', and excludes them at the prior level.");

  app->add_option("--max-ndead", settings->max_ndead,
                  "Alternative termination criterion. Stop after max_ndead "
                  "iterations. Set negative to ignore.");

  app->add_option(
      "--boost-posterior", settings->boost_posterior,
      "Increase the number of posterior samples produced. This can be set "
      "arbitrarily high, but you won't be able to boost by more than "
      "num_repeats Warning: in high dimensions PolyChord produces _a lot_ "
      "of posterior samples. You probably don't need to change this");

  AddFlag(app, "--write-weighted-samples,!--no-weighted-samples",
          settings->posteriors,
          "Produce (weighted) posterior samples. Stored in <root>.txt.");

  AddFlag(app, "--write-samples,!--no-samples", settings->equals,
          "Produce (equally weighted) posterior samples. Stored in "
          "<root>_equal_weights.txt.");

  AddFlag(app, "--cluster-posteriors,!--no-cluster-posteriors",
          settings->cluster_posteriors,
          "Produce posterior files for each cluster? Does nothing if "
          "do_clustering=False.");

  AddFlag(app, "--write-resume,!--no-write-resume", settings->write_resume,
          "Create a resume file.");

  AddFlag(app, "--resume,!--overwrite", settings->read_resume,
          "Read from resume file.");

  AddFlag(app, "--write-stats,!--no-stats", settings->write_stats,
          "Write an evidence statistics file.");

  AddFlag(app, "--write-live,!--no-live", settings->write_live,
          "Write a live points file.");

  AddFlag(app, "--write-dead,!--no-dead", settings->write_dead,
          "Write a dead points file.");

  AddFlag(app, "--write-prior,!--no-prior", settings->write_prior,
          "Write a prior points file.");

  AddFlag(app, "--maximise,!--no-maximise", settings->maximise,
          "Perform maximisation at the end of the run to find the "
          "maximum likelihood point and value.");

  app->add_option("--compression-factor", settings->compression_factor,
                  "How often to update the files and do clustering.")
      ->check(CLI::PositiveNumber);

  AddFlag(
      app, "--synchronous,!--asynchronous", settings->synchronous,
      "Parallelise with synchronous workers, rather than asynchronous ones. "
      "This can be set to False if the likelihood speed is known to be "
      "approximately constant across the parameter space. Synchronous "
      "parallelisation is less effective than asynchronous by a factor "
      "~O(1) for large parallelisation.");

  app->add_option("--base-dir", settings->base_dir,
                  "Where to store output files.");

  app->add_option("--file-root", settings->file_root,
                  "Root name of the files produced.");

  app->add_option(
      "--grade-frac", settings->grade_frac,
      "The amount of time to spend in each speed. If any of grade_frac are "
      "<= 1, then polychord will time each sub-speed, and then choose "
      "num_repeats for the number of slowest repeats, and spend the "
      "proportion of time indicated by grade_frac. Otherwise this indicates "
      "the number of repeats to spend in each speed.");

  app->add_option("--grade-dims", settings->grade_dims,
                  "The number of parameters within each speed. "
                  "If [0], defaults to [nDims].");
  app->add_option(
         "--nlives", settings->nlives,
         "Variable number of live points option. This dictionary is a mapping "
         "between loglike contours and nlive. You should still set nlive to be "
         "a sensible number, as this indicates how often to update the "
         "clustering, and to define the default value.")
      ->expected(0, -1);

  app->add_option("--seed", settings->seed,
                  "Choose the seed to seed the random number generator. Note "
                  "**Positive seeds only** a negative seed indicates that you "
                  "should use the system time in milliseconds.");
}

}  // end namespace polystan

#endif  // POLYSTAN_POLYCHORD_CLI_HPP_
