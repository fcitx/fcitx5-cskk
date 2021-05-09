if (GOOGLETEST)
    message("Setting up test")
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest ${PROJECT_SOURCE_DIR}/third_party/googletest/build)
    enable_testing()
    add_executable(runTest ${PROJECT_SOURCE_DIR}/test/main.cpp ${PROJECT_SOURCE_DIR}/test/basic_test.cpp)

    target_include_directories(
            runTest PUBLIC
            ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include
    )
    target_link_libraries(
            runTest
            gtest
    )

    add_test(NAME runTest COMMAND runTest)
endif ()