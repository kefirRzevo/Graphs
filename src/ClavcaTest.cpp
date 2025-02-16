#include "clavca/GraphColoring.hpp"
#include "clavca/Options.hpp"
#include "Utils.hpp"

auto RepoPath = fs::path{__FILE__}.parent_path().parent_path();

int main(int argc, const char *argv[]) {
  using VertexTy = graphs::clavca::Vertex<double>;
  using GraphTy = graphs::Graph<VertexTy>;
  auto CfgOrOpt = graphs::clavca::readConfig(argc, argv);
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
  graphs::clavca::Solver S{G};
  if (Cfg.ParamA.has_value()) {
    S.setParamA(Cfg.ParamA.value());
  }
  if (Cfg.ParamB.has_value()) {
    S.setParamB(Cfg.ParamB.value());
  }
  if (Cfg.ParamE.has_value()) {
    S.setParamE(Cfg.ParamE.value());
  }
  if (Cfg.Seed.has_value()) {
    S.setSeed(Cfg.Seed.value());
  }
  auto ResDir = RepoPath / "res";
  graphs::clavca::ComplicatedDumper Dumper{std::cout, ResDir};
  S.solve(Dumper);
  return 0;
}
