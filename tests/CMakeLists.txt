cmake_minimum_required(VERSION 3.16)

find_package(GTest REQUIRED)
enable_testing()

# ── test_log ──────────────────────────────────────────────────────────────────
add_executable(test_log
    "${CMAKE_SOURCE_DIR}/common/log/logger_test.cc"
)
set_target_properties(test_log PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

set(COMMON_LIBS
    ${DDS_LIBS}
    log::log
    GTest::gtest
    GTest::gtest_main
    pthread
    rt
)

target_include_directories(test_log PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/nlohmann/include")
target_link_libraries(test_log PRIVATE ${COMMON_LIBS})
target_compile_options(test_log PRIVATE -O2)
add_test(NAME test_log COMMAND test_log)
set_tests_properties(test_log PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

# ── test_timer ────────────────────────────────────────────────────────────────
add_executable(test_timer
    "${CMAKE_SOURCE_DIR}/common/timer_wheel/timer_scheduler_test.cc"
)
set_target_properties(test_timer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)
target_include_directories(test_timer PRIVATE ${COMMON_INCLUDE})
target_link_libraries(test_timer PRIVATE ${COMMON_LIBS})
target_compile_options(test_timer PRIVATE -O2)
add_test(NAME test_timer COMMAND test_timer)
set_tests_properties(test_timer PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests")


