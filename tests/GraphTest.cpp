#include "Graph.hpp"
#include <gtest/gtest.h>

using namespace graphs;

class GraphTest : public ::testing::Test {
protected:
  Graph<> graph;

  void SetUp() override {}

  void TearDown() override { graph.clear(); }
};

TEST_F(GraphTest, ComplexGraphStructureTest) {
  // Create a complex graph structure
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();
  auto nodeId3 = graph.addNode();
  auto nodeId4 = graph.addNode();

  graph.addEdge(nodeId1, nodeId2);
  graph.addEdge(nodeId2, nodeId3);
  graph.addEdge(nodeId3, nodeId4);
  graph.addEdge(nodeId4, nodeId1); // Creating a cycle

  EXPECT_EQ(graph.getNumNodes(), 4);
  EXPECT_EQ(graph.getNumEdges(), 4);
  EXPECT_EQ(graph.getNodeDegree(nodeId1),
            2); // Node 1 is connected to Node 2 and Node 4
  EXPECT_EQ(graph.getNodeDegree(nodeId2),
            2); // Node 2 is connected to Node 1 and Node 3
}

TEST_F(GraphTest, RemoveNodeWithEdgesTest) {
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();
  auto nodeId3 = graph.addNode();

  graph.addEdge(nodeId1, nodeId2);
  graph.addEdge(nodeId2, nodeId3);

  EXPECT_EQ(graph.getNumNodes(), 3);
  EXPECT_EQ(graph.getNumEdges(), 2);

  graph.removeNode(nodeId2); // This should remove edges connected to nodeId2

  EXPECT_EQ(graph.getNumNodes(), 2);
  EXPECT_EQ(graph.getNumEdges(), 0); // All edges should be removed
}

TEST_F(GraphTest, AddAndRemoveMultipleEdgesTest) {
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();
  auto nodeId3 = graph.addNode();

  graph.addEdge(nodeId1, nodeId2);
  auto edgeId2 = graph.addEdge(nodeId2, nodeId3);
  auto edgeId3 = graph.addEdge(nodeId1, nodeId3);

  EXPECT_EQ(graph.getNumEdges(), 3);

  graph.removeEdge(edgeId2); // Remove edge between nodeId2 and nodeId3

  EXPECT_EQ(graph.getNumEdges(), 2);
  EXPECT_EQ(graph.findEdge(nodeId2, nodeId3), Graph<>::invalidEdgeId());
  EXPECT_EQ(graph.findEdge(nodeId1, nodeId3), edgeId3);
}

TEST_F(GraphTest, RetrieveNodeAttributesTest) {
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();

  graph.getNodeAttrs(nodeId1) =
      DefaultNodeAttrs(); // Set attributes (dummy in this case)
  graph.getNodeAttrs(nodeId2) = DefaultNodeAttrs();

  EXPECT_EQ(graph.getNodeAttrs(nodeId1), DefaultNodeAttrs());
  EXPECT_EQ(graph.getNodeAttrs(nodeId2), DefaultNodeAttrs());
}

TEST_F(GraphTest, CheckNonExistentEdgeTest) {
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();
  auto edgeId = graph.addEdge(nodeId1, nodeId2);

  graph.removeEdge(edgeId); // Remove the edge

  EXPECT_EQ(graph.findEdge(nodeId1, nodeId2), Graph<>::invalidEdgeId());
}

TEST_F(GraphTest, CycleDetectionTest) {
  auto nodeId1 = graph.addNode();
  auto nodeId2 = graph.addNode();
  auto nodeId3 = graph.addNode();

  graph.addEdge(nodeId1, nodeId2);
  graph.addEdge(nodeId2, nodeId3);
  graph.addEdge(nodeId3, nodeId1); // Creating a cycle

  // Here you would typically call a cycle detection function if implemented
  // For the sake of this example, we will just check the node degrees
  EXPECT_EQ(graph.getNodeDegree(nodeId1), 2);
  EXPECT_EQ(graph.getNodeDegree(nodeId2), 2);
  EXPECT_EQ(graph.getNodeDegree(nodeId3), 2);
}

int main(int argc, char *argv[]) {
  try {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
  return RUN_ALL_TESTS();
}
