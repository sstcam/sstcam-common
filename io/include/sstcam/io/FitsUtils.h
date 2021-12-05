// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_FITSUTILS_H_
#define SSTCAM_IO_FITSUTILS_H_

#include <string>
#include <iostream>
#include <sstream>
#include <fitsio.h>


namespace sstcam::io { // NOLINT(modernize-concat-nested-namespaces)
/**a
 * @brief A namespace for utility functions for the cfitsio library.
 */
namespace fitsutils {

std::string ErrorMessage(int status);

long long int GetNRows(fitsfile* fits, int hdu_num);

bool HasHeaderKey(fitsfile* fits, const std::string& key);

template <typename T, int TFITS>
T GetHeaderKeyValue(fitsfile* fits, const std::string& key) {
    if (fits == nullptr) throw std::runtime_error("FITS file is not open");

    int status = 0;
    int hdutype = IMAGE_HDU;
    // Move to primary HDU header
    if (fits_movabs_hdu(fits, 1, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to the primary HDU" << ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    status = 0;
    if (fits_read_keyword(fits, key.c_str(), keyvalue, comment, &status)) {
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
        if (fits_read_key(fits, TSTRING, key.c_str(), value, comment, &status)) {
            std::ostringstream ss;
            ss << "Cannot read the header keyword:" << key << " " << ErrorMessage(status);
            throw std::runtime_error(ss.str());
        }
        return std::string(value);
    } else {
        T value;
        if (fits_read_key(fits, TFITS, key.c_str(), &value, comment, &status)) {
            std::ostringstream ss;
            ss << "Cannot read the header keyword:" << key << " " << ErrorMessage(status);
            throw std::runtime_error(ss.str());
        }
        return value;
    }
}

template<typename T, int TFITS>
void AddHeaderKeyValue(fitsfile* fits, const std::string& key, T value, const std::string& comment) {
    if (fits == nullptr) throw std::runtime_error("FITS file is not open");

    int status = 0;
    int hdutype;
    // Always write in the header of the primary HDU
    if (fits_movabs_hdu(fits, 1, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to the primary HDU" << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    if (fits_write_key(fits, TFITS, key.c_str(), &value, comment.c_str(), &status)) {
        std::ostringstream ss;
        ss << "Cannot write the keyword: " << key << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
}

template<typename T, int TFITS>
void UpdateHeaderKeyValue(fitsfile* fits, const std::string& key, T value, const std::string& comment) {
    if (fits == nullptr) throw std::runtime_error("FITS file is not open");

    int status = 0;
    int hdutype;
    // Always write in the header of the primary HDU
    if (fits_movabs_hdu(fits, 1, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to the primary HDU" << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    if (fits_update_key(fits, TFITS, key.c_str(), &value, comment.c_str(), &status)) {
        std::ostringstream ss;
        ss << "Cannot update the keyword: " << key << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
}

}}

#endif  // SSTCAM_IO_FITSUTILS_H_
