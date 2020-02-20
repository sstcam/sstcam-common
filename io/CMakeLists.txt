cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_io VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)
set(LIBTARGET ${PROJECT_NAME}_c)
set(PYTARGET ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${CMAKE_BINARY_DIR}/generated/")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(CFITSIO REQUIRED)

# src
set(HEADER_LIST include/sstcam/io/TIOReader.h include/sstcam/io/FitsUtils.h)
add_library(${LIBTARGET} SHARED src/TIOReader.cc src/FitsUtils.cc ${HEADER_LIST})
target_link_libraries(${LIBTARGET} sstcam_descriptions_c ${CFITSIO_LIBRARIES})
target_include_directories(${LIBTARGET} PUBLIC ${CFITSIO_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include/>
        )
target_compile_features(${LIBTARGET} PUBLIC cxx_std_11)
install (TARGETS ${LIBTARGET} LIBRARY DESTINATION lib)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python module
set(PYTHON_SRC pybind/module.cc pybind/TIOReader.cc)
python_module(${PYTARGET} ${LIBTARGET} io "${PYTHON_SRC}")

# ctests
add_library(test_main_io OBJECT ctests/test_main.cc)
target_include_directories(test_main_io PUBLIC ctests ${DOCTEST_INCLUDE_DIR})
sstcam_tests(test_FitsUtils ctests/FitsUtils.cc test_main_io ${LIBTARGET})
sstcam_tests(test_TIOReader ctests/TIOReader.cc test_main_io ${LIBTARGET})

# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)
