// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformEvent.h"
#include "doctest.h"
#include <fstream>

namespace sstcam {
namespace descriptions {

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
    GetHardcodedModuleSituation(slots, n_pixels, first_active_module_slot);
    CHECK(n_pixels == 2048+64);
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
        CHECK(event.GetPackets()[0] != packet_ptr2.get()); // TODO: remove

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
        CHECK(event.GetPackets()[0] != packet_ptr2.get()); // TODO: remove

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

    WaveformEventR0 event_r0(n_packets_per_event, n_pixels, first_active_module_slot);
    event_r0.AddPacket(packet.get());
    WaveformEventR1 event_r1(n_packets_per_event, n_pixels, first_active_module_slot);
    event_r1.AddPacket(packet.get());

    SUBCASE("WaveformEventR0 Metadata") {
        CHECK(event_r0.GetNPixels() == n_pixels);
        CHECK(event_r0.GetNSamples() == n_samples);
        CHECK(event_r0.GetFirstCellID() == 1448);
        CHECK(event_r0.GetTACK() == 2165717354592);
        CHECK(!event_r0.IsStale());
        CHECK(!event_r0.IsMissingPackets());
    }

    SUBCASE("GetSampleR0 Sample") {
        Waveform waveform;
        WaveformDataPacket* packet_ = event_r0.GetPackets()[0];
        waveform.Associate(packet_, packet_->GetWaveformStart(0));
        CHECK(GetSampleR0(waveform, 0) == 636);
    }

    SUBCASE("GetSampleR1 Sample") {
        Waveform waveform;
        WaveformDataPacket* packet_ = event_r1.GetPackets()[0];
        waveform.Associate(packet_, packet_->GetWaveformStart(0));
        CHECK(GetSampleR1(waveform, 0) == 636.0f);
    }

    SUBCASE("GetSampleR0 Sample Scale&Offset") {
        Waveform waveform;
        WaveformDataPacket* packet_ = event_r0.GetPackets()[0];
        waveform.Associate(packet_, packet_->GetWaveformStart(0));
        CHECK(GetSampleR0(waveform, 0, 10, 3) == 636);
    }

    SUBCASE("GetSampleR1 Sample Scale&Offset") {
        Waveform waveform;
        WaveformDataPacket* packet_ = event_r1.GetPackets()[0];
        waveform.Associate(packet_, packet_->GetWaveformStart(0));
        CHECK(GetSampleR1(waveform, 0, 10, 3) == 60.60f);
    }

    WaveformEventR1 event_r1_so(n_packets_per_event, n_pixels, first_active_module_slot, 10, 3);
    event_r1_so.AddPacket(packet.get());

    SUBCASE("WaveformEventR1 Scale&Offset") {
        CHECK(event_r1.GetScale() == 1);
        CHECK(event_r1.GetOffset() == 0);

        CHECK(event_r1_so.GetScale() == 10);
        CHECK(event_r1_so.GetOffset() == 3);
    }

    SUBCASE("WaveformEventR0 Waveforms") {
        std::vector<uint16_t> waveforms = event_r0.GetWaveforms();
        bool none_zero = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (ipix >= 32) continue;
                if (waveforms[ipix * n_samples + isam] == 0) none_zero = false;
            }
        }
        CHECK(none_zero);
    }

    SUBCASE("WaveformEventR1 Waveforms") {
        std::vector<float> waveforms = event_r1.GetWaveforms();
        bool none_zero = true;
        for (size_t ipix = 0; ipix < n_pixels; ipix++) {
            for (size_t isam = 0; isam < n_samples; isam++) {
                if (ipix >= 32) continue;
                if (waveforms[ipix * n_samples + isam] == 0) none_zero = false;
            }
        }
        CHECK(none_zero);
    }
}

}}
