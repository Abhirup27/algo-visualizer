# Breadth First Search — uses graph.queue instead of graph.stack.
"""
graph.nodes       -> {id: Node}
graph.adjacency   -> {id: [id, ...]}
graph.root_id     -> int
graph.queue       -> queue
visited = set()
await dfs(graph, graph.root_id, visited)
"""


async def main(graph):
    visited = {graph.root_id}

    await graph.queue.enqueue(graph.root_id)
    await graph.mark_discovered(graph.root_id)

    while not graph.queue.empty:
        node_id = await graph.queue.dequeue()

        await graph.set_current(node_id)
        await graph.mark_visited(node_id)
        print(f"visit {node_id}")

        for neighbor_id in graph.neighbors(node_id):
            if neighbor_id not in visited:
                visited.add(neighbor_id)
                await graph.mark_discovered(neighbor_id)
                await graph.queue.enqueue(neighbor_id)
