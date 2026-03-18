cmake_minimum_required(VERSION 3.22)
project(rvc-controller
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Options ----------
option(ENABLE_TESTING  "Build unit tests"       ON)
option(ENABLE_COVERAGE "Enable coverage flags"  OFF)

# ---------- Coverage flags ----------
if(ENABLE_COVERAGE)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()

# ---------- Subdirectories ----------
add_subdirectory(src)

if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()