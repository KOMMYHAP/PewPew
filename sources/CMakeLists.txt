target_sources(PewPew PRIVATE
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
add_subdirectory(resources)
add_subdirectory(tests)