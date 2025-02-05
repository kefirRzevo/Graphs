#include "include/GraphColoring.hh"

int main() {
  graphs::Graph<graphs::Vertex<double>> g;
  auto v1 = g.addNode(g);
  auto v2 = g.addNode(g);
  auto v3 = g.addNode(g);
  auto v4 = g.addNode(g);
  auto v5 = g.addNode(g);
  auto v6 = g.addNode(g);
  g.addEdge(v1, v2);
  g.addEdge(v2, v3);
  g.addEdge(v3, v4);
  g.addEdge(v3, v1);
  g.addEdge(v5, v6);
  g.addEdge(v1, v5);
  g.addEdge(v6, v2);
  g.addEdge(v6, v1);

  graphs::Solver s;
  s.solve(g);
  std::cout << "nodes " << g.getNumNodes() << std::endl;
  std::cout << "edges " << g.getNumEdges() << std::endl;
  return 0;
}
