target_sources(${PROJECT_TARGET} PRIVATE
        main.cpp
        game_renderer.cpp
        game_renderer.h
        game_camera.cpp
        game_camera.h
        game_services.h
        game_services.cpp
        physics_world.h
        physics_world.cpp
        game_statistics.cpp
        game_statistics.h
)

add_subdirectory(components)
add_subdirectory(debug_ui)
add_subdirectory(utils)
add_subdirectory(tests)