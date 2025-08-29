cmake_minimum_required(VERSION 3.30)
project(PewPew LANGUAGES CXX)

include(FetchContent)
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


# Fetch SFML
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)


# Fetch Entt
FetchContent_Declare(Entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.15.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(Entt)


# Fetch GSL
FetchContent_Declare(GSL
        GIT_REPOSITORY https://github.com/microsoft/GSL.git
        GIT_TAG v4.2.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(GSL)


# Fetch Box2d
FetchContent_Declare(box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v3.1.1
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(box2d)


# Fetch ImGui
FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.1
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_USE_DEFAULT_CONFIG OFF)
set(IMGUI_SFML_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources/debug_ui/core)
set(IMGUI_SFML_CONFIG_NAME imconfig_pewpew.h)
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)

# Fetch ImGui-SFML
FetchContent_Declare(imgui_sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG v3.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(imgui_sfml)


# Setup project's target
set(PROJECT_TARGET main)
add_executable(${PROJECT_TARGET})
add_subdirectory(sources)
target_compile_features(${PROJECT_TARGET} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_TARGET} PRIVATE EnTT::EnTT GSL box2d ImGui-SFML::ImGui-SFML)
set_target_properties(${PROJECT_TARGET} PROPERTIES UNITY_BUILD OFF)
target_precompile_headers(${PROJECT_TARGET} PRIVATE sources/pch.h)
target_include_directories(${PROJECT_TARGET} PRIVATE sources)


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