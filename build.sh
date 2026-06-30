#!/bin/bash
# ============================================================================
# build.sh — Native build script
# Usage:
#   ./build.sh              # default build (Debug)
#   ./build.sh Release      # release build
#   ./build.sh clean        # clean build directory
# ============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${1:-Debug}"

if [[ "${BUILD_TYPE}" == "clean" ]]; then
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    exit 0
fi

mkdir -p "${BUILD_DIR}"

echo "=== Native Build (${BUILD_TYPE}) ==="
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DENABLE_TESTS=ON

cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo ""
echo "=== Build complete ==="
echo "Binary: ${BUILD_DIR}/demo_exec"
echo "Tests:  ${BUILD_DIR}/tests/"
