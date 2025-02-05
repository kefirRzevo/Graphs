#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <random>
#include <unordered_set>
#include <vector>

#include "Graph.hh"

namespace fs = std::filesystem;

auto RepoPath = fs::path{__FILE__}.parent_path().parent_path();

namespace graphs {

template <typename Float> struct Vertex final {
  using GraphTy = Graph<Vertex<Float>>;
  using NodeId = typename GraphTy::NodeId;
  using SizeType = size_t;
  using FloatType = Float;

  static constexpr FloatType ParamA = FloatType{0.5};
  static constexpr FloatType ParamB = FloatType{0.1};
  static constexpr FloatType ParamE = FloatType{0.1};

  const GraphTy *G;
  NodeId Id;
  SizeType ColorDegree;
  std::vector<FloatType> Probabilities;
  FloatType Threshold;
  SizeType DynamicThreshold;
  SizeType SelectedColorIdx;
  bool Rewarded;

  void initialize(NodeId NId) {
    auto Degree = G->getNodeDegree(NId);
    Id = NId;
    ColorDegree = Degree + 1;
    auto Probability = static_cast<FloatType>(1) / (Degree + 1);
    Probabilities = std::vector<FloatType>(Degree + 1, Probability);
    Threshold = 1 - ParamE / Degree;
    DynamicThreshold = Degree + 1;
  }

  void chooseColor() {
    using DistributionType = std::discrete_distribution<SizeType>;
    static std::random_device RD;
    static std::mt19937 Generator(RD());
    DistributionType D(Probabilities.begin(), Probabilities.end());
    SelectedColorIdx = D(Generator);
  }

  bool needPenalizeAdjColorChoice() const {
    auto EIds = G->adjEdgeIds(Id);
    return std::any_of(EIds.begin(), EIds.end(), [&](auto &&EId) {
      auto NId = G->getEdgeOtherNodeId(EId, Id);
      const auto &Attrs = G->getNodeAttrs(NId);
      return Attrs.SelectedColorIdx == SelectedColorIdx;
    });
  }

  void penalize() {
    auto NewProbabilities = Probabilities;
    for (auto j = SizeType{}, End = Probabilities.size(); j != End; ++j) {
      auto P = Probabilities[j];
      if (j == SelectedColorIdx) {
        NewProbabilities[j] = (1 - ParamB) * P;
      } else {
        NewProbabilities[j] = ParamB / (End - 1) + (1 - ParamB) * P;
      }
    }
    Probabilities = NewProbabilities;
    Rewarded = false;
  }

  void computeColorDegree() {
    auto Colors = std::unordered_set<SizeType>{};
    Colors.emplace(SelectedColorIdx);
    auto EIds = G->adjEdgeIds(Id);
    for (auto &&EId : EIds) {
      auto NId = G->getEdgeOtherNodeId(EId, Id);
      const auto &Attrs = G->getNodeAttrs(NId);
      auto Color = Attrs.SelectedColorIdx;
      Colors.emplace(Color);
    }
    ColorDegree = Colors.size();
  }

  bool needRewardDynamicThreshold() { return ColorDegree <= DynamicThreshold; }

  void reward() {
    auto NewProbabilities = Probabilities;
    for (auto j = SizeType{}, End = Probabilities.size(); j != End; ++j) {
      auto P = Probabilities[j];
      if (j == SelectedColorIdx) {
        NewProbabilities[j] = P + ParamA * (1 - P);
      } else {
        NewProbabilities[j] = (1 - ParamA) * P;
      }
    }
    Probabilities = NewProbabilities;
    Rewarded = true;
  }

  bool checkAdjColorReward() const {
    auto EIds = G->adjEdgeIds(Id);
    return std::all_of(EIds.begin(), EIds.end(), [&](auto &&EId) {
      auto NId = G->getEdgeOtherNodeId(EId, Id);
      const auto &Attrs = G->getNodeAttrs(NId);
      return Attrs.Rewarded;
    });
  }

  void updateDynamicThreshold() { DynamicThreshold = ColorDegree; }

  bool needHaltSelectedColorProbability() const {
    auto P = Probabilities[SelectedColorIdx];
    return P > Threshold;
  }

  Vertex(const GraphTy &G)
      : G(&G), Id(G.invalidNodeId()), ColorDegree(0), Probabilities(),
        Threshold(FloatType{}), DynamicThreshold(0), SelectedColorIdx(0),
        Rewarded(false) {}
};

template <typename Float>
std::ostream &operator<<(std::ostream &OS, const Vertex<Float> &V) {
  auto dumpProbabilities = [](std::ostream &OS,
                              const std::vector<Float> &Probabilities) {
    auto Separator = "";
    OS << "Probabilities ";
    for (auto P : Probabilities) {
      OS << Separator << P;
      Separator = ", ";
    }
    OS << "|";
  };
  OS << std::setprecision(2);
  OS << "Id " << V.Id << "|";
  OS << "Color Degree " << V.ColorDegree << "|";
  dumpProbabilities(OS, V.Probabilities);
  OS << "Threshold " << V.Threshold << "|";
  OS << "Dynamic threshold " << V.DynamicThreshold << "|";
  OS << "Selected Color " << V.SelectedColorIdx << "|";
  OS << "Rewarded " << V.Rewarded;
  return OS;
}

class Solver final {
public:
  using VertexTy = Vertex<double>;
  using GraphTy = Graph<VertexTy>;

private:
  struct VertexAttrs {
    VertexTy *V = nullptr;
    bool NeedCheckAdjColors = false;
  };

public:
  void solve(GraphTy &G) {
    auto Verteces = std::list<VertexAttrs>{};
    auto NIds = G.nodeIds();
    std::transform(NIds.begin(), NIds.end(), std::back_inserter(Verteces),
                   [&](auto &&NId) {
                     auto &Attrs = G.getNodeAttrs(NId);
                     Attrs.initialize(NId);
                     return VertexAttrs{&Attrs, false};
                   });
    fs::remove_all(RepoPath / "res");
    fs::create_directory(RepoPath / "res");
    unsigned StageNumber = 0;
    unsigned MaxStageNumber = 1000;
    for (;;) {
      if (Verteces.empty() || StageNumber == MaxStageNumber) {
        break;
      }
      for (auto &[V, Need] : Verteces) {
        V->chooseColor();
      }
      std::cout << "Vertixes Num " << Verteces.size() << std::endl;
      auto filepath =
          RepoPath / "res" / ("dump" + std::to_string(StageNumber) + ".dot");
      auto os = std::ofstream{filepath};
      G.dotPrint(os);
      for (auto &[V, Need] : Verteces) {
        Need = false;
        if (V->needPenalizeAdjColorChoice()) {
          std::cout << "penalized1 at stage " << StageNumber << std::endl;
          V->penalize();
        } else {
          V->computeColorDegree();
          if (V->needRewardDynamicThreshold()) {
            std::cout << "rewarded at stage " << StageNumber << std::endl;
            V->reward();
            Need = true;
          } else {
            std::cout << "penalized2 at stage " << StageNumber << std::endl;
            V->penalize();
          }
        }
      }
      for (auto &[V, Need] : Verteces) {
        if (Need) {
          if (!V->checkAdjColorReward()) {
            V->updateDynamicThreshold();
          }
        }
      }
      StageNumber++;
      Verteces.remove_if(
          [](auto &&VA) { return VA.V->needHaltSelectedColorProbability(); });
    }
    std::cout << "Vertixes Num " << Verteces.size() << std::endl;
    auto filepath =
        RepoPath / "res" / ("dump" + std::to_string(StageNumber) + ".dot");
    auto os = std::ofstream{filepath};
    G.dotPrint(os);
  }
};

} // namespace graphs
