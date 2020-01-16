// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_UTIL_H_
#define SSTCAM_IO_UTIL_H_

#include <string>

namespace sstcam {
namespace io {

/**
 * @brief A namespace for utility functions
 *
 * All utility functions should be declared and defined inside this namespace
 * instead of being naked, so that users can easily distinguish them from those
 * in other libraries.
 */
namespace util {

/**
 * Check the value type of a keyword.
 * This implementation is based on the FITS Standard
 * "Definition of the Flexible Image Transport System (FITS)" ver. 3.0
 * We do not support any NON-standard values except for lower cases of 'E'
 * and 'D'
 * We do not support int64_t complex because CFITSIO doesn't
 */
//int32_t CheckFitsValueType(const std::string &pValue);

std::string FitsErrorMessage(int pStatus);

//float GetVersionOfCFITSIO();

}}}

#endif  // SSTCAM_IO_UTIL_H_
