add_library(pew_utils STATIC)

target_sources(pew_utils
        PRIVATE
        sample_counter.cpp
        PUBLIC
        sample_counter.h
        std_headers.h
)
target_include_directories(pew_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(pew_utils PRIVATE std_headers.h)
target_compile_features(pew_utils PRIVATE cxx_std_23)