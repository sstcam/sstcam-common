// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_DESCRIPTIONS_EVENT_H
#define SSTCAM_DESCRIPTIONS_EVENT_H

#include "sstcam/descriptions/WaveformDataPacket.h"
#include "sstcam/descriptions/Waveform.h"
#include <memory>
#include <utility>
#include <vector>
#include <set>


namespace sstcam::descriptions {

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
 * A set containing all of the active module slots.
 * @param n_pixels
 * Number of pixels (returned by reference).
 * @param first_active_module_slot
 * Module slot of the first active module (returned by reference). Important
 * for the single module case where the slot may not be 0.
 */
void GetHardcodedModuleSituation(std::set<uint8_t>& active_modules,
    size_t& n_pixels, uint8_t& first_active_module_slot);

/*!
 * @brief Obtain an R0 sample from the waveform. The last two parameters do
 * not have any effect, they only exist to ensure a common API for the GetSample methods.
 * @param waveform
 * The Waveform to extract the sample from.
 * @param isample
 * The sample index in the waveform.
 */
inline uint16_t GetSampleR0(Waveform& waveform, uint16_t isample,
        float=1., float=0.) {
    return waveform.GetSample12bit(isample);
}

/*!
 * @brief Obtain an R1 sample from the waveform.
 * @param waveform
 * The Waveform to extract the sample from.
 * @param isample
 * The sample index in the waveform.
 * @param scale
 * The scaling that was used to convert the original floating point R1 sample
 * into uint16_t.
 * @param offset
 * The offset that was used to convert the original floating point R1 sample
 * into uint16_t.
 */
inline float GetSampleR1(Waveform& waveform, uint16_t isample,
        float scale=1., float offset=0.) {
    auto sample = static_cast<float>(waveform.GetSample16bit(isample));
    return (sample / scale) - offset;
}

/*!
 * @class WaveformEvent
 * @brief Container to be used for the building and reading of events.
 * By default this class does not own nor manage the memory used for the data packets. Its
 * purpose is a wrapper class for a collection of data packets, to provide bookkeeping
 * and convenient waveform access.
 *
 * Also used as the base class for the WaveformEventR0 and WaveformEventR1 classes,
 * which each define the correct way to build a waveform vector from
 * the samples contained in the WaveformDataPackets.
 */
class WaveformEvent {
public:
    /*!
     * @param n_packets_per_event
     * Number of packets contained in an event
     * @param n_pixels
     * Number of pixels. Used only for building the waveform vector returned
     * in `GetWaveformSamples`. Should either be 64 (single module) or 2048
     * (full camera). Use the `GetHardcodedModuleSituation` method to obtain
     * the correct value.
     * @param first_active_module_slot
     * Module slot of the first active module. Used only for building the
     * waveform vector returned in `GetWaveformSamples`. If a single module is
     * connected, should equal the slot index. Otherwise it should equal 0.
     * Use the `GetHardcodedModuleSituation` method to obtain the correct value.
     * @param cpu_time_second
     * The event creation time in number of seconds since epoch.
     * @param cpu_time_nanosecond
     * The number of nanoseconds that elapsed since cpu_time_second at
     * event creation time.
     * @param scale
     * The amount of scaling applied to the samples to fit the floating point
     * value into uint16_t. Only relevant for R1 events.
     * @param offset
     * The offset applied to ensure the sample is positive when converting the
     * floating point value into uint16_t. Only relevant for R1 events.
     * @param index
     * Index of this event within the ring buffer or file.
     */
    explicit WaveformEvent(
        size_t n_packets_per_event,
        size_t n_pixels=DEFAULT_N_MODULES*N_PIXELS_PER_MODULE,
        uint8_t first_active_module_slot=0,
        int64_t cpu_time_second=0, int64_t cpu_time_nanosecond=0,
        float scale=1., float offset=0., size_t index=0);

    virtual ~WaveformEvent() = default;

    // Does the event have no packets?
    [[nodiscard]] inline bool IsEmpty() const { return packet_index_ == 0; }

    // Is the event fully filled with events?
    [[nodiscard]] inline bool IsFilled() const {
        return packet_index_ >= n_packets_per_event_;
    }

    // Get number of packets that have been added to the event.
    [[nodiscard]] inline uint16_t GetNPacketsAdded() const { return packet_index_; }

    // Add a WaveformDataPacket raw pointer to the event. The event does not
    // have ownership of the memory in this packet.
    void AddPacket(WaveformDataPacket* packet);

    // Add a WaveformDataPacket shared pointer to the event. The event shares
    // ownership of the memory in this packet. The packets pointed to by this
    // event are then guaranteed to remain alive during the event's lifetime.
    // This method is normally only called when reading packets from file.
    void AddPacketShared(const std::shared_ptr<WaveformDataPacket>& packet);

    // Declare the event empty and ready to be filled with new packets. Does
    // not delete the packets.
    void Reset();

    // Obtain the vector containing the pointers to the packets.
    [[nodiscard]] inline std::vector<WaveformDataPacket*> GetPackets() const {
        return packets_;
    }

    // Number of pixels in the event.
    [[nodiscard]] inline size_t GetNPixels() const { return n_pixels_; }

