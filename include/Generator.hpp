#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace graphs {

class RandomGraphGenerator final {
public:
  using SizeTy = size_t;
  using SeedTy = int;
  using RatioTy = float;

private:
  SizeTy Seed;
  RatioTy Ratio;

  template <typename T> class RandomizedSet final {
    using IndexTy = size_t;

    std::vector<T> Values;
    std::unordered_map<T, IndexTy> Indexes;

    bool swap(const T &LHS, const T &RHS) {
      auto foundLHS = Indexes.find(LHS);
      if (foundLHS == Indexes.end())
        return false;
      auto foundRHS = Indexes.find(RHS);
      if (foundRHS == Indexes.end())
        return false;
      auto indexLHS = foundLHS->second;
      auto indexRHS = foundRHS->second;
      std::swap(Values.at(indexLHS), Values.at(indexRHS));
      foundLHS->second = indexRHS;
      foundRHS->second = indexLHS;
      return true;
    }

  public:
    RandomizedSet() = default;

    bool insert(const T &Val) {
      auto found = Indexes.find(Val);
      if (found != Indexes.end())
        return false;
      Values.push_back(Val);
      Indexes.emplace(Val, Values.size() - 1);
      return true;
    }

    bool remove(const T &Val) {
      if (Values.empty())
        return false;
      auto lastVal = Values.back();
      if (!swap(Val, lastVal))
        return false;
      Values.pop_back();
      auto erased = Indexes.erase(Val);
      assert(erased == 1);
      return true;
    }

    const T &getRandom() {
      assert(Values.empty() == false);
      auto Index = rand() % Values.size();
      return Values.at(Index);
    }
  };

  static RandomizedSet<SizeTy> createRandSet(SizeTy first, SizeTy last) {
    auto Set = RandomizedSet<SizeTy>{};
    for (auto Val = first; Val != last; ++Val) {
      Set.insert(Val);
    }
    return Set;
  }

  // Must be assotiative
  struct Edge final {
    SizeTy Start;
    SizeTy End;
  };

  struct EdgeHash final {
    size_t operator()(const Edge &E) const noexcept {
      uintmax_t LHS = std::hash<SizeTy>{}(E.Start);
      uintmax_t RHS = std::hash<SizeTy>{}(E.End);
      return LHS + RHS;
    }
  };

  struct EdgeEqual final {
    bool operator()(const Edge &LHS, const Edge &RHS) const {
      return (LHS.Start == RHS.Start && LHS.End == RHS.End) ||
             (LHS.Start == RHS.End && LHS.End == RHS.Start);
    }
  };

public:
  RandomGraphGenerator(SizeTy Seed, RatioTy Ratio) : Seed(Seed), Ratio(Ratio) {
    assert(Ratio >= 0 && Ratio <= 1);
  }

  void setSeed(SeedTy S) { Seed = S; }

  void setRatio(RatioTy R) {
    assert(R >= 0 && R <= 1);
    Ratio = R;
  }

  void generate(SizeTy VerticesCount, std::ostream &OS) {
    assert(VerticesCount > 0);
    auto InGraph = createRandSet(SizeTy{}, SizeTy{1});
    auto OutGraph = createRandSet(SizeTy{1}, VerticesCount);
    auto Total = createRandSet(SizeTy{}, VerticesCount);
    auto Edges = std::unordered_set<Edge, EdgeHash, EdgeEqual>{};
    auto EdgesMaxCount = VerticesCount * (VerticesCount - 1) / 2;
    auto StageFirst = SizeTy{};
    auto StageLast =
        static_cast<SizeTy>(static_cast<RatioTy>(EdgesMaxCount) * Ratio);
    for (auto Stage = StageFirst; Stage != StageLast; ++Stage) {
      auto VStart = SizeTy{};
      auto VEnd = SizeTy{};
      if (Stage < VerticesCount - 1) {
        do {
          VStart = InGraph.getRandom();
          InGraph.insert(VEnd);
          VEnd = OutGraph.getRandom();
          OutGraph.remove(VEnd);
        } while (Edges.count(Edge{VStart, VEnd}));
      } else {
        do {
          VStart = Total.getRandom();
          Total.remove(VStart);
          VEnd = Total.getRandom();
          Total.insert(VStart);
        } while (Edges.count(Edge{VStart, VEnd}));
      }
      Edges.emplace(VStart, VEnd);
      OS << VStart << " - " << VEnd << "\n";
    }
  }
};

} // namespace graphs
