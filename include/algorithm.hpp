
#pragma once
#include "scene_registry.hpp"
#import <emscripten.h>
#include <emscripten/bind.h>
typedef enum AlgorithmState {
  Idle,
  Stepping,
  Running,
  Done
} AlgorithmState;

// enum class AlgorithmState {Idle, Stepping, Running, Done};

class IAlgorithm {

public:
  AlgorithmId m_algoId;
  AlgorithmState algorithm_state;
  virtual ~IAlgorithm() = default;
  IAlgorithm();
  IAlgorithm(AlgorithmId);
  int getAlgorithmId();
  virtual void reset() = 0;
  virtual void start() = 0;
  virtual void step() = 0;
  virtual void stop() = 0;
  virtual void pause() = 0;

  virtual void unload() = 0;

  virtual const char *getStackJSON();
  virtual const char *getQueueJSON();
};

extern "C" {
const char *EMSCRIPTEN_KEEPALIVE get_stack_json();
const char *EMSCRIPTEN_KEEPALIVE get_queue_json();
}
