#pragma once

#include <string_view>

namespace graphs {

struct NoDumper {
  void dumpDbgMsg(std::string_view Msg) {}

  void dumpMsg(std::string_view Msg) {}

  template<typename GraphTy>
  void dumpGraphByStage(std::string_view Path, const GraphTy &G) {}
};

}
