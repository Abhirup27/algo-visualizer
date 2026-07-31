#import "algorithm.hpp"
#import "app.hpp"
#include "scene.hpp"
#include "utils.hpp"
EMSCRIPTEN_BINDINGS(algorithm_bindings) {

  emscripten::class_<IAlgorithm>("Algorithm")
      .function("getStackJSON", &IAlgorithm::getStackJSON,
                emscripten::allow_raw_pointers());
};
IAlgorithm::IAlgorithm() = default;

IAlgorithm::IAlgorithm(AlgorithmId id) : m_algoId(id) {}

const char *IAlgorithm::getStackJSON() { return ""; }

const char *IAlgorithm::getQueueJSON() { return ""; }
extern "C" {

const char *get_stack_json() {
  App::m_GetInstance().current_scene->m_algorithmInstance->getStackJSON();
}

const char *get_queue_json() {
  App::m_GetInstance().current_scene->m_algorithmInstance->getQueueJSON();
}
}
