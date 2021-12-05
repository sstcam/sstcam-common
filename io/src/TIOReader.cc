// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include <utility>
#include <iostream>
#include <cinttypes>
#include <memory>
#include <cstring>

namespace sstcam::io {

TIOReader::TIOReader(const std::string& path)
    : fits_(nullptr),
      event_hdu_num_(0),
      n_event_headers_(0),
      n_packets_per_event_(0),
      packet_size_(0),
      n_events_(0),
      n_pixels_(0),
      n_samples_(0),
      first_active_module_slot_(0),
      scale_(1.),
      offset_(0.)
{
    // Open fits file
    int status = 0;
    if (fits_open_file(&fits_, path.c_str(), READONLY, &status)) {
        std::ostringstream ss;
        ss << "Cannot open: " << path << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    // Get EventHeaderVersion
    std::string key = "EVENT_HEADER_VERSION";
    uint16_t version = fitsutils::GetHeaderKeyValue<int16_t, TINT>(fits_, key);
    if (version < 1) {
        Close();
        std::ostringstream ss;
        ss << "Incompatible EVENT_HEADER_VERSION: " << version;
        throw std::runtime_error(ss.str());
    }

    // Move pointer to EVENTS HDU
    if (fits_movnam_hdu(fits_, BINARY_TBL, const_cast<char *>("EVENTS"), 0, &status)) {
        Close();
        std::ostringstream ss;
        ss << "Cannot move to the HDU: EVENTS " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    fits_get_hdu_num(fits_, &event_hdu_num_);

    char comment[FLEN_COMMENT];
    char value[FLEN_VALUE];
    char tform[FLEN_KEYWORD];

    // Get Number of columns in EVENTS HDU
    long n_event_columns;
    if (fits_read_key_lng(fits_, "TFIELDS", &n_event_columns, comment, &status)) {
        Close();
        std::ostringstream ss;
        ss << "Cannot read TFIELDS " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }

    // Find first packet column (and therefore calculate the number of headers)
    for (int64_t i = 0; i < n_event_columns && i < 256; ++i) {
        snprintf(tform, FLEN_KEYWORD, "TTYPE%" PRIi64, i + 1);
        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
            Close();
            std::ostringstream ss;
            ss << "Cannot read TTYPE " << i + 1 << fitsutils::ErrorMessage(status);
            throw std::runtime_error(ss.str());
        } else {
            // Found the first packet column
            if (strcmp(value, "EVENT_PACKET_0") == 0) {
                n_event_headers_ = static_cast<uint8_t>(i);
                break;
            }
        }
    }

    // The number of packets in an event is the total number of columns minus
    // the number of header columns
    n_packets_per_event_ = static_cast<size_t>(n_event_columns - n_event_headers_);

    // Obtain the size of a waveform packet, and check that all waveform packet
    // columns share this packet size
    status = 0;
    comment[0] = 0;
    value[0] = 0;
    tform[0] = 0;
    for (int i = n_event_headers_; i < n_event_columns; ++i) {
        snprintf(tform, FLEN_KEYWORD, "TFORM%d", i + 1);
        if (fits_read_key_str(fits_, tform, value, comment, &status)) {
            Close();
            std::ostringstream ss;
            ss << "Cannot read TFORM " << i + 1 << fitsutils::ErrorMessage(status);
            throw std::runtime_error(ss.str());
        }

        uint16_t packet_size_i;
        if (sscanf(value, "%" SCNu16 "B", &packet_size_i) == 1) {
            if (packet_size_ == 0) {
                packet_size_ = packet_size_i;
            }
            else if (packet_size_i != packet_size_) {
                Close();
                std::ostringstream ss;
                ss << "Expected value of TFORM " << i << " is "
                << packet_size_ << " but it is " << value;
                throw std::runtime_error(ss.str());
            }
        } else {
            Close();
            std::ostringstream ss;
            ss << "Expected value of TFORM " << i << " is xxxB, but it is " << value;
            throw std::runtime_error(ss.str());
        }
    }

    // Get number of events in file
    n_events_ = static_cast<size_t>(fitsutils::GetNRows(fits_, event_hdu_num_));

    // Obtain the modules that were active and obtain the hardcoded module situation
    for (uint32_t ipack=0; ipack < n_packets_per_event_; ipack++) {
        active_modules_.insert(ReadPacket(0, ipack)->GetSlotID());
    }
    sstcam::descriptions::GetHardcodedModuleSituation(
        active_modules_, n_pixels_, first_active_module_slot_);

    // Get n_samples from first packet
    n_samples_ = ReadPacket(0, 0)->GetWaveformNSamples();

    // Get scale and offset to return to float from uint16
    if (IsR1()) {
        scale_ = fitsutils::GetHeaderKeyValue<float, TFLOAT>(fits_, "SCALE");
        offset_ = fitsutils::GetHeaderKeyValue<float, TFLOAT>(fits_, "OFFSET");
    }
}

void TIOReader::Close() {
    if (!IsOpen()) return;

    int status = 0;
    if (fits_close_file(fits_, &status)) {
        std::ostringstream ss;
        ss << "Cannot close the file " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    fits_ = nullptr;
}

uint32_t TIOReader::GetRunID() const {
    return fitsutils::GetHeaderKeyValue<int32_t, TINT>(fits_, "RUNNUMBER");
}

bool TIOReader::IsR1() const {
    if (fitsutils::HasHeaderKey(fits_, "R1")) {
        return fitsutils::GetHeaderKeyValue<bool, TLOGICAL>(fits_, "R1");
    } else {
        return false;
    }
}

std::string TIOReader::GetCameraVersion() const {
    if (fitsutils::HasHeaderKey(fits_, "CAMERAVERSION")) {
        auto camera_version = fitsutils::GetHeaderKeyValue<
            std::string, TSTRING>(fits_, "CAMERAVERSION");
        return camera_version;
    } else {
        return "1.1.0"; // Default = CHEC-S;
    }
}

uint32_t TIOReader::GetEventID(size_t event_index) const {
    MoveToEventHDU();
    uint32_t event_id;
    int status = 0;
    fits_read_col(fits_, TUINT, 1, event_index + 1, 1, 1, nullptr,
                  &event_id, nullptr, &status);
    if (status != 0) {
        std::ostringstream ss;
        ss << "Error reading Event ID from file " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return event_id;
}

uint64_t TIOReader::GetEventTACK(size_t event_index) const {
    MoveToEventHDU();
    uint32_t tack32msb, tack32lsb;
    int status = 0;
    fits_read_col(fits_, TUINT, 2, event_index + 1, 1, 1, nullptr,
                  &tack32msb, nullptr, &status);
    fits_read_col(fits_, TUINT, 3, event_index + 1, 1, 1, nullptr,
                  &tack32lsb, nullptr, &status);
    if (status != 0) {
        std::ostringstream ss;
        ss << "Error reading event TACK from file " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return (static_cast<uint64_t>(tack32msb) << 32u) | static_cast<uint64_t>(tack32lsb);
}

uint16_t TIOReader::GetEventNPacketsFilled(size_t event_index) const{
    MoveToEventHDU();
    int status = 0;
    uint16_t n_filled = 0;
    fits_read_col(fits_, TUSHORT, 4, event_index + 1, 1, 1, nullptr,
                  &n_filled, nullptr, &status);
    if (status != 0) {
        std::ostringstream ss;
        ss << "Error reading Event 'Number of Packets Filled' from file "
            << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return n_filled;
}

int64_t TIOReader::GetEventCPUSecond(size_t event_index) const {
    MoveToEventHDU();
    int status = 0;
    int64_t cpu_s = 0;
    fits_read_col(fits_, TLONGLONG, 5, event_index + 1, 1, 1, nullptr,
                  &cpu_s, nullptr, &status);
    if (status != 0) {
        std::ostringstream ss;
        ss << "Error reading event CPU timestamp (seconds) from file "
           << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return cpu_s;
}

int64_t TIOReader::GetEventCPUNanosecond(size_t event_index) const {
    MoveToEventHDU();
    int status = 0;
    int64_t cpu_ns = 0;
    fits_read_col(fits_, TLONGLONG, 6, event_index + 1, 1, 1, nullptr,
                  &cpu_ns, nullptr, &status);
    if (status != 0) {
        std::ostringstream ss;
        ss << "Error reading event CPU timestamp (nanosecond) from file "
           << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return cpu_ns;
}

void TIOReader::MoveToEventHDU() const {
    if (!IsOpen()) throw std::runtime_error("File is not open");

    int status = 0;
    int hdutype = BINARY_TBL;
    if (fits_movabs_hdu(fits_, event_hdu_num_, &hdutype, &status)) {
        std::ostringstream ss;
        ss << "Cannot move to the event HDU " << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
}

std::shared_ptr<WaveformDataPacket> TIOReader::ReadPacket(
        size_t event_index, uint16_t packet_id) const {
    auto packet = std::make_shared<WaveformDataPacket>(packet_size_);

    MoveToEventHDU();

    int status = 0;
    int anynull;
    if (fits_read_col(fits_, TBYTE, packet_id + n_event_headers_ + 1,
                      event_index + 1, 1, packet->GetPacketSize(), nullptr,
                      packet->GetDataPacket(), &anynull, &status)) {
        std::ostringstream ss;
        ss << "Cannot read the " << packet_id << "th packet of "
           << "the " << event_index << "th event "
           << fitsutils::ErrorMessage(status);
        throw std::runtime_error(ss.str());
    }
    return packet;
}

}
