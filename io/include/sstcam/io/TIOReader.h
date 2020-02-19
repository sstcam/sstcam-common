// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_TIOREADER_H_
#define SSTCAM_IO_TIOREADER_H_

#include "sstcam/io/FitsUtils.h"
#include "sstcam/descriptions/WaveformDataPacket.h"
#include "sstcam/descriptions/Waveform.h"
#include "sstcam/descriptions/WaveformEvent.h"


namespace sstcam {
namespace io {

using WaveformDataPacket = sstcam::descriptions::WaveformDataPacket;
using Waveform = sstcam::descriptions::Waveform;
using WaveformEventR0 = sstcam::descriptions::WaveformEventR0;
using WaveformEventR1 = sstcam::descriptions::WaveformEventR1;
using time_point = std::chrono::time_point<std::chrono::system_clock>;


class TIOReader {
public:
    explicit TIOReader(const std::string& path);

    ~TIOReader() { Close(); }

    void Close();

    bool IsOpen() const { return fits_ != nullptr; }

    std::string GetPath() const;

    size_t GetNEvents() const { return n_events_; }

    size_t GetNPixels() const { return n_pixels_; }

    size_t GetNSamples() const { return n_samples_; }

    uint32_t GetRunID() const;

    bool IsR1() const;

    std::string GetCameraVersion() const;

    WaveformEventR0 GetEventR0(uint32_t event_index) const;

    WaveformEventR1 GetEventR1(uint32_t event_index) const;

    uint32_t GetEventID(uint32_t event_index) const;

    uint64_t GetEventTACK(uint32_t event_index) const;

    uint16_t GetEventNPacketsFilled(uint32_t event_index) const;

    int64_t GetEventCPUSecond(uint32_t event_index) const;

    int64_t GetEventCPUNanosecond(uint32_t event_index) const;


private:
    fitsfile* fits_;
    int32_t event_hdu_num_;
    uint8_t n_event_headers_;
    size_t n_packets_per_event_;
    size_t packet_size_;
    size_t n_events_;
    size_t n_pixels_;
    size_t n_samples_;
    uint8_t first_active_module_slot_;
    float scale_;
    float offset_;

    void MoveToEventHDU() const;

    std::shared_ptr<WaveformDataPacket> ReadPacket(
        uint32_t event_index, uint16_t packet_id) const;
};

}
}


#endif //SSTCAM_IO_TIOREADER_H_
