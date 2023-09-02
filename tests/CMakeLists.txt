function(MakeTest folder test_case)
    Log("Add test: ${test_case}")

    get_filename_component(TEST_PATH "./${folder}" ABSOLUTE)
    file(GLOB_RECURSE TEST_CXX_SOURCES ${TEST_PATH}/*.cpp)

    add_executable(${test_case} ${TEST_CXX_SOURCES})
    target_link_libraries(${test_case} harmony gtest_main)
    add_test(NAME ${test_case}_test COMMAND ${test_case})
endfunction()

Log("Configuring tests")

MakeTest(threads test_threads)
MakeTest(support test_support)
MakeTest(executors test_executors)
MakeTest(coro test_coro)
MakeTest(. test_all)
