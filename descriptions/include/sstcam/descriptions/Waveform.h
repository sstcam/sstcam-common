// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_DESCRIPTIONS_WAVEFORM_H
#define SSTCAM_DESCRIPTIONS_WAVEFORM_H

#include "sstcam/descriptions/WaveformDataPacket.h"
#include <cstdint>
#include <iostream>
#include <ostream>

namespace sstcam::descriptions {

constexpr uint8_t CHANNELS_PER_ASIC = 16;

/*!
 * @class Waveform
 * @brief Container for a waveform segment of the WaveformDataPacket to
 * provide convenience access methods for waveform contents.
 */
class Waveform {
public:
    Waveform() = default;
    ~Waveform() = default;

    // Associate this Waveform with a WaveformDataPacket.
    void Associate(const WaveformDataPacket* packet, uint16_t waveform_index) {
        uint16_t waveform_start = packet->GetWaveformStart(waveform_index);
        waveform_ = &(packet->GetDataPacket()[waveform_start]);
    }

    // Associate this Waveform with a WaveformDataPacket.
    void Associate(const WaveformDataPacket& packet, uint16_t waveform_index) {
        Associate(&packet, waveform_index);
    }

    // Is this Waveform associated with a WaveformDataPacket?
    [[nodiscard]] inline bool IsAssociated() const { return waveform_; }

    [[nodiscard]] inline bool IsErrorFlagOn() const {
        return waveform_[0] & 0x1u;
    }

    // Channel ID (i.e. pixel within the ASIC).
    [[nodiscard]] inline uint8_t GetChannelID() const {
        return static_cast<uint8_t>(waveform_[0] >> 1u) & 0xFu;
    }

    // ASIC ID.
    [[nodiscard]] inline uint8_t GetASICID() const {
        return static_cast<uint8_t>(waveform_[0] >> 5u) & 0x3u;
    }

    // Number of waveform samples.
    [[nodiscard]] inline uint16_t GetNSamples() const {
        return 16 * (waveform_[1] & 0x3Fu);
    }
    [[nodiscard]] inline bool IsZeroSuppressed() const {
        return waveform_[1] & 0x80u;
    }

    // Get the value of a sample (12bit - R0 file).
    [[nodiscard]] inline uint16_t GetSample12bit(uint16_t sample_index) const {
        uint16_t first_part = static_cast<uint16_t>(
            waveform_[2 + 2 * sample_index] & 0xFu
        ) << 8u;
        uint16_t second_part = waveform_[3 + 2 * sample_index];
        return first_part | second_part;
    }

    // Get the value of a sample (16bit - R1 file).
    [[nodiscard]] inline uint16_t GetSample16bit(uint16_t sample_index) const {
        uint16_t first_part = static_cast<uint16_t>(
                waveform_[2 + 2 * sample_index]
        ) << 8u;
        uint16_t second_part = waveform_[3 + 2 * sample_index];
        return first_part | second_part;
    };

    // Set the value of a sample (12bit - R0 file).
    inline void SetSample12bit(uint16_t sample_index, uint16_t value) {
        waveform_[2 + 2 * sample_index] = static_cast<uint8_t>(value >> 8u) & 0xFu;
        waveform_[3 + 2 * sample_index] = (value & 0xFFu);
    }

    // Set the value of a sample (16bit - R1 file) (Uses the unused 4 bits).
    inline void SetSample16bit(uint16_t sample_index, uint16_t value) {
        waveform_[2 + 2 * sample_index] = static_cast<uint8_t>(value >> 8u);
        waveform_[3 + 2 * sample_index] = (value & 0xFFu);
    }

    // Calculate the pixel ID within the TARGET module.
    [[nodiscard]] inline uint16_t GetPixelID() const {
        return GetASICID() * CHANNELS_PER_ASIC + GetChannelID();
    }

    friend std::ostream& operator<<(std::ostream& os, const Waveform& waveform);

private:
    uint8_t* waveform_ = nullptr; // Memory owned by WaveformDataPacket.
};

}

#endif //SSTCAM_DESCRIPTIONS_WAVEFORM_H
