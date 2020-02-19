// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/FitsUtils.h"

namespace sstcam {
namespace io {
namespace fitsutils {

std::string ErrorMessage(int status) {
    static char error[100];
    fits_get_errstatus(status, error);
    std::ostringstream ss;
    ss << " (FITSIO: " << error << ")";
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
        std::ostringstream ss;
        ss << "Cannot move to the primary HDU" << ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    char keyvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    status = 0;
    return fits_read_keyword(fits_, key.c_str(), keyvalue, comment, &status) == 0;
}

}}}
