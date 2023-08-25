function(MakeTest folder)
    Log("Add test: ${folder}")

    get_filename_component(TEST_PATH "./${folder}" ABSOLUTE)
    file(GLOB_RECURSE TEST_CXX_SOURCES ${TEST_PATH}/*.cpp)

    add_executable(${folder} ${TEST_CXX_SOURCES})
    target_link_libraries(${folder} harmony gtest_main)
    add_test(NAME ${folder}_test COMMAND ${folder})
endfunction()

Log("Configuring tests")

MakeTest(threads)
