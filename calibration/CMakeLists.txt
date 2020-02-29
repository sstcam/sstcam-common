cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_calibration VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)
set(PYTARGET ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# setting up library
set(HEADER_LIST include/sstcam/calibration/Calibrator.h)
sstcam_library(TARGET_SRCS src/Calibrator.cc
               HEADER_LIST ${HEADER_LIST})

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
set(PYTHON_SRC pybind/module.cc)
python_module(${PYTARGET} ${LIBTARGET} calibration "${PYTHON_SRC}")
target_include_directories(${PYTARGET} PRIVATE ${SSTCAM_COMMON_VERSION_INCLUDE})

# ctests
add_library(test_main_calibration OBJECT ctests/test_main.cc)
target_include_directories(test_main_calibration PUBLIC ctests ${DOCTEST_INCLUDE_DIR})

# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

