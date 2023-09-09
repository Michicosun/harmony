cmake_minimum_required(VERSION 3.16)

project(harmony)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build Harmony tests" OFF)
option(BUILD_EXAMPLES "Build Harmony examples" OFF)
option(BUILD_PLAYGROUND "Build Harmony playground" OFF)

include(cmake/Logging.cmake)
include(cmake/CCache.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CheckPlatform.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/Sanitize.cmake)

add_subdirectory(third_party)

add_compile_options(-Werror)
add_subdirectory(harmony)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_PLAYGROUND)
    add_subdirectory(play)
endif()
