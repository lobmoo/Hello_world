cmake_minimum_required(VERSION 3.16)
project(DEMO LANGUAGES C CXX)

# ── Standard ─────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Detect cross-compilation ────────────────────────────────────────────────
# CMAKE_CROSSCOMPILING is automatically set to TRUE when a toolchain file
# specifies CMAKE_SYSTEM_NAME different from the host.
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "C Compiler  : ${CMAKE_C_COMPILER}")
    message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
else()
    message(STATUS "Native compiling on: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

# ── Architecture-dependent thirdparty paths ──────────────────────────────────
# Directory layout:
#   thirdparty/<lib>/lib/x86/   — x86_64 libraries
#   thirdparty/<lib>/lib/arm/   — ARM libraries
if(CMAKE_CROSSCOMPILING)
    set(ARCH_DIR "arm")
else()
    set(ARCH_DIR "x86")
endif()

# ── Subdirectories ───────────────────────────────────────────────────────────
add_subdirectory(common/log)
add_subdirectory(core)

option(ENABLE_TESTS "Enable unit tests" ON)
if(ENABLE_TESTS AND NOT CMAKE_CROSSCOMPILING)
    # Tests are only built for native builds (need to run on host)
    add_subdirectory(tests)
elseif(ENABLE_TESTS AND CMAKE_CROSSCOMPILING)
    message(STATUS "Tests disabled for cross-compilation build")
endif()

option(SAMPLE "Build example" OFF)
if(SAMPLE)
    add_subdirectory(sample/)
endif()

# ── Install ──────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config/
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
        OPTIONAL)
