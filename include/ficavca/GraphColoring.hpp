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
    for (auto NId : NIds) {
      auto &V = G.getNodeAttrs(NId);
      V.Color = SizeTy{};
      V.Colored = false;
      V.PurityValue = FloatTy{1};
      V.VoteWeight = SizeTy{};
      UncoloredVerteces.emplace_back(NId);
    }
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
      std::for_each(EIds.begin(), EIds.end(), [&](auto &&EId) {
        auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
        auto AdjEIds = G.adjEdgeIds(AdjNId);
        auto &AdjV = G.getNodeAttrs(AdjNId);
        if (!AdjV.Colored && (AdjV.PurityValue > V.PurityValue ||
            AdjEIds.size() >= EIds.size())) {
          AdjV.VoteWeight += EIds.size();
          HasSuperior = true;
        }
      });
      if (!HasSuperior) {
        NominateList.emplace_back(NId);
      }

      // for (auto EId : EIds) {
      //   auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
      //   auto AdjEIds = G.adjEdgeIds(AdjNId);
      //   auto &AdjV = G.getNodeAttrs(AdjNId);
      //   if (AdjV.PurityValue >= V.PurityValue &&
      //       AdjEIds.size() >= EIds.size()) {
      //     AdjV.VoteWeight += EIds.size();
      //   } else {
      //     std::cout << "NId " << NId << " and " << " AdjNId " << AdjNId <<
      //     std::endl; NominateList.emplace(NId);
      //   }
      // }
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

public:
  Solver(GraphTy &G) : G(G) {}

  template <typename DumperTy> void solve(DumperTy &Dumper) {
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
    std::cout << "Color degree " << ColorDegree << std::endl;
  }
};

} // namespace ficavca

} // namespace graphs
