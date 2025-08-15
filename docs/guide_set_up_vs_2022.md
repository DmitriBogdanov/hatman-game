# Setting up Visual Studio 2022 & building from scratch

[<- to README.md](..)

## Set up Visual Studio

Download `Visual Studio 2022` from the [official website](https://visualstudio.microsoft.com/downloads/)

In the installer select `Desktop development with C++`.

## Set up CMake presets-based project

Open VS2022, clone the repository or open existing project from directory.

In the top bar select `msvc` preset and `bin/hatman.exe` target.

Press `Project` -> `Delete Cache and Reconfigure` in the top bar.

## Build the project

Press `Build` -> `Build All` in the top bar.

The executable bundled with all its asset & binary dependencies will reside in `build/bin/`.

> [!Tip]
> `Start with/without debugging` will perform those two steps automatically.

> [!Note]
> We bundle binary (`.dll`) dependencies of [SFML 2.5.1](https://www.sfml-dev.org/download/sfml/2.5.1) with the repo and automatically include them after the build when targeting Windows. Combined with CMake `FetchContent()` it leads to a rather steamlined process.

## (optional) Set up integrated `bash` terminal

Bash isn't necessary for building the project, but it is convenient to have for developent.

Download `Git Bash` from the [official website](https://git-scm.com/downloads). Install to any directory `<git_bash_path>`.

In Visual Studio, press `Ctrl` + `` ` `` to bring up integrated terminal. Press the settings button `⚙`, in the `Environment` -> `Terminal` -> `Add` tab add `<git_bash_path>/bin/sh.exe` as a new terminal. Set it as default if desirable.

Bash can now be selected as an integrated terminal option.

> [!Note]
> Adding `<git_bash_path>/git-bash.exe` instead will cause integrated terminal to spawn a new window each time. This is usually undesirable.