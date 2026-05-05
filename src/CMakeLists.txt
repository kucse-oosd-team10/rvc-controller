# Library target (used by tests and main)
add_library(rvc_lib
    rvc_controller.cpp
    timer.cpp
    off_state.cpp
    default_avoid_strategy.cpp
    movement_manager.cpp
    dust_sensor_subject.cpp
    obstacle_sensor_subject.cpp
    cleaning_manager.cpp
    avoiding_state.cpp
)

target_include_directories(rvc_lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Executable
add_executable(rvc_main main.cpp)
target_link_libraries(rvc_main PRIVATE rvc_lib)
