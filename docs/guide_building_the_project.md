# Building the project

[<- to README.md](..)

This project uses [CMake](https://cmake.org) build system with [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) as a main way of managing platform-dependent configuration. By default, the repo provides following presets:
```
gcc
gcc-debug
clang
clang-debug
msvc
msvc-debug
```

Dependencies that aren't embedded in the repo are automatically fetched by CMake during the configuration step.

## Building with CMake

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/hatman-game.git &&
cd "hatman-game/"
```

Configure **CMake**:

```bash
cmake --preset gcc
```

Build the project:

```bash
cmake --build --preset gcc
```

Launch the game:

```bash
./build/main
```

## Building with a script

To reduce the tedium of entering verbose commands during development, this repo provides [`actions.sh`](./../actions.sh) script, containing shortcuts for all the actions above set up for `gcc-debug` preset.

For example, we can clear previous build (if present), configure, build and run with a single command:

```bash
bash actions.sh clear config build run
```