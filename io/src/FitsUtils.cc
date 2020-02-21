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

long long int GetNRows(fitsfile* fits, int hdu_num) {
    if (fits == nullptr) throw std::runtime_error("FITS file is not open");

    int status = 0;
    int hdutype = BINARY_TBL;
    if (fits_movabs_hdu(fits, hdu_num, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to HDU: " << hdu_num << ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    status = 0;
    long long int n_rows;
    if (fits_get_num_rowsll(fits, &n_rows, &status)) {
        std::ostringstream ss;
        ss << "Cannot read the number of rows" << ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    return n_rows;
}

bool HasHeaderKey(fitsfile* fits, const std::string& key) {
    if (fits == nullptr) {
        std::ostringstream ss;
        ss << "FITS File is not open";
        throw std::runtime_error(ss.str());
    }

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
    return fits_read_keyword(fits, key.c_str(), keyvalue, comment, &status) == 0;
}

}}}
