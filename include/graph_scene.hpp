#pragma once
#include "colors.h"
#include "debug_panel_fmt.hpp"
#include "events.hpp"
#include "raylib.h"
#include "scene.hpp"
#include "scene_registry.hpp"
#include <cstddef>
#include <cstdint>
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
struct GraphView {
  std::vector<Node> &nodes;
  std::vector<Edge> &edges;
  std::unordered_map<uint32_t, size_t> &id_to_node_idx;
  uint32_t &root_id;
};

class IGraphAlgorithm : public IAlgorithm {

public:
  GraphView *m_graphView = nullptr;
  float m_algoSpeed = 1.0f;

  uint32_t m_currentNode = UINT32_MAX;

  IGraphAlgorithm();

  IGraphAlgorithm(AlgorithmId, AlgorithmState);

  // clear this algorithm's own stack/queue/visited/etc state. Every algorithm must override this for GraphScene::resetScene()
  void reset() override;

  // binds the algorithm to the scene's graph and resets step state for it.  GraphScene owns a single, stable GraphView (GraphScene::m_graphView)
  virtual void reset(GraphView &);
  virtual void traverse() = 0;

  virtual const char *getStackJSON();
  virtual const char *getQueueJSON();
};

IGraphAlgorithm *g_CreateGraphAlgorithm(AlgorithmId id, Arena *algoArena);

class GraphScene : public Scene {
  AlgorithmId a_id;

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
    EdgeNONSENSE = 7
  } m_input_mode;

  int main_mode = 0; // 0=FREE, 1=NODE, 2=EDGE
  int sub_mode = 0;  // 0=SELECT, 1=CREATE, 2=EDIT
  //

  // Node *root;
  uint32_t root_id;
  std::unordered_map<uint32_t, size_t> id_to_node_idx;
  std::vector<Node> nodes;
  std::vector<Edge> edges;

  GraphView m_graphView;

  //NOTE: Refactor these iterators to ids
  std::vector<Node>::iterator selected_node;
  std::vector<Node>::iterator selected_edge_origin;

  bool moveCamera = false;
  size_t hoveredEdgeIdx = SIZE_MAX;
  size_t hoveredNodeIdx = SIZE_MAX;

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

  static GraphScene *scene_ptr;
};
