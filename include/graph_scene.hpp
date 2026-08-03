#pragma once
#include "colors.h"
#include "debug_panel_fmt.hpp"
#include "events.hpp"
#include "raylib.h"
#include "scene.hpp"
#include "scene_registry.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <stack>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/em_macros.h>
#include <emscripten/em_types.h>
#endif // PLATOFRM_WEB

struct Edge {
  size_t from;
  size_t to;
};

struct Node {
  uint32_t id;

  Vector2 pos;
  Vector2 oldPos;
  uint16_t radius;
  Rectangle collider;

  int64_t data;
  std::vector<int> edges;
};

class GraphScene : public Scene {

public:
  // void GuiAlgoViz(BaseGuiState *state);

  enum class InteractionMode {
    None = 0, // can hover and delete
    NodeSelect = 1,
    EdgeSelect = 2,

    NodeCreate = 3,
    NodeEdit = 4,

    EdgeCreate = 5,
    EdgeEdit = 6,
  } m_input_mode;

  int main_mode = 0; // 0=FREE, 1=NODE, 2=EDGE
  int sub_mode = 0;  // 0=SELECT, 1=CREATE, 2=EDIT
  //

  // Node *root;
  uint32_t root_id;
  std::unordered_map<uint32_t, size_t> id_to_node_idx;
  std::vector<Node> nodes;
  std::vector<Edge> edges;

  //NOTE: Refactor these iterators to ids
  std::vector<Node>::iterator selected_node;
  std::vector<Node>::iterator selected_edge_origin;

  bool moveCamera = false;
  size_t hoveredEdgeIdx = SIZE_MAX;
  size_t hoveredNodeIdx = SIZE_MAX;

  // --- Scripted (Python-driven) execution state -----------------------
  //
  // This remains as the source of truth.
  std::vector<uint32_t> script_stack;
  std::deque<uint32_t> script_queue;
  std::vector<bool> visited;    // processed
  std::vector<bool> discovered; // seen, queued/pushed, but not yet processed
  uint32_t active_node_id = UINT32_MAX; // node currently being processed

  GraphScene(Font *, Arena);
  void init() override;
  void input() override;
  void update_input_mode();
  void draw(IVector2 *) override;
  void drawUI(IVector2);
  void update(IVector2 *resolution) override;
  bool IsMouseHoveringEdge(const Vector2 &, const Vector2 &, const Vector2 &,
                           float thickness = 5.0f);

  void resetScene() override;
  void resetRunState() override;

  void switchAlgorithm(AlgorithmId id) override;
  //updates the camera position when the mouse is hovring over an UI element
  void gotoNode(IVector2 *resolution);
  void gotoPos(IVector2 *resolution);
  void resetCameraPos() override;

  void updateMode(int, int) override;
  void setRootNode(uint32_t) override;
  void setNodeVal(uint32_t, int) override;
  void setHoverState(bool, uint32_t) override;

  const char *getAdjJSON() override;
  const char *getNodeListJSON() override;
  const char *getRootNodeJSON() override;

  // --- Python API surface ---------------------------------------
  uint32_t scriptStackPush(uint32_t node_id) override;
  uint32_t scriptStackPop() override;
  size_t scriptStackSize() override;

  void scriptQueueEnqueue(uint32_t node_id) override;
  uint32_t scriptQueueDequeue() override;
  size_t scriptQueueSize() override;

  void markVisited(uint32_t node_id, bool visited_flag) override;
  void markDiscovered(uint32_t node_id, bool discovered_flag) override;
  void setActiveNode(uint32_t node_id) override;

  uint32_t addNode(int64_t data) override;
  void addEdge(uint32_t from_id, uint32_t to_id) override;
  void removeNode(uint32_t node_id) override;
  void removeEdge(uint32_t from_id, uint32_t to_id) override;

  void setAlgoState(AlgorithmState state) override;

  const char *getScriptStackJSON() override;
  const char *getScriptQueueJSON() override;

  static GraphScene *scene_ptr;
};
