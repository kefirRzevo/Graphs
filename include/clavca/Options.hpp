#pragma once

#include <optional>
#include <string>

namespace graphs {

namespace clavca {

struct Config final {
  std::optional<double> ParamA;
  std::optional<double> ParamB;
  std::optional<double> ParamE;
  std::optional<unsigned int> Seed;
  std::string GraphInput;
};

std::optional<Config> readConfig(int argc, const char *argv[]);

} // namespace clavca

} // namespace graphs
