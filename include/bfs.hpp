#pragma once
#include "graph_scene.hpp"
#include <queue>
#include <vector>

class A_BFS : public IGraphAlgorithm {

public:
  A_BFS(AlgorithmId, AlgorithmState);

  std::queue<uint32_t> bfs_queue;
  std::vector<bool> visited;

  void traverse() override;

  void reset() override;
  void reset(GraphView &) override;
  void start() override;
  void step() override;
  void stop() override;
  void pause() override;

  void unload() override;

  const char *getQueueJSON() override;
};
