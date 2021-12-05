// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/Waveform.h"

namespace sstcam::descriptions {

std::ostream& operator<<(std::ostream& os, const Waveform& waveform) {
    os << "Address: " << (void*) waveform.waveform_ << std::endl;
    os << "IsAssociated: " << waveform.IsAssociated() << std::endl;
    os << "IsErrorFlagOn: " << waveform.IsErrorFlagOn() << std::endl;
    os << "GetChannelID: " << static_cast<uint16_t>(waveform.GetChannelID()) << std::endl;
    os << "GetASICID: " << static_cast<uint16_t>(waveform.GetASICID()) << std::endl;
    os << "GetNSamples: " << waveform.GetNSamples() << std::endl;
    os << "IsZeroSuppressed: " << waveform.IsZeroSuppressed() << std::endl;
    os << "GetPixelID: " << waveform.GetPixelID() << std::endl;
    return os;
}

}
