# Depth First Search (advanced) — iterative version using an explicit stack,
# with an ENTER/EXIT phase per node so you can see backtracking distinctly
# from first-visit.

ENTER, EXIT = "enter", "exit"


async def main(graph):
    visited = set()
    # Each stack frame is (node_id, phase). We store just the node id in the
    # visual stack (graph.stack.push takes a node id), and track phase
    # locally — the visualization doesn't need to know about phases to be
    # useful, but you could extend the hidden API with a `data=` payload if
    # you want phase-aware coloring too.
    work = [(graph.root_id, ENTER)]

    while work:
        node_id, phase = work.pop()

        if phase == ENTER:
            if node_id in visited:
                continue
            visited.add(node_id)

            await graph.stack.push(node_id)
            await graph.set_current(node_id)
            await graph.mark_visited(node_id)
            print(f"enter {node_id}")

            work.append((node_id, EXIT))
            for neighbor_id in reversed(graph.neighbors(node_id)):
                if neighbor_id not in visited:
                    await graph.mark_discovered(neighbor_id)
                    work.append((neighbor_id, ENTER))
        else:
            await graph.stack.pop()
            print(f"exit {node_id}")
