# AlgoPlex

**Making complex algorithms simple.**

AlgoPlex is an interactive algorithm and data structure visualization platform designed to make complex concepts easier to understand through visualization and exploration.

Built with **C++**, **WebAssembly (WASM)**, **Raylib**, **React**, and **TypeScript**, AlgoPlex combines the performance of native rendering with the flexibility of modern web technologies.

The visualization engine runs in C++ and is compiled to WebAssembly, while React and TypeScript power the user interface, providing a responsive and interactive learning experience.

## Demo

[![AlgoPlex Demo](https://img.youtube.com/vi/TKAdQdjWHqI/maxresdefault.jpg)](https://youtu.be/TKAdQdjWHqI)

---

## Features

- [x] Visualize graph and tree data structures
- [x] Create, edit, and remove nodes directly on the canvas
- [x] Create, edit, and remove edges between nodes
- [x] Step through algorithm execution
- [x] Interactive canvas navigation and manipulation
- [x] Runtime algorithm editing and debugging
- [ ] Code block highlighting and execution tracking
- [ ] Additional visualizations and learning tools

---

## Supported Algorithms

### Graph Algorithms

- [x] Depth First Search (DFS)
- [x] Breadth First Search (BFS)
- [ ] Traveling Salesman Problem (TSP)
- [ ] More coming soon

---

## Technology Stack

### Core Engine

- C++
- Raylib
- WebAssembly (Emscripten)

### Frontend

- React
- TypeScript
- Vite

---

## Building

### Prerequisites

- Node.js
- npm
- Emscripten SDK
- CMake

Ensure your Emscripten environment variables are configured correctly or sourced from your shell setup before building.

### Development Build

```bash
npm install

emcmake cmake -B build_wasm \
  -DCMAKE_BUILD_TYPE=Release \
  -DOPENGL_VERSION="ES 3.0"

cmake --build build_wasm -j$(nproc)

npm run dev
```

---

## Project Goals

AlgoPlex aims to go beyond static visualizations by exposing the internal state of algorithms as they execute.

Future plans include:

- Algorithm state inspection
- Call stack visualization
- Data structure internals
- Pseudocode synchronization
- Runtime code execution
- Additional computer science visualizations

---

## Production Builds

To create a production-ready build, enable the `PRODUCTION_BUILD` option in `CMakeLists.txt`.

Production builds:

- Use deployment-friendly resource paths
- Disable debugging facilities
- Optimize build settings for distribution
- Remove development-specific behavior

After changing this flag, it is recommended to perform a clean build by deleting the build directory and rebuilding the project from scratch.

