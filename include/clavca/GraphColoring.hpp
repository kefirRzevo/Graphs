#pragma once

#include "Graph.hpp"
#include "clavca/Dumpers.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

namespace graphs {

namespace clavca {

template <typename FloatTy> class Solver final {
public:
  using VertexTy = Vertex<FloatTy>;
  using GraphTy = Graph<VertexTy>;
  using SizeType = typename VertexTy::SizeType;
  using NodeId = typename GraphTy::NodeId;

private:
  FloatTy ParamA = FloatTy{1.0};
  FloatTy ParamB = FloatTy{0.0};
  FloatTy ParamE = FloatTy{0.2};

  std::optional<unsigned int> Seed;
  GraphTy &G;

  void initialize(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto Degree = G.getNodeDegree(NId);
    auto Probability = static_cast<FloatTy>(1) / (Degree + 1);
    V.ColorDegree = Degree + 1;
    V.Probabilities = std::vector<FloatTy>(Degree + 1, Probability);
    V.Threshold = 1 - ParamE / Degree;
    V.DynamicThreshold = Degree + 1;
    V.Rewarded = false;
  }

  void chooseColor(NodeId NId) {
    using DistributionTy = std::discrete_distribution<SizeType>;

    auto &V = G.getNodeAttrs(NId);
    if (!Seed.has_value()) {
      std::random_device RD;
      Seed.emplace(RD());
    }
    static std::mt19937 Generator(Seed.value());
    DistributionTy D(V.Probabilities.begin(), V.Probabilities.end());
    V.SelectedColorIdx = D(Generator);
  }

  bool needPenalizeAdjColorChoice(NodeId NId) const {
    const auto &V = G.getNodeAttrs(NId);
    auto EIds = G.adjEdgeIds(NId);
    return std::any_of(EIds.begin(), EIds.end(), [&](auto &&EId) {
      auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
      const auto &AdjV = G.getNodeAttrs(AdjNId);
      return AdjV.SelectedColorIdx == V.SelectedColorIdx;
    });
  }

  void penalize(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto NewProbabilities = V.Probabilities;
    auto End = NewProbabilities.size();
    auto Idx = SizeType{};
    std::transform(V.Probabilities.begin(), V.Probabilities.end(),
                   NewProbabilities.begin(), [&](auto &&P) {
                     return Idx++ == V.SelectedColorIdx
                                ? (1 - ParamB) * P
                                : ParamB / (End - 1) + (1 - ParamB) * P;
                   });
    V.Probabilities = NewProbabilities;
    V.Rewarded = false;
  }

  void computeColorDegree(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto Colors = std::unordered_set<SizeType>{};
    Colors.emplace(V.SelectedColorIdx);
    auto EIds = G.adjEdgeIds(NId);
    for (auto EId : EIds) {
      auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
      const auto &AdjV = G.getNodeAttrs(AdjNId);
      auto Color = AdjV.SelectedColorIdx;
      Colors.emplace(Color);
    }
    V.ColorDegree = Colors.size();
  }

  bool needRewardDynamicThreshold(NodeId NId) const {
    const auto &V = G.getNodeAttrs(NId);
    return V.ColorDegree <= V.DynamicThreshold;
  }

  void reward(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    auto NewProbabilities = V.Probabilities;
    auto Idx = SizeType{};
    std::transform(V.Probabilities.begin(), V.Probabilities.end(),
                   NewProbabilities.begin(), [&](auto &&P) {
                     return Idx++ == V.SelectedColorIdx ? P + ParamA * (1 - P)
                                                        : (1 - ParamA) * P;
                   });
    V.Probabilities = NewProbabilities;
    V.Rewarded = true;
  }

  bool checkAdjColorReward(NodeId NId) const {
    const auto &V = G.getNodeAttrs(NId);
    auto EIds = G.adjEdgeIds(NId);
    return std::all_of(EIds.begin(), EIds.end(),
                       [&](auto &&EId) {
                         auto AdjNId = G.getEdgeOtherNodeId(EId, NId);
                         const auto &AdjV = G.getNodeAttrs(AdjNId);
                         return AdjV.Rewarded;
                       }) &&
           V.Rewarded;
  }

  void updateDynamicThreshold(NodeId NId) {
    auto &V = G.getNodeAttrs(NId);
    V.DynamicThreshold = V.ColorDegree;
  }

  bool needHaltSelectedColorProbability(NodeId NId) const {
    const auto &V = G.getNodeAttrs(NId);
    auto P = V.Probabilities[V.SelectedColorIdx];
    return P > V.Threshold;
  }

  struct VertexAttrs {
    NodeId NId;
    bool NeedCheckAdjColors;
  };

public:
  Solver(GraphTy &G) : G(G) {}

  void setParamA(FloatTy A) { ParamA = A; }

  void setParamB(FloatTy B) { ParamB = B; }

  void setParamE(FloatTy E) { ParamE = E; }

  void setSeed(unsigned int S) { Seed.emplace(S); }

  template <typename DumperTy = NoDumper> void solve(DumperTy& Dumper) {
    auto Verteces = std::list<VertexAttrs>{};
    auto NIds = G.nodeIds();
    std::transform(NIds.begin(), NIds.end(), std::back_inserter(Verteces),
                   [&](auto &&NId) {
                     initialize(NId);
                     return VertexAttrs{NId, false};
                   });
    unsigned StageNumber = 0;
    Dumper.dumpGraphByStage("dump" + std::to_string(StageNumber) + ".dot", G);
    unsigned MaxStageNumber = 1000;
    for (;;) {
      if (Verteces.empty() || StageNumber == MaxStageNumber) {
        break;
      }
      Dumper.dumpDbgMsg("Stage " + std::to_string(StageNumber) + "\n");
      for (auto [NId, _] : Verteces) {
        chooseColor(NId);
      }
      for (auto &[NId, NeedCheckAdjColors] : Verteces) {
        NeedCheckAdjColors = false;
        if (needPenalizeAdjColorChoice(NId)) {
          Dumper.dumpDbgMsg("penalized1\n");
          penalize(NId);
        } else {
          computeColorDegree(NId);
          if (needRewardDynamicThreshold(NId)) {
            Dumper.dumpDbgMsg("rewarded\n");
            reward(NId);
            NeedCheckAdjColors = true;
          } else {
            Dumper.dumpDbgMsg("penalized2\n");
            penalize(NId);
          }
        }
      }
      for (auto [NId, NeedCheckAdjColors] : Verteces) {
        if (NeedCheckAdjColors) {
          if (checkAdjColorReward(NId)) {
            updateDynamicThreshold(NId);
          }
        }
      }
      StageNumber++;
      Dumper.dumpGraphByStage("dump" + std::to_string(StageNumber) + ".dot", G);
      Verteces.remove_if(
          [&](auto &&VA) { return needHaltSelectedColorProbability(VA.NId); });
    }
    Dumper.dumpMsg("Stages " + std::to_string(StageNumber) + "\n");
    assert(Seed.has_value());
    Dumper.dumpMsg("Seed " + std::to_string(Seed.value()) + "\n");
  }
};

} // namespace clavca

} // namespace graphs
