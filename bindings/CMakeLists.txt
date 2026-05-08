include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(rvc rvc_module.cpp)
target_link_libraries(rvc PRIVATE rvc_lib)
target_compile_features(rvc PRIVATE cxx_std_20)
