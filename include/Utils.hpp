#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

namespace graphs {

template <typename GraphTy> inline GraphTy readGraph(std::istream &is) {
  using NodeId = typename GraphTy::NodeId;

  auto G = GraphTy{};
  auto Verteces = std::unordered_map<size_t, NodeId>{};
  auto getVertexByNum = [&G, &Verteces](auto VertexNum) {
    auto Found = Verteces.find(VertexNum);
    if (Found == Verteces.end()) {
      auto V = G.addNode();
      Verteces.emplace(VertexNum, V);
      return V;
    }
    return Found->second;
  };

  auto VertexNum1 = size_t{};
  auto VertexNum2 = size_t{};
  auto PatternNext = std::string{};
  while (is >> VertexNum1 >> PatternNext >> VertexNum2) {
    assert(PatternNext == "-");
    auto V1 = getVertexByNum(VertexNum1);
    auto V2 = getVertexByNum(VertexNum2);
    G.addEdge(V1, V2);
  }
  return G;
}

} // namespace graphs
