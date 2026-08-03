# Depth First Search (basic)
#
# `graph` is provided by the AlgoPlex runtime — you don't create it yourself

"""
graph.nodes       -> {id: Node}
graph.adjacency   -> {id: [id, ...]}
graph.root_id     -> int
graph.stack       -> Stack
visited = set()
await dfs(graph, graph.root_id, visited)
"""


async def main(graph):
    visited = set()
    await dfs(graph, graph.root_id, visited)


async def dfs(graph, node_id, visited):
    if node_id in visited:
        return
    visited.add(node_id)

    await graph.stack.push(node_id)
    await graph.set_current(node_id)
    await graph.mark_visited(node_id)

    print(f"visit {node_id}")

    for neighbor_id in graph.neighbors(node_id):
        if neighbor_id not in visited:
            await dfs(graph, neighbor_id, visited)

    await graph.stack.pop()
