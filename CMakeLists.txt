cmake_minimum_required(VERSION 3.0)

project(riscv-uarch-attacks)

add_subdirectory(src)

find_package(Threads REQUIRED)

option(BUILD_TESTS "Build with cmocka tests" OFF)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if (DCMAKE_C_COMPILER EQUAL riscv64-unknown-linux-gnu-gcc)
    message("hi from riscv")
    find_program(RV64_COMPILER_PRESENT riscv64-unknown-linux-gnu-gcc)
    if (!RV64_COMPILER_PRESENT)
        message("RISC-V compiler riscv64-unknown-linux-gnu-gcc not found.")
    endif()
endif()

# find_package(Doxygen)
# if (DOXYGEN_FOUND)
#     set(doxyfile_in doc/doxyfile_in)
#     set(doxyfile doc/Doxyfile)
#     configure_file(${doxyfile} ${doxyfile_in} @ONLY)
#     add_custom_target(doc ALL
#         ${DOXYGEN_EXECUTABLE} ${doxyfile})
# endif()