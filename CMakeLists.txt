cmake_minimum_required(VERSION 3.11)

project(
    Hatman
    VERSION 2.0.1
    DESCRIPTION "2D metroidvania with trichromatic artstyle.."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/hatman-game"
)

# Fetch Dependencies
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML.git"
    GIT_TAG 2.5.1
)
FetchContent_MakeAvailable(sfml)

# Include
include_directories(hatman/include)
include_directories(hatman/firstparty)

# Source
add_executable(
    main 
    hatman/source/main.cpp
    hatman/source/audio.cpp
    hatman/source/background.cpp
    hatman/source/camera.cpp
    hatman/source/CMakeLists.txt
    hatman/source/controls.cpp
    hatman/source/emit.cpp
    hatman/source/entity_base.cpp
    hatman/source/entity_type_m.cpp
    hatman/source/entity_type_s.cpp
    hatman/source/entity_unique_m.cpp
    hatman/source/entity_unique_s.cpp
    hatman/source/flags.cpp
    hatman/source/game.cpp
    hatman/source/geometry_utils.cpp
    hatman/source/graphics.cpp
    hatman/source/gui.cpp
    hatman/source/hide_console.cpp
    hatman/source/input.cpp
    hatman/source/inventory.cpp
    hatman/source/item_base.cpp
    hatman/source/item_unique.cpp
    hatman/source/launch_info.cpp
    hatman/source/level.cpp
    hatman/source/main.cpp
    hatman/source/player.cpp
    hatman/source/saver.cpp
    hatman/source/script.cpp
    hatman/source/solid.cpp
    hatman/source/sound.cpp
    hatman/source/sprite.cpp
    hatman/source/stats.cpp
    hatman/source/tags.cpp
    hatman/source/tile_base.cpp
    hatman/source/tile_unique.cpp
    hatman/source/timer.cpp
)

target_compile_features(main PRIVATE cxx_std_17)

target_compile_options(main PRIVATE
    -O2
    -g3
    -Wall -Wextra -Wpedantic
)
target_link_libraries(main PRIVATE sfml-system sfml-window sfml-graphics sfml-audio -fsanitize=undefined,address,leak)
target_include_directories(main PRIVATE hatman/include)
#target_link_directories(main PRIVATE hatman/source)
#target_link_options(main PRIVATE -fsanitize=undefined,address,leak)