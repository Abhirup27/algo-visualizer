"""
AlgoPlex hidden runtime API.

This module is injected by the JS harness (src/pyodide/usePyRunner.ts)
BEFORE the user's algorithm file is executed. It is never shown in the
Monaco editor. Its only public entry point, from the user's point of view,
is the `graph` (or `bars`) object passed as an argument into their
entry-point function:

    async def main(graph):
        await graph.stack.push(graph.root_id)
        ...

Design rules that make single-stepping possible
-------------------------------------------------
1. C++ is the single source of truth. Nothing in this file keeps its own
   "real" copy of the visualization state — every mutating call immediately
   forwards to the C++ Scene through `_bridge`, and C++ dispatches a
   `scene_event` that the existing React views already listen for.

2. Every mutating call is `async` and does exactly this, in order:

       await self._bridge.wait_for_step()   # (a) possibly suspend here
       await self._bridge.<actual_call>(...)  # (b) then mutate C++

   (a) is what makes the "Step" button work: it's a JS Promise that the JS
   StepController resolves either immediately (Run mode, after a delay) or
   only when the user clicks Step again (Paused mode). Because Pyodide
   awaits JS promises transparently, `await` genuinely suspends this
   coroutine without blocking the browser thread — no worker, no
   SharedArrayBuffer/Atomics required.

3. The user's own code never talks to `_bridge` directly — only through
   these wrapper objects. That's the "hidden API" boundary.
"""

import json


class AlgoplexError(Exception):
    """Raised for API misuse (e.g. a bad node id)."""


class Node:
    __slots__ = ("id", "data")

    def __init__(self, id, data):
        self.id = id
        self.data = data

    def __repr__(self):
        return f"Node({self.id}, data={self.data})"

    def __eq__(self, other):
        return isinstance(other, Node) and other.id == self.id

    def __hash__(self):
        return hash(self.id)


class Stack:
    """graph.stack — drives the C++ generic script_stack (Scene::script_stack)."""

    def __init__(self, graph):
        self._graph = graph
        self._bridge = graph._bridge

    async def push(self, node_id):
        """Push a node id. Pauses here on the next Step click."""
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.wait_for_step()
        await self._bridge.push_stack(node_id)
        await self._graph._set_current_no_wait(node_id)

    async def pop(self):
        """Pop and return the top node id, or None if empty."""
        await self._bridge.wait_for_step()
        result = await self._bridge.pop_stack()
        await self._graph._set_current_no_wait(result)
        return None if result is None or result < 0 else result

    def __len__(self):
        return self._bridge.stack_size()

    @property
    def empty(self):
        return len(self) == 0


class Queue:
    """graph.queue — drives the C++ generic script_queue (Scene::script_queue)."""

    def __init__(self, bridge):
        self._bridge = bridge

    async def enqueue(self, node_id):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.wait_for_step()
        await self._bridge.enqueue(node_id)

    async def dequeue(self):
        await self._bridge.wait_for_step()
        result = await self._bridge.dequeue()
        return None if result is None or result < 0 else result

    def __len__(self):
        return self._bridge.queue_size()

    @property
    def empty(self):
        return len(self) == 0


class Bars:
    """
    bars.* — API surface for array/"Bar" (sorting) scenes.

    NOTE: there is no C++ BarScene in the codebase yet
    """

    def __init__(self, bridge):
        self._bridge = bridge

    async def swap(self, i, j):
        await self._bridge.wait_for_step()
        await self._bridge.swap_bars(i, j)

    async def compare(self, i, j):
        await self._bridge.wait_for_step()
        return await self._bridge.compare_bars(i, j)

    async def set_value(self, i, value):
        await self._bridge.wait_for_step()
        await self._bridge.set_bar(i, value)

    def __len__(self):
        return self._bridge.bar_count()


class Graph:
    """
    The object passed into `async def main(graph):`.

    graph.nodes       -> {id: Node}      (local mirror, refreshed on add/remove)
    graph.adjacency   -> {id: [id, ...]} (local mirror)
    graph.root_id     -> int
    graph.stack       -> Stack
    graph.queue       -> Queue
    """

    def __init__(self, bridge):
        self._bridge = bridge
        self.stack = Stack(self)
        self.queue = Queue(bridge)

        node_list = json.loads(bridge.initial_nodes_json())
        adj_list = json.loads(bridge.initial_adjacency_json())
        root = json.loads(bridge.initial_root_json())

        self.nodes = {n["node"]: Node(n["node"], n["data"]) for n in node_list}
        self.adjacency = {a["node"]: list(a["edges"]) for a in adj_list}
        self.root_id = root["node"]

    # --- read-only helpers ---------------------------------------------
    def neighbors(self, node_id):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        return list(self.adjacency.get(node_id, []))

    def root(self):
        return self.nodes[self.root_id]

    # --- mutating hidden-API calls --------------------------------------
    async def mark_visited(self, node_id, visited=True):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.wait_for_step()
        await self._bridge.mark_visited(node_id, visited)

    async def mark_discovered(self, node_id, discovered=True):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.wait_for_step()
        await self._bridge.mark_discovered(node_id, discovered)

    async def _set_current_no_wait(self, node_id):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.set_current(node_id)

    async def set_current(self, node_id):
        await self._bridge.wait_for_step()
        await self._set_current_no_wait(node_id)

    async def add_node(self, data=0):
        await self._bridge.wait_for_step()
        new_id = await self._bridge.add_node(data)
        node = Node(new_id, data)
        self.nodes[new_id] = node
        self.adjacency[new_id] = []
        return node

    async def add_edge(self, from_id, to_id):
        if hasattr(from_id, "id"):
            from_id = from_id.id
        if hasattr(to_id, "id"):
            to_id = to_id.id
        await self._bridge.wait_for_step()
        await self._bridge.add_edge(from_id, to_id)
        self.adjacency.setdefault(from_id, []).append(to_id)

    async def remove_node(self, node_id):
        if hasattr(node_id, "id"):
            node_id = node_id.id
        await self._bridge.wait_for_step()
        await self._bridge.remove_node(node_id)
        self.nodes.pop(node_id, None)
        self.adjacency.pop(node_id, None)

    async def remove_edge(self, from_id, to_id):
        if hasattr(from_id, "id"):
            from_id = from_id.id
        if hasattr(to_id, "id"):
            to_id = to_id.id
        await self._bridge.wait_for_step()
        await self._bridge.remove_edge(from_id, to_id)
        if to_id in self.adjacency.get(from_id, []):
            self.adjacency[from_id].remove(to_id)


def _make_graph(bridge):
    return Graph(bridge)


def _make_bars(bridge):
    return Bars(bridge)
