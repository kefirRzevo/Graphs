#pragma once

#include "Dumper.hpp"
#include "Graph.hpp"
#include "ficavca/Vertex.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace graphs {

namespace ficavca {

struct ComplicatedDumper {
  std::ostream &OS;
  fs::path Dir;

  ComplicatedDumper(std::ostream &O, fs::path D) : OS(O), Dir(D) {
    fs::remove_all(D);
    fs::create_directory(D);
  }

  void complicatedVertexDump(std::ostream &OS, const Vertex &V) {
    OS << std::setprecision(2);
    OS << "Colored " << V.Colored << "|";
    OS << "Color " << V.Color << "|";
    OS << "Purity " << V.PurityValue << "|";
    OS << "VoteWeight " << V.VoteWeight << "|";
  }

  void dumpDbgMsg(std::string_view Msg) {}

  void dumpMsg(std::string_view Msg) { OS << Msg; }

  template <typename GraphTy>
  void dumpGraphByStage(std::string_view Path, const GraphTy &G) {
    auto OS = std::ofstream{(Dir / Path).c_str()};
    OS << "graph {\n";
    OS << "\trankdir=LR;\n";
    OS << "\tnode[shape=record, style=filled, fontcolor=black];\n";
    for (auto NId : G.nodeIds()) {
      const auto &V = G.getNodeAttrs(NId);
      OS << "\tnode_" << NId << "[label = \"Id " << NId << "|";
      complicatedVertexDump(OS, V);
      OS << "Neighbors " << G.adjEdgeIds(NId).size() << "\"];\n";
    }
    for (auto Id : G.edgeIds()) {
      auto V1 = G.getEdgeNode1Id(Id);
      auto V2 = G.getEdgeNode2Id(Id);
      OS << "\tnode_" << V1 << " -- node_" << V2 << ";\n";
    }
    OS << "}" << std::endl;
  }
};

struct SimpleDumper {
  std::ostream &OS;
  fs::path Dir;

  SimpleDumper(std::ostream &O, fs::path D) : OS(O), Dir(D) {
    fs::remove_all(D);
    fs::create_directory(D);
  }

  void dumpDbgMsg(std::string_view Msg) {}

  void dumpMsg(std::string_view Msg) { OS << Msg; }

  void simpleVertexDump(std::ostream &OS, const Vertex &V) {
    auto AsString = std::to_string(V.Color);
    auto Hash = std::hash<std::string>{}(AsString);
    auto FitHash = Hash & 0xFFFFFF;
    OS << "\"#" << std::hex << FitHash << "\"";
  }

  template <typename GraphTy>
  void dumpGraphByStage(std::string_view Path, const GraphTy &G) {
    auto OS = std::ofstream{(Dir / Path).c_str()};
    OS << "graph {\n";
    OS << "\trankdir=LR;\n";
    OS << "\tnode[style=filled, fontcolor=black];\n";
    for (auto Id : G.nodeIds()) {
      const auto &V = G.getNodeAttrs(Id);
      OS << "\tnode_" << Id << "[label = \"Id " << Id << "\", ";
      OS << "fillcolor=";
      simpleVertexDump(OS, V);
      OS << "];\n";
    }
    for (auto Id : G.edgeIds()) {
      auto V1 = G.getEdgeNode1Id(Id);
      auto V2 = G.getEdgeNode2Id(Id);
      OS << "\tnode_" << V1 << " -- node_" << V2 << ";\n";
    }
    OS << "}" << std::endl;
  }
};

} // namespace ficavca

} // namespace graphs
