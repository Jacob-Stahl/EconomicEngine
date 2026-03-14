#!/bin/bash

set -euo pipefail

if ! command -v emcc >/dev/null 2>&1; then
    echo "emcc not found. Install and activate the Emscripten SDK before building." >&2
    exit 1
fi

# Ensure output directory exists
mkdir -p webdemo

echo "Compiling eelib to WebAssembly..."

emcc -O3 \
    eelib/wasm_bindings.cpp \
    eelib/abm.cpp \
    eelib/agent.cpp \
    eelib/agent_manager.cpp \
    eelib/matcher.cpp \
    eelib/notifier.cpp \
    eelib/order.cpp \
    -I eelib \
    -std=c++17 \
    -lembind \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createEelib" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -o webdemo/eelib.js

echo "Build complete! Check webdemo/eelib.js and webdemo/eelib.wasm"
