cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_calibration VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)
set(LIBTARGET ${PROJECT_NAME}_c)
set(PYTARGET ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# src
set(HEADER_LIST include/sstcam/calibration/Calibrator.h)
add_library(${LIBTARGET} SHARED src/Calibrator.cc ${HEADER_LIST})
target_include_directories(${LIBTARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include/>
        )
target_compile_features(${LIBTARGET} PUBLIC cxx_std_11)
install (TARGETS ${LIBTARGET} EXPORT sstcam-base-targets LIBRARY DESTINATION lib)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
set(PYTHON_SRC pybind/module.cc)
python_module(${PYTARGET} ${LIBTARGET} calibration "${PYTHON_SRC}")

# ctests
add_library(test_main_calibration OBJECT ctests/test_main.cc)
target_include_directories(test_main_calibration PUBLIC ctests ${DOCTEST_INCLUDE_DIR})

# data files
#file(GLOB DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/share/sstcam/calibration/*")
#file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/share/sstcam/calibration/)
#install(DIRECTORY "${CMAKE_BINARY_DIR}/share" DESTINATION .)

# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

