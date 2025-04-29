#ifndef POLYSTAN_TEST_HPP_
#define POLYSTAN_TEST_HPP_

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include <boost/math/distributions/kolmogorov_smirnov.hpp>

#include "read.hpp"

namespace bm = boost::math;

namespace polystan {
namespace test {

constexpr double pi = boost::math::constants::pi<double>();

void sort_by_birth(std::vector<double>& death, std::vector<double>& birth) {
  std::vector<int> order(birth.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(),
            [&](int i, int j) -> bool { return birth[i] < birth[j]; });

  std::vector<double> ordered_death(death.size());
  std::transform(order.begin(), order.end(), ordered_death.begin(),
                 [&](int i) { return death[i]; });

  death = ordered_death;
  std::sort(birth.begin(), birth.end());
}

std::vector<int> insertion_indexes(const std::vector<double>& death,
                                   const std::vector<double>& birth) {
  std::vector<int> indexes;
  const int sample_size = birth.size();

  for (int i = 0; i < sample_size; ++i) {
    int idx = 0;

    for (int j = 0; j < sample_size; ++j) {
      if (birth[j] <= birth[i] && death[j] > birth[i] && death[i] > death[j]) {
        idx += 1;
      }
    }

    indexes.push_back(idx);
  }

  return indexes;
}

std::vector<double> empirical_cmf(const std::vector<int>& indexes, int nlive) {
  std::vector<double> pmf(nlive + 1, 0.);
  const int n = indexes.size();

  for (const auto& i : indexes) {
    pmf[i] += 1. / n;
  }

  std::vector<double> cmf(nlive + 1, 0.);
  std::partial_sum(pmf.begin(), pmf.end(), cmf.begin());
  return cmf;
}

double ks_test_uniform(const std::vector<double>& cmf, int sample_size) {
  double max_d = 0.;
  const double n = static_cast<double>(cmf.size()) - 1.;

  for (int i = 0; i < cmf.size(); ++i) {
    const double d = std::abs(cmf[i] - i / n);
    max_d = std::max(max_d, d);
  }

  bm::kolmogorov_smirnov_distribution ksd(sample_size);
  return bm::cdf(bm::complement(ksd, max_d));
}

double insertion_index_p_value(const std::vector<int> indexes, int nlive) {
  const std::vector<double> cmf = empirical_cmf(indexes, nlive);
  return ks_test_uniform(cmf, indexes.size());
}

double insertion_index_p_value(const std::string& death_birth_file_name,
                               int nlive) {
  const auto [death, birth] = read::death_birth(death_birth_file_name);
  const std::vector<int> indexes = insertion_indexes(death, birth);
  return insertion_index_p_value(indexes, nlive);
}

std::vector<std::vector<int>> split(const std::vector<int>& data,
                                    int part_size) {
  std::vector<std::vector<int>> parts;

  const int total_size = data.size();
  const int remainder = total_size % part_size;
  const int n = total_size / part_size;

  int start = 0;

  for (int i = 0; i < n; ++i) {
    size_t current_part_size = part_size + (i < remainder ? 1 : 0);
    std::vector<int> part(data.begin() + start,
                          data.begin() + start + current_part_size);
    parts.push_back(part);
    start += current_part_size;
  }

  return parts;
}

double insertion_index_p_value(const std::string& death_birth_file_name,
                               int nlive, int batch) {
  if (batch == 0) {
    return insertion_index_p_value(death_birth_file_name, nlive);
  }

  auto [death, birth] = read::death_birth(death_birth_file_name);
  sort_by_birth(death, birth);

  const std::vector<int> indexes = insertion_indexes(death, birth);
  const std::vector<std::vector<int>> batches = split(indexes, batch * nlive);

  double p_value = 1.;

  for (const auto& b : batches) {
    p_value = std::min(p_value, insertion_index_p_value(b, nlive));
  }

  return 1. - std::pow(1. - p_value, batches.size());
}

}  // end namespace test
}  // end namespace polystan

#endif  // POLYSTAN_TEST_HPP_
