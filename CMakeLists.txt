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
set(SSTCAM_COMMON_VERSION_INCLUDE "${CMAKE_BINARY_DIR}/generated/")

message(STATUS "SSTCam-common version: ${SSTCAM_COMMON_VERSION}")
# These are the targets to be exported for install by the subprojects
set(PROJECT_TARGETS sstcam-common-targets)

project(sstcam-common)

message(STATUS "${Blue}Configuring sstcam-constants${ColourReset}")
add_subdirectory(constants)
message(STATUS "${Blue}Configuring sstcam-descriptions${ColourReset}")
add_subdirectory(descriptions)
message(STATUS "${Blue}Configuring sstcam-io${ColourReset}")
add_subdirectory(io)
message(STATUS "${Blue}Configuring sstcam-calibration${ColourReset}")
add_subdirectory(calibration)

# -----------------------------------------------------------------------------
# Export targets for use in downstream CMake projects
# -----------------------------------------------------------------------------
include ( CMakePackageConfigHelpers )


# set protobuf include library
set(Protobuf_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/descriptions/protobuf/ PARENT_SCOPE)

# Make importable from build directory
write_basic_package_version_file(
 "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
 VERSION ${SSTCAM_COMMON_VERSION}
 COMPATIBILITY AnyNewerVersion
)
export(EXPORT ${PROJECT_TARGETS} FILE ${PROJECT_NAME}Config.cmake)

# Make importable from install location
set(_config_dir share/${PROJECT_NAME})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
 DESTINATION ${_config_dir}
)
install(EXPORT ${PROJECT_TARGETS}
 DESTINATION ${_config_dir}
)