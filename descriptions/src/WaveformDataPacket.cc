// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformDataPacket.h"

namespace sstcam {
namespace descriptions {

uint64_t WaveformDataPacket::GetTACK() const {
    return (static_cast<uint64_t>(packet_[12]) << 56u) |
           (static_cast<uint64_t>(packet_[13]) << 48u) |
           (static_cast<uint64_t>(packet_[10]) << 40u) |
           (static_cast<uint64_t>(packet_[11]) << 32u) |
           (static_cast<uint64_t>(packet_[8]) << 24u) |
           (static_cast<uint64_t>(packet_[9]) << 16u) |
           (static_cast<uint64_t>(packet_[2]) << 8u) |
           (static_cast<uint64_t>(packet_[3]));
}

uint16_t WaveformDataPacket::CalculatePacketSizeBytes(
        uint16_t n_samples_per_waveform, uint16_t n_waveforms_per_packet) {
    uint16_t n_buffers = 2 * (n_samples_per_waveform / n_waveforms_per_packet);
    return n_waveforms_per_packet *
           (n_buffers * SAMPLES_PER_WAVEFORM_BLOCK + 2) +
           2 * (PACKET_HEADER_WORDS + PACKET_FOOTER_WORDS);
}

uint16_t WaveformDataPacket::CalculateCellID(
        uint16_t row, uint16_t column, uint16_t blockphase) {
    return (column * 8u + row) * SAMPLES_PER_WAVEFORM_BLOCK + blockphase;
}

void WaveformDataPacket::CalculateRowColumnBlockPhase(const uint16_t cell_id,
        uint16_t& row, uint16_t& column, uint16_t& blockphase) {
    blockphase = (cell_id % SAMPLES_PER_WAVEFORM_BLOCK);
    row = (cell_id / SAMPLES_PER_WAVEFORM_BLOCK) % 8u;
    column = (cell_id / SAMPLES_PER_WAVEFORM_BLOCK) / 8u;
}

bool WaveformDataPacket::IsValid() const {
    if (packet_size_ == 0) return false;
    if (packet_size_ != GetPacketNBytes()) return false;

#ifdef PERFORM_CRC_CHECK
    uint16_t crc = GetCRC();
    if (crc != Util::CRC(fPacketSize - 4, fData)) return False;  // TODO: CRC calculation
#endif

    if (GetMBZ() != 0) return false;
    if (IsTimeout()) return false;
    if (IsError()) return false;

    return true;
}


}  // namespace descriptions
}  // namespace sstcam
