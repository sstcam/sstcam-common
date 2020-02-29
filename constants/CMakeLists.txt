cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_constants VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)
set(PYTARGET ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# setting up library
set(HEADER_LIST include/sstcam/constants/Geometry.h)
sstcam_library(TARGET_SRCS src/Geometry.cc
               HEADER_LIST ${HEADER_LIST})

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
set(PYTHON_SRC pybind/module.cc)
python_module(${PYTARGET} ${LIBTARGET} constants "${PYTHON_SRC}")
target_include_directories(${PYTARGET} PRIVATE ${SSTCAM_COMMON_VERSION_INCLUDE})

# ctests
add_library(test_main_constants OBJECT ctests/test_main.cc)
target_include_directories(test_main_constants PUBLIC ctests ${DOCTEST_INCLUDE_DIR})


# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

