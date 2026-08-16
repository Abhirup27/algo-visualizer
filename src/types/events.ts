// AUTO-GENERATED — do not edit. Source: include/events.hpp

export const EventTarget = {
  Node: 0,
  Edge: 1,
  Bar: 2,
  Stack: 3,
  Queue: 4,
} as const;
export type EventTarget = typeof EventTarget[keyof typeof EventTarget];

export const EventTargetLabel: Record<EventTarget, string> = {
  [0]: "Node",
  [1]: "Edge",
  [2]: "Bar",
  [3]: "Stack",
  [4]: "Queue",
};

export const EventAction = {
  Add: 0,
  Remove: 1,
  Edit: 2,
  AlgoStateUpdate: 3,
} as const;
export type EventAction = typeof EventAction[keyof typeof EventAction];

export const EventActionLabel: Record<EventAction, string> = {
  [0]: "Add",
  [1]: "Remove",
  [2]: "Edit",
  [3]: "AlgoStateUpdate",
};
