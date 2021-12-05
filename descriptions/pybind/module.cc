// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam_common_version.h"
#include <pybind11/pybind11.h>

namespace sstcam::descriptions {

namespace py = pybind11;

void waveform_data_packet(py::module &m);
void waveform(py::module &m);
void waveform_event(py::module &m);
void waveform_event_r0(py::module &m);
void waveform_event_r1(py::module &m);
void get_hardcoded_module_situation(py::module &m);

PYBIND11_MODULE(sstcam_descriptions, m) {
    m.def("_get_version",&getSSTCamCommonGitVersion);
    waveform_data_packet(m);
    waveform(m);
    waveform_event(m);
    waveform_event_r0(m);
    waveform_event_r1(m);
    get_hardcoded_module_situation(m);
}

}
