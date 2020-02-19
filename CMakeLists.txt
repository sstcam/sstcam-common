cmake_minimum_required(VERSION 3.11...3.16)

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