cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_io VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(CFITSIO REQUIRED)

# setting up library
set(HEADER_LIST include/sstcam/io/TIOReader.h include/sstcam/io/FitsUtils.h)
sstcam_library(TARGET_SRCS src/TIOReader.cc src/FitsUtils.cc
               HEADER_LIST ${HEADER_LIST}
               ADD_INCLUDE_DIRS ${CFITSIO_INCLUDE_DIRS}
               LINK_LIBRARIES sstcam_descriptions_c ${CFITSIO_LIBRARIES})
# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python module
sstcam_python_module(MODULE_NAME io
                     LIBTARGETS ${LIBTARGET}
                     SRC_FILES pybind/module.cc pybind/TIOReader.cc
                     INCLUDE_DIRS ${SSTCAM_COMMON_VERSION_INCLUDE})


# ctests
add_library(test_main_io OBJECT ctests/test_main.cc)
target_include_directories(test_main_io PUBLIC ctests ${DOCTEST_INCLUDE_DIR})
sstcam_tests(test_FitsUtils ctests/FitsUtils.cc test_main_io ${LIBTARGET})
sstcam_tests(test_TIOReader ctests/TIOReader.cc test_main_io ${LIBTARGET})


