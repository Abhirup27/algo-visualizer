# Bubble Sort — written against the planned `bars` hidden API.
#
# NOTE: this will run in Pyodide but won't draw anything yet. There is no
# C++ BarScene in this codebase yet (only the Graph Scene used by
# dfs/dfs_a/bfs). See public/pyapi/algoplex_api.py -> class Bars for what
# needs a matching Scene on the C++ side (swap_bars/compare_bars/set_bar),
# built the same way scriptStackPush/scriptStackPop were built for graphs.


async def main(bars):
    n = len(bars)
    for i in range(n):
        for j in range(n - i - 1):
            if await bars.compare(j, j + 1) > 0:
                await bars.swap(j, j + 1)
