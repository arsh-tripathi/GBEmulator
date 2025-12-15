find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES *.cpp)

add_executable(GBEmulator_tests
    ${TEST_SOURCES}
)

target_link_libraries(GBEmulator_tests
    PRIVATE
        GBCore
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(GBEmulator_tests)
