#!/usr/bin/env bash
set -e

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build"

echo "[INFO] Project root: $PROJECT_ROOT"
echo "[INFO] Build dir: $BUILD_DIR"

mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Debug

cmake --build "$BUILD_DIR" -j$(nproc)

echo "[INFO] Build finished."
