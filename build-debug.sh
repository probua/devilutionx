#!/usr/bin/env bash
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="build-debug"
JOBS=$(nproc)

cd "$ROOT_DIR"

echo -e "${GREEN}[BUILD]${NC} Configuring Debug build..."
cmake -S. -B"$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF

echo -e "${GREEN}[BUILD]${NC} Compiling ($JOBS jobs)..."
cmake --build "$BUILD_DIR" -j"$JOBS"

echo ""
echo -e "${GREEN}[BUILD]${NC} Done. Binary: $BUILD_DIR/devilutionx"
