# Find fitsio.h and cfitsio library
# Ref. https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Find-Libraries

find_path(CFITSIO_INCLUDE_DIR fitsio.h PATH_SUFFIXES cfitsio)
find_library(CFITSIO_LIBRARY cfitsio PATH_SUFFIXES cfitsio)
set(CFITSIO_INCLUDE_DIRS ${CFITSIO_INCLUDE_DIR})
set(CFITSIO_LIBRARIES ${CFITSIO_LIBRARY})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CFITSIO DEFAULT_MSG CFITSIO_LIBRARY CFITSIO_INCLUDE_DIR)
mark_as_advanced(CFITSIO_INCLUDE_DIR CFITSIO_LIBRARY)