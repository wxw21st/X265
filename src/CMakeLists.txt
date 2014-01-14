if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release."
        FORCE)
endif()

project (x265-Milestone)
cmake_minimum_required (VERSION 2.8)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake" "${CMAKE_MODULE_PATH}")

if ("${CMAKE_SIZEOF_VOID_P}" MATCHES 8)
    set(X64 1)
    add_definitions(-DX86_64)
endif()

# Enforce coding standards.  Full warnings and warnings as errors
if(MSVC)
    add_definitions(/W4 /D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/Ob2) # always inline
    add_definitions(/Oi)  # enable intrinsics
    add_definitions(/MP)  # multithreaded build
    add_definitions(/openmp)        # OpenMP
    add_definitions(/arch:SSE2)     # Enabled SSE2
endif(MSVC)
include_directories(../inc)

if("$ENV{CXX}" STREQUAL "icpc")
    set(GCC 1)
    add_definitions(-Wall -Wextra -Wunused-variable -Wunused-function -Wshadow -no-vec -msse4)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(GCC 1)
    add_definitions(-Wall -Wextra -Wunused-variable -Wunused-function -Wshadow -msse4)
endif()

option(CHECK_SEI "Generate MD5 checksum" ON)
if(CHECK_SEI)
    add_definitions(-DCHECK_SEI=1)
else(CHECK_SEI)
    add_definitions(-UCHECK_SEI)
endif(CHECK_SEI)

option(DEBUG_VIS "Visual debug support" ON)
if(DEBUG_VIS)
    add_definitions(-DDEBUG_VIS=1)
else(DEBUG_VIS)
    add_definitions(-UDEBUG_VIS)
endif(DEBUG_VIS)

option(USE_INSTRINSIC_ASM "Use of SIMD optimize" ON)
if(USE_INSTRINSIC_ASM)
    add_definitions(-DUSE_ASM=ASM_SSE4)
else(USE_INSTRINSIC_ASM)
    add_definitions(-DUSE_ASM=ASM_NONE)
endif(USE_INSTRINSIC_ASM)

find_package(Yasm)
if(YASM_FOUND)
    if (YASM_VERSION_STRING VERSION_LESS "1.2.0")
        message(STATUS "Yasm version ${YASM_VERSION_STRING} is too old. 1.2.0 or later required")
    else()
        message(STATUS "Found Yasm ${YASM_VERSION_STRING} to build assembly primitives")
        option(ENABLE_ASM "Enable use of assembly coded primitives" ON)
    endif()
endif(YASM_FOUND)


if(ENABLE_ASM)
    add_subdirectory(x86)
    target_link_libraries(ASM_LIB)
endif(ENABLE_ASM)

if(UNIX)
    SET(PLATFORM_LIBS pthread rt m)
endif(UNIX)

#include_directories(. Lib encoder)

# Main CLI application
if(MSVC)
    add_definitions(/wd4244) # type conversion, possible loss of data
    add_definitions(/wd4100) # unreferenced formal parameter
    add_definitions(/wd4505) # unreferenced local function has been removed
    #add_definitions(/wd4018) # signed/unsigned mismatch
    add_definitions(/wd4189) # local variable is initialized but not referenced
    
endif(MSVC)
if(GCC)
    add_definitions(-Wno-sign-compare)
    add_definitions(-Wno-unused-parameter)
endif(GCC)
#set_source_files_properties(x265n.cpp PROPERTIES COMPILE_FLAGS -DX265_VERSION=${X265_VERSION})


file(GLOB APPSRC *.cpp)
file(GLOB APPINC ../inc/*.h)

source_group(INC  FILES ${APPINC})
source_group(SRC  FILES ${APPSRC})

add_executable(x265-Milestone
    x265.cpp ${EXTRAS} ${APPSRC} ${APPINC}
)

if(ENABLE_ASM)
    target_link_libraries(x265-Milestone ASM_LIB ${PLATFORM_LIBS})
else(ENABLE_ASM)
    target_link_libraries(x265-Milestone ${PLATFORM_LIBS})
endif(ENABLE_ASM)

