cmake_minimum_required(VERSION 3.15)
project(gingo VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# C++ core library (static)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE LIB_SOURCES "cpp/src/*.cpp")

add_library(gingo_core STATIC ${LIB_SOURCES})

target_include_directories(gingo_core
    PUBLIC  cpp/include
)

target_compile_features(gingo_core PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# pybind11 Python extension module
# ---------------------------------------------------------------------------
find_package(pybind11 CONFIG QUIET)

if(pybind11_FOUND)
    pybind11_add_module(_gingo bindings/pybind_module.cpp)
    target_link_libraries(_gingo PRIVATE gingo_core)
    target_include_directories(_gingo PRIVATE cpp/include)
    install(TARGETS _gingo DESTINATION gingo)
else()
    message(STATUS "pybind11 not found — Python bindings will not be built.")
endif()

# ---------------------------------------------------------------------------
# C++ tests (optional)
# ---------------------------------------------------------------------------
option(GINGO_BUILD_TESTS "Build C++ tests" OFF)

if(GINGO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/cpp)
endif()
