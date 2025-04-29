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

std::vector<std::string> param_names(const std::string& csv) {
  std::stringstream stream(csv);
  std::vector<std::string> result;

  while (stream.good()) {
    std::string substr;
    std::getline(stream, substr, ',');
    result.push_back(substr);
  }

  return result;
}

std::vector<std::vector<double>> samples(
    const std::string& equal_weights_file_name) {
  std::ifstream ifs(equal_weights_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read equally weighted samples from "
                             + equal_weights_file_name);
  }

  std::vector<std::vector<double>> data;
  std::string record;

  while (std::getline(ifs, record)) {
    std::istringstream iss(record);
    std::istream_iterator<double> iter(iss);
    iter++;  // we ignore weight column
    std::vector<double> row((iter), std::istream_iterator<double>());

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

std::array<std::vector<double>, 2> death_birth(
    const std::string& death_birth_file_name) {
  std::ifstream ifs(death_birth_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read dead points from "
                             + death_birth_file_name);
  }

  std::array<std::vector<double>, 2> data;
  std::string record;

  while (std::getline(ifs, record)) {
    std::istringstream iss(record);
    std::istream_iterator<double> iter(iss);
    std::vector<double> row((iter), std::istream_iterator<double>());
    data[0].push_back(row[row.size() - 2]);
    data[1].push_back(row[row.size() - 1]);
  }

  return data;
}

int neval(const std::string& stats_file_name) {
  const std::string prefix = " nlike:";

  std::ifstream ifs(stats_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read neval from " + stats_file_name);
  }

  std::string record;

  while (std::getline(ifs, record)) {
    if (record.rfind(prefix, 0) == 0) {
      break;
    }
  }

  const std::string data(record.substr(prefix.size()));
  return std::stoi(data);
}

int ess(const std::string& stats_file_name) {
  const std::string prefix = " nequals:";

  std::ifstream ifs(stats_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read ESS from " + stats_file_name);
  }

  std::string record;

  while (std::getline(ifs, record)) {
    if (record.rfind(prefix, 0) == 0) {
      break;
    }
  }

  const std::string data(record.substr(prefix.size()));
  return std::stoi(data);
}

std::array<double, 2> evidence(const std::string& stats_file_name) {
  const std::string prefix = "log(Z)       =";
  const std::string delim = "+/-";

  std::ifstream ifs(stats_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not read evidence from " + stats_file_name);
  }

  std::string record;

  while (std::getline(ifs, record)) {
    if (record.rfind(prefix, 0) == 0) {
      break;
    }
  }

  const std::string data(record.substr(prefix.size()));

  const double logz = std::stof(data.substr(0, data.find(delim)));
  const double err = std::stof(data.substr(data.find(delim) + delim.size()));

  return {logz, err};
}

std::vector<double> weight(const std::string& txt_file_name) {
  std::ifstream ifs(txt_file_name);

  if (!ifs) {
    throw std::runtime_error("Could not weights from " + txt_file_name);
  }

  std::vector<double> data;
  std::string record;

  while (std::getline(ifs, record)) {
    std::istringstream iss(record);
    std::istream_iterator<double> iter(iss);
    std::vector<double> row((iter), std::istream_iterator<double>());
    data.push_back(row[0]);
  }

  return data;
}

}  // end namespace read
}  // end namespace polystan

#endif  // POLYSTAN_READ_HPP_
