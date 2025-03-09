#include "Generator.hpp"
#include "Utils.hpp"
#include "ficavca/GraphColoring.hpp"
#include <numeric>
#include <sstream>
#include <unordered_map>

using namespace graphs;

auto RepoPath = std::filesystem::path(__FILE__).parent_path().parent_path();

struct TestedGraph {
  int DemDeg;
  int AutDeg;
  size_t Seed;
  float Ratio;
  int VertexCount;
};

template <typename InputIt>
void printResults(InputIt first, InputIt last, std::ostream &OS) {
  std::unordered_map<int, size_t> DiffMap;
  std::for_each(first, last, [&](auto &&G) {
    auto Diff = G.AutDeg - G.DemDeg;
    DiffMap[Diff]++;
  });
  std::vector<std::pair<int, size_t>> DiffVector{DiffMap.begin(),
                                                 DiffMap.end()};
  std::sort(DiffVector.begin(), DiffVector.end(),
            [](auto &&LHS, auto &&RHS) { return LHS.first < RHS.first; });
  for (auto &&[Diff, Count] : DiffVector) {
    OS << "Diff: " << Diff << " Count: " << Count << "\n";
  }
}

void process1(std::ostream &OS) {
  using VertexTy = graphs::ficavca::Vertex;
  using GraphTy = graphs::Graph<VertexTy>;
  auto Verteces = std::vector<int>(30);
  std::iota(Verteces.begin(), Verteces.end(), 5);
  auto Ratios = std::vector<float>{0.1,  0.15, 0.2,  0.25, 0.3,  0.35, 0.4,
                                   0.45, 0.5,  0.55, 0.6,  0.65, 0.7,  0.75};
  NoDumper Dumper{};
  auto DemDeg = int{};
  auto AutDeg = int{};
  for (auto &&VCount : Verteces) {
    for (auto &&Ratio : Ratios) {
      std::stringstream Buffer;
      RandomGraphGenerator Generator{0, Ratio};
      Generator.generate(VCount, Buffer);
      auto G = readGraph<GraphTy>(Buffer);
      auto CopyG = G;
      {
        ficavca::Solver S{G};
        S.setDemocracy(false);
        AutDeg = S.solve(Dumper);
      }
      {
        ficavca::Solver S{CopyG};
        S.setDemocracy(true);
        DemDeg = S.solve(Dumper);
      }
      OS << "DemDeg " << DemDeg << "; AutDeg " << AutDeg << "\n";
      OS << "VCount " << VCount << "; Ratio " << Ratio << "\n";
    }
  }
}

void process2(std::ostream &OS) {
  using VertexTy = graphs::ficavca::Vertex;
  using GraphTy = graphs::Graph<VertexTy>;
  auto Verteces = std::vector<int>{300, 400, 500, 600, 700};
  auto Ratios = std::vector<float>{0.1,  0.15, 0.2,  0.25, 0.3,  0.35, 0.4,
                                   0.45, 0.5,  0.55, 0.6,  0.65, 0.7,  0.75};
  NoDumper Dumper{};
  auto DemDeg = int{};
  auto AutDeg = int{};
  for (auto &&VCount : Verteces) {
    for (auto &&Ratio : Ratios) {
      std::vector<TestedGraph> Tests;
      for (auto Seed = size_t{}; Seed != 1000; ++Seed) {
        std::stringstream Buffer;
        RandomGraphGenerator Generator{Seed, Ratio};
        Generator.generate(VCount, Buffer);
        auto G = readGraph<GraphTy>(Buffer);
        auto CopyG = G;
        {
          ficavca::Solver S{G};
          S.setDemocracy(false);
          AutDeg = S.solve(Dumper);
        }
        {
          ficavca::Solver S{CopyG};
          S.setDemocracy(true);
          DemDeg = S.solve(Dumper);
        }
        Tests.emplace_back(DemDeg, AutDeg, Seed, Ratio, VCount);
      }
      OS << "VCount: " << VCount << " Ratio: " << Ratio << "\n";
      printResults(Tests.begin(), Tests.end(), OS);
      OS << std::endl;
    }
  }
}

int main() {
  std::ofstream OS{RepoPath / "results" / "distribution.txt"};
  process2(OS);
  return 0;
}
