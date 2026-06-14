cmake_minimum_required(VERSION 3.21)

project(LBM_HPC_Simulator LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ROCM_PATH "/mnt/c/Program Files/AMD/ROCm/7.1")

set(HPC_OPTIMIZATION_FLAGS "-O3 -ffast-math")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HPC_OPTIMIZATION_FLAGS}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    "${ROCM_PATH}/include"
)

set(CORE_SOURCES
    src/config.c
    src/memory.c
    src/io.c
    src/lbm_kernels.c
)

add_library(lbm_simulator_core STATIC ${CORE_SOURCES})
target_compile_options(lbm_simulator_core PRIVATE -Wall -Wextra)

add_executable(lbm_simulator src/main.c)
target_link_libraries(lbm_simulator PRIVATE lbm_simulator_core)
target_compile_options(lbm_simulator PRIVATE -Wall -Wextra)
