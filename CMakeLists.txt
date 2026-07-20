cmake_minimum_required(VERSION 3.28)
project(TohruPhysics VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# KleaSCM: no exceptions, no RTTI
add_compile_options(-fno-exceptions -fno-rtti)
add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Werror -Wno-unused-parameter)

# Align to 64-byte cache lines for SIMD
add_compile_options(-march=native -mtune=native)

# Use MARCH_NATIVE to toggle -march=native for cross-builds
option(TP_USE_MARCH_NATIVE "Enable -march=native" ON)
if(NOT TP_USE_MARCH_NATIVE)
	string(REPLACE "-march=native" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REPLACE "-mtune=native" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

option(TP_BUILD_TESTS "Build unit tests" ON)
option(TP_BUILD_DEMOS "Build demos" ON)

add_subdirectory(Source)

if(TP_BUILD_TESTS)
	add_subdirectory(Test)
endif()

if(TP_BUILD_DEMOS AND EXISTS "${CMAKE_SOURCE_DIR}/Demo/CMakeLists.txt")
	add_subdirectory(Demo)
endif()
