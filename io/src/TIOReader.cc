// Copyright 2019 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include "sstcam/io/Util.h"
#include <utility>
#include <iostream>
#include <cinttypes>

namespace sstcam {
namespace io {

TIOReader::TIOReader(std::string path)
    : path_(std::move(path)),
      fits_(nullptr),
      n_event_headers_(0),
      n_packets_per_event_(0)
{
    // Open fits file
    int status = 0;
    if (fits_open_file(&fits_, path.c_str(), READONLY, &status)) {
        std::cerr << "Cannot open " << path << " " << util::FitsErrorMessage(status);
        Close();
        return;
    }

    // TODO: Implement and only allow > 1
    // Get EventHeaderVersion

    // Move pointer to EVENTS HDU
    if (fits_movnam_hdu(fits_, BINARY_TBL, const_cast<char*>("EVENTS"), 0, &status)) {
        std::cerr << "Cannot move to the HDU (EVENTS) " << util::FitsErrorMessage(status);
        Close();
        return;
    }

    // Get Number of columns in EVENTS HDU
    long n_columns;
    char comment[FLEN_COMMENT];
    if (fits_read_key_lng(fits_, "TFIELDS", &n_columns, comment, &status)) {
        std::cerr << "Cannot read TFIELDS " << util::FitsErrorMessage(status);
        Close();
        return;
    }

    char value[FLEN_VALUE];
    char tform[FLEN_KEYWORD];

    // Find first packet column
    for (int64_t i = 0; i < n_columns && i < 256; ++i) {
        snprintf(tform, FLEN_KEYWORD, "TTYPE%" PRIi64, i + 1);
        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
            std::cerr << "Cannot read TTYPE" << i + 1 << " " << util::FitsErrorMessage(status);
            Close();
            return;
        } else {
            // Found the first packet column
            if (strcmp(value, "EVENT_PACKET_0") == 0) {
                n_event_headers_ = static_cast<uint8_t>(i);
                break;
            }
        }
    }

    n_packets_per_event_ = static_cast<uint32_t>(n_columns - n_event_headers_);

//    // Calculate packet size
//    for (int i = n_event_headers_; i < n_columns; ++i) {
//        snprintf(tform, FLEN_KEYWORD, "TFORM%d", i + 1);
//        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
//            std::cerr << "Cannot read TFORM" << i + 1 << " " << util::FitsErrorMessage(status) << std::endl;
//            Close();
//            return;
//        }
//
//        uint16_t packet_size;
//        if (sscanf(value, "%" SCNu16 "B", &packet_size) == 1) {
//            if (packet_size != fPacketSize) {
//                std::cerr << "Expected value of TFORM" << i << " is " << fPacketSize
//                          << "B, but it is " << value << std::endl;
//                Close();
//            }
//            if (fData == 0) {
//                fPacketSize = packet_size;
//                fData = new uint8_t[fPacketSize];
//            }
//        } else {
//            fPacketSize = 0;
//            std::cerr << "Expected value of TFORM" << i << " is xxxB, but it is "
//                      << value << std::endl;
//            Close();
//        }
//    }

}

}
}