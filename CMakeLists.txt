cmake_minimum_required(VERSION 3.11...3.16)


#Get git version number
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=7
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE VERSION
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(SSTCAM_COMMON_VERSION_LONG ${VERSION})
#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
set(SSTCAM_COMMON_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

configure_file(sstcam_common_version.h.in ${CMAKE_BINARY_DIR}/generated/sstcam_common_version.h )
include_directories("${CMAKE_BINARY_DIR}/generated/")

message(STATUS "SSTCam-common version: ${SSTCAM_COMMON_VERSION}")

project(sstcam-common)
message(STATUS "Configuring sstcam-constants")
add_subdirectory(constants)
message(STATUS "Configuring sstcam-descriptions")
add_subdirectory(descriptions)
message(STATUS "Configuring sstcam-io")
add_subdirectory(io)
message(STATUS "Configuring sstcam-calibration")
add_subdirectory(calibration)

# -----------------------------------------------------------------------------
# Export targets for use in downstream CMake projects
# -----------------------------------------------------------------------------
include ( CMakePackageConfigHelpers )

## Make importable from build directory
#write_basic_package_version_file(
#  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#  VERSION 1.0
#  COMPATIBILITY AnyNewerVersion
#)
#export(EXPORT sstcam-base-targets FILE ${PROJECT_NAME}Config.cmake)
#
## Make importable from install location
#set(_config_dir share/${PROJECT_NAME})
#install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#  DESTINATION ${_config_dir}
#)
#install(EXPORT sstcam-base-targets
#  DESTINATION ${_config_dir}
#)