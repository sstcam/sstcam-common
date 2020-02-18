// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformEvent.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace sstcam {
namespace descriptions {

namespace py = pybind11;

void get_hardcoded_module_situation(py::module &m) {
    m.def("GetHardcodedModuleSituation",
          [](std::set<uint8_t> active) {
              size_t n_pixels;
              uint8_t first;
              GetHardcodedModuleSituation(active, n_pixels, first);
              return std::make_tuple(n_pixels, first);
          });
}

template<typename T, typename TEvent>
py::array_t<T> GetWaveforms(const TEvent& waveform_event) {
    auto v = new std::vector<T>(waveform_event.GetWaveforms());
    auto capsule = py::capsule(v, [](void *v) { delete reinterpret_cast<std::vector<T>*>(v); });
    auto n_pixels = static_cast<long>(waveform_event.GetNPixels());
    auto n_samples = static_cast<long>(waveform_event.GetNSamples());
    auto shape = std::vector<ptrdiff_t>{n_pixels, n_samples};
    return py::array(shape, v->data(), capsule);
}

template<typename T, typename TEvent>
void waveform_event_template(py::module &m, const std::string& name) {
    py::class_<TEvent> waveform_event(m, name.c_str());
    waveform_event.def(py::init<size_t, size_t, uint8_t>());
    waveform_event.def("GetWaveforms", GetWaveforms<T, TEvent>);
    waveform_event.def("GetPackets", &TEvent::GetPackets);
    waveform_event.def("GetNPixels", &TEvent::GetNPixels);
    waveform_event.def("GetNSamples", &TEvent::GetNSamples);
    waveform_event.def("IsMissingPackets", &TEvent::IsMissingPackets);
    waveform_event.def("GetFirstCellID", &TEvent::GetFirstCellID);
    waveform_event.def("GetTACK", &TEvent::GetTACK);
    waveform_event.def("IsStale", &TEvent::IsStale);
}

void waveform_event(py::module &m) {
    waveform_event_template<uint16_t, WaveformEventR0>(m, "WaveformEventR0");
    waveform_event_template<float, WaveformEventR1>(m, "WaveformEventR1");
}

}}
