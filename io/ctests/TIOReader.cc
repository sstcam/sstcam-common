// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include "doctest.h"
#include <fstream>

namespace sstcam {
namespace io {

TEST_CASE("TIOReader") {
//    // TODO: Add small test file for both R0, R1, full camera, and SM
    std::string path = "/Users/Jason/Downloads/tempdata/obs/Run12917_r0.tio";
    std::ifstream file (path);
    REQUIRE(file.good());
    file.close();

    SUBCASE("TIOReader Constructor") {
        auto reader = TIOReader(path);
        CHECK(reader.IsOpen());
        reader.Close();
        CHECK(!reader.IsOpen());
    }

    SUBCASE("TIOReader missing file") {
        CHECK_THROWS(TIOReader("/not/a/file.fits"));
    }

    auto reader = TIOReader(path);
    std::cout << "GE" << std::endl;

    SUBCASE("TIOReader Run-wise Getters") {
        CHECK(reader.GetPath() == path);
        CHECK(reader.GetNEvents() == 20814);
        CHECK(reader.GetNPixels() == 2048);
        CHECK(reader.GetNSamples() == 128);
        CHECK(reader.GetRunID() == 12917);
        CHECK(!reader.IsR1());
        CHECK(reader.GetCameraVersion() == "1.1.0");
    }

//    SUBCASE("TIOReader GetEventR0") {
//        size_t n_pixels = reader.GetNPixels();
//        size_t n_samples = reader.GetNSamples();
//
//        WaveformEventR0 event0 = reader.GetEventR0(0);
//        std::vector<uint16_t> waveforms0 = event0.GetWaveforms();
//        bool none_zero0 = true;
//        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
//            for (size_t isam = 0; isam < n_samples; isam++) {
//                if (waveforms0[ipix * n_samples + isam] == 0) none_zero0 = false;
//            }
//        }
//        CHECK(none_zero0);
//
//        WaveformEventR0 event1 = reader.GetEventR0(1);
//        std::vector<uint16_t> waveforms1 = event0.GetWaveforms();
//        bool none_zero1 = true;
//        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
//            for (size_t isam = 0; isam < n_samples; isam++) {
//                if (waveforms0[ipix * n_samples + isam] == 0) none_zero1 = false;
//            }
//        }
//        CHECK(none_zero1);
//    }
//
//    SUBCASE("TIOReader GetEventR1") {
//        size_t n_pixels = reader.GetNPixels();
//        size_t n_samples = reader.GetNSamples();
//
//        WaveformEventR1 event0 = reader.GetEventR1(0);
//        std::vector<float> waveforms0 = event0.GetWaveforms();
//        bool none_zero0 = true;
//        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
//            for (size_t isam = 0; isam < n_samples; isam++) {
//                if (waveforms0[ipix * n_samples + isam] == 0) none_zero0 = false;
//            }
//        }
//        CHECK(none_zero0);
//
//        WaveformEventR1 event1 = reader.GetEventR1(1);
//        std::vector<float> waveforms1 = event0.GetWaveforms();
//        bool none_zero1 = true;
//        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
//            for (size_t isam = 0; isam < n_samples; isam++) {
//                if (waveforms0[ipix * n_samples + isam] == 0) none_zero1 = false;
//            }
//        }
//        CHECK(none_zero1);
//    }
//
//    SUBCASE("TIOReader Event-wise Getters") {
//        CHECK(reader.GetEventID(0) == 182337065);
//        CHECK(reader.GetEventTACK(0) == 2165717354592);
//        CHECK(reader.GetEventNPacketsFilled(0) == 64);
//        CHECK(reader.GetEventCPUSecond(0) == 1556831439);
//        CHECK(reader.GetEventCPUNanosecond(0) == 700908697);
//
//        CHECK(reader.GetEventID(100) == 230875737);
//        CHECK(reader.GetEventTACK(100) == 2170571221743);
//        CHECK(reader.GetEventNPacketsFilled(100) == 64);
//        CHECK(reader.GetEventCPUSecond(100) == 1556831444);
//        CHECK(reader.GetEventCPUNanosecond(100) == 554884573);
//    }
//
//    SUBCASE("TIOReader Event-wise TACK matches packets") {
//        CHECK(reader.GetEventTACK(0) == reader.GetEventR0(0).GetPackets()[0]->GetTACK());
//        CHECK(reader.GetEventTACK(100) == reader.GetEventR0(100).GetPackets()[0]->GetTACK());
//    }
}

}}
