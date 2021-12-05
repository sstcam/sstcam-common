// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

//  #define PERFORM_CRC_CHECK

#ifndef SSTCAM_DESCRIPTIONS_WAVEFORMDATAPACKET_H
#define SSTCAM_DESCRIPTIONS_WAVEFORMDATAPACKET_H

#include <string>
#include <cstdint>
#include <iostream>
#include <memory>

namespace sstcam::descriptions {

constexpr uint8_t SAMPLES_PER_WAVEFORM_BLOCK = 32;
constexpr uint8_t PACKET_HEADER_WORDS = 8;
constexpr uint8_t PACKET_FOOTER_WORDS = 2;
constexpr uint8_t WAVEFORM_HEADER_WORDS = 1;

/*!
 * @class WaveformDataPacket
 * @brief Data packet for the waveform data from TARGET modules
 */
class WaveformDataPacket {
public:
    explicit WaveformDataPacket(size_t packet_size)
            : packet_(std::make_unique<uint8_t[]>(packet_size)),
              packet_size_(packet_size) {}

    ~WaveformDataPacket() = default;

    // Move constructor
    WaveformDataPacket(WaveformDataPacket&& a) noexcept
        : packet_size_(a.packet_size_)
    {
        packet_ = std::move(a.packet_);
        a.packet_ = nullptr;
    }

    // Move assignment
    WaveformDataPacket& operator=(WaveformDataPacket&& a) noexcept
    {
        // Self-assignment detection
        if (&a == this)
            return *this;

        // Transfer ownership
        packet_ = std::move(a.packet_);
        a.packet_ = nullptr;

        return *this;
    }

    // Get a reference to the packet_ pointer for filling.
    [[nodiscard]] inline uint8_t* GetDataPacket() const { return packet_.get(); }

    // Size of the packet.
    [[nodiscard]] inline size_t GetPacketSize() const { return packet_size_; }

    // Bit-shifting to extract values from packet_______________________________

    // Number of waveforms contained in the data packet.
    [[nodiscard]] inline uint16_t GetNWaveforms() const { return packet_[0] & 0x7Fu; }
    [[nodiscard]] inline uint8_t GetRegisterSetup() const {
        return static_cast<uint8_t>(packet_[0] >> 7u) & 0x1u;
    }

    [[nodiscard]] inline bool IsLastSubPacket() const { return packet_[1] & 0x1u; }
    [[nodiscard]] inline bool IsFirstSubPacket() const { return packet_[1] & 0x2u; }
    [[nodiscard]] inline uint16_t GetNBuffers() const {
        return static_cast<uint8_t>(packet_[1] >> 2u) & 0x3Fu ;
    }

    // TACK attached to the data packet (should be same for all packets in event).
    [[nodiscard]] inline uint64_t GetTACK() const{
        return (static_cast<uint64_t>(packet_[12]) << 56u) |
               (static_cast<uint64_t>(packet_[13]) << 48u) |
               (static_cast<uint64_t>(packet_[10]) << 40u) |
               (static_cast<uint64_t>(packet_[11]) << 32u) |
               (static_cast<uint64_t>(packet_[8]) << 24u) |
               (static_cast<uint64_t>(packet_[9]) << 16u) |
               (static_cast<uint64_t>(packet_[2]) << 8u) |
               (static_cast<uint64_t>(packet_[3]));
    }

    [[nodiscard]] inline uint16_t GetEventNumber() const { // TODO: What is this?
        return static_cast<uint16_t>(packet_[2] << 8u) | packet_[3];
    }

    // TODO: Technically "CTA ID", but is currently used for slot number.
    //  SCT Use "Detector ID" for slot number.
    //  Does usage/document need to be corrected?
    [[nodiscard]] inline uint8_t GetSlotID() const { return packet_[4]; }

    [[nodiscard]] inline uint8_t GetDetectorID() const { return packet_[5]; }

    [[nodiscard]] inline uint8_t GetEventSequenceNumber() const { return packet_[6]; }

    [[nodiscard]] inline uint8_t GetDetectorUniqueTag() const { return packet_[7]; }

    // ASIC storage array column ID for the first sample in this WaveformDataPacket.
    [[nodiscard]] inline uint8_t GetColumn() const { return packet_[14] & 0x3Fu; }
    [[nodiscard]] inline uint8_t GetStaleBit() const {
        return static_cast<uint8_t>(packet_[14] >> 6u) & 0x1u;
    }
    [[nodiscard]] inline bool IsZeroSupressionEnabled() const {
        return packet_[14] & 0x80u;
    }

