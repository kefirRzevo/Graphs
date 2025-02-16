#pragma once

#include <cstddef>

namespace graphs {

namespace ficavca {

struct Vertex final {
  using FloatTy = float;
  using SizeTy = size_t;

  bool Colored;
  SizeTy Color;
  FloatTy PurityValue;
  SizeTy VoteWeight;
};

} // namespace ficavca

} // namespace graphs
