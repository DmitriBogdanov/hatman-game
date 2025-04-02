# __________________________________ CONTENTS ___________________________________
#   
#   This script contains shortcuts for building, running
#   and testing the project. All action keywords can be
#   chained which causes them to be executed one after another.
#
#   See "docs/guide_building_project.md" for the whole building guide.
#   
# ____________________________________ GUIDE ____________________________________
#   
#   Usage format:
#     > bash actions.sh [ACTIONS]
#   
#   Actions:
#     clear   - Clears "build/" folder
#     config  - Configures CMake with appropriate args
#     build   - Builds the project (requires configured CMake)
#     test    - Runs CTest tests (requires successful build)
#     run     - Runs currently selected executable
#     profile - Runs currently selected executable with Callgrind profiler
#   
#   Usage example:
#     > bash actions.sh clear config build test  
# _______________________________________________________________________________

# =======================
# ------ Functions ------
# =======================

source bash/variables.sh
source bash/functions.sh

command_clear() {
    if [ -d "$directory_build" ]; then
        rm --recursive $directory_build
        echo "Cleared directory [ $directory_build ]."
    else
        echo "Directory [ $directory_build ] is clear."
    fi
}

command_config() {
    require_command_exists "cmake"
    require_command_exists "$compiler"
    cmake -D CMAKE_CXX_COMPILER=$compiler -B $directory_build -S .
}

command_build() {
    # Run CMake build
    require_command_exists "cmake"
    cmake --build $directory_build --parallel $build_jobs
}

command_run() {
    ./$path_executable
}

command_check() {
    # Invoke script to run static analyzers
    if [ -f "$script_run_static_analysis" ]; then
        printf "${ansi_green}Running static analyzers...${ansi_reset}\n"
        bash "$script_run_static_analysis"
        printf "${ansi_green}Analysis complete.${ansi_reset}\n"
    else
        printf "${ansi_red}# Error: Could not find \"$script_run_static_analysis\".${ansi_reset}\n"
    fi
}

# =======================
# --- Action selector ---
# =======================

valid_command=false

for var in "$@"
do
    valid_command=false
    
    if [ "$var" = "clear" ]; then
        printf "${ansi_purple}# Action: Clear Files${ansi_reset}\n"
        command_clear
        valid_command=true
    fi

    if [ "$var" = "config" ]; then
        printf "${ansi_purple}# Action: CMake Configure${ansi_reset}\n"
        command_config
        valid_command=true
    fi

    if [ "$var" = "build" ]; then
        printf "${ansi_purple}# Action: CMake Build${ansi_reset}\n"
        command_build
        valid_command=true
    fi
    
    if [ "$var" = "run" ]; then
        printf "${ansi_purple}# Action: Run${ansi_reset}\n"
        command_run
        valid_command=true
    fi
    
    if [ "$var" = "check" ]; then
        printf "${ansi_purple}# Action: Run Static Analysis${ansi_reset}\n"
        command_check
        valid_command=true
    fi
    
    if [ $valid_command = false ]; then
        printf "${ansi_red}# Error: Invalid action name -> ${var}${ansi_reset}\n"
        break
    fi

done