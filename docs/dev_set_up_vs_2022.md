# Setting up Visual Studio 2022 & building from scratch

[<- to README.md](..)

## Set up Visual Studio

Download `Visual Studio 2022` from the [official website](https://visualstudio.microsoft.com/downloads/)

In the installer, select the classic tooling for C++ development, this should include CMake tools by default.

## Set up CMake presets-based project

Open VS2022, clone the repository or open project from directory (if already cloned).

Visual Studio will configure the project automatically. To do so manually select `Project` -> `Configure Hatman` in the top bar.

In the top bar select `msvc` preset and `bin/hatman.exe` target.

## (optional) Download dependencies

**Windows (64-bit):** This step can be skipped. [SFML 2.5.1](https://www.sfml-dev.org/download/sfml/2.5.1) `dll` dependencies are embedded in the repo and will be automatically bundled with executable upon build.

**Linux (64-bit):** On Ubuntu and other Debian-based distributions, SFML dependencies can be installed with the following commands:

```sh
sudo apt update

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

## Build the project

Press `Start with/without debugging` to build and run the project.

The executable bundled with all its asset & binary dependencies will reside in `build/bin/`.

## (optional) Set up integrated bash terminal

Bash isn't necessary for building the project, but it is convenient to have for developent.

Download `Git Bash` from the [official website](https://git-scm.com/downloads). Install to any directory `<git_bash_path>`.

Press `Ctrl` + `` ` `` to bring up integrated terminal.

Press the settings button `⚙`, in `Environment` -> `Terminal` -> `Add` add `<git_bash_path>/bin/sh.exe`.

> [!Note]
> Adding `<git_bash_path>/git-bash.exe` instead will cause integrated terminal to spawn a new window each time. This is usually undesirable.

Bash can now be selected as an integrated terminal option.