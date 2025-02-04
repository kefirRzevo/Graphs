#pragma once

#include <cassert>
#include <vector>

#include "IGraph.hh"

namespace graphs {

template <typename NodeAttrs = DefaultNodeAttrs,
          typename EdgeAttrs = DefaultEdgeAttrs>
class Graph final
    : public IGraph<NodeAttrs, EdgeAttrs, Graph<NodeAttrs, EdgeAttrs>> {
  using BaseGraph = IGraph<NodeAttrs, EdgeAttrs, Graph<NodeAttrs, EdgeAttrs>>;

public:
  using typename BaseGraph::NodeId;
  using typename BaseGraph::EdgeId;

private:
  class NodeEntry {
  public:
    using AdjEdgeList = std::vector<EdgeId>;
    using AdjEdgeIdx = typename AdjEdgeList::size_type;
    using AdjEdgeItr = typename AdjEdgeList::const_iterator;

    template <typename... NodeAttrsArgs>
    NodeEntry(NodeAttrsArgs &&...Args)
        : Attrs(std::forward<NodeAttrsArgs>(Args)...) {}

    static AdjEdgeIdx getInvalidAdjEdgeIdx() {
      return std::numeric_limits<AdjEdgeIdx>::max();
    }

    AdjEdgeIdx addAdjEdgeId(EdgeId EId) {
      auto Idx = AdjEdgeIds.size();
      AdjEdgeIds.push_back(EId);
      return Idx;
    }

    void removeAdjEdgeId(Graph &G, NodeId ThisNId, AdjEdgeIdx Idx) {
      G.getEdge(AdjEdgeIds.back()).setAdjEdgeIdx(ThisNId, Idx);
      AdjEdgeIds[Idx] = AdjEdgeIds.back();
      AdjEdgeIds.pop_back();
    }

    const AdjEdgeList &getAdjEdgeIds() const { return AdjEdgeIds; }

    NodeAttrs Attrs;

  private:
    AdjEdgeList AdjEdgeIds;
  };

  class EdgeEntry {
  public:
    using AdjEdgeIdx = typename NodeEntry::AdjEdgeIdx;

    template <typename... EdgeAttrsArgs>
    EdgeEntry(NodeId N1Id, NodeId N2Id, EdgeAttrsArgs &&...Args)
        : Attrs(std::forward<EdgeAttrsArgs>(Args)...) {
      NIds[0] = N1Id;
      NIds[1] = N2Id;
      ThisEdgeAdjIdxs[0] = NodeEntry::getInvalidAdjEdgeIdx();
      ThisEdgeAdjIdxs[1] = NodeEntry::getInvalidAdjEdgeIdx();
    }

    void connectToN(Graph &G, EdgeId ThisEdgeId, unsigned NIdx) {
      assert(ThisEdgeAdjIdxs[NIdx] == NodeEntry::getInvalidAdjEdgeIdx() &&
             "Edge already connected to NIds[NIdx].");
      NodeEntry &N = G.getNode(NIds[NIdx]);
      ThisEdgeAdjIdxs[NIdx] = N.addAdjEdgeId(ThisEdgeId);
    }

    void connect(Graph &G, EdgeId ThisEdgeId) {
      connectToN(G, ThisEdgeId, 0);
      connectToN(G, ThisEdgeId, 1);
    }

    void setAdjEdgeIdx(NodeId NId, AdjEdgeIdx NewIdx) {
      if (NId == NIds[0])
        ThisEdgeAdjIdxs[0] = NewIdx;
      else {
        assert(NId == NIds[1] && "Edge not connected to NId");
        ThisEdgeAdjIdxs[1] = NewIdx;
      }
    }

    void disconnectFromN(Graph &G, unsigned NIdx) {
      assert(ThisEdgeAdjIdxs[NIdx] != NodeEntry::getInvalidAdjEdgeIdx() &&
             "Edge not connected to NIds[NIdx].");
      NodeEntry &N = G.getNode(NIds[NIdx]);
      N.removeAdjEdgeId(G, NIds[NIdx], ThisEdgeAdjIdxs[NIdx]);
      ThisEdgeAdjIdxs[NIdx] = NodeEntry::getInvalidAdjEdgeIdx();
    }

    void disconnectFrom(Graph &G, NodeId NId) {
      if (NId == NIds[0])
        disconnectFromN(G, 0);
      else {
        assert(NId == NIds[1] && "Edge does not connect NId");
        disconnectFromN(G, 1);
      }
    }

    NodeId getN1Id() const { return NIds[0]; }
    NodeId getN2Id() const { return NIds[1]; }

    EdgeAttrs Attrs;

  private:
    NodeId NIds[2];
    AdjEdgeIdx ThisEdgeAdjIdxs[2];
  };

  using NodeVector = std::vector<NodeEntry>;
  using FreeNodeVector = std::vector<NodeId>;
  using EdgeVector = std::vector<EdgeEntry>;
  using FreeEdgeVector = std::vector<EdgeId>;

  NodeVector Nodes;
  FreeNodeVector FreeNodeIds;
  EdgeVector Edges;
  FreeEdgeVector FreeEdgeIds;

  NodeEntry &getNode(NodeId NId) {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }
  const NodeEntry &getNode(NodeId NId) const {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }

  EdgeEntry &getEdge(EdgeId EId) { return Edges[EId]; }
  const EdgeEntry &getEdge(EdgeId EId) const { return Edges[EId]; }

  NodeId addConstructedNode(NodeEntry N) {
    NodeId NId = 0;
    if (!FreeNodeIds.empty()) {
      NId = FreeNodeIds.back();
      FreeNodeIds.pop_back();
      Nodes[NId] = std::move(N);
    } else {
      NId = Nodes.size();
      Nodes.push_back(std::move(N));
    }
    return NId;
  }

  EdgeId addConstructedEdge(EdgeEntry E) {
    assert(findEdge(E.getN1Id(), E.getN2Id()) == BaseGraph::invalidEdgeId() &&
           "Attempt to add duplicate edge.");
    EdgeId EId = 0;
    if (!FreeEdgeIds.empty()) {
      EId = FreeEdgeIds.back();
      FreeEdgeIds.pop_back();
      Edges[EId] = std::move(E);
    } else {
      EId = Edges.size();
      Edges.push_back(std::move(E));
    }

    EdgeEntry &NE = getEdge(EId);

    // Add the edge to the adjacency sets of its nodes.
    NE.connect(*this, EId);
    return EId;
  }

public:
  using AdjEdgeItr = typename NodeEntry::AdjEdgeItr;

  class NodeItr {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = NodeId;
    using difference_type = int;
    using pointer = NodeId *;
    using reference = NodeId &;

    NodeItr(NodeId CurNId, const Graph &G)
        : CurNId(CurNId), EndNId(G.Nodes.size()), FreeNodeIds(G.FreeNodeIds) {
      this->CurNId = findNextInUse(CurNId);
    }

    bool operator==(const NodeItr &O) const { return CurNId == O.CurNId; }
    bool operator!=(const NodeItr &O) const { return !(*this == O); }
    NodeItr &operator++() {
      CurNId = findNextInUse(++CurNId);
      return *this;
    }
    NodeId operator*() const { return CurNId; }

  private:
    NodeId findNextInUse(NodeId NId) const {
      while (NId < EndNId && std::find(FreeNodeIds.begin(), FreeNodeIds.end(),
                                       NId) != FreeNodeIds.end()) {
        ++NId;
      }
      return NId;
    }

    NodeId CurNId, EndNId;
    const FreeNodeVector &FreeNodeIds;
  };

  class EdgeItr {
  public:
    EdgeItr(EdgeId CurEId, const Graph &G)
        : CurEId(CurEId), EndEId(G.Edges.size()), FreeEdgeIds(G.FreeEdgeIds) {
      this->CurEId = findNextInUse(CurEId);
    }

    bool operator==(const EdgeItr &O) const { return CurEId == O.CurEId; }
    bool operator!=(const EdgeItr &O) const { return !(*this == O); }
    EdgeItr &operator++() {
      CurEId = findNextInUse(++CurEId);
      return *this;
    }
    EdgeId operator*() const { return CurEId; }

  private:
    EdgeId findNextInUse(EdgeId EId) const {
      while (EId < EndEId && std::find(FreeEdgeIds.begin(), FreeEdgeIds.end(),
                                       EId) != FreeEdgeIds.end()) {
        ++EId;
      }
      return EId;
    }

    EdgeId CurEId, EndEId;
    const FreeEdgeVector &FreeEdgeIds;
  };

  class NodeIdSet {
  public:
    NodeIdSet(const Graph &G) : G(G) {}

    NodeItr begin() const { return NodeItr(0, G); }
    NodeItr end() const { return NodeItr(G.Nodes.size(), G); }

    bool empty() const { return G.Nodes.empty(); }

    typename NodeVector::size_type size() const {
      return G.Nodes.size() - G.FreeNodeIds.size();
    }

  private:
    const Graph &G;
  };

  class EdgeIdSet {
  public:
    EdgeIdSet(const Graph &G) : G(G) {}

    EdgeItr begin() const { return EdgeItr(0, G); }
    EdgeItr end() const { return EdgeItr(G.Edges.size(), G); }

    bool empty() const { return G.Edges.empty(); }

    typename NodeVector::size_type size() const {
      return G.Edges.size() - G.FreeEdgeIds.size();
    }

  private:
    const Graph &G;
  };

  class AdjEdgeIdSet {
  public:
    AdjEdgeIdSet(const NodeEntry &NE) : NE(NE) {}

    typename NodeEntry::AdjEdgeItr begin() const {
      return NE.getAdjEdgeIds().begin();
    }

    typename NodeEntry::AdjEdgeItr end() const {
      return NE.getAdjEdgeIds().end();
    }

    bool empty() const { return NE.getAdjEdgeIds().empty(); }

    typename NodeEntry::AdjEdgeList::size_type size() const {
      return NE.getAdjEdgeIds().size();
    }

  private:
    const NodeEntry &NE;
  };

  template <typename... NodeAttrsArgs>
  NodeId addNode(NodeAttrsArgs &&...AttrsArgs) {
    NodeId NId = addConstructedNode(
        NodeEntry(std::forward<NodeAttrsArgs>(AttrsArgs)...));
    return NId;
  }

  template <typename... EdgeAttrsArgs>
  EdgeId addEdge(NodeId N1Id, NodeId N2Id, EdgeAttrsArgs &&...AttrsArgs) {
    EdgeId EId = addConstructedEdge(
        EdgeEntry(N1Id, N2Id, std::forward<EdgeAttrsArgs>(AttrsArgs)...));
    return EId;
  }

  bool empty() const { return NodeIdSet(*this).empty(); }

  NodeIdSet nodeIds() const { return NodeIdSet(*this); }

  EdgeIdSet edgeIds() const { return EdgeIdSet(*this); }

  AdjEdgeIdSet adjEdgeIds(NodeId NId) const { return AdjEdgeIdSet(getNode(NId)); }

  unsigned getNumNodes() const { return NodeIdSet(*this).size(); }

  unsigned getNumEdges() const { return EdgeIdSet(*this).size(); }

  NodeAttrs &getNodeAttrs(NodeId NId) { return getNode(NId).Attrs; }

  const NodeAttrs &getNodeAttrs(NodeId NId) const { return getNode(NId).Attrs; }

  typename NodeEntry::AdjEdgeList::size_type getNodeDegree(NodeId NId) const {
    return getNode(NId).getAdjEdgeIds().size();
  }

  EdgeAttrs &getEdgeAttrs(EdgeId EId) { return getEdge(EId).Attrs; }

  const EdgeAttrs &getEdgeAttrs(EdgeId EId) const { return getEdge(EId).Attrs; }

  NodeId getEdgeNode1Id(EdgeId EId) const { return getEdge(EId).getN1Id(); }

  NodeId getEdgeNode2Id(EdgeId EId) const { return getEdge(EId).getN2Id(); }

  NodeId getEdgeOtherNodeId(EdgeId EId, NodeId NId) {
    EdgeEntry &E = getEdge(EId);
    if (E.getN1Id() == NId) {
      return E.getN2Id();
    }
    return E.getN1Id();
  }

  EdgeId findEdge(NodeId N1Id, NodeId N2Id) {
    for (auto &&AEId : adjEdgeIds(N1Id)) {
      if ((getEdgeNode1Id(AEId) == N2Id) || (getEdgeNode2Id(AEId) == N2Id)) {
        return AEId;
      }
    }
    return BaseGraph::invalidEdgeId();
  }

  void removeNode(NodeId NId) {
    NodeEntry &N = getNode(NId);
    for (AdjEdgeItr AEItr = N.adjEdgesBegin(), AEEnd = N.adjEdgesEnd();
         AEItr != AEEnd;) {
      EdgeId EId = *AEItr;
      ++AEItr;
      removeEdge(EId);
    }
    FreeNodeIds.push_back(NId);
  }

  void removeEdge(EdgeId EId) {
    EdgeEntry &E = getEdge(EId);
    E.disconnect();
    FreeEdgeIds.push_back(EId);
  }

  void clear() {
    Nodes.clear();
    FreeNodeIds.clear();
    Edges.clear();
    FreeEdgeIds.clear();
  }

  void dotPrint(std::ostream &OS) const {
    OS << "graph {\n";
    OS << "\trankdir=LR;\n";
    OS << "\tnode[shape=record, style=filled, fontcolor=black];\n";
    for (auto Id : nodeIds()) {
      const auto &Entry = getNode(Id);
      OS << "\tnode_" << Id << "[label = \"" << Entry.Attrs << "\"];\n";
    }
    for (auto Id : edgeIds()) {
      const auto &Entry = getEdge(Id);
      OS << "\tnode_" << Entry.getN1Id() << " -- node_";
      OS << Entry.getN2Id() << "[label = \"" << Entry.Attrs << "\"];\n";
    }
    OS << "}" << std::endl;
  }
};

} // namespace graphs
