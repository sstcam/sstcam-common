// Copyright 2019 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_TIOREADER_H_
#define SSTCAM_IO_TIOREADER_H_

#include "sstcam/io/FitsUtils.h"
#include "sstcam/interfaces/WaveformDataPacket.h"
#include "sstcam/interfaces/Waveform.h"
#include "sstcam/interfaces/WaveformEvent.h"
#include <string>
#include <fitsio.h>


namespace sstcam {
namespace io {

using WaveformDataPacket = sstcam::interfaces::WaveformDataPacket;
using Waveform = sstcam::interfaces::Waveform;
using WaveformRunHeader = sstcam::interfaces::WaveformRunHeader;
using WaveformEventR0 = sstcam::interfaces::WaveformEventR0;
using WaveformEventR1 = sstcam::interfaces::WaveformEventR1;


class TIOReader {
public:
    explicit TIOReader(const std::string& path);

    ~TIOReader() { Close(); }

    void Close();

    bool IsOpen() const { return fits_ != nullptr; }

    std::shared_ptr<WaveformRunHeader> GetRunHeader() const { return run_header_; }

    std::string GetPath() const;

    uint32_t GetNEvents() const;

    uint32_t GetRunID() const {
        return fitsutils::GetHeaderKeyValue<int32_t, TINT>(fits_, "RUNNUMBER");
    }

    bool IsR1() const {
        if (fitsutils::HasHeaderKey(fits_, "R1")) {
            return fitsutils::GetHeaderKeyValue<bool, TLOGICAL>(fits_, "R1");
        } else {
            return false;
        }
    }

    std::string GetCameraVersion() const {
        auto camera_version = fitsutils::GetHeaderKeyValue<std::string, TSTRING>(fits_, "CAMERAVERSION");
        return camera_version.empty() ? "1.1.0" : camera_version; // Default = CHEC-S;
    }

    WaveformEventR0 GetEventR0(uint32_t event_index) const {
        return GetEvent<WaveformEventR0>(event_index);
    }

    WaveformEventR1 GetEventR1(uint32_t event_index) const {
        return GetEvent<WaveformEventR1>(event_index);
    }

private:
    std::shared_ptr<WaveformRunHeader> run_header_;

    fitsfile* fits_;
    int32_t event_hdu_num_;
    long n_event_columns_;
    uint8_t n_event_headers_;

    void CreateWaveformRunHeader();

    size_t ExtractPacketSize() const;

    std::set<uint8_t> ExtractActiveModules(size_t n_packets_per_event, size_t packet_size) const;

    size_t ExtractNSamples(size_t packet_size) const;

    void AssociatePacket(WaveformDataPacket& packet, uint16_t packet_id, uint32_t event_index) const;

    template<typename T>
    T GetEvent(uint32_t event_index) const {
        T event(run_header_);

        event.index = event_index;

        // TODO: fill missing header values

        // TODO: Check difference in tack between header and packet

        for (uint32_t ipack = 0; ipack < event.packets.size(); ipack++) {
            AssociatePacket(event.packets[ipack], ipack, event_index);
        }

        event.SetEventHeaderFromPackets();

        return event;
    }

};

}
}


#endif //SSTCAM_IO_TIOREADER_H_
