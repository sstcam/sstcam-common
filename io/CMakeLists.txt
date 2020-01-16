cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_io VERSION 1.0 LANGUAGES CXX)
set(LIBTARGET ${PROJECT_NAME}_c)
set(PYTARGET ${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Save executables to bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Dependencies
#find_package(sstcam-base REQUIRED)

find_package(pybind11 REQUIRED)

#if(CMAKE_INSTALL_PREFIX)
#    set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
#endif(CMAKE_INSTALL_PREFIX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(CFITSIO REQUIRED)
#list(APPEND EXTLIBS ${CFITSIO_LIBRARIES})

include(CTest)

# src
set(HEADER_LIST include/sstcam/io/TIOReader.h include/sstcam/io/Util.h)
add_library(${LIBTARGET} SHARED src/TIOReader.cc src/Util.cc  ${HEADER_LIST})
target_link_libraries(${LIBTARGET} sstcam_interfaces_c ${CFITSIO_LIBRARIES})
target_include_directories(${LIBTARGET} PUBLIC include ${CFITSIO_INCLUDE_DIRS})
target_compile_features(${LIBTARGET} PUBLIC cxx_std_11)
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
install (TARGETS ${LIBTARGET} LIBRARY DESTINATION lib)

# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)
