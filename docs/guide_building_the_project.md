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

**(Linux only)** Install dependencies for **SFML**:
```
sudo apt update &&
sudo apt install     \
    libxrandr-dev    \
    libxcursor-dev   \
    libxi-dev        \
    libudev-dev      \
    libfreetype-dev  \
    libflac-dev      \
    libvorbis-dev    \
    libgl1-mesa-dev  \
    libegl1-mesa-dev \
    libfreetype-dev
```

Build the project:

```bash
cmake --build --preset gcc
```

Launch the game:

```bash
./build/bin/hatman
```

> [!Note]
> [Visual Studio](https://visualstudio.microsoft.com/downloads/) also handles preset-based projects quite gracefully, the only difference is that instead of running commands manually the steps will be done through a GUI. See a [step-by-step guide](./guide_set_up_vs_2022.md) on how to set up VS2022 and build the project from scratch.

## Building with a script

To reduce the tedium of entering verbose commands during development, this repo provides [`actions.sh`](./../actions.sh) script, containing shortcuts for all the actions above set up for `gcc-debug` preset.

For example, we can clear previous build (if present), configure, build and run with a single command:

```bash
bash actions.sh clear config build run
```