
#pragma once

// every algorithm drives the scene identically through the Python hidden-API surface one call at a time.
typedef enum AlgorithmState {
  Idle,
  Stepping,
  Running,
  Done
} AlgorithmState;
