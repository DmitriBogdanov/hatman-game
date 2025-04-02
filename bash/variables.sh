# __________________________________ CONTENTS ___________________________________
#
#    All the variables used by other scripts, edit this file to configure
#       - compiler used by CMake
#       - test flags
#       - build jobs
#       ...
# _______________________________________________________________________________

# ===================
# ---- Constants ----
# ===================

path_include="hatman/include/*.h*"
path_executable="build/bin/main"

directory_build="build/"
directory_tests="${directory_build}tests/"

script_run_static_analysis="bash/run_static_analysis.sh"

cppcheck_suppressions_file=".cppcheck"
cppcheck_cache_directory=".cache-cppcheck"

# =======================
# ---- Configuration ----
# =======================

compiler="g++" # clang++-11
test_flags="--rerun-failed --output-on-failure --timeout 60"
build_jobs="6"
