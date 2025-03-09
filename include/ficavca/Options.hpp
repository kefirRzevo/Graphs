#pragma once

#include <optional>
#include <string>

namespace graphs {

namespace ficavca {

struct Config final {
  std::optional<bool> Democracy;
  std::optional<unsigned int> Seed;
  std::string GraphInput;
};

std::optional<Config> readConfig(int argc, const char *argv[]);

} // namespace ficavca

} // namespace graphs
