// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/Waveform.h"

namespace sstcam {
namespace descriptions {

uint16_t Waveform::GetSample12bit(uint16_t sample_index) const {
    uint16_t first_part = static_cast<uint16_t>(
            waveform_[2 + 2 * sample_index] & 0xFu
    ) << 8u;
    uint16_t second_part = waveform_[3 + 2 * sample_index];
    return first_part | second_part;
}

uint16_t Waveform::GetSample16bit(uint16_t sample_index) const {
    uint16_t first_part = static_cast<uint16_t>(
            waveform_[2 + 2 * sample_index]
    ) << 8u;
    uint16_t second_part = waveform_[3 + 2 * sample_index];
    return first_part | second_part;
}

void Waveform::SetSample12bit(uint16_t sample_index, uint16_t value) {
    waveform_[2 + 2 * sample_index] = static_cast<uint8_t>(value >> 8u) & 0xFu;
    waveform_[3 + 2 * sample_index] = (value & 0xFFu);
}

void Waveform::SetSample16bit(uint16_t sample_index, uint16_t value) {
    waveform_[2 + 2 * sample_index] = static_cast<uint8_t>(value >> 8u);
    waveform_[3 + 2 * sample_index] = (value & 0xFFu);
}

}  // namespace descriptions
}  // namespace sstcam
