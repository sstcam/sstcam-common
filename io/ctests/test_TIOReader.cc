// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include "sstcam/descriptions/WaveformEvent.h"
#include "doctest.h"
#include <fstream>

namespace sstcam::io {

using WaveformEventR0 = sstcam::descriptions::WaveformEventR0;
using WaveformEventR1 = sstcam::descriptions::WaveformEventR1;


TEST_CASE("TIOReader") {
    std::string path_tm_r0 = "../../share/sstcam/io/targetmodule_r0.tio";
    std::string path_tm_r1 = "../../share/sstcam/io/targetmodule_r1.tio";
    std::string path_camera_r1 = "../../share/sstcam/io/chec_r1.tio";

    std::ifstream file_tm_r0 (path_tm_r0);
    REQUIRE(file_tm_r0.good());
    file_tm_r0.close();

    std::ifstream file_rm_r1 (path_tm_r1);
    REQUIRE(file_rm_r1.good());
    file_rm_r1.close();

    std::ifstream file_camera_r1 (path_camera_r1);
    REQUIRE(file_camera_r1.good());
    file_camera_r1.close();

    SUBCASE("TIOReader Constructor") {
        auto reader = TIOReader(path_tm_r0);
        CHECK(reader.IsOpen());
        reader.Close();
        CHECK(!reader.IsOpen());
    }

    SUBCASE("TIOReader missing file") {
        CHECK_THROWS(TIOReader("/not/a/file.fits"));
    }

    auto reader_tm_r0 = TIOReader(path_tm_r0);
    auto reader_tm_r1 = TIOReader(path_tm_r1);
    auto reader_camera_r1 = TIOReader(path_camera_r1);

    SUBCASE("TIOReader Run-wise Getters") {
        CHECK(reader_tm_r0.GetPath() == path_tm_r0);
        CHECK(reader_tm_r0.GetNEvents() == 8);
        CHECK(reader_tm_r0.GetNPixels() == 64);
        CHECK(reader_tm_r0.GetNSamples() == 128);
        CHECK(reader_tm_r0.GetRunID() == 15639);
        CHECK(!reader_tm_r0.IsR1());
        CHECK(reader_tm_r0.GetCameraVersion() == "1.1.0");

        CHECK(reader_camera_r1.IsR1());
        CHECK(reader_camera_r1.GetNPixels() == 2048);
    }

    SUBCASE("TIOReader GetEventR0") {
        WaveformEventR0 event0 = reader_tm_r0.GetEventR0(0);
        CHECK(event0.GetIndex() == 0);

        // Check waveform size
        size_t n_pixels = reader_tm_r0.GetNPixels();
        size_t n_samples = reader_tm_r0.GetNSamples();
        std::vector<uint16_t> waveforms_ev0 = event0.GetWaveformSamplesVector();
        CHECK(waveforms_ev0.size() == n_pixels * n_samples);

        // Check first event is not overwritten when reading new event
        WaveformEventR0 event1 = reader_tm_r0.GetEventR0(1);
        CHECK(event1.GetIndex() == 1);
        CHECK(event1.GetPackets() == event1.GetPackets());
        CHECK(event1.GetPackets() != event0.GetPackets());
    }

    SUBCASE("TIOReader GetEventR1") {
        WaveformEventR1 event0 = reader_tm_r1.GetEventR1(0);
        CHECK(event0.GetIndex() == 0);

        // Check waveform size
        size_t n_pixels = reader_tm_r1.GetNPixels();
        size_t n_samples = reader_tm_r1.GetNSamples();
        std::vector<float> waveforms_ev0 = event0.GetWaveformSamplesVector();
        CHECK(waveforms_ev0.size() == n_pixels * n_samples);

        // Check first event is not overwritten when reading new event
        WaveformEventR1 event1 = reader_tm_r1.GetEventR1(1);
        CHECK(event1.GetIndex() == 1);
        CHECK(event1.GetPackets() == event1.GetPackets());
        CHECK(event1.GetPackets() != event0.GetPackets());
    }

    SUBCASE("TIOReader Event-wise Getters") {
        CHECK(reader_tm_r0.GetEventID(0) == 1314276426);
        CHECK(reader_tm_r0.GetEventTACK(0) == 4426394938696);
        CHECK(reader_tm_r0.GetEventNPacketsFilled(0) == 64);
        CHECK(reader_tm_r0.GetEventCPUSecond(0) == 1510245014);
        CHECK(reader_tm_r0.GetEventCPUNanosecond(0) == 506765167);

        CHECK(reader_tm_r0.GetEventID(5) == 1314376427);
        CHECK(reader_tm_r0.GetEventTACK(5) == 4426404938700);
        CHECK(reader_tm_r0.GetEventNPacketsFilled(5) == 64);
        CHECK(reader_tm_r0.GetEventCPUSecond(5) == 1510245014);
        CHECK(reader_tm_r0.GetEventCPUNanosecond(5) == 516798324);
    }

    SUBCASE("TIOReader Event-wise TACK matches packets") {
        uint32_t event_index_0 = 0;
        uint64_t expected_tack_0 = reader_camera_r1.GetEventTACK(event_index_0);
        auto event_0 = reader_camera_r1.GetEventR1(event_index_0);
        bool matches_0 = true;
        for (auto& packet : event_0.GetPackets()) {
            if (packet->GetTACK() != expected_tack_0) matches_0 = false;
        }
        CHECK(matches_0);

        uint32_t event_index_5 = 5;
        uint64_t expected_tack_5 = reader_camera_r1.GetEventTACK(event_index_5);
        auto event_5 = reader_camera_r1.GetEventR1(event_index_5);
        bool matches_5 = true;
        for (auto& packet : event_5.GetPackets()) {
            if (packet->GetTACK() != expected_tack_5) matches_5 = false;
        }
        CHECK(matches_5);
    }

    SUBCASE("TIOReader Card Getter Order") {
        bool is_r1_0 = reader_camera_r1.IsR1();
        uint32_t run_id_0 = reader_camera_r1.GetRunID();
        (void)reader_tm_r0.GetEventID(0);
        bool is_r1_1 = reader_camera_r1.IsR1();
        uint32_t run_id_1 = reader_camera_r1.GetRunID();
        CHECK(is_r1_0 == is_r1_1);
        CHECK(run_id_0 == run_id_1);
    }
}

}
