// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_INTERFACES_EVENT_H
#define SSTCAM_INTERFACES_EVENT_H

#include "sstcam/descriptions/WaveformDataPacket.h"
#include "sstcam/descriptions/Waveform.h"
#include <memory>
#include <utility>
#include <vector>
#include <set>


namespace sstcam {
namespace descriptions {

constexpr uint8_t N_ASICS = 4;
constexpr uint8_t N_PIXELS_PER_MODULE = descriptions::CHANNELS_PER_ASIC * N_ASICS;
constexpr uint8_t DEFAULT_N_MODULES = 32;
// TODO: move default value to core

/*!
 * @brief Obtain the hardcoded module situation corresponding to the modules
 * currently active. Used to define the size of the waveform arrays, ensuring
 * that missing modules are correctly handled.
 * Two scenarios are supported: single module (n_pixels = 64) and full camera
 * (n_pixels = 2048). If there are more than 1 active module, but less than 32,
 * then the missing modules will be filled with zeros.
 * @param active_modules
 * A set containing all of the active module slots
 * @param n_pixels
 * Number of pixels (returned by reference)
 * @param first_active_module_slot
 * Module slot of the first active module (returned by reference). Important
 * for the single module case where the slot may not be 0.
 */
void GetHardcodedModuleSituation(std::set<uint8_t>& active_modules,
    size_t& n_pixels, uint8_t& first_active_module_slot);

inline uint16_t GetSampleR0(Waveform& wf, uint16_t isam, float=1., float=0.) {
    return wf.GetSample12bit(isam);
}

inline float GetSampleR1(Waveform& wf, uint16_t isam, float scale=1., float offset=0.) {
    auto sample = static_cast<float>(wf.GetSample16bit(isam));
    return (sample / scale) - offset;
}

/*!
 * @class WaveformEvent
 * @brief Container for an event to be used for the building and reading of events.
 * By default this class does not own nor manage the memory used for the data packets. Its
 * purpose is a wrapper class for a collection of data packets, to provide bookkeeping
 * and convenient waveform access.
 *
 * A template class is used here for the following reasons:
 *
 * - It is important to distinguish between R0 and R1 events, as the samples
 * are accessed differently from the packets
 *
 * - Code duplication is avoided as much as possible
 *
 * - It is appropriate and accurate to represent R0 waveforms as uint16 and R1
 * waveforms as float
 *
 * - A class inheritance structure, defining the GetSample method for R0 and R1
 * respectively, would result in many virtual table lookups in the GetWaveforms
 * method. This template structure avoids that by inlining the function into
 * the loop. (See http://groups.di.unipi.it/~nids/docs/templates_vs_inheritance.html)
 */
template<typename T, T TGetSample(Waveform&, uint16_t, float, float)>
class WaveformEvent {
public:
    /*!
     * @param n_packets_per_event
     * Number of packets contained in an event
     * @param n_pixels
     * Number of pixels. Used only for building the waveform vector returned
     * in `GetWaveforms`. Should either be 1 or 32. Use the
     * `GetHardcodedModuleSituation` method to obtain the correct value.
     * @param first_active_module_slot
     * Module slot of the first active module. Used only for building the
     * waveform vector returned in `GetWaveforms`. Should either be 1 or 32.
     * Use the `GetHardcodedModuleSituation` method to obtain the correct value.
     */
    explicit WaveformEvent(size_t n_packets_per_event,
        size_t n_pixels=DEFAULT_N_MODULES*N_PIXELS_PER_MODULE,
        uint8_t first_active_module_slot=0, float scale=1., float offset=0.)
        : n_packets_per_event_(n_packets_per_event),
          packets_(n_packets_per_event),
          packets_owned_(n_packets_per_event),
          packet_index_(0),
          n_pixels_(n_pixels),
          first_active_module_slot_(first_active_module_slot),
          scale_(scale),
          offset_(offset)
    { }

    virtual ~WaveformEvent() = default;

    // Does the event have no packets?
    bool IsEmpty() const { return packet_index_ == 0; }

    // Is the event fully filled with events?
    bool IsFilled() const { return packet_index_ >= n_packets_per_event_; }

    // Get number of packets that have been added to the event
    uint16_t GetNPacketsAdded() const { return packet_index_; }

