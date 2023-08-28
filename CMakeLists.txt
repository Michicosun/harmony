cmake_minimum_required(VERSION 3.16)

project(harmony)

option(BUILD_TESTS "Build Harmony tests" OFF)

include(cmake/Logging.cmake)
include(cmake/CCache.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CheckPlatform.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/Development.cmake)
include(cmake/Sanitize.cmake)

add_subdirectory(third_party)

# This must be included after library so that
# third party code is not affected
include(cmake/Werror.cmake)

add_subdirectory(harmony)

if(BUILD_TESTS)
    add_subdirectory(play)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()

