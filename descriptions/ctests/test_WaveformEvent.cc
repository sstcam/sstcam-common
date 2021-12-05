// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#define DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS

#include "sstcam/descriptions/WaveformEvent.h"
#include "doctest.h"
#include <fstream>

namespace sstcam::descriptions {

TEST_CASE("GetHardcodedModuleSituation") {
    std::set<uint8_t> slots = {5};
    size_t n_pixels = 0;
    uint8_t first_active_module_slot = 0;

    GetHardcodedModuleSituation(slots, n_pixels, first_active_module_slot);
    CHECK(n_pixels == 64);
    CHECK(first_active_module_slot == 5);

    slots.insert(4);
    n_pixels = 0;
    first_active_module_slot = 0;
    GetHardcodedModuleSituation(slots, n_pixels, first_active_module_slot);
    CHECK(n_pixels == 2048);
    CHECK(first_active_module_slot == 0);

    for (uint8_t slot=0; slot < 32; slot++) {
        slots.insert(slot);
        GetHardcodedModuleSituation(slots, n_pixels, first_active_module_slot);
        CHECK(n_pixels == 2048);
    }

    slots.insert(32);
    CHECK_THROWS_AS(
        GetHardcodedModuleSituation(slots, n_pixels, first_active_module_slot),
        std::runtime_error
    );
}

TEST_CASE("WaveformEvent") {
    std::string path = "../../share/sstcam/descriptions/waveform_data_packet_example.bin";
    size_t packet_size = 8276;
    std::ifstream file (path, std::ios::in | std::ios::binary);
    CHECK(file.is_open());
    auto packet = std::make_shared<WaveformDataPacket>(packet_size);
    file.read(reinterpret_cast<char*>(packet->GetDataPacket()), packet_size);

    size_t n_packets_per_event = 1;
    std::set<uint8_t> active_module_slots = {packet->GetSlotID()};
    size_t n_samples = packet->GetWaveformNSamples();
    size_t n_pixels;
    uint8_t first_active_module_slot;
    GetHardcodedModuleSituation(active_module_slots, n_pixels, first_active_module_slot);

    SUBCASE("WaveformEventR0 Constructor") {
        WaveformEventR0 event(n_packets_per_event);
        CHECK(event.IsEmpty());
    }

    SUBCASE("WaveformEventR1 Constructor") {
        WaveformEventR1 event(n_packets_per_event);
        CHECK(event.IsEmpty());
    }

    SUBCASE("Empty Event Getters") {
        WaveformEventR0 event(n_packets_per_event);
        REQUIRE(event.IsEmpty());
        CHECK_THROWS_AS(event.GetNSamples(), std::runtime_error);
        CHECK_THROWS_AS(event.GetFirstCellID(), std::runtime_error);
        CHECK_THROWS_AS(event.GetTACK(), std::runtime_error);
        CHECK_THROWS_AS(event.IsStale(), std::runtime_error);
        CHECK_THROWS_AS(event.GetWaveformSamplesVector(), std::runtime_error);
    }

    SUBCASE("Half Empty Event Getters") {
        WaveformEventR0 event(2);
        event.AddPacket(packet.get());
        REQUIRE(!event.IsEmpty());
        REQUIRE(!event.IsFilled());
        CHECK(event.GetNSamples() == 128);
        CHECK(event.GetFirstCellID() == 1448);
        CHECK(event.GetTACK() == 2165717354592);
        CHECK(!event.IsStale());
    }

    SUBCASE("WaveformEventR0 Adding Packets") {
        WaveformEventR0 event(n_packets_per_event);
        CHECK(event.IsEmpty());
        CHECK(!event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 0);
        CHECK(event.IsMissingPackets());

        event.AddPacket(packet.get());
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(!event.IsMissingPackets());

        auto packet_ptr2 = std::make_shared<WaveformDataPacket>(packet_size);
        event.AddPacket(packet_ptr2.get());
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(event.GetPackets()[0] == packet.get());
        CHECK(event.GetPackets()[0] != packet_ptr2.get());

        event.Reset();
        CHECK(event.IsEmpty());
        CHECK(!event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 0);
        CHECK(event.GetPackets()[0] == nullptr);
        event.AddPacket(packet_ptr2.get());
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(event.GetPackets()[0] == packet_ptr2.get());
    }

    SUBCASE("WaveformEventR0 Adding Packets (shared)") {
        WaveformEventR0 event(n_packets_per_event);
        CHECK(event.IsEmpty());
        CHECK(!event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 0);
        CHECK(event.IsMissingPackets());

        event.AddPacketShared(packet);
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(!event.IsMissingPackets());

        auto packet_ptr2 = std::make_shared<WaveformDataPacket>(packet_size);
        event.AddPacketShared(packet_ptr2);
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(event.GetPackets()[0] == packet.get());
        CHECK(event.GetPackets()[0] != packet_ptr2.get());

        event.Reset();
        CHECK(event.IsEmpty());
        CHECK(!event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 0);
        CHECK(event.GetPackets()[0] == nullptr);
        event.AddPacketShared(packet_ptr2);
        CHECK(!event.IsEmpty());
        CHECK(event.IsFilled());
        CHECK(event.GetNPacketsAdded() == 1);
        CHECK(event.GetPackets()[0] == packet_ptr2.get());
    }

    SUBCASE("WaveformEventR0 Owning Packets") {
        WaveformEventR0 event(n_packets_per_event);
        auto packet_ptr = std::make_shared<WaveformDataPacket>(packet_size);
        CHECK(packet_ptr.use_count() == 1);
        event.AddPacketShared(packet_ptr);
        CHECK(packet_ptr.use_count() == 2);
    }

    WaveformEventR0 event_r0(n_packets_per_event, n_pixels,
        first_active_module_slot, 20, 30);
    event_r0.AddPacket(packet.get());
    WaveformEventR1 event_r1(n_packets_per_event, n_pixels,
        first_active_module_slot, 20, 30);
    event_r1.AddPacket(packet.get());

    SUBCASE("WaveformEventR0 Metadata") {
        CHECK(event_r0.GetNPixels() == n_pixels);
        CHECK(event_r0.GetNSamples() == n_samples);
        CHECK(event_r0.GetFirstActiveModuleSlot() == first_active_module_slot);
        CHECK(event_r0.GetCPUTimeSecond() == 20);
        CHECK(event_r0.GetCPUTimeNanosecond() == 30);
        CHECK(event_r0.GetScale() == 1.);
        CHECK(event_r0.GetOffset() == 0.);
        CHECK(event_r0.GetIndex() == 0.);
        CHECK(event_r0.GetFirstCellID() == 1448);
        CHECK(event_r0.GetTACK() == 2165717354592);
        CHECK(!event_r0.IsStale());
        CHECK(!event_r0.IsMissingPackets());
    }

    WaveformEventR1 event_r1_so(n_packets_per_event, n_pixels,
        first_active_module_slot, 20, 30, 10, 3, 100);
    event_r1_so.AddPacket(packet.get());

    SUBCASE("WaveformEventR1 Metadata") {
        CHECK(event_r1.GetScale() == 1);
        CHECK(event_r1.GetOffset() == 0);

        CHECK(event_r1_so.GetScale() == 10);
        CHECK(event_r1_so.GetOffset() == 3);

        CHECK(event_r1_so.GetIndex() == 100);
    }

    SUBCASE("WaveformEventR0 Waveform Samples") {
        std::vector<uint16_t> waveforms = event_r0.GetWaveformSamplesVector();
        bool none_zero = true;
        for (size_t ipix = 0; ipix < 32; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms[ipix * n_samples + isam] == 0) none_zero = false;
            }
        }
        CHECK(none_zero);