    // Add a WaveformDataPacket raw pointer to the event. The event does not
    // have ownership of the memory in this packet
    void AddPacket(WaveformDataPacket* packet) {
        if (IsFilled()) { // TODO: Remove this check for speed improvement?
            std::cerr << "WaveformEvent is full" << std::endl;
            return;
        }
        packets_[packet_index_++] = packet;
    }

    // Add a WaveformDataPacket shared pointer to the event. The event shares
    // ownership of the memory in this packet. The packets pointed to by this
    // event are then guaranteed to remain alive during the event's lifetime.
    // This method is normally only called when reading packets from file
    void AddPacketShared(const std::shared_ptr<WaveformDataPacket>& packet) {
        if (IsFilled()) {
            std::cerr << "WaveformEvent is full" << std::endl;
            return;
        }
        packets_[packet_index_] = packet.get();
        packets_owned_[packet_index_] = packet;
        packet_index_++;
    }

    // Declare the event empty and ready to be filled with new packets. Does
    // not delete any packets.
    void Reset() {
        std::fill(packets_.begin(), packets_.end(), nullptr);
        packet_index_ = 0;
    }

    // Get the waveforms of the event as a contiguous 1D vector
    std::vector<T> GetWaveforms() const {
        size_t size = n_pixels_ * GetNSamples();
        std::vector<T> samples(size, 0);
        Waveform waveform;

        for (WaveformDataPacket* packet : packets_) {
            uint16_t n_waveforms = packet->GetNWaveforms();
            uint8_t module = packet->GetSlotID() - first_active_module_slot_;
            for (unsigned short iwav = 0; iwav < n_waveforms; iwav++) {
                waveform.Associate(packet, iwav);
                uint16_t n_samples = waveform.GetNSamples();
                uint16_t ipix = module*N_PIXELS_PER_MODULE + waveform.GetPixelID();
                for (unsigned short isam = 0; isam < n_samples; isam++) {
                    samples[ipix * n_samples + isam] = TGetSample(
                        waveform, isam, scale_, offset_);
                }
            }
        }
        return samples;
    }

    // Obtain the vector containing the pointers to the packets
    std::vector<WaveformDataPacket*> GetPackets() const { return packets_; }

    size_t GetNPixels() const { return n_pixels_; }

    size_t GetNSamples() const { return GetFirstPacket()->GetWaveformNSamples(); }

    float GetScale() { return scale_; }

    float GetOffset() { return offset_; }

    // Convenience methods for event information from packets___________________

    // Check if the event is fully filled, and that all packets still exist
    // and are not empty.
    bool IsMissingPackets() const {
        if (!IsFilled()) return true;
        if (packets_.size() != n_packets_per_event_) return true;
        for (WaveformDataPacket* packet : packets_) {
            if (!packet || packet->IsEmpty()) return true;
        }
        return false;
    }

    // Get the first_cell_id from the first filled packet
    uint16_t GetFirstCellID() const { return GetFirstPacket()->GetFirstCellID(); }

    // Get the TACK timestamp from the first filled packet
    uint64_t GetTACK() const { return GetFirstPacket()->GetTACK(); }

    // Check if the event is stale from the first filled packet
    bool IsStale() const { return GetFirstPacket()->GetStaleBit() == 1; }

private:
    size_t n_packets_per_event_;
    std::vector<WaveformDataPacket*> packets_;
    std::vector<std::shared_ptr<WaveformDataPacket>> packets_owned_;
    uint16_t packet_index_;
    size_t n_pixels_;
    uint8_t first_active_module_slot_;
    float scale_;
    float offset_;

    // Get the first packet that is not empty
    WaveformDataPacket* GetFirstPacket() const {
        for (WaveformDataPacket* packet : packets_) {
            if (packet && !packet->IsEmpty()) return packet;
        }
        std::cerr << "No filled WaveformDataPacket found in event" << std::endl;
        return nullptr;
    }
};

/*!
 * @class WaveformEventR0
 * @brief WaveformEvent class for R0 events (raw from the camera before calibration)
 */
using WaveformEventR0 = WaveformEvent<uint16_t, GetSampleR0>;

/*!
 * @class WaveformEventR1
 * @brief WaveformEvent class for R1 events (post low-level calibration)
 */
using WaveformEventR1 = WaveformEvent<float, GetSampleR1>;

}}

#endif //SSTCAM_INTERFACES_EVENT_H
