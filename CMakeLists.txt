cmake_minimum_required(VERSION 3.21)

project(LBM_HPC_Simulator LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ROCM_PATH "/opt/rocm")

set(HPC_OPTIMIZATION_FLAGS "-O3 -ffast-math")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HPC_OPTIMIZATION_FLAGS}")

add_compile_definitions(__HIP_PLATFORM_AMD__)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    "${ROCM_PATH}/include"
)

link_directories("${ROCM_PATH}/lib")

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
target_link_libraries(lbm_simulator_core PUBLIC amdhip64)
target_link_libraries(lbm_simulator PRIVATE lbm_simulator_core amdhip64)
target_compile_options(lbm_simulator PRIVATE -Wall -Wextra)
