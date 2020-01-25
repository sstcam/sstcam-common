// Copyright 2019 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include <utility>
#include <iostream>
#include <cinttypes>
#include <memory>
#include <set>

namespace sstcam {
namespace io {

TIOReader::TIOReader(const std::string& path)
    : fits_(nullptr),
      event_hdu_num_(0),
      n_event_columns_(0),
      n_event_headers_(0)
{
    // Open fits file
    int status = 0;
    if (fits_open_file(&fits_, path.c_str(), READONLY, &status)) {
        std::cerr << "Cannot open " << path << " "
                  << fitsutils::ErrorMessage(status) << std::endl;
        Close();
        return;
    }

    // Get EventHeaderVersion
    uint16_t version = fitsutils::GetHeaderKeyValue<int16_t, TINT>(
        fits_, "EVENT_HEADER_VERSION");
    if (version < 1) {
        std::cerr << "Incompatible EVENT_HEADER_VERSION: " << version << std::endl;
        Close();
        return;
    }

    // Move pointer to EVENTS HDU
    if (fits_movnam_hdu(fits_, BINARY_TBL, const_cast<char *>("EVENTS"), 0, &status)) {
        std::cerr << "Cannot move to the HDU (EVENTS) "
                  << fitsutils::ErrorMessage(status) << std::endl;
        Close();
        return;
    }
    fits_get_hdu_num(fits_, &event_hdu_num_);

    char comment[FLEN_COMMENT];
    char value[FLEN_VALUE];
    char tform[FLEN_KEYWORD];

    // Get Number of columns in EVENTS HDU
    if (fits_read_key_lng(fits_, "TFIELDS", &n_event_columns_, comment, &status)) {
        std::cerr << "Cannot read TFIELDS " << fitsutils::ErrorMessage(status) << std::endl;
        Close();
        return;
    }

    // Find first packet column
    for (int64_t i = 0; i < n_event_columns_ && i < 256; ++i) {
        snprintf(tform, FLEN_KEYWORD, "TTYPE%" PRIi64, i + 1);
        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
            std::cerr << "Cannot read TTYPE" << i + 1 << " "
                      << fitsutils::ErrorMessage(status);
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

    CreateWaveformRunHeader();

    std::cout << "[TIOReader] Path: " << GetPath() << std::endl;
    std::cout << "[TIOReader] RunID: " << GetRunID() << std::endl;
    std::cout << "[TIOReader] CameraVersion: " << GetCameraVersion() << std::endl;
    std::cout << "[TIOReader] IsR1: " << std::boolalpha << IsR1() << std::endl;
    std::cout << "[TIOReader] ActiveModuleSlots: ";
    for (uint8_t slot: run_header_->active_modules) {
        std::cout << (unsigned) slot << " ";
    }
    std::cout << std::endl;
    std::cout << "[TIOReader] NEvents: " << GetNEvents() << std::endl;
    std::cout << "[TIOReader] NPixels: " << run_header_->n_pixels << std::endl;
    std::cout << "[TIOReader] NSamples: " << run_header_->n_samples << std::endl;
}

void TIOReader::CreateWaveformRunHeader() {
    auto n_packets_per_event = static_cast<size_t>(n_event_columns_ - n_event_headers_);
    size_t packet_size = ExtractPacketSize();
    std::set<uint8_t> active_modules = ExtractActiveModules(n_packets_per_event, packet_size);
    size_t n_samples = ExtractNSamples(packet_size);

    if (IsR1()) {
        auto scale = fitsutils::GetHeaderKeyValue<float, TFLOAT>(fits_, "SCALE");
        auto offset = fitsutils::GetHeaderKeyValue<float, TFLOAT>(fits_, "OFFSET");
        run_header_ = std::make_unique<WaveformRunHeader>(
            n_packets_per_event, packet_size, active_modules, n_samples,
            true, scale, offset);
    } else {
        run_header_ = std::make_unique<WaveformRunHeader>(
            n_packets_per_event, packet_size,active_modules, n_samples);
    }
}

size_t TIOReader::ExtractPacketSize() const {
    size_t packet_size = 0;
    int status = 0;
    char comment[FLEN_COMMENT];
    char value[FLEN_VALUE];
    char tform[FLEN_KEYWORD];

    for (int i = n_event_headers_; i < n_event_columns_; ++i) {
        snprintf(tform, FLEN_KEYWORD, "TFORM%d", i + 1);
        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
            std::cerr << "Cannot read TFORM" << i + 1 << " "
                      << fitsutils::ErrorMessage(status) << std::endl;
            return 0;
        }

        uint16_t packet_size_i;
        if (sscanf(value, "%" SCNu16 "B", &packet_size_i) == 1) {
            if (packet_size == 0) {
                packet_size = packet_size_i;
            }
            else if (packet_size_i != packet_size) {
                std::cerr << "Expected value of TFORM" << i << " is "
                          << packet_size << "B, but it is " << value << std::endl;
                return 0;
            }
        } else {
            std::cerr << "Expected value of TFORM" << i << " is xxxB, but it is "
                      << value << std::endl;
            return 0;
        }
    }
    return packet_size;
}

std::set<uint8_t> TIOReader::ExtractActiveModules(size_t n_packets_per_event, size_t packet_size) const {
    WaveformDataPacket packet(packet_size);
    std::set<uint8_t> active_modules;
    for (uint32_t ipack =0; ipack < n_packets_per_event; ipack++) {
        AssociatePacket(packet, ipack, 0);
        active_modules.insert(packet.GetSlotID());
    }
    return active_modules;
}

size_t TIOReader::ExtractNSamples(size_t packet_size) const{
    WaveformDataPacket packet(packet_size);
    AssociatePacket(packet, 0, 0);
    return packet.GetWaveformNSamples();
}

void TIOReader::Close() {
    if (!IsOpen()) {
        return;
    }

    int status = 0;
    if (fits_close_file(fits_, &status)) {
        std::cerr << "Cannot close the file "
                  << fitsutils::ErrorMessage(status) << std::endl;
    } else {
        fits_ = nullptr;
    }
}

std::string TIOReader::GetPath() const {
    if (!IsOpen()) {
        return "";
    }

    return std::string(fits_->Fptr->filename);
}

uint32_t TIOReader::GetNEvents() const {
    if (!IsOpen()) {
        std::cerr << "File is not open" << std::endl;
        return 0u;
    }

    int status = 0;
    int hdutype = BINARY_TBL;
    if (fits_movabs_hdu(fits_, event_hdu_num_, &hdutype, &status)) {
        std::cerr << "Cannot move to the event HDU "
                  << fitsutils::ErrorMessage(status) << std::endl;
        return 0u;
    }

    status = 0;
    LONGLONG n_rows;
    if (fits_get_num_rowsll(fits_, &n_rows, &status)) {
        std::cerr << "Cannot read the number of rows "
                  << fitsutils::ErrorMessage(status) << std::endl;
        return 0u;
    }

    return static_cast<uint32_t>(n_rows);
}

void TIOReader::AssociatePacket(WaveformDataPacket& packet, uint16_t packet_id, uint32_t event_index) const {
    if (!IsOpen()) {
        std::cerr << "File is not open" << std::endl;
    }

    int status = 0;
    int hdutype = BINARY_TBL;
    if (fits_movabs_hdu(fits_, event_hdu_num_, &hdutype, &status)) {
        std::cerr << "Cannot move to the event HDU "
                  << fitsutils::ErrorMessage(status) << std::endl;
    }

    status = 0;
    int anynull;
    if (fits_read_col(fits_, TBYTE, packet_id + n_event_headers_ + 1,
                      event_index + 1, 1, packet.GetPacketSize(), nullptr,
                      packet.GetDataPacket(), &anynull, &status)) {
        std::cerr << "Cannot read the " << packet_id << "th packet of "
                  << "the " << event_index << "th event "
                  << fitsutils::ErrorMessage(status);
    }
}


}}
