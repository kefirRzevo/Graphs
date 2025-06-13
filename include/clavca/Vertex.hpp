#include <vector>

namespace graphs {

namespace clavca {

template <typename FloatTy> struct Vertex final {
  using SizeType = size_t;

  SizeType ColorDegree;
  std::vector<FloatTy> Probabilities;
  FloatTy Threshold;
  SizeType DynamicThreshold;
  SizeType SelectedColorIdx;
  bool Rewarded;
  std::string Label;
};

} // namespace clavca

} // namespace graphs
