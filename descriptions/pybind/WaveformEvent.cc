// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformEvent.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace sstcam::descriptions {

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
py::array_t<T> GetWaveformSamplesArray(const TEvent& waveform_event) {
    auto n_pixels = static_cast<long>(waveform_event.GetNPixels());
    auto n_samples = static_cast<long>(waveform_event.GetNSamples());
    auto shape = std::vector<ptrdiff_t>{n_pixels, n_samples};
    auto array = py::array_t<T>(shape);
    auto buf = array.request();
    auto* ptr = (T*) buf.ptr;
    waveform_event.FillWaveformSamplesArray(ptr);
    return array;
}

void waveform_event(py::module &m) {
    py::class_<WaveformEvent> waveform_event(m, "WaveformEvent");
    waveform_event.def(
        py::init<size_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t, float, float>());
    waveform_event.def("add_packet_shared",
        &WaveformEvent::AddPacketShared);
    waveform_event.def_property_readonly("packets",
        &WaveformEvent::GetPackets, py::return_value_policy::reference_internal);
    waveform_event.def_property_readonly("n_pixels",
        &WaveformEvent::GetNPixels);
    waveform_event.def_property_readonly("n_samples",
        &WaveformEvent::GetNSamples);
    waveform_event.def_property_readonly("cpu_sec",
        &WaveformEvent::GetCPUTimeSecond);
    waveform_event.def_property_readonly("cpu_ns",
        &WaveformEvent::GetCPUTimeNanosecond);
    waveform_event.def_property_readonly("scale",
        &WaveformEvent::GetScale);
    waveform_event.def_property_readonly("offset",
        &WaveformEvent::GetOffset);
    waveform_event.def_property_readonly("index",
        &WaveformEvent::GetIndex);
    waveform_event.def_property_readonly("missing_packets",
        &WaveformEvent::IsMissingPackets);
    waveform_event.def_property_readonly("first_cell_id",
        &WaveformEvent::GetFirstCellID);
    waveform_event.def_property_readonly("tack",
        &WaveformEvent::GetTACK);
    waveform_event.def_property_readonly("stale",
        &WaveformEvent::IsStale);
}

void waveform_event_r0(py::module &m) {
    py::class_<WaveformEventR0, WaveformEvent> waveform_event(m, "WaveformEventR0");
    waveform_event.def(
        py::init<size_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t, float, float>());
    waveform_event.def("get_array",
        &GetWaveformSamplesArray<uint16_t, WaveformEventR0>);
}

void waveform_event_r1(py::module &m) {
    py::class_<WaveformEventR1, WaveformEvent> waveform_event(m, "WaveformEventR1");
    waveform_event.def(
        py::init<size_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t>());
    waveform_event.def(
        py::init<size_t, size_t, uint8_t, int64_t, int64_t, float, float>());
    waveform_event.def("get_array",
        &GetWaveformSamplesArray<float , WaveformEventR1>);
}

}
