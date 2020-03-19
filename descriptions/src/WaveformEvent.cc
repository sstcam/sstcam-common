// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformEvent.h"

namespace sstcam {
namespace descriptions {

void GetHardcodedModuleSituation(std::set<uint8_t>& active_modules,
        size_t& n_pixels, uint8_t& first_active_module_slot) {
    first_active_module_slot = *active_modules.begin();
    size_t n_active_modules = active_modules.size();
    size_t n_modules;

    // Hardcoded n_module situations
    if (n_active_modules == 1) {
        n_modules = 1; // Single module case
    }
    else if (n_active_modules > 1 && n_active_modules <= 32) {
        n_modules = 32; // SST camera case
        first_active_module_slot = 0;
    }
    else {
        n_modules = n_active_modules;
        std::cout << "WARNING: No case set up for files with N modules:"
        << n_active_modules << std::endl;
    }
    n_pixels = n_modules * N_PIXELS_PER_MODULE;
}

WaveformEvent::WaveformEvent(size_t n_packets_per_event, size_t n_pixels,
    uint8_t first_active_module_slot,
    int64_t cpu_time_second, int64_t cpu_time_nanosecond,
    float scale, float offset)
    : n_packets_per_event_(n_packets_per_event),
      packets_(n_packets_per_event),
      packets_owned_(n_packets_per_event),
      packet_index_(0),
      n_pixels_(n_pixels),
      first_active_module_slot_(first_active_module_slot),
      cpu_time_second_(cpu_time_second),
      cpu_time_nanosecond_(cpu_time_nanosecond),
      scale_(scale),
      offset_(offset)
{ }

void WaveformEvent::AddPacket(WaveformDataPacket* packet) {
    if (IsFilled()) { // TODO: Remove this check for speed improvement?
        std::cerr << "WaveformEvent is full" << std::endl;
        return;
    }
    packets_[packet_index_++] = packet;
}

void WaveformEvent::AddPacketShared(const std::shared_ptr<WaveformDataPacket>& packet) {
    if (IsFilled()) {
        std::cerr << "WaveformEvent is full" << std::endl;
        return;
    }
    packets_[packet_index_] = packet.get();
    packets_owned_[packet_index_] = packet;
    packet_index_++;
}

void WaveformEvent::Reset() {
    std::fill(packets_.begin(), packets_.end(), nullptr);
    packet_index_ = 0;
}

bool WaveformEvent::IsMissingPackets() const {
    if (!IsFilled()) return true;
    if (packets_.size() != n_packets_per_event_) return true;
    for (WaveformDataPacket* packet : packets_) {
        if (!packet || packet->IsEmpty()) return true;
    }
    return false;
}

WaveformDataPacket* WaveformEvent::GetFirstPacket() const {
    for (WaveformDataPacket* packet : packets_) {
        if (packet && !packet->IsEmpty()) return packet;
    }
    throw std::runtime_error("WaveformEvent is empty");
}

}}
