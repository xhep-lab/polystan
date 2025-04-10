#ifndef POLYSTAN_SPLASH_HPP_
#define POLYSTAN_SPLASH_HPP_

#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "polystan/version.hpp"
#include "polystan/model.hpp"
#include "polystan/mpi.hpp"

namespace polystan {
namespace splash {

const char PREFIX[] = "| ";
const char RESET[] = "\x1B[0m";
const char COLOR[] = "\x1B[34m";

template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
  out << '[';

  for (const auto& elem : vec) {
    out << elem << ", ";
  }

  if (!vec.empty()) {
    out << "\b\b";
  }

  out << "]";
  return out;
}

std::string start(const Model& model, const std::string& toml_file_name) {
  std::stringstream splash;

  splash << COLOR << PREFIX << "PolyStan\n"
         << PREFIX << "\n"
         << PREFIX << "TOML file of arguments: " << toml_file_name << "\n"
         << PREFIX << "Version: " << version << "\n"
         << PREFIX << "PolyChord version: " << polychord_version << "\n"
         << PREFIX << "Stan file: " << model.stan_file_name << "\n"
         << PREFIX << "Data file: " << model.data_file_name << "\n"
         << PREFIX << "Hypercube parameters: " << model.param_names() << "\n"
         << PREFIX << "Derived parameters: " << model.derived_names() << "\n"
         << PREFIX << "\n"
         << PREFIX << "Stan build info:\n";

  std::istringstream ss(model.stan_build_info());
  std::string line;
  while (std::getline(ss, line)) {
    splash << PREFIX << line << "\n";
  }

  splash << PREFIX << "\n"
#ifdef USE_MPI
         << PREFIX << "Using MPI with size: " << mpi::get_size() << "\n"
#else
         << PREFIX << "Not compiled with MPI" << "\n"
#endif
         << PREFIX << "\n"
         << PREFIX << "Running PolyChord\n"
         << RESET;

  return splash.str();
}

std::string end(const std::string& json_file_name, const Model& model) {
  std::stringstream splash;

  splash << COLOR << "\n"
         << PREFIX << "Finished PolyChord\n"
         << PREFIX << "Native PolyChord results at " << model.basename()
         << "*\n"
         << PREFIX << "PolyStan JSON summary at " << json_file_name << "\n"
         << PREFIX << "\n"
         << PREFIX << "If you use these results, you are required to cite\n"
         << PREFIX << "https://arxiv.org/abs/1502.01856\n"
         << PREFIX << "https://arxiv.org/abs/1506.00171\n"
         << PREFIX << "and agree to the license\n"
         << PREFIX
         << "https://github.com/PolyChord/PolyChordLite/raw/refs/heads/master/"
            "LICENCE"
         << RESET;

  return splash.str();
}

}  // end namespace splash
}  // end namespace polystan

#endif  // POLYSTAN_SPLASH_HPP_
