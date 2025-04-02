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
include_directories(hatman/dependencies)

# Source
add_executable(
    main
    
    hatman/source/entity/base.cpp
    hatman/source/entity/player.cpp
    hatman/source/entity/type_m.cpp
    hatman/source/entity/type_s.cpp
    hatman/source/entity/unique_m.cpp
    hatman/source/entity/unique_s.cpp
    
    hatman/source/graphics/camera.cpp
    hatman/source/graphics/graphics.cpp
    hatman/source/graphics/gui.cpp
    
    hatman/source/modules/inventory.cpp
    hatman/source/modules/solid.cpp
    hatman/source/modules/sound.cpp
    hatman/source/modules/sprite.cpp
    hatman/source/modules/stats.cpp
    
    hatman/source/objects/item_base.cpp
    hatman/source/objects/item_unique.cpp
    hatman/source/objects/script.cpp
    hatman/source/objects/tile_base.cpp
    hatman/source/objects/tile_unique.cpp
    
    hatman/source/systems/audio.cpp
    hatman/source/systems/controls.cpp
    hatman/source/systems/emit.cpp
    hatman/source/systems/flags.cpp
    hatman/source/systems/game.cpp
    hatman/source/systems/input.cpp
    hatman/source/systems/level.cpp
    hatman/source/systems/saver.cpp
    hatman/source/systems/timer.cpp
    
    hatman/source/utility/geometry.cpp
    hatman/source/utility/launch_info.cpp
    hatman/source/utility/tags.cpp
    
    hatman/source/main.cpp
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