// AUTO-GENERATED — do not edit. Source: include/scene_registry.hpp

export const SceneType = {
  Graph: 0,
  Sort: 1,
} as const;
export type SceneType = typeof SceneType[keyof typeof SceneType];

export const SceneTypeLabel: Record<SceneType, string> = {
  [0]: "Graph & Tree",
  [1]: "Sort",
};

export const AlgorithmId = {
  DFS: 0,
  DFS_A: 1,
  BFS: 2,
  Bubble: 3,
} as const;
export type AlgorithmId = typeof AlgorithmId[keyof typeof AlgorithmId];

export const AlgorithmIdLabel: Record<AlgorithmId, string> = {
  [0]: "DFS",
  [1]: "DFS_A",
  [2]: "BFS",
  [3]: "Bubble",
};
