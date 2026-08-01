#include "dfs.hpp"
#include "debug_panel_fmt.hpp"
#include "events.hpp"
#include <cstdint>
#include <format>
#include <iterator>
#include <string>

A_DFS_ADV::A_DFS_ADV(AlgorithmId id, AlgorithmState state) {
  m_algoId = id;
  algorithm_state = state;
}

void A_DFS_ADV::start() {
  algorithm_state = AlgorithmState::Running;

  while (!dfs_stack.empty())
    dfs_stack.pop();

  visited.assign(m_graphView->nodes.size(), false);

  if (!m_graphView->nodes.empty()) {
    dfs_stack.push({m_graphView->root_id, DFSPhase::ENTER}); // root

    EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Stack);
    dispatchSceneEvent(e);
  }
}

void A_DFS_ADV::step() {
  algorithm_state = AlgorithmState::Stepping;
  traverse();
  EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Stack);
  dispatchSceneEvent(e);
}

void A_DFS_ADV::stop() {
  algorithm_state = AlgorithmState::Idle;

  while (!dfs_stack.empty()) {
    dfs_stack.pop();
  }
}

void A_DFS_ADV::pause() {}

void A_DFS_ADV::unload() {}

void A_DFS_ADV::traverse() {
  if (dfs_stack.empty()) {
    algorithm_state = AlgorithmState::Done;
    m_currentNode = UINT32_MAX;
    return;
  }

  DFSFrame frame = dfs_stack.top();
  dfs_stack.pop();

  uint32_t current = frame.node;

  if (frame.phase == DFSPhase::ENTER) {
    if (visited[current])
      return;

    visited[current] = true;
    m_currentNode = current;

    dfs_stack.push({current, DFSPhase::EXIT});

    // Push the children in reverse order so they pop off (and get visited) in their original left-to-right order.
    const std::vector<int> &neighbors =
        m_graphView->nodes[m_graphView->id_to_node_idx[current]].edges;
    for (int i = static_cast<int>(neighbors.size()) - 1; i >= 0; --i) {
      uint32_t neighbor = neighbors[i];
      if (!visited[neighbor]) {
        dfs_stack.push({neighbor, DFSPhase::ENTER});
      }
    }
  } else {
    m_currentNode = current;
  }

  if (dfs_stack.empty())
    algorithm_state = AlgorithmState::Done;
}

void A_DFS_ADV::reset() {
  while (!dfs_stack.empty())
    dfs_stack.pop();
  visited.clear();
  m_currentNode = UINT32_MAX;
  algorithm_state = AlgorithmState::Idle;
}

void A_DFS_ADV::reset(GraphView &graph) {
  IGraphAlgorithm::reset(graph);
  reset();
  EventDescriptor e(EventAction::AlgoStateUpdate, EventTarget::Stack);
  dispatchSceneEvent(e);
}

const char *A_DFS_ADV::getStackJSON() {
  static std::string result;

  std::stack<DFSFrame> copy = dfs_stack;
  result.clear();

  auto out = std::back_inserter(result);
  std::format_to(out, "[");

  bool first = true;
  while (!copy.empty()) {
    const DFSFrame &f = copy.top();
    if (!first)
      std::format_to(out, ",");
    std::format_to(out, GET_STACK_FMT(DFS_A), f.node,
                   dfsPhaseToString(f.phase));

    copy.pop();
    first = false;
  }

  std::format_to(out, "]");
  return result.c_str();
}
