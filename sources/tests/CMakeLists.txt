project(pew_auto_tests)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(googletest)

add_executable(pew_auto_tests
        main.cpp
        resources.cpp
)
target_compile_features(pew_auto_tests PRIVATE cxx_std_23)
target_link_libraries(pew_auto_tests PRIVATE gtest_main pew_utils nlohmann_json::nlohmann_json)

enable_testing()
include(GoogleTest)
gtest_discover_tests(pew_auto_tests)
