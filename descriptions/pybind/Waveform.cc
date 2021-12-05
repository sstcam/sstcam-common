// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/Waveform.h"
#include <pybind11/pybind11.h>

namespace sstcam::descriptions {

namespace py = pybind11;

void waveform(py::module &m) {
    py::class_<Waveform> waveform(m, "Waveform");
    waveform.def(py::init<>());
    waveform.def("Associate", (void (Waveform::*)(
        const WaveformDataPacket&, uint16_t)) &Waveform::Associate);
    waveform.def("IsAssociated",
        &Waveform::IsAssociated);
    waveform.def("IsErrorFlagOn",
        &Waveform::IsErrorFlagOn);
    waveform.def("GetChannelID",
        &Waveform::GetChannelID);
    waveform.def("GetASICID",
        &Waveform::GetASICID);
    waveform.def("GetNSamples",
        &Waveform::GetNSamples);
    waveform.def("IsZeroSuppressed",
        &Waveform::IsZeroSuppressed);
    waveform.def("GetSample12bit",
        &Waveform::GetSample12bit);
    waveform.def("GetSample16bit",
        &Waveform::GetSample16bit);
    waveform.def("SetSample12bit",
        &Waveform::SetSample12bit);
    waveform.def("SetSample16bit",
        &Waveform::SetSample16bit);
    waveform.def("GetPixelID",
        &Waveform::GetPixelID);
}

}
