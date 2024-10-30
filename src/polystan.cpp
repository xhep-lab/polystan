#include <iostream>
#include <filesystem>
#include <string>

#include "polystan/splash.hpp"
#include "polystan/model.hpp"
#include "polystan/polychord_cli.hpp"
#include "polystan/version.hpp"
#include "polystan/mpi.hpp"

#include "CLI11/CLI11.hpp"
#include "polychord/interfaces.hpp"

namespace ps = polystan;


template<typename T>
class Display {
 public:
  Display(T data) : data(data) {}
  void operator() (int) {
    throw CLI::CallForVersion(data, CLI::ExitCodes::Success);
  }
 private:
  T data;  
};


int main(int argc, char** argv) {
    // parse cli

    CLI::App app("PolyStan built with " + std::string(ps::stan_file_name));

    // construct settings before knowing nDims & nDerived. they aren't knowable until
    // constructing the model as could depend on data file

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

    CLI::App* pc = app.add_subcommand("polychord", "PolyChord settings");
    ps::AddPolyChord(pc, &settings);

    CLI::App* data = app.add_subcommand("data", "Data settings");
    std::string data_file_name;
    data->add_option("--file", data_file_name, "Data file name")->check(CLI::ExistingFile);

    CLI::App* random = app.add_subcommand("random", "Control Stan random number generation");
    unsigned int seed = 0;
    random->add_option("--seed", seed, "Random seed")->check(CLI::PositiveNumber);

    CLI::App* output = app.add_subcommand("output", "Control Stan output");
    std::string json_file_name = std::string(ps::stan_model_name) + ".json";
    output->add_option("--file", json_file_name , "JSON file output name");

    app.set_version_flag("--version", ps::version);
    app.add_flag("--polychord-version", Display(ps::polychord_version), "Display PolyChord version information and exit");
    app.add_flag("--stan-file-name", Display(ps::stan_file_name), "Display Stan file name and exit");

    CLI11_PARSE(app, argc, argv);

    data_file_name = std::filesystem::weakly_canonical(data_file_name);
    json_file_name = std::filesystem::weakly_canonical(json_file_name);

    // invoke main program

    ps::mpi::initialize();

    const ps::Model model(data_file_name, seed, settings);

    if (ps::mpi::is_rank_zero()) {
      std::cout << ps::splash::start(model) << std::endl;
    }

    ps::mpi::barrier();
  
    model.run();

    // write results to disk in json format

    if (ps::mpi::is_rank_zero()) {
      model.write(json_file_name);
      std::cout << ps::splash::end(json_file_name, model) << std::endl;
    }

    ps::mpi::finalize();
}
