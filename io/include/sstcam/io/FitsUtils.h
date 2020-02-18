// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_FITSUTILS_H_
#define SSTCAM_IO_FITSUTILS_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fitsio.h>

namespace sstcam {
namespace io {

/**
 * @brief A namespace for utility functions for the cfitsio library
 */
namespace fitsutils {


std::string ErrorMessage(int status);

bool HasHeaderKey(fitsfile* fits_, const std::string& key);

template <typename T, int FITS_DATATYPE>
T GetHeaderKeyValue(fitsfile* fits_, const std::string& key) {
    if (fits_ == nullptr) throw std::runtime_error("FITS file is not open");

    int status = 0;
    int hdutype = IMAGE_HDU;
    // Move to primary HDU header
    if (fits_movabs_hdu(fits_, 1, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to the primary HDU" << ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    status = 0;
    if (fits_read_keyword(fits_, key.c_str(), keyvalue, comment, &status)) {
        std::ostringstream ss;
        ss << "Cannot find the header keyword:" << key << " " << ErrorMessage(status);
        throw std::runtime_error(ss.str());
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
            std::ostringstream ss;
            ss << "Cannot read the header keyword:" << key << " " << ErrorMessage(status);
            throw std::runtime_error(ss.str());
        }
        return std::string(value);
    } else {
        T value;
        if (fits_read_key(fits_, FITS_DATATYPE, key.c_str(), &value, comment, &status)) {
            std::ostringstream ss;
            ss << "Cannot read the header keyword:" << key << " " << ErrorMessage(status);
            throw std::runtime_error(ss.str());
        }
        return value;
    }
}

}}}

#endif  // SSTCAM_IO_FITSUTILS_H_
