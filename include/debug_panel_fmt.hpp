/*
 * This header is used to declare a contract between the typescript/ javascript
 * code and the C++ code. It acts as a single source of truth for the info panel
 * formats like the stack panel.
 *
 * */

// @stack             DFS                 node:int
#define STACK_FORMAT_DFS "{{\"node\":\"{}\"}}"

// @AdjacencyMatrix   DFS_A               node:int        edges:int[]
#define ADJ_MAT_FMT_DFS_A "{{\"node\":{},\"edges\":[{}]}}"

//@NodeList           Graph               node:int        data:int
#define GRAPH_NODE_LIST_FMT "{{\"node\":{},\"data\":{}}}"

//@Node               Graph               node:int        data:int
#define GRAPH_NODE_FMT "{{\"node\":{},\"data\":{}}}"

#define NODE_EDIT_EVENT_FMT                                                    \
  "{{\"node\":{},\"data\":{\"pos\":{\"x\":{}, \"y\":{}},\"current_val\":{}}}}"

// @ScriptStack   Generic   node:int
#define SCRIPT_STACK_FMT "{{\"node\":{}}}"
// @ScriptQueue   Generic   node:int
#define SCRIPT_QUEUE_FMT "{{\"node\":{}}}"
// @Bars                Sort                       index:int   value:int
#define BAR_FMT "{{\"index\":{},\"value\":{}}}"

// Indirection macro so you can pass an algo name token
#define GET_STACK_FMT(algo) STACK_FORMAT_##algo
#define GET_QUEUE_FMT(algo) QUEUE_FORMAT_##algo
#define GET_ADJ_MAT_FMT(algo) ADJ_MAT_FMT_##algo
