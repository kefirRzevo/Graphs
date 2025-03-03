#pragma once

#include "Graph.hpp"
#include "ficavca/Dumpers.hpp"
#include "ficavca/Vertex.hpp"
#include <algorithm>
#include <list>
#include <unordered_set>
#include <vector>

namespace graphs {

namespace ficavca {

class Solver final {
public:
  using VertexTy = Vertex;
  using GraphTy = Graph<VertexTy>;
  using SizeTy = typename VertexTy::SizeTy;
  using FloatTy = typename VertexTy::FloatTy;
  using NodeId = typename GraphTy::NodeId;

private:
  void initialize() {
    auto NIds = G.nodeIds();
    assert(NIds.begin() != NIds.end());
    std::transform(NIds.begin(), NIds.end(), std::back_inserter(UncoloredVerteces), [&](auto&& NId) {
      auto &V = G.getNodeAttrs(NId);
      V.Color = SizeTy{};
      V.Colored = false;
      V.PurityValue = FloatTy{1};
      V.VoteWeight = SizeTy{};
      return NId;
    });
    ColorDegree = SizeTy{1};
  }

  void vote() {
    NominateList.clear();
    for (auto NId : UncoloredVerteces) {
      auto &V = G.getNodeAttrs(NId);
      V.VoteWeight = SizeTy{};
    }
    for (auto NId : UncoloredVerteces) {
      const auto &V = G.getNodeAttrs(NId);
      if (!(V.PurityValue > Epsilon)) {
        continue;
      }
      auto EIds = G.adjEdgeIds(NId);
      auto HasSuperior = false;
      bool PurityCompare = false;
      std::for_each(EIds.begin(), EIds.end(), [&](auto &&EId) {
        auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
        auto AdjEIds = G.adjEdgeIds(AdjNId);
        auto &AdjV = G.getNodeAttrs(AdjNId);
        if (Democracy) {
          PurityCompare = AdjV.PurityValue >= V.PurityValue;
        } else {
          PurityCompare = AdjV.PurityValue > V.PurityValue;
        }
        if (!AdjV.Colored && (PurityCompare && AdjEIds.size() > EIds.size())) {
          AdjV.VoteWeight += EIds.size();
          HasSuperior = true;
        }

      });
      if (!HasSuperior) {
        NominateList.emplace_back(NId);
      }
    }
  }

  void color() {
    if (!NominateList.empty()) {
      for (auto NId : NominateList) {
        colorVertex(NId);
      }
    } else {
      assert(!UncoloredVerteces.empty());
      auto HighestVote =
          std::max_element(UncoloredVerteces.begin(), UncoloredVerteces.end(),
                           [&](auto &&NId1, auto &&NId2) {
                             const auto &V1 = G.getNodeAttrs(NId1);
                             const auto &V2 = G.getNodeAttrs(NId2);
                             return V1.VoteWeight < V2.VoteWeight;
                           });
      colorVertex(*HighestVote);
    }
    for (auto NId : UncoloredVerteces) {
      const auto &V = G.getNodeAttrs(NId);
      if (V.PurityValue < Epsilon) {
        colorVertex(NId);
      }
    }
  }

  void colorVertex(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto EIds = G.adjEdgeIds(NId);
    auto AdjColors = std::unordered_set<SizeTy>{};
    for (auto EId : EIds) {
      auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
      const auto &AdjV = G.getNodeAttrs(AdjNId);
      if (AdjV.Colored) {
        AdjColors.emplace(AdjV.Color);
      }
    }
    auto Color = SizeTy{};
    for (; Color != ColorDegree; ++Color) {
      if (!AdjColors.count(Color)) {
        break;
      }
    }
    if (Color == ColorDegree) {
      ColorDegree++;
    }
    V.Color = Color;
    V.Colored = true;
  }

  void update() {
    for (auto NId : UncoloredVerteces) {
      updatePurity(NId);
    }
    UncoloredVerteces.remove_if([&](auto &&NId) {
      const auto &V = G.getNodeAttrs(NId);
      return V.Colored;
    });
  }

  void updatePurity(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto EIds = G.adjEdgeIds(NId);
    auto Count = std::count_if(EIds.begin(), EIds.end(), [&](auto &&EId) {
      auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
      const auto &AdjV = G.getNodeAttrs(AdjNId);
      return !AdjV.Colored;
    });
    V.PurityValue = static_cast<float>(Count) / EIds.size();
  }

  GraphTy &G;
  FloatTy Epsilon = FloatTy{1e-5};
  std::vector<NodeId> NominateList;
  std::list<NodeId> UncoloredVerteces;
  SizeTy ColorDegree;
  bool Democracy = false;

public:
  Solver(GraphTy &G) : G(G) {}

  void setDemocracy(bool D) { Democracy = D; }

  template <typename DumperTy> SizeTy solve(DumperTy &Dumper) {
    if (G.empty()) { return SizeTy{1}; }
    initialize();
    for (auto Stage = SizeTy{};; ++Stage) {
      vote();
      color();
      update();
      Dumper.dumpGraphByStage("dump" + std::to_string(Stage) + ".dot", G);
      if (UncoloredVerteces.empty()) {
        break;
      }
    }
    return ColorDegree;
  }

  bool validate() const {
    auto EIds = G.edgeIds();
    return std::all_of(EIds.begin(), EIds.end(), [&](auto&& EId) {
      auto N1Id = G.getEdgeNode1Id(EId);
      auto N2Id = G.getEdgeNode2Id(EId);
      const auto& V1 = G.getNodeAttrs(N1Id);
      const auto& V2 = G.getNodeAttrs(N2Id);
      return V1.Colored && V2.Colored && V1.Color != V2.Color;
    });
  }
};

} // namespace ficavca

} // namespace graphs
