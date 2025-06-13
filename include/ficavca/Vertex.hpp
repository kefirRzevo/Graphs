#pragma once

#include <cstddef>
#include <string>

namespace graphs {

namespace ficavca {

struct Vertex final {
  using FloatTy = float;
  using SizeTy = size_t;

  bool Colored;
  SizeTy Color;
  FloatTy PurityValue;
  SizeTy VoteWeight;
  std::string Label;
};

} // namespace ficavca

} // namespace graphs
