#include "events.hpp"
#include "graph_scene.hpp"

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

  //empty stack;
  while (!dfs_stack.empty()) {
    dfs_stack.pop();
  }
}

void A_DFS_ADV::pause() {}

void A_DFS_ADV::traverse() {}
void A_DFS_ADV::unload() {}

void A_DFS_ADV::dfs() {}
void A_DFS_ADV::createStack() {}

void A_DFS_ADV::reset(GraphView graph) {
  IGraphAlgorithm::reset(graph);
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
void A_DFS_ADV::reset() {}