    // ASIC storage array row ID for the first sample in this WaveformDataPacket.
    [[nodiscard]] inline uint8_t GetRow() const {
        return static_cast<uint8_t>(packet_[15] >> 5u) & 0x7u;
    }
    // ASIC storage array block phase for the first sample in this WaveformDataPacket.
    [[nodiscard]] inline uint8_t GetBlockPhase() const { return packet_[15] & 0x1Fu; }

    // CRC (Cyclic Redundancy Check) code attached to this packet.
    [[nodiscard]] inline uint16_t GetCRC() const {
        uint16_t first_part = static_cast<uint16_t>(packet_[packet_size_ - 4]) << 8u;
        uint16_t second_part = packet_[packet_size_ - 3];
        return first_part | second_part;
    }

    [[nodiscard]] inline uint16_t GetMBZ() const {
        uint16_t first_part = static_cast<uint16_t>(packet_[packet_size_ - 2]) << 6u;
        uint16_t second_part = static_cast<uint16_t>(packet_[packet_size_-1]>>2u) & 0x3Fu;
        return first_part | second_part;
    }

    [[nodiscard]] inline bool IsTimeout() const {
        return packet_[packet_size_ - 1] & 0x1u;
    }
    [[nodiscard]] inline bool IsError() const {
        return packet_[packet_size_ - 1] & 0x2u;
    }

    // Utility functions________________________________________________________

    // TODO: Replace usage with TACK? See https://github.com/sstcam/sstcam-common/issues/5
    [[nodiscard]] inline uint32_t GetEventID() const {
        return static_cast<uint32_t>(GetTACK() / 100);
    }

    [[nodiscard]] inline uint16_t GetWaveformSamplesNBlocks() const {
        return (GetNBuffers() / 2);
    }

    // Size (bytes) of entire waveform (samples only).
    [[nodiscard]] inline uint16_t GetWaveformSamplesNBytes() const {
        return (GetNBuffers() * SAMPLES_PER_WAVEFORM_BLOCK);
    }

    // Number of samples in the waveforms of this WaveformDataPacket.
    [[nodiscard]] inline uint16_t GetWaveformNSamples() const {
        return (GetWaveformSamplesNBytes() / 2);
    }

    // Size (bytes) of entire waveform (waveform header + samples).
    [[nodiscard]] inline uint16_t GetWaveformNBytes() const {
        return GetWaveformSamplesNBytes() + WAVEFORM_HEADER_WORDS*2;
    }

    // Full size (bytes) of this WaveformDataPacket.
    [[nodiscard]] inline uint16_t GetPacketNBytes() const {
        return GetNWaveforms() *
               (GetNBuffers() * SAMPLES_PER_WAVEFORM_BLOCK + 2) +
               2 * (PACKET_HEADER_WORDS + PACKET_FOOTER_WORDS);
    }

    // Get the index within the byte array where a waveform begins.
    [[nodiscard]] inline uint16_t GetWaveformStart(uint16_t waveform_index) const {
      return 2 * PACKET_HEADER_WORDS + waveform_index * GetWaveformNBytes();
    }

    // Calculate the size (bytes) of a WaveformDataPacket.
    static uint16_t CalculatePacketSizeBytes(uint16_t n_samples_per_waveform,
            uint16_t n_waveforms_per_packet);

    // Calculate the ASIC storage array cell id of a sample.
    static uint16_t CalculateCellID(uint16_t row, uint16_t column, uint16_t blockphase);

    // ASIC storage array cell ID for the first sample in this WaveformDataPacket.
    [[nodiscard]] uint16_t GetFirstCellID() const {
        return CalculateCellID(GetRow(), GetColumn(), GetBlockPhase());
    }

    // Calculate the position within the ASIC storage array for a sample.
    static void CalculateRowColumnBlockPhase(uint16_t cell_id,
            uint16_t& row, uint16_t& column, uint16_t& blockphase);

    // Check if the packet is valid / consistent.
    [[nodiscard]] bool IsValid() const;

    // Check if the packet is empty by checking if its contents are all zero.
    [[nodiscard]] bool IsEmpty() const {
        if (packet_) {
            for (size_t i = 0; i < packet_size_; i++) {
                if (packet_[i] != 0) return false;
            }
        }
        return true;
    }

private:
    std::unique_ptr<uint8_t[]> packet_; // Memory owned by class
    size_t packet_size_;

};
}


#endif //SSTCAM_DESCRIPTIONS_WAVEFORMDATAPACKET_H
