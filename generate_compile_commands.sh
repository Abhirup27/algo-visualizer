#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Generate compilation database with CMake
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build_wasm

# Link the compile_commands.json to the project root for better tool integration
if [ ! -f compile_commands.json ]; then
    ln -s build_wasm/compile_commands.json .
fi

echo "Compilation database generated successfully."
