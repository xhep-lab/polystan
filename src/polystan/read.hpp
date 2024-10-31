#ifndef POLYSTAN_READ_HPP_
#define POLYSTAN_READ_HPP_

#include <array>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace polystan {
namespace read {

std::vector<std::string> param_names(std::string csv) {
  std::stringstream ss(csv);
  std::vector<std::string> result;

  while (ss.good()) {
    std::string substr;
    std::getline(ss, substr, ',');
    result.push_back(substr);
  }

  return result;
}

std::vector<std::vector<double>> samples(std::string equal_weights_file_name) {
  std::ifstream ifs(equal_weights_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read equally weighted samples from "
                             + equal_weights_file_name);
  }

  std::vector<std::vector<double>> data;
  std::string record;

  while (std::getline(ifs, record)) {
    std::istringstream is(record);
    std::istream_iterator<double> it(is);
    it++;  // we ignore weight column
    std::vector<double> row((it), std::istream_iterator<double>());

    if (data.empty()) {
      data.resize(row.size());
    }

    data[0].push_back(-0.5 * row[0]);  // convert from -2 * loglike

    // look at parameters

    for (int i = 1; i < row.size(); i++) {
      data[i].push_back(row[i]);
    }
  }

  return data;
}

std::array<double, 2> evidence(std::string stats_file_name) {
  const std::string prefix = "log(Z)       =";
  const std::string delim = "+/-";

  std::ifstream ifs(stats_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read evidence from " + stats_file_name);
  }

  std::string record;

  while (record.rfind(prefix, 0) != 0) {
    std::getline(ifs, record);
  }

  const std::string data(record.substr(prefix.size()));

  const double logz = std::stof(data.substr(0, data.find(delim)));
  const double error_logz
      = std::stof(data.substr(data.find(delim) + delim.size()));

  return {logz, error_logz};
}

}  // end namespace read
}  // end namespace polystan

#endif  // POLYSTAN_READ_HPP_
