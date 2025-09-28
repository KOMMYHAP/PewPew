add_library(pew_resources STATIC)

target_sources(pew_resources
        PRIVATE
        resource_pch.h
        resource_error.cpp
        resource_adapter.cpp
        resource_url.cpp
        resource_system.cpp
        resource_id.cpp
        resource_context.cpp
        PUBLIC
        resource_error.h
        resource_adapter.h
        resource_url.h
        resource_system.h
        resource_id.h
        resource_context.h
        resource_context.hpp
)
target_include_directories(pew_resources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pew_resources PRIVATE pew_utils)
target_precompile_headers(pew_resources PRIVATE resource_pch.h)
target_compile_features(pew_resources PRIVATE cxx_std_23)