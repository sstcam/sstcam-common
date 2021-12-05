// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_TIOREADER_H_
#define SSTCAM_IO_TIOREADER_H_

#include "sstcam/io/FitsUtils.h"
#include "sstcam/descriptions/WaveformDataPacket.h"
#include "sstcam/descriptions/Waveform.h"
#include "sstcam/descriptions/WaveformEvent.h"
#include <set>
#include <ostream>
#include <variant>


namespace sstcam::io {

using WaveformDataPacket = sstcam::descriptions::WaveformDataPacket;
using WaveformEvent = sstcam::descriptions::WaveformEvent;
using WaveformEventR0 = sstcam::descriptions::WaveformEventR0;
using WaveformEventR1 = sstcam::descriptions::WaveformEventR1;


/*!
 * @class TIOReader
 * @brief Reader for the TIO file format containing waveforms from the SST camera
 */
class TIOReader {
public:
    explicit TIOReader(const std::string& path);
    ~TIOReader() { Close(); }

    // Close the file.
    void Close();

    // Is the file open?
    [[nodiscard]] inline bool IsOpen() const { return fits_ != nullptr; }

    // Path to the file if it is open.
    [[nodiscard]] inline std::string GetPath() const {
        return fits_ ? std::string(fits_->Fptr->filename) : "";
    }

    // Compression scale for the R1 waveform samples.
    [[nodiscard]] inline float GetScale() const { return scale_; }

    // Compression offset for the R1 waveform samples.
    [[nodiscard]] inline float GetOffset() const { return offset_; }

    // Number of events in the file.
    [[nodiscard]] inline size_t GetNEvents() const { return n_events_; }

    // Number of camera pixels per event.
    [[nodiscard]] inline size_t GetNPixels() const { return n_pixels_; }

    // Number of samples for pixel waveform.
    [[nodiscard]] inline size_t GetNSamples() const { return n_samples_; }

    // Number of WaveformEventPackets per event.
    [[nodiscard]] inline size_t GetNPacketsPerEvent() const {
        return n_packets_per_event_;
    }

    // Get the lowest module slot ID among the modules that are active in the event.
    // This value is used to offset the slot ID of a waveform during waveform
    // sample array filling, in cases where there are only 1 module active.
    [[nodiscard]] inline size_t GetFirstActiveModuleSlot() const {
        return first_active_module_slot_;
    }

    // The IDs of active module present in the file.
    [[nodiscard]] inline std::set<uint8_t> GetActiveModules() const {
        return active_modules_;
    }

    // Run ID. Obtained from the file header.
    [[nodiscard]] uint32_t GetRunID() const;

    // Has waveform calibration (R1) already been applied to the waveform
    // samples? Obtained from the file header.
    [[nodiscard]] bool IsR1() const;

    // Version of the camera from the file header.
    [[nodiscard]] std::string GetCameraVersion() const;

    // Obtain event ID for a particular event index from the event header.
    [[nodiscard]] uint32_t GetEventID(size_t event_index) const;

    // Obtain TACK for a particular event index from the event header.
    [[nodiscard]] uint64_t GetEventTACK(size_t event_index) const;

    // Obtain number of packets filled for a particular event from the event header.
    [[nodiscard]] uint16_t GetEventNPacketsFilled(size_t event_index) const;

    // Obtain CPU timestamp (seconds) for a particular event from the event header.
    [[nodiscard]] int64_t GetEventCPUSecond(size_t event_index) const;

    // Obtain CPU timestamp (nanoseconds) for a particular event from the event header.
    [[nodiscard]] int64_t GetEventCPUNanosecond(size_t event_index) const;

    // Obtain an event (pre-waveform calibration formatted).
    [[nodiscard]] inline WaveformEventR0 GetEventR0(size_t event_index) const {
        return GetEvent<WaveformEventR0>(event_index);
    }

    // Obtain an event (post-waveform calibration formatted).
    [[nodiscard]] inline WaveformEventR1 GetEventR1(size_t event_index) const {
        return GetEvent<WaveformEventR1>(event_index);
    }

private:
    fitsfile* fits_;
    int32_t event_hdu_num_;
    uint8_t n_event_headers_;
    size_t n_packets_per_event_;
    size_t packet_size_;
    size_t n_events_;
    size_t n_pixels_;
    size_t n_samples_;
    std::set<uint8_t> active_modules_;
    uint8_t first_active_module_slot_;
    float scale_;
    float offset_;

    void MoveToEventHDU() const;

    // Read a WaveformDataPacket from the file.
    [[nodiscard]] std::shared_ptr<WaveformDataPacket> ReadPacket(
        size_t event_index, uint16_t packet_id) const;

    // Template to define how events are read from the file.
    template<typename TWaveformEvent>
    [[nodiscard]] inline TWaveformEvent GetEvent(size_t event_index) const {
        if (event_index >= GetNEvents())
            throw std::runtime_error("Event index out of range");

        int64_t cpu_time_second = GetEventCPUSecond(event_index);
        int64_t cpu_time_nanosecond = GetEventCPUNanosecond(event_index);
        TWaveformEvent event(
            n_packets_per_event_,
            n_pixels_, first_active_module_slot_,
            cpu_time_second, cpu_time_nanosecond, scale_, offset_, event_index);
        for (uint32_t ipack = 0; ipack < n_packets_per_event_; ipack++) {
            event.AddPacketShared(ReadPacket(event_index, ipack));
        }
        return event;
    }
};

}

#endif //SSTCAM_IO_TIOREADER_H_
