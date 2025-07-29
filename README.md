[<img src ="docs/images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="docs/images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="docs/images/icon_itch_io.svg">](https://hatmangame.itch.io/hatman-adventure)

# Hatman

A classic 2D metroidvania with trichromatic artstyle.  You, a player, is a wandering spirit exploring a dark limbo-like realm. Fight your way through the enemies, discover secrets scattered generously through every map, gather power and defeat the ultimate Big Baddie! The game is extremely lightweight and should have FPS in hundreds/thousands on most machines.

<img src ="docs/images/promo_enemies_large.png" width="800">
<img src ="docs/images/promo_items_large.png" width="800">
<img src ="docs/images/promo_boss_large.png" width="800">

## Screenshots

TODO:

## Executable

Pre-built binaries are available at the corresponding [itch.io page](https://hatmangame.itch.io/hatman-adventure) for:
- Windows x64

## Dependencies

| Dependency | License | Used for                        | Type        | Build process    |
| ---------- | ------- | ------------------------------- | ----------- | ---------------- |
| SFML       |         | Graphics, audio, input handling | Third-party | Fetched by CMake |
| UTL        | MIT     | JSON                            | First-party | Fetched by CMake |

## Changelog

See [`changelog.md`](./changelog.md) for a detailed development history.

## Some history

This game was my first truly large personal project. The codebase underwent several style changes, switched multiple dependencies (SDL, tinyxml -> SDL, simple_audio, nlohmann_json, SFML -> SFML, UTL)

## Known bugs

- If player saves the game for the first time and goes back to the main menu, the "continue" button will be missing, it appears after a re-launch
- `temp` directory isn't created automatically if missing, which means player won't be able to save if he deletes it
- Do away with the console in debug mode, log things to a file

## License

This project is licensed under the MIT License - see the LICENSE.md file for details
