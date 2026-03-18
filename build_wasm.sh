#!/bin/bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

cd "$PROJECT_ROOT"

if ! command -v emcc >/dev/null 2>&1; then
    echo "emcc not found. Install and activate the Emscripten SDK before building." >&2
    exit 1
fi

if ! command -v node >/dev/null 2>&1; then
    echo "node not found. Emscripten uses node when generating TypeScript declarations." >&2
    exit 1
fi

OUT_DIR="$PROJECT_ROOT/webdemo"

mkdir -p "$OUT_DIR"

echo "Compiling eelib to WebAssembly ES module..."

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
    -sWASM=1 \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -sEXPORT_NAME=createEelib \
    -sALLOW_MEMORY_GROWTH=1 \
    -sENVIRONMENT=web \
    --emit-tsd "$OUT_DIR/eelib.d.ts" \
    -o "$OUT_DIR/eelib.mjs"

echo "Build complete!"
echo "Generated:"
echo "  $OUT_DIR/eelib.mjs"
echo "  $OUT_DIR/eelib.wasm"
echo "  $OUT_DIR/eelib.d.ts"