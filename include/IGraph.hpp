#pragma once

#include <cstddef>
#include <limits>
#include <utility>

namespace graphs {

struct DefaultNodeAttrs {
  auto operator<=>(const DefaultNodeAttrs &) const = default;
};

struct DefaultEdgeAttrs {
  auto operator<=>(const DefaultEdgeAttrs &) const = default;
};

template <typename NodeAttrs, typename EdgeAttrs, typename Impl> class IGraph {
  Impl *impl() { return static_cast<Impl *>(this); }

protected:
  class NodeEntry;
  class EdgeEntry;

public:
  using NodeId = unsigned;
  using EdgeId = unsigned;

  /// Returns a value representing an invalid (non-existent) node.
  static NodeId invalidNodeId() { return std::numeric_limits<NodeId>::max(); }

  /// Returns a value representing an invalid (non-existent) edge.
  static EdgeId invalidEdgeId() { return std::numeric_limits<EdgeId>::max(); }

  class NodeIdSet;
  class EdgeIdSet;
  class AdjEdgeIdSet;

  /// Add a node with the given args.
  /// @param AttrsArgs Args for the new node.
  /// @return Node iterator for the added node.
  template <typename... NodeAttrsArgs>
  NodeId addNode(NodeAttrsArgs &&...AttrsArgs) {
    return impl()->addNode(std::forward(AttrsArgs)...);
  }

  /// Add an edge between the given nodes with the given args.
  /// @param N1Id First node.
  /// @param N2Id Second node.
  /// @param AttrsArgs Args for new edge.
  /// @return Edge iterator for the added edge.
  template <typename... EdgeAttrsArgs>
  EdgeId addEdge(NodeId N1Id, NodeId N2Id, EdgeAttrsArgs &&...AttrsArgs) {
    return impl()->addEdge(N1Id, N2Id,
                           std::forward<EdgeAttrsArgs>(AttrsArgs)...);
  }

  /// Returns true if the graph is empty.
  bool empty() const { return impl()->empty(); }

  NodeIdSet nodeIds() const { return impl()->nodeIds(); }

  EdgeIdSet edgeIds() const { return impl()->edgeIds(); }

  AdjEdgeIdSet adjEdgeIds(NodeId NId) { return impl()->adjEdgeIds(NId); }

  unsigned getNumNodes() const { return impl()->getNumNodes(); }

  /// Get the number of edges in the graph.
  /// @return Number of edges in the graph.
  unsigned getNumEdges() const { return impl()->getNumEdges(); }

  NodeAttrs &getNodeAttrs(NodeId NId) { return impl()->getNodeAttrs(NId); }

  const NodeAttrs &getNodeAttrs(NodeId NId) const {
    return impl()->getNodeAttrs(NId);
  }

  size_t getNodeDegree(NodeId NId) const { return impl()->getNodeDegree(NId); }

  EdgeAttrs &getEdgeAttrs(EdgeId EId) { return impl()->getEdgeAttrs(EId); }

  const EdgeAttrs &getEdgeAttrs(EdgeId EId) const {
    return impl()->getEdgeAttrs(EId);
  }

  /// Get the first node connected to this edge.
  /// @param EId Edge id.
  /// @return The first node connected to the given edge.
  NodeId getEdgeNode1Id(EdgeId EId) const {
    return impl()->getEdgeNode1Id(EId);
  }

  /// Get the second node connected to this edge.
  /// @param EId Edge id.
  /// @return The second node connected to the given edge.
  NodeId getEdgeNode2Id(EdgeId EId) const {
    return impl()->getEdgeNode2Id(EId);
  }

  /// Get the "other" node connected to this edge.
  /// @param EId Edge id.
  /// @param NId Node id for the "given" node.
  /// @return The iterator for the "other" node connected to this edge.
  NodeId getEdgeOtherNodeId(EdgeId EId, NodeId NId) const {
    return impl()->getEdgeOtherNodeId(EId, NId);
  }

  /// Get the edge connecting two nodes.
  /// @param N1Id First node id.
  /// @param N2Id Second node id.
  /// @return An id for edge (N1Id, N2Id) if such an edge exists,
  ///         otherwise returns an invalid edge id.
  EdgeId findEdge(NodeId N1Id, NodeId N2Id) const {
    return impl()->findEdge(N1Id, N2Id);
  }

  /// Remove a node from the graph.
  /// @param NId Node id.
  void removeNode(NodeId NId) { impl()->removeNode(NId); }

  /// Remove an edge from the graph.
  /// @param EId Edge id.
  void removeEdge(EdgeId EId) { impl()->removeEdge(EId); }

  /// Remove all nodes and edges from the graph.
  void clear() { impl()->clear(); }
};

} // namespace graphs