        bool all_zero = true;
        for (size_t ipix = 32; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms[ipix * n_samples + isam] != 0) all_zero = false;
            }
        }
        CHECK(all_zero);
    }

    SUBCASE("WaveformEventR0 Waveforms (not fully filled)") {
        WaveformEventR0 event_r0_2(2, n_pixels, first_active_module_slot);
        event_r0_2.AddPacket(packet.get());
        REQUIRE(!event_r0_2.IsEmpty());
        REQUIRE(!event_r0_2.IsFilled());

        std::vector<uint16_t> waveforms = event_r0_2.GetWaveformSamplesVector();
        bool none_zero = true;
        for (size_t ipix = 0; ipix < 32; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms[ipix * n_samples + isam] == 0) none_zero = false;
            }
        }
        CHECK(none_zero);

        bool all_zero = true;
        for (size_t ipix = 32; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms[ipix * n_samples + isam] != 0) all_zero = false;
            }
        }
        CHECK(all_zero);
    }

    SUBCASE("WaveformEventR1 Waveform Samples") {
        std::vector<float> waveforms = event_r1.GetWaveformSamplesVector();
        bool none_zero = true;
        for (size_t ipix = 0; ipix < 32; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (waveforms[ipix * n_samples + isam] == 0) none_zero = false;
            }
        }
        CHECK(none_zero);
    }
}

}
