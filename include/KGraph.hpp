#pragma once

#include "IGraph.hpp"
#include <vector>

namespace graphs {

template <typename NodeAttrs = DefaultNodeAttrs,
          typename EdgeAttrs = DefaultEdgeAttrs>
class KGraph final
    : public IGraph<NodeAttrs, EdgeAttrs, Graph<NodeAttrs, EdgeAttrs>> {

  struct NodeEntry {
    using NodeIdx = std::vector<NodeEntry>::size_type;

    NodeIdx ParamA;
    NodeIdx ParamT;
    NodeIdx ParamN;
    NodeIdx ParamP;
  };
};

} // namespace graphs
