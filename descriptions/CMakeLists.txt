cmake_minimum_required(VERSION 3.11...3.16)

project(sstcam_descriptions VERSION ${SSTCAM_COMMON_VERSION} LANGUAGES CXX)

# setting up library
set(HEADER_LIST include/sstcam/descriptions/WaveformDataPacket.h include/sstcam/descriptions/Waveform.h include/sstcam/descriptions/WaveformEvent.h)
sstcam_library(TARGET_SRCS src/WaveformDataPacket.cc src/Waveform.cc src/WaveformEvent.cc
               HEADER_LIST ${HEADER_LIST})
# Compilation options
target_compile_options(${LIBTARGET} PUBLIC -O2 -Wall -pedantic -Werror -Wextra)

# Makes it easier for IDEs to find all headers
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# python_module
sstcam_python_module(MODULE_NAME descriptions
                     LIBTARGETS ${LIBTARGET}
                     SRC_FILES pybind/module.cc pybind/WaveformDataPacket.cc pybind/Waveform.cc pybind/WaveformEvent.cc
                     INCLUDE_DIRS ${SSTCAM_COMMON_VERSION_INCLUDE})

# ctests
sstcam_tests(TESTS test_WaveformDataPacket test_Waveform test_WaveformEvent
             LIBTARGETS ${LIBTARGET})


# data files
file(GLOB DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/share/sstcam/descriptions/*")
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/share/sstcam/descriptions/)
install(DIRECTORY "${CMAKE_BINARY_DIR}/share" DESTINATION .)


