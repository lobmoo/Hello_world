#!/bin/bash
# ============================================================================
# build-arm.sh — Cross-compilation build for ARM64 (aarch64)
#
# Usage:
#   ./build-arm.sh              # default build (Release)
#   ./build-arm.sh Debug        # debug build
#   ./build-arm.sh clean        # clean build directory
#
# Prerequisites:
#   sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
#
# Thirdparty libraries for ARM64:
#   Place ARM64 versions under thirdparty/<lib>/lib-aarch64/
#   Example:
#     thirdparty/jemalloc/lib-aarch64/libjemalloc.so
# ============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-arm"
BUILD_TYPE="${1:-Release}"
TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/aarch64-linux-gnu.cmake"

if [[ "${BUILD_TYPE}" == "clean" ]]; then
    echo "Cleaning ARM build directory..."
    rm -rf "${BUILD_DIR}"
    exit 0
fi

# ── Preflight checks ────────────────────────────────────────────────────────
if ! command -v aarch64-linux-gnu-g++ &>/dev/null; then
    echo "ERROR: aarch64-linux-gnu-g++ not found."
    echo "Install with: sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
    exit 1
fi

if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
    echo "ERROR: Toolchain file not found: ${TOOLCHAIN_FILE}"
    exit 1
fi

mkdir -p "${BUILD_DIR}"

echo "=== Cross-compiling for aarch64 (${BUILD_TYPE}) ==="
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DENABLE_TESTS=OFF

cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo ""
echo "=== ARM64 build complete ==="
echo "Binary: ${BUILD_DIR}/core/demo_exec"

# ── Show binary info ────────────────────────────────────────────────────────
if command -v file &>/dev/null; then
    echo ""
    file "${BUILD_DIR}/core/demo_exec"
fi
