// Copyright 2019 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include "doctest.h"
#include <fstream>

namespace sstcam {
namespace io {

TEST_CASE("TIOReader") {
    // TODO: Add small test file for both R0, R1, full camera, and SM
    std::string path = "/Users/Jason/Downloads/tempdata/obs/Run12917_r0.tio";
    std::ifstream file (path);
    CHECK(file.good());
    file.close();

    SUBCASE("TIOReader Constructor") {
        auto reader = TIOReader(path);
        CHECK(reader.IsOpen());
        reader.Close();
        CHECK(!reader.IsOpen());
    }

    SUBCASE("TIOReader Getters") {
        auto reader = TIOReader(path);
        CHECK(reader.GetPath() == path);
        CHECK(reader.GetNEvents() == 20814);
        CHECK(reader.GetRunID() == 12917);
        CHECK(!reader.IsR1());
        CHECK(reader.GetCameraVersion() == "1.1.0");

        CHECK(reader.GetRunHeader()->n_pixels == 2048);
        CHECK(reader.GetRunHeader()->n_samples == 128);
        CHECK(reader.GetRunHeader()->is_r1 == reader.IsR1());
    }

    SUBCASE("TIOReader GetEventR0") {
        auto reader = TIOReader(path);
        size_t n_pixels = reader.GetRunHeader()->n_pixels;
        size_t n_samples = reader.GetRunHeader()->n_samples;

        WaveformEventR0 event0 = reader.GetEventR0(0);
        CHECK(event0.index == 0);
        std::vector<uint16_t> waveforms0 = event0.GetWaveforms();
        bool none_zero0 = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms0[ipix * n_samples + isam] == 0) none_zero0 = false;
            }
        }
        CHECK(none_zero0);

        WaveformEventR0 event1 = reader.GetEventR0(1);
        CHECK(event1.index == 1);
        // TODO: Check other event headers
        std::vector<uint16_t> waveforms1 = event0.GetWaveforms();
        bool none_zero1 = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms0[ipix * n_samples + isam] == 0) none_zero1 = false;
            }
        }
        CHECK(none_zero1);
    }

    SUBCASE("TIOReader GetEventR1") {
        auto reader = TIOReader(path);
        size_t n_pixels = reader.GetRunHeader()->n_pixels;
        size_t n_samples = reader.GetRunHeader()->n_samples;

        WaveformEventR1 event0 = reader.GetEventR1(0);
        CHECK(event0.index == 0);
        std::vector<float> waveforms0 = event0.GetWaveforms();
        bool none_zero0 = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms0[ipix * n_samples + isam] == 0) none_zero0 = false;
            }
        }
        CHECK(none_zero0);

        WaveformEventR1 event1 = reader.GetEventR1(1);
        CHECK(event1.index == 1);
        std::vector<float> waveforms1 = event0.GetWaveforms();
        bool none_zero1 = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms0[ipix * n_samples + isam] == 0) none_zero1 = false;
            }
        }
        CHECK(none_zero1);
    }
}

}}
