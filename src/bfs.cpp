#include "bfs.hpp"
#include "debug_panel_fmt.hpp"
#include "events.hpp"
#include <cstdint>
#include <format>
#include <iterator>
#include <string>

A_BFS::A_BFS(AlgorithmId id, AlgorithmState state) {
  m_algoId = id;
  algorithm_state = state;
}

void A_BFS::start() {
  algorithm_state = AlgorithmState::Running;

  while (!bfs_queue.empty())
    bfs_queue.pop();

  visited.assign(m_graphView->nodes.size(), false);

  if (!m_graphView->nodes.empty()) {
    uint32_t root = m_graphView->root_id;
    visited[root] = true;
    bfs_queue.push(root);

    EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Queue);
    dispatchSceneEvent(e);
  }
}

void A_BFS::step() {
  algorithm_state = AlgorithmState::Stepping;
  traverse();
  EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Queue);
  dispatchSceneEvent(e);
}

void A_BFS::stop() {
  algorithm_state = AlgorithmState::Idle;

  while (!bfs_queue.empty())
    bfs_queue.pop();
}

void A_BFS::pause() {}

void A_BFS::unload() {}

void A_BFS::traverse() {
  if (bfs_queue.empty()) {
    algorithm_state = AlgorithmState::Done;
    m_currentNode = UINT32_MAX;
    return;
  }

  uint32_t current = bfs_queue.front();
  bfs_queue.pop();
  m_currentNode = current;

  const std::vector<int> &neighbors =
      m_graphView->nodes[m_graphView->id_to_node_idx[current]].edges;
  for (int neighbor : neighbors) {
    if (!visited[neighbor]) {
      visited[neighbor] = true;
      bfs_queue.push(neighbor);
    }
  }

  if (bfs_queue.empty())
    algorithm_state = AlgorithmState::Done;
}

void A_BFS::reset() {
  while (!bfs_queue.empty())
    bfs_queue.pop();
  visited.clear();
  m_currentNode = UINT32_MAX;
  algorithm_state = AlgorithmState::Idle;
}

void A_BFS::reset(GraphView &graph) {
  IGraphAlgorithm::reset(graph);
  reset();
  EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Queue);
  dispatchSceneEvent(e);
}

const char *A_BFS::getQueueJSON() {
  static std::string result;

  std::queue<uint32_t> copy = bfs_queue;
  result.clear();

  auto out = std::back_inserter(result);
  std::format_to(out, "[");

  bool first = true;
  while (!copy.empty()) {
    if (!first)
      std::format_to(out, ",");
    std::format_to(out, GET_QUEUE_FMT(BFS), copy.front());

    copy.pop();
    first = false;
  }

  std::format_to(out, "]");
  return result.c_str();
}
