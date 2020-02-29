cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_constants VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)

# setting up library
set(HEADER_LIST include/sstcam/constants/Geometry.h)
sstcam_library(TARGET_SRCS src/Geometry.cc
               HEADER_LIST ${HEADER_LIST})

# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
sstcam_python_module(MODULE_NAME constants
                     LIBTARGETS ${LIBTARGET}
                     SRC_FILES pybind/module.cc
                     INCLUDE_DIRS ${SSTCAM_COMMON_VERSION_INCLUDE})

# ctests
sstcam_tests(TESTS
             LIBTARGETS ${LIBTARGET})

# add_library(test_main_constants OBJECT ctests/test_main.cc)
# target_include_directories(test_main_constants PUBLIC ctests ${DOCTEST_INCLUDE_DIR})




