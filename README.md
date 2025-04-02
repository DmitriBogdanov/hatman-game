# Hatman

A classic 2D metroidvania with trichromatic artstyle.  You, a player, is a wandering spirit exploring a dark limbo-like realm. Fight your way through the enemies, discover secrets scattered generously through every map, gather power and defeat the ultimate Big Baddie! The game is extremely lightweight and should have around a few hundred FPS on most machines.

## Executable, game assets, screenshots

Archived executable, game assets, screenshots and etc can be found at corresponding [itch.io page][itch_io_link].

[itch_io_link]: https://hatmangame.itch.io/hatman-adventure

## Dependencies

* SFML (graphics, input handling)
* SDL2 (audio)
* jakebesworth/Simple-SDL2-Audio (audio)
* nlohmann_json (json parsing)

## Version History

See changelog.md for detailed development history.

## Known bugs

- If player saves the game for the first time and goes back to the main menu, the "continue" button will be missing, it appears after a re-launch
- `temp` directory isn't created automatically if missing, which means player won't be able to save if he deletes it
- Music transition is a bit jarring, need to implement gradual transitions
- Do away with the console in debug mode, log things to a file

## License

This project is licensed under the MIT License - see the LICENSE.md file for details
