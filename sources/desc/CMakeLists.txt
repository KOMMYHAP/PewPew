add_library(pew_desc STATIC)

target_sources(pew_desc
        PRIVATE
            desc_pch.h
            desc_parser.cpp
            desc_registry.cpp
        PUBLIC
            desc.h
            desc_parser.h
            desc_parser.hpp
            desc_registry.h
            desc_builder.h
            desc_builder.hpp
)
target_include_directories(pew_desc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pew_desc PRIVATE pew_utils)
target_link_libraries(pew_desc PUBLIC nlohmann_json::nlohmann_json)
target_precompile_headers(pew_desc PRIVATE desc_pch.h)
target_compile_features(pew_desc PRIVATE cxx_std_23)