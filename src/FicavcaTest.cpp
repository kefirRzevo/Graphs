#include "Utils.hpp"
#include "ficavca/GraphColoring.hpp"
#include "ficavca/Options.hpp"

auto RepoPath = fs::path{__FILE__}.parent_path().parent_path();

int main(int argc, const char *argv[]) {
  using VertexTy = graphs::ficavca::Vertex;
  using GraphTy = graphs::Graph<VertexTy>;
  auto CfgOrOpt = graphs::ficavca::readConfig(argc, argv);
  if (!CfgOrOpt.has_value()) {
    return 0;
  }
  auto Cfg = CfgOrOpt.value();
  auto GraphInput = Cfg.GraphInput;
  if (GraphInput.empty()) {
    GraphInput = RepoPath / "graphs" / "graph.txt";
  }
  std::ifstream ifs{GraphInput};
  auto G = graphs::readGraph<GraphTy>(ifs);
  graphs::ficavca::Solver S{G};
  if (Cfg.Democracy.has_value()) {
    S.setDemocracy(Cfg.Democracy.value());
  }
  auto ResDir = RepoPath / "res";
  graphs::ficavca::SimpleDumper Dumper{std::cout, ResDir};
  auto Degree = S.solve(Dumper);
  std::cout << "Nodes count " << G.nodeIds().size() << "\n";
  std::cout << "Color degree " << Degree << "\n";
  assert(S.validate() == true);
  return 0;
}
