#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "polystan/splash.hpp"
#include "polystan/model.hpp"
#include "polystan/polychord_cli.hpp"
#include "polystan/version.hpp"
#include "polystan/metadata.hpp"
#include "polystan/mpi.hpp"

#include "CLI11/CLI11.hpp"
#include "polychord/interfaces.hpp"

namespace ps = polystan;

template <typename T>
class Display {
 public:
  explicit Display(T data) : data(data) {}
  void operator()(int flag) {
    if (flag > 0) {
      throw CLI::CallForVersion(data, CLI::ExitCodes::Success);
    }
  }

 private:
  T data;
};

auto weakly_canonical = CLI::Validator(
    [](std::string& input) {
      input = std::filesystem::weakly_canonical(input);
      return std::string();
    },
    "Weakly canonicalized", "WeaklyCanonical");

int main(int argc, char** argv) {
  // make cli

  CLI::App app("PolyStan built with " + std::string(ps::stan_file_name));

  // allow settings from a toml file

  app.set_config("--from-toml", "", "Read CLI settings from a TOML file");
  app.allow_config_extras(false);

  // capture defaults

  app.option_defaults()->always_capture_default();

  // construct settings before knowing nDims & nDerived. they aren't knowable
  // until constructing the model as could depend on data file

  Settings settings(0, 0);

  // adjust some default settings

  settings.maximise = false;
  settings.write_live = true;
  settings.write_dead = true;
  settings.write_resume = true;
  settings.equals = true;
  settings.write_stats = true;
  settings.read_resume = true;
  settings.file_root = ps::stan_model_name;

  // add options to cli

  CLI::App* pc_cli = app.add_subcommand("polychord", "PolyChord settings");
  ps::AddPolyChord(pc_cli, &settings);

  CLI::App* data = app.add_subcommand("data", "Data settings");
  std::string data_file_name;
  data->add_option("--file", data_file_name, "Data file name")
      ->check(CLI::ExistingFile)
      ->transform(weakly_canonical);

  CLI::App* random
      = app.add_subcommand("random", "Control Stan random number generation");
  unsigned int seed = 0;
  random->add_option("--seed", seed, "Random seed")
      ->check(CLI::NonNegativeNumber);

  CLI::App* output = app.add_subcommand("output", "Control Stan output");
  std::string json_file_name = std::string(ps::stan_model_name) + ".json";
  output->add_option("--json-file", json_file_name, "JSON file output name")
      ->transform(weakly_canonical);
  std::string toml_file_name = std::string(ps::stan_model_name) + ".toml";
  output->add_option("--toml-file", toml_file_name, "TOML file output name")
      ->transform(weakly_canonical);

  // add version flags

  app.set_version_flag("--version", ps::version);
  app.add_flag("--polychord-version", Display(ps::polychord_version),
               "Display PolyChord version information and exit");
  app.add_flag("--stan-file-name", Display(ps::stan_file_name),
               "Display Stan file name and exit");

  // parse options

  CLI11_PARSE(app, argc, argv);

  // dump cli to toml file

  std::ofstream toml_file;
  toml_file.open(toml_file_name);
  toml_file << app.config_to_str(true, true);
  toml_file.close();

  // invoke main program

  ps::mpi::initialize();

  std::optional<ps::Model> optional_model;

  try {
    optional_model.emplace(data_file_name, seed, settings);
  } catch (const std::exception& ex) {
    return app.exit(
        CLI::ConstructionError(ex.what(), CLI::ExitCodes::InvalidError));
  }

  const ps::Model model = optional_model.value();

  if (ps::mpi::is_rank_zero()) {
    std::cout << ps::splash::start(model, toml_file_name) << "\n";
  }

  ps::mpi::barrier();

  model.run();

  // write results to disk in json format

  if (ps::mpi::is_rank_zero()) {
    model.write(json_file_name);
    std::cout << ps::splash::end(json_file_name, model) << "\n";
  }

  ps::mpi::finalize();
}
