#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <list>

#include "Graph.hh"

namespace fs = std::filesystem;

auto RepoPath = fs::path{__FILE__}.parent_path().parent_path();

namespace graphs {

template <typename Float> struct Vertex final {
  using GraphTy = Graph<Vertex<Float>>;
  using NodeId = typename GraphTy::NodeId;
  using SizeType = size_t;
  using FloatType = Float;

  static constexpr FloatType ParamA = FloatType{0.1};
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

  bool checkAdjColorChoice() const {
    auto EIds = G->adjEdgeIds(Id);
    return std::any_of(EIds.begin(), EIds.end(), [&](auto &&EId) {
      auto N1Id = G->getEdgeNode1Id(EId);
      auto N2Id = G->getEdgeNode2Id(EId);
      auto NId = Id == N1Id ? N2Id : N1Id;
      return NId == Id;
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
		auto EIds = G->adjEdgeIds(Id);
		for (auto && EId:EIds) {
			auto N1Id = G->getEdgeNode1Id(EId);
      auto N2Id = G->getEdgeNode2Id(EId);
      auto NId = Id == N1Id ? N2Id : N1Id;
			auto& Attrs = G->getNodeAttrs(NId);
			auto Color = Attrs.SelectedColorIdx;
			Colors.emplace(Color);
		}
		ColorDegree = Colors.size();
	}

  bool checkDynamicThreshold() { return ColorDegree <= DynamicThreshold; }

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
      auto N1Id = G->getEdgeNode1Id(EId);
      auto N2Id = G->getEdgeNode2Id(EId);
      auto NId = Id == N1Id ? N2Id : N1Id;
			auto& Attrs = G->getNodeAttrs(NId);
      return Attrs.Rewarded;
    });
	}

  void updateDynamicThreshold() { DynamicThreshold = ColorDegree; }

  bool checkSelectedColorProbability() const {
		auto P = Probabilities[SelectedColorIdx];
		return P <= Threshold;
	}

  Vertex(const GraphTy &G)
      : G(&G), Id(G.invalidNodeId()), ColorDegree(0), Probabilities(),
        Threshold(FloatType{}), DynamicThreshold(0), SelectedColorIdx(0),
        Rewarded(false) {}
};

template<typename Float>
std::ostream &operator<<(std::ostream &OS, const Vertex<Float> &V) {
	auto dumpProbabilities = [](std::ostream &OS, const std::vector<Float>& Probabilities) {
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
    VertexTy* V = nullptr;
    bool NeedCheckAdjColors = false;
  };

public:
  void solve(GraphTy &G) {
    auto Verteces = std::list<VertexAttrs>{};
    auto NIds = G.nodeIds();
    std::transform(NIds.begin(), NIds.end(), std::back_inserter(Verteces), [&](auto &&NId){
      auto& Attrs = G.getNodeAttrs(NId);
      Attrs.initialize(NId);
      return VertexAttrs{&Attrs, false};
    });
		auto filepath = RepoPath / "res" / "dump.dot";
		auto os = std::ofstream{filepath};
		G.dotPrint(os);
    unsigned StageNumber = 0;
    for (;;) {
      if (Verteces.empty()) {
        break;
      }
      for (auto& [V, Need] : Verteces) {
        V->chooseColor();
      }
      for (auto& [V, Need] : Verteces) {
        Need = false;
        if (V->checkAdjColorChoice()) {
          V->penalize();
        } else {
          V->computeColorDegree();
          if (V->checkDynamicThreshold()) {
            V->reward();
            Need = true;
          } else {
            V->penalize();
          }
        }
      }
      for (auto& [V, Need] : Verteces) {
        if (Need) {
          if (V->checkAdjColorReward()) {
            V->updateDynamicThreshold();
          }
        }
      }
      auto filepath = RepoPath / "res" / ("dump" + std::to_string(StageNumber) + ".dot");
      auto os = std::ofstream{filepath};
      G.dotPrint(os);
      StageNumber++;
      Verteces.erase(std::remove_if(Verteces.begin(), Verteces.end(), [](auto&& VA){
        return VA.V->checkSelectedColorProbability();
      }));
    }
  }
};

} // namespace graphs
