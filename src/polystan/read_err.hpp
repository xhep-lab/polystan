#ifndef POLYSTAN_READ_ERR_HPP_
#define POLYSTAN_READ_ERR_HPP_

#include <regex>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>

namespace polystan {

const char RESET[] = "\x1B[0m";
const char COLOR[] = "\x1B[31m";

std::string read_line(const std::string& file_name, int line_number) {
  std::ifstream stream(file_name);
  std::string line;

  for (int i = 1; i <= line_number; i++) {
    std::getline(stream, line);
  }

  return line;
}

std::string markup_lines(const std::string& file_name, int line_number,
                         int start, int end) {
  const std::string before = read_line(file_name, line_number - 1);
  const std::string line = read_line(file_name, line_number);
  const std::string after = read_line(file_name, line_number + 1);

  std::string bold;
  bold.resize(line.size());
  for (int i = 0; i < bold.size(); i++) {
    if (i >= start && i <= end) {
      bold[i] = '^';
    } else {
      bold[i] = ' ';
    }
  }

  std::stringstream out;

  out << before << "\n"
      << line << "\n"
      << COLOR << bold << "\n"
      << RESET << after;

  return out.str();
}

std::optional<std::string> match(const std::string& text,
                                 const std::string& pattern) {
  std::smatch match;

  if (!std::regex_search(text, match, std::regex(pattern))) {
    return std::nullopt;
  }

  return match[1].str();
}

std::optional<std::string> read_err(const std::string& err) {
  const auto file_name = match(err, "in '(.*)',");
  const auto line_number = match(err, ", line (.*),");
  const auto start = match(err, ", column (.*) to");
  const auto end = match(err, "to column (.*)\\)");

  if (!(file_name.has_value() && line_number.has_value() && start.has_value()
        && end.has_value())) {
    return std::nullopt;
  }

  const auto result
      = markup_lines(file_name.value(), std::stoi(line_number.value()),
                     std::stoi(start.value()), std::stoi(end.value()));
  return std::optional<std::string>(result);
}

std::string add_to_err(const std::string& err) {
  const auto add = read_err(err);
  if (add.has_value()) {
    return err + "\n" + add.value() + "\n";
  }
  return err;
}

std::string add_to_err(char* err) { return add_to_err(std::string(err)); }

}  // end namespace polystan

#endif  // POLYSTAN_READ_ERR_HPP_
