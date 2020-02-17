// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/FitsUtils.h"
#include <iostream>
#include <sstream>

namespace sstcam {
namespace io {
namespace fitsutils {

std::string ErrorMessage(int status) {
    static char error[100];
    fits_get_errstatus(status, error);
    std::ostringstream ss;
    ss << "(FITSIO: " << error << ")";
    return ss.str();
}

bool HasHeaderKey(fitsfile* fits_, const std::string& key) {
    if (fits_ == nullptr) {
        std::cerr << "File is not open" << std::endl;
        return false;
    }

    int status = 0;
    int hdutype = IMAGE_HDU;
    // Move to primary HDU header
    if (fits_movabs_hdu(fits_, 1, &hdutype, &status)) {
        std::cerr << "Cannot move to the primary HDU " << ErrorMessage(status) << std::endl;
        return false;
    }

    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    status = 0;
    return fits_read_keyword(fits_, key.c_str(), keyvalue, comment, &status) == 0;
}

//template <>
//std::string GetHeaderKeyValue<std::string, TSTRING>(fitsfile* fits_, const std::string& key) {
//    if (fits_ == nullptr) {
//        std::cerr << "File is not open" << std::endl;
//        return "";
//    }
//
//    int status = 0;
//    int hdutype = IMAGE_HDU;
//    // Move to primary HDU header
//    if (fits_movabs_hdu(fits_, 1, &hdutype, &status)) {
//        std::cerr << "Cannot move to the primary HDU " << ErrorMessage(status) << std::endl;
//        return "";
//    }
//
//    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
//    status = 0;
//    if (fits_read_keyword(fits_, key.c_str(), keyvalue, comment, &status)) {
//        std::cerr << "Cannot find the keyword '" << key << "' " << ErrorMessage(status) << std::endl;
//        return "";
//    }
//
//    if (key == "COMMENT" || key == "HISTORY") {
//        return comment;
//    }
//
//    char value[81];
//    if (fits_read_key(fits_, TSTRING, key.c_str(), value, comment, &status)) {
//        std::cerr << "Cannot read the keyword '" << key << "' " << ErrorMessage(status) << std::endl;
//        return "";
//    }
//    return std::string(value);
//}


}}}
