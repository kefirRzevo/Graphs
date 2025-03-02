#include "ficavca/GraphColoring.hpp"
#include "Utils.hpp"

auto RepoPath = fs::path{__FILE__}.parent_path().parent_path();

int main(int argc, const char *argv[]) {
  using VertexTy = graphs::ficavca::Vertex;
  using GraphTy = graphs::Graph<VertexTy>;

  auto GraphInput = std::string{};
  if (argc > 1) {
    GraphInput = argv[1];
  }
  if (GraphInput.empty()) {
    GraphInput = RepoPath / "graphs" / "graph.txt";
  }
  std::ifstream ifs{GraphInput};
  auto G = graphs::readGraph<GraphTy>(ifs);
  graphs::ficavca::Solver S{G};
  auto ResDir = RepoPath / "res";
  graphs::ficavca::SimpleDumper Dumper{std::cout, ResDir};
  S.solve(Dumper);
  assert(S.validate() == true);
  return 0;
}
