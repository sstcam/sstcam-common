// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformDataPacket.h"
#include "doctest.h"
#include <fstream>

namespace sstcam::descriptions {

TEST_CASE("WaveformDataPacket") {
    std::string path = "../../share/sstcam/descriptions/waveform_data_packet_example.bin";
    size_t packet_size = 8276;
    std::ifstream file (path, std::ios::in | std::ios::binary);
    CHECK(file.is_open());

    SUBCASE("Generic uint8_t* packet") {
        auto* raw_pointer = new uint8_t[packet_size]();
        REQUIRE(raw_pointer[0] == 0);
        file.read(reinterpret_cast<char*>(raw_pointer), packet_size);
        CHECK(raw_pointer[0] == 32);
    }

    SUBCASE("Constructor") {
        WaveformDataPacket packet(packet_size);
        CHECK(packet.GetPacketSize() == packet_size);
        REQUIRE(packet.GetDataPacket()[0] == 0);
        CHECK(packet.IsEmpty());
        file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);
        CHECK(packet.GetDataPacket()[0] == 32);
        CHECK(!packet.IsEmpty());
    }

    SUBCASE("Getters") {
        WaveformDataPacket packet(packet_size);
        file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);
        CHECK(packet.GetNWaveforms() == 32);
        CHECK(packet.GetRegisterSetup() == 0);
        CHECK(!packet.IsLastSubPacket());
        CHECK(packet.IsFirstSubPacket());
        CHECK(packet.GetNBuffers() == 8);
        CHECK(packet.GetTACK() == 2165717354592);
        CHECK(packet.GetEventNumber() == 18528);
        CHECK(packet.GetSlotID() == 22);
        CHECK(packet.GetDetectorID() == 22);
        CHECK(packet.GetEventSequenceNumber() == 236);
        CHECK(packet.GetDetectorUniqueTag() == 160);
        CHECK(packet.GetColumn() == 5);
        CHECK(packet.GetStaleBit() == 0);
        CHECK(packet.IsZeroSupressionEnabled() == 0);
        CHECK(packet.GetRow() == 5);
        CHECK(packet.GetBlockPhase() == 8);
        CHECK(packet.GetCRC() == 38078);
        CHECK(packet.GetMBZ() == 0);
        CHECK(!packet.IsTimeout());
        CHECK(!packet.IsError());

        CHECK(packet.GetWaveformSamplesNBlocks() == 4);
        CHECK(packet.GetWaveformSamplesNBytes() == 256);
        CHECK(packet.GetWaveformNSamples() == 128);
        CHECK(packet.GetWaveformNBytes() == 258);
        CHECK(packet.GetPacketNBytes() == 8276);
    }

    SUBCASE("Utility") {
        WaveformDataPacket packet(packet_size);
        file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);
        CHECK(WaveformDataPacket::CalculatePacketSizeBytes(
                packet.GetWaveformNSamples(), packet.GetNWaveforms()
        ) == packet.GetPacketNBytes());

        uint16_t row_0 = 2, column_0 = 1, blockphase_0 = 12;
        uint16_t cell_id = WaveformDataPacket::CalculateCellID(
            row_0, column_0, blockphase_0);
        CHECK(cell_id == 332);
        uint16_t row = 0, column = 0, blockphase = 0;
        WaveformDataPacket::CalculateRowColumnBlockPhase(
            cell_id, row, column, blockphase);
        CHECK(row_0 == row);
        CHECK(column_0 == column);
        CHECK(blockphase_0 == blockphase);

        CHECK(packet.GetFirstCellID() == packet.CalculateCellID(5, 5, 8));
        CHECK(packet.GetFirstCellID() == 1448);
    }

    SUBCASE("Valid") {
        WaveformDataPacket data_packet_0(0);
        CHECK(!data_packet_0.IsValid());

        WaveformDataPacket packet(packet_size);
        CHECK(!packet.IsValid());

        file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);
        CHECK(packet.IsValid());
    }

    SUBCASE("Waveform Start") {
        WaveformDataPacket packet(packet_size);
        file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);

        uint16_t wf_start_0 = packet.GetWaveformStart(0);
        uint16_t wf_start_1 = packet.GetWaveformStart(1);
        uint8_t* wf_pointer_0 = &packet.GetDataPacket()[wf_start_0];
        uint8_t* wf_pointer_1 = &packet.GetDataPacket()[wf_start_1];
        uint16_t last_wf = packet.GetNWaveforms() - 1;
        CHECK(wf_pointer_1 == &wf_pointer_0[packet.GetWaveformNBytes()]);

        uint16_t wf_start_last = packet.GetWaveformStart(last_wf);
        uint8_t* wf_pointer_last = &packet.GetDataPacket()[wf_start_last];
        size_t end_index_0 = packet.GetWaveformNBytes() + PACKET_FOOTER_WORDS * 2 - 1;
        uint8_t* end_pointer_0 = &wf_pointer_last[end_index_0];
        uint8_t* end_pointer_1 = &packet.GetDataPacket()[packet.GetPacketNBytes() - 1];
        CHECK(end_pointer_0 == end_pointer_1);
    }
}

}
