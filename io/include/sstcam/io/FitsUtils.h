// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_FITSUTILS_H_
#define SSTCAM_IO_FITSUTILS_H_

#include <string>
#include <fitsio.h>
#include <iostream>

namespace sstcam {
namespace io {

/**
 * @brief A namespace for utility functions for the cfitsio library
 */
namespace fitsutils {


std::string ErrorMessage(int pStatus);

bool HasHeaderKey(fitsfile* fits_, const std::string& key);

template <typename T, int FITS_DATATYPE>
T GetHeaderKeyValue(fitsfile* fits_, const std::string& key) {
    T error_return_value;
    if constexpr(std::is_same<T, std::string>::value) {
        error_return_value = "";
    } else {
        error_return_value = static_cast<T>(0);
    }

    if (fits_ == nullptr) {
        std::cerr << "File is not open" << std::endl;
        return error_return_value;
    }

    int status = 0;
    int hdutype = IMAGE_HDU;
    // Move to primary HDU header
    if (fits_movabs_hdu(fits_, 1, &hdutype, &status)) {
        std::cerr << "Cannot move to the primary HDU " << ErrorMessage(status) << std::endl;
        return error_return_value;
    }

    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    status = 0;
    if (fits_read_keyword(fits_, key.c_str(), keyvalue, comment, &status)) {
        std::cerr << "Cannot find the keyword '" << key << "' " << ErrorMessage(status) << std::endl;
        return error_return_value;
    }

    if (key == "COMMENT" || key == "HISTORY") {
        if constexpr(std::is_same<T, std::string>::value) {
            return comment;
        } else {
            return static_cast<T>(std::atof(comment));
        }
    }

    if constexpr(std::is_same<T, std::string>::value) {
        char value[81];
        if (fits_read_key(fits_, TSTRING, key.c_str(), value, comment, &status)) {
            std::cerr << "Cannot read the keyword '" << key << "' " << ErrorMessage(status) << std::endl;
            return error_return_value;
        }
        return std::string(value);
    } else {
        T value;
        if (fits_read_key(fits_, FITS_DATATYPE, key.c_str(), &value, comment, &status)) {
            std::cerr << "Cannot read the keyword '" << key << "' " << ErrorMessage(status) << std::endl;
            return error_return_value;
        }
        return value;
    }
}

//template <>
//std::string GetHeaderKeyValue<std::string, TSTRING>(fitsfile* fits_, const std::string& key);

}}}

#endif  // SSTCAM_IO_FITSUTILS_H_
