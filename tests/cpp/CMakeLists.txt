include(FetchContent)

FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz
)
FetchContent_MakeAvailable(Catch2)

file(GLOB TEST_SOURCES "test_*.cpp")

add_executable(gingo_tests ${TEST_SOURCES})

target_link_libraries(gingo_tests
    PRIVATE gingo_core Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(gingo_tests)
