// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformDataPacket.h"
#include "sstcam/descriptions/Waveform.h"
#include "doctest.h"
#include <fstream>

namespace sstcam::descriptions {

TEST_CASE("Waveform") {
    std::string path = "../../share/sstcam/descriptions/waveform_data_packet_example.bin";
    size_t packet_size = 8276;
    std::ifstream file (path, std::ios::in | std::ios::binary);
    CHECK(file.is_open());
    WaveformDataPacket packet(packet_size);
    file.read(reinterpret_cast<char*>(packet.GetDataPacket()), packet_size);

    SUBCASE("Associate") {
        Waveform waveform{};
        CHECK(!waveform.IsAssociated());
        waveform.Associate(packet, 0);
        CHECK(waveform.IsAssociated());
    }

    SUBCASE("Getters") {
        Waveform waveform{};
        waveform.Associate(packet, 20);
        CHECK(!waveform.IsErrorFlagOn());
        CHECK(waveform.GetChannelID() == 4);
        CHECK(waveform.GetASICID() == 1);
        CHECK(waveform.GetNSamples() == 128);
        CHECK(!waveform.IsZeroSuppressed());
        CHECK(waveform.GetSample12bit(0) == 584);
        CHECK(waveform.GetSample12bit(10) == 582);
        CHECK(waveform.GetSample16bit(0) == 584);
        CHECK(waveform.GetSample16bit(10) == 8774);
        CHECK(waveform.GetPixelID() == 20);
    }

    SUBCASE("Setters") {
        Waveform waveform{};
        waveform.Associate(packet, 20);
        REQUIRE(waveform.GetSample12bit(0) != 4000);
        waveform.SetSample12bit(0, 4000);
        CHECK(waveform.GetSample12bit(0) == 4000);
        CHECK(waveform.GetSample16bit(0) == 4000);

        REQUIRE(waveform.GetSample16bit(0) != 65000);
        waveform.SetSample16bit(0, 65000);
        CHECK(waveform.GetSample12bit(0) != 65000); // Overflow
        CHECK(waveform.GetSample16bit(0) == 65000);
    }

    SUBCASE("Consistency") {
        Waveform waveform;
        waveform.Associate(packet, 0);
        uint16_t n_samples = waveform.GetNSamples();
        for (unsigned short iwav = 0; iwav < packet.GetNWaveforms(); iwav++) {
            waveform.Associate(packet, iwav);
            CHECK(waveform.GetNSamples() == n_samples);
        }
    }
}

}
