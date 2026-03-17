include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(rvc_tests
    test_controller.cpp
)

target_link_libraries(rvc_tests
    PRIVATE rvc_lib
    PRIVATE GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(rvc_tests)