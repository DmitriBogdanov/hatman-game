cmake_minimum_required(VERSION 3.21.0...3.22.1)

project(
    Hatman
    VERSION 3.4.1
    DESCRIPTION "2D metroidvania with trichromatic artstyle."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/hatman-game"
)

# Fetch SFML
include(FetchContent)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML.git"
    GIT_TAG 2.5.1
    GIT_SHALLOW 1
)
FetchContent_MakeAvailable(sfml)

# Add platform-specific resources
#
# There is essentially no cross-platform way to set an app icon. Windows handles
# it throught the use of '.rc' resource files passed to the compiler. By default
# (aka without additional configuration) only MSVC knows how to handle '.rc'.
set(hatman_platform_resources)
if(MSVC)
    list(APPEND hatman_platform_resources "hatman/resources/windows/icon.rc")
endif()

# Create game target
add_executable(
    hatman
    
    "hatman/source/entity/base.cpp"
    "hatman/source/entity/player.cpp"
    "hatman/source/entity/type_m.cpp"
    "hatman/source/entity/type_s.cpp"
    "hatman/source/entity/unique_m.cpp"
    "hatman/source/entity/unique_s.cpp"
    
    "hatman/source/graphics/camera.cpp"
    "hatman/source/graphics/graphics.cpp"
    "hatman/source/graphics/gui.cpp"
    
    "hatman/source/modules/inventory.cpp"
    "hatman/source/modules/solid.cpp"
    "hatman/source/modules/sound.cpp"
    "hatman/source/modules/sprite.cpp"
    "hatman/source/modules/stats.cpp"
    
    "hatman/source/objects/item_base.cpp"
    "hatman/source/objects/item_unique.cpp"
    "hatman/source/objects/script.cpp"
    "hatman/source/objects/tile_base.cpp"
    "hatman/source/objects/tile_unique.cpp"
    
    "hatman/source/systems/audio.cpp"
    "hatman/source/systems/controls.cpp"
    "hatman/source/systems/emit.cpp"
    "hatman/source/systems/flags.cpp"
    "hatman/source/systems/game.cpp"
    "hatman/source/systems/input.cpp"
    "hatman/source/systems/level.cpp"
    "hatman/source/systems/saver.cpp"
    "hatman/source/systems/timer.cpp"
    
    "hatman/source/utility/geometry.cpp"
    "hatman/source/utility/launch_info.cpp"
    "hatman/source/utility/tags.cpp"
    
    "hatman/source/main.cpp"

	${hatman_platform_resources}
)

# Build
target_include_directories(hatman PRIVATE "hatman/include"                                )
target_include_directories(hatman PRIVATE "hatman/dependencies"                           )
target_compile_features   (hatman PRIVATE ${HATMAN_COMPILE_FEATURES}                      )
target_compile_options    (hatman PRIVATE ${HATMAN_COMPILER_FLAGS}                        )
target_link_options       (hatman PRIVATE ${HATMAN_LINKER_FLAGS}                          )
target_link_libraries     (hatman PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)

set_target_properties(hatman PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Copy asset dependencies
add_custom_target(
    hatman_copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_LIST_DIR}/dependencies/content
            $<TARGET_PROPERTY:hatman,RUNTIME_OUTPUT_DIRECTORY>
)
add_dependencies(hatman hatman_copy_assets) # the command will run after every build

# Copy shared library dependencies
if(MSVC OR MSYS OR MINGW)
    add_custom_target(
        hatman_copy_dlls
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${CMAKE_CURRENT_LIST_DIR}/dependencies/win64
                $<TARGET_PROPERTY:hatman,RUNTIME_OUTPUT_DIRECTORY>
    )
    add_dependencies(hatman hatman_copy_dlls)
endif()