cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_calibration VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# setting up library
set(HEADER_LIST include/sstcam/calibration/Calibrator.h)
sstcam_library(TARGET_SRCS src/Calibrator.cc
               HEADER_LIST ${HEADER_LIST})
# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
sstcam_python_module(MODULE_NAME calibration
                     LIBTARGETS ${LIBTARGET}
                     SRC_FILES pybind/module.cc
                     INCLUDE_DIRS ${SSTCAM_COMMON_VERSION_INCLUDE})

# ctests
add_library(test_main_calibration OBJECT ctests/test_main.cc)
target_include_directories(test_main_calibration PUBLIC ctests ${DOCTEST_INCLUDE_DIR})