    // Assume the number of samples per pixel in the event based on
    // the first filled WaveformDataPacket.
    [[nodiscard]] inline size_t GetNSamples() const {
        return GetFirstPacket()->GetWaveformNSamples();
    }

    // Get the lowest module slot ID among the modules that are active in the event.
    // This value is used to offset the slot ID of a waveform during waveform
    // sample array filling, in cases where there are only 1 module active.
    [[nodiscard]] inline uint8_t GetFirstActiveModuleSlot() const {
        return first_active_module_slot_;
    }

    // CPU time: seconds. TODO: unix?
    [[nodiscard]] inline int64_t GetCPUTimeSecond() const {
        return cpu_time_second_;
    }

    // CPU time: nanoseconds. TODO: unix?
    [[nodiscard]] inline int64_t GetCPUTimeNanosecond() const {
        return cpu_time_nanosecond_;
    }

    // Compression scale for the R1 waveform samples.
    [[nodiscard]] inline float GetScale() const { return scale_; }

    // Compression offset for the R1 waveform samples.
    [[nodiscard]] inline float GetOffset() const { return offset_; }

    // Event index (defined by the reader or event builder).
    [[nodiscard]] inline size_t GetIndex() const { return index_; }

    // Convenience methods for event information from packets___________________

    // Check if the event is fully filled, and that all packets still exist
    // and are not empty.
    [[nodiscard]] bool IsMissingPackets() const;

    // Assume the first_cell_id for the event from the first filled WaveformDataPacket.
    // Consequently assumes that all packets in the event share the same
    // first_cell_id (which should be true).
    [[nodiscard]] inline uint16_t GetFirstCellID() const {
        return GetFirstPacket()->GetFirstCellID();
    }

    // Get the TACK timestamp from the first filled packet.
    [[nodiscard]] inline uint64_t GetTACK() const {
        return GetFirstPacket()->GetTACK();
    }

    // Check if the event is stale from the first filled packet.
    [[nodiscard]] inline bool IsStale() const {
        return GetFirstPacket()->GetStaleBit() == 1;
    }

protected:
    size_t n_packets_per_event_;
    std::vector<WaveformDataPacket*> packets_;
    std::vector<std::shared_ptr<WaveformDataPacket>> packets_owned_;
    uint16_t packet_index_;
    size_t n_pixels_;
    uint8_t first_active_module_slot_;
    int64_t cpu_time_second_;
    int64_t cpu_time_nanosecond_;
    float scale_;
    float offset_;
    size_t index_;

    // Template to define how a waveform sample array is built by an event.
    // The template is specialized by the R0 and R1 subclasses.
    template<typename T, T TGetSample(Waveform&, uint16_t, float, float)>
    inline void FillWaveformSamplesArrayTemplate(T* samples) const {
        Waveform waveform;

        for (WaveformDataPacket* packet : packets_) {
            if (!packet) continue;
            uint16_t n_waveforms = packet->GetNWaveforms();
            uint8_t module = packet->GetSlotID() - first_active_module_slot_;
            for (unsigned short i_waveform = 0; i_waveform < n_waveforms; i_waveform++) {
                waveform.Associate(packet, i_waveform);
                uint16_t n_samples = waveform.GetNSamples();
                uint16_t i_pixel = module * N_PIXELS_PER_MODULE + waveform.GetPixelID();
                for (unsigned short i_sample = 0; i_sample < n_samples; i_sample++) {
                    samples[i_pixel * n_samples + i_sample] = TGetSample(
                        waveform, i_sample, scale_, offset_);
                }
            }
        }
    }

private:
    // Get the first packet that is not empty.
    [[nodiscard]] WaveformDataPacket* GetFirstPacket() const;
};

/*!
 * @class WaveformEventR0
 * @brief WaveformEvent class for R0 events (raw from the camera
 * before waveform calibration).
 */
class WaveformEventR0 : public WaveformEvent{
public:
    using WaveformEvent::WaveformEvent;

    // Fill a supplied array with the waveform samples.
    // No range checks are included.
    inline void FillWaveformSamplesArray(uint16_t* samples) const {
        FillWaveformSamplesArrayTemplate<uint16_t, GetSampleR0>(samples);
    }

    // Get the waveforms of the event as a contiguous 1D vector
    [[nodiscard]] inline std::vector<uint16_t> GetWaveformSamplesVector() const {
        size_t size = n_pixels_ * GetNSamples();
        std::vector<uint16_t > samples(size, 0);
        FillWaveformSamplesArray(samples.data());
        return samples;
    }
};

/*!
 * @class WaveformEventR1
 * @brief WaveformEvent class for R1 events (post waveform calibration).
 */
class WaveformEventR1 : public WaveformEvent {
public:
    using WaveformEvent::WaveformEvent;

    // Fill a supplied array with the waveform samples.
    // No range checks are included.
    inline void FillWaveformSamplesArray(float* samples) const {
        FillWaveformSamplesArrayTemplate<float, GetSampleR1>(samples);
    }

    // Get the waveforms of the event as a contiguous 1D vector.
    [[nodiscard]] inline std::vector<float> GetWaveformSamplesVector() const {
        size_t size = n_pixels_ * GetNSamples();
        std::vector<float> samples(size, 0);
        FillWaveformSamplesArray(samples.data());
        return samples;
    }
};

}

#endif //SSTCAM_DESCRIPTIONS_EVENT_H
