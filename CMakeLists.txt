cmake_minimum_required(VERSION 3.0)

project(riscv-uarch-attacks)

add_subdirectory(src)

option(BUILD_TESTS "Build with cmocka tests" OFF)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()