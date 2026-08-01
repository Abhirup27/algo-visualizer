#pragma once
#include "graph_scene.hpp"
#include <stack>
#include <vector>

class A_DFS_ADV : public IGraphAlgorithm {

public:
  A_DFS_ADV(AlgorithmId, AlgorithmState);

  enum class DFSPhase {
    ENTER,
    EXIT
  };

  struct DFSFrame {
    uint32_t node;
    DFSPhase phase;
  };

  inline const char *dfsPhaseToString(DFSPhase phase) {
    switch (phase) {
    case DFSPhase::ENTER:
      return "ENTER";
    case DFSPhase::EXIT:
      return "EXIT";
    default:
      return "UNKNOWN";
    }
  }

  std::stack<DFSFrame> dfs_stack;
  std::vector<bool> visited;

  void traverse() override;

  void reset() override;
  void reset(GraphView &) override;
  void start() override;
  void step() override;
  void stop() override;
  void pause() override;

  void unload() override;

  const char *getStackJSON() override;
};
