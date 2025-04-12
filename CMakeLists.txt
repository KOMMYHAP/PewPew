cmake_minimum_required(VERSION 3.30)
project(PewPew LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Setup unity build.
# Enable global option to build SFML with unity build.
# Assumed unity build will be disabled for project's target.
set(CMAKE_UNITY_BUILD ON)


# Setup Address Saninizer
# Set global compile options to build SFML with enabled asan and link it statically
option(ENABLE_ASAN "Build project with AddressSanitizer" OFF)
if (${ENABLE_ASAN})
    if (MSVC)
        add_compile_options(/fsanitize=address /MD)
    else ()
        message(WARNING "Current platform does not supported for asan build")
    endif ()
endif ()


# Setup SFML
include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)


# Setup project's target
set(PROJECT_TARGET main)
add_executable(${PROJECT_TARGET})
add_subdirectory(sources)
target_compile_features(${PROJECT_TARGET} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_TARGET} PRIVATE SFML::Graphics)
set_target_properties(${PROJECT_TARGET} PROPERTIES UNITY_BUILD OFF)


# Copy dll to project's binary
if (${ENABLE_ASAN})
    find_library(ASAN_LIBRARY
            NAMES clang_rt.asan_dynamic-x86_64.lib
            DOC "Path to ASan runtime library"
            REQUIRED
    )
    get_filename_component(ASAN_RUNTIME_DIRECTORY ${ASAN_LIBRARY} DIRECTORY CACHE)
    set(ASAN_RUNTIME_LIBRARY "${ASAN_RUNTIME_DIRECTORY}/../../bin/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll")

    add_custom_command(TARGET ${PROJECT_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -E copy
            -t $<TARGET_FILE_DIR:${PROJECT_TARGET}>
            $<TARGET_RUNTIME_DLLS:${PROJECT_TARGET}>
            $<$<BOOL:${ENABLE_ASAN}>:${ASAN_RUNTIME_LIBRARY}>
            COMMAND_EXPAND_LISTS
    )
endif ()