# Library target (used by tests and main)
add_library(rvc_lib
    controller.cpp
)

target_include_directories(rvc_lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Executable
add_executable(rvc_main main.cpp)
target_link_libraries(rvc_main PRIVATE rvc_lib)