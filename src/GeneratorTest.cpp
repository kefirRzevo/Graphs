#include "Generator.hpp"
#include "Utils.hpp"
#include "ficavca/GraphColoring.hpp"
#include <numeric>
#include <sstream>

using namespace graphs;

int main(int argc, const char *argv[]) {
  using VertexTy = graphs::ficavca::Vertex;
  using GraphTy = graphs::Graph<VertexTy>;
  auto Verteces = std::vector<int>(300);
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
      if (DemDeg > AutDeg + 2) {
        std::cout << "DemDeg " << DemDeg << "; AutDeg " << AutDeg << "\n";
        std::cout << "VCount " << VCount << "; Ratio " << Ratio << "\n";
        std::cout << Buffer.str() << std::endl;
      }
    }
  }
  return 0;
}
