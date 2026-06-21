cmake_minimum_required(VERSION 3.21)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

if(WIN32)
    if(DEFINED ENV{HIP_PATH})
        file(TO_CMAKE_PATH "$ENV{HIP_PATH}" ROCM_PATH)
    else()
        set(ROCM_PATH "C:/Program Files/AMD/ROCm/7.1")
    endif()
    set(CMAKE_CXX_COMPILER "${ROCM_PATH}/bin/clang++.exe" CACHE FILEPATH "" FORCE)
    set(CMAKE_C_COMPILER   "${ROCM_PATH}/bin/clang.exe"   CACHE FILEPATH "" FORCE)
else()
    set(ROCM_PATH "/opt/rocm")
endif()

project(LBM_HPC_Simulator LANGUAGES C CXX)

set(HIP_TARGET_ARCH "gfx1200" CACHE STRING "GPU target arch (gfx1200 = RX 9060 XT)")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HPC_OPTIMIZATION_FLAGS "-O3 -ffast-math")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HPC_OPTIMIZATION_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HPC_OPTIMIZATION_FLAGS}")

add_compile_definitions(__HIP_PLATFORM_AMD__)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    "${ROCM_PATH}/include"
)
link_directories("${ROCM_PATH}/lib")

if(WIN32)
    if(DEFINED ENV{VCToolsInstallDir})
        file(TO_CMAKE_PATH "$ENV{VCToolsInstallDir}" VC_TOOLS_DIR)
        link_directories("${VC_TOOLS_DIR}/lib/x64")
    endif()
    if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
        file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" WIN_SDK_DIR)
        set(WIN_SDK_VER "$ENV{WindowsSDKVersion}")
        string(REGEX REPLACE "\\\\$" "" WIN_SDK_VER "${WIN_SDK_VER}")
        link_directories("${WIN_SDK_DIR}/Lib/${WIN_SDK_VER}/um/x64")
        link_directories("${WIN_SDK_DIR}/Lib/${WIN_SDK_VER}/ucrt/x64")
    endif()
endif()

set(HIP_COMMON_FLAGS "--offload-arch=${HIP_TARGET_ARCH}")
if(WIN32)
    set(HIP_COMMON_FLAGS "${HIP_COMMON_FLAGS} -Xclang -fno-cuda-host-device-constexpr")
endif()

set(CORE_SOURCES
    src/config.c
    src/memory.c
    src/io.c
    src/lbm_kernels.hip
)

set_source_files_properties(${CORE_SOURCES} PROPERTIES
    LANGUAGE CXX
    COMPILE_FLAGS "-x hip ${HIP_COMMON_FLAGS}"
)
set_source_files_properties(src/main.c PROPERTIES
    LANGUAGE CXX
    COMPILE_FLAGS "-x hip ${HIP_COMMON_FLAGS}"
)

if(WIN32)
    add_library(lbm_simulator_core STATIC ${CORE_SOURCES})
else()
    add_library(lbm_simulator_core SHARED ${CORE_SOURCES})
endif()

target_compile_options(lbm_simulator_core PRIVATE -Wall -Wextra)
target_link_options(lbm_simulator_core PRIVATE ${HIP_COMMON_FLAGS})

if(WIN32)
    target_link_libraries(lbm_simulator_core PUBLIC amdhip64)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    target_link_libraries(lbm_simulator_core PUBLIC amdhip64)
endif()

add_executable(lbm_simulator src/main.c)
target_link_libraries(lbm_simulator PRIVATE lbm_simulator_core)
target_compile_options(lbm_simulator PRIVATE -Wall -Wextra)
target_link_options(lbm_simulator PRIVATE ${HIP_COMMON_FLAGS})

if(WIN32)
    target_link_options(lbm_simulator PRIVATE
        "-Xlinker" "/subsystem:console"
    )
endif()
