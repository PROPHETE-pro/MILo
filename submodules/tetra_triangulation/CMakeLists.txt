cmake_minimum_required(VERSION 3.16.3)
project(fast_traversal LANGUAGES CXX CUDA)
enable_language(CUDA)

include(FetchContent)
# Add paths to our CMake code to the module path, so they can be found automatically by
# CMake.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE "Release")

find_library(libcnpy cnpy ${CONDA_PREFIX}/lib)
include_directories(${CONDA_PREFIX}/include)

# TODO: fix this
find_package(Torch REQUIRED)
find_package(CGAL REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(Python3 COMPONENTS Development.Module QUIET)
if(NOT COMMAND python3_add_library)
    find_package(Python3 COMPONENTS Development REQUIRED)
endif()
# pybind11 in "new mode" expects python3_add_library from FindPython3.
if(COMMAND python3_add_library)
    set(PYBIND11_FINDPYTHON ON)
else()
    set(PYBIND11_FINDPYTHON COMPAT)
endif()
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND AND Python3_Interpreter_FOUND)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PYBIND11_PYTHON_QUERY_STATUS
        ERROR_QUIET
    )
    if(PYBIND11_PYTHON_QUERY_STATUS EQUAL 0 AND EXISTS "${PYBIND11_CMAKE_DIR}/pybind11Config.cmake")
        message(STATUS "Using pybind11 from Python package: ${PYBIND11_CMAKE_DIR}")
        set(pybind11_DIR "${PYBIND11_CMAKE_DIR}" CACHE PATH "Path to pybind11Config.cmake")
        find_package(pybind11 CONFIG REQUIRED)
    endif()
endif()

if(NOT pybind11_FOUND)
    set(PYBIND11_GIT_REPOSITORY "https://github.com/pybind/pybind11.git" CACHE STRING "Git repository used to fetch pybind11")
    set(PYBIND11_GIT_TAG "v2.9.2" CACHE STRING "Git tag used to fetch pybind11")
    message(STATUS "pybind11 was not found locally. Fetching from ${PYBIND11_GIT_REPOSITORY}")
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY ${PYBIND11_GIT_REPOSITORY}
        GIT_TAG        ${PYBIND11_GIT_TAG}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(pybind11)
endif()


# Put all the runtime stuff in the same directory.  By default, CMake puts each targets'
# output into their own directory.  We want all the targets to be put in the same
# directory, and we can do this by setting these variables.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(BUILD_DIRECTORY "${CMAKE_BINARY_DIR}/build")
set(CUDA_GENERATED_OUTPUT_DIR "${BUILD_DIRECTORY}")


#------------------------------------------------------------------------------
# set environment
#------------------------------------------------------------------------------

# find_package(CUDA REQUIRED)
# include_directories(${CUDA_INCLUDE_DIRS})

# configure_file("${CMAKE_SOURCE_DIR}/path_util.py.in" "${CMAKE_SOURCE_DIR}/../examples/path_util.py")


#------------------------------------------------------------------------------
# main build
#------------------------------------------------------------------------------

# configure_file(${generated_files} file.ptx.h)
add_link_options(-flto=auto)  # disables some warning
pybind11_add_module(tetranerf_cpp_extension 
    src/triangulation.cpp
    src/py_binding.cpp)

target_link_libraries(tetranerf_cpp_extension PRIVATE ${CUDA_LIBRARIES} ${TORCH_LIBRARIES})
target_link_libraries(tetranerf_cpp_extension PRIVATE CGAL::CGAL)
target_compile_features(tetranerf_cpp_extension PRIVATE cxx_std_17)
set_target_properties(tetranerf_cpp_extension
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tetranerf/utils/extension"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# target_compile_definitions(tetranerf_cpp_extension PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
