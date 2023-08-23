# locate folder
get_filename_component(HARMONY_INCLUDE_PATH ".." ABSOLUTE)

# find all source files
file(GLOB_RECURSE HARMONY_SRCS "*.cpp")

# add library
add_library(harmony STATIC ${HARMONY_SRCS})

target_include_directories(harmony PUBLIC ${HARMONY_INCLUDE_PATH})
target_link_libraries(harmony PUBLIC pthread)
