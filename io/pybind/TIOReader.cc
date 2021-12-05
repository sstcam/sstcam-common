// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

namespace sstcam::io {

namespace py = pybind11;
using WaveformEventR0 = sstcam::descriptions::WaveformEventR0;
using WaveformEventR1 = sstcam::descriptions::WaveformEventR1;


// Obtain the correct WaveformEvent subclass based on the file type
py::object GetEvent(const TIOReader& reader, uint32_t event_index) {
    if (reader.IsR1()) return py::cast(reader.GetEventR1(event_index));
    else return py::cast(reader.GetEventR0(event_index));
}

class TIOIter {
private:
    const TIOReader& reader;
    size_t index;
public:
    TIOIter(const TIOReader& reader, size_t event_index )
        : reader(reader), index(event_index) {}

    bool operator==(const TIOIter& iter) { return iter.index == index; }
    bool operator!=(const TIOIter& iter) { return !(*this==iter); }
    py::object operator*() { return GetEvent(reader, index); }
    TIOIter& operator++() { ++index; return *this; }
    TIOIter operator++(int) {  // NOLINT(cert-dcl21-cpp)
        TIOIter clone(*this); ++index; return clone;
    }
};

std::string ReaderInfo(const TIOReader& reader) {
    std::stringstream ss;
    ss << "path: " << reader.GetPath() << std::endl;
    ss << "run_id: " << reader.GetRunID() << std::endl;
    ss << "n_events: " << reader.GetNEvents() << std::endl;
    ss << "n_pixles: " << reader.GetNPixels() << std::endl;
    ss << "n_samples: " << reader.GetNSamples() << std::endl;
    ss << "is_r1: " << std::boolalpha << reader.IsR1() << std::endl;
    ss << "active_module_slots: ";
    for (uint8_t slot: reader.GetActiveModules()) {
        ss << (unsigned) slot << " ";
    }
    ss << std::endl;
    return ss.str();
}

void tio_reader(py::module &m) {
    py::class_<TIOReader> tio_reader(m, "TIOReader");
    py::module::import("sstcam.descriptions"); // Require the WaveformEvent wrappings
    tio_reader.def(py::init<std::string>());
    tio_reader.def("close", &TIOReader::Close);
    tio_reader.def_property_readonly("is_open", &TIOReader::IsOpen);
    tio_reader.def_property_readonly("path", &TIOReader::GetPath);
    tio_reader.def_property_readonly("scale", &TIOReader::GetScale);
    tio_reader.def_property_readonly("offset", &TIOReader::GetOffset);
    tio_reader.def_property_readonly("n_events", &TIOReader::GetNEvents);
    tio_reader.def_property_readonly("n_pixels", &TIOReader::GetNPixels);
    tio_reader.def_property_readonly("n_samples", &TIOReader::GetNSamples);
    tio_reader.def_property_readonly("n_packets_per_event", &TIOReader::GetNPacketsPerEvent);
    tio_reader.def_property_readonly("first_active_module_slot", &TIOReader::GetFirstActiveModuleSlot);
    tio_reader.def_property_readonly("run_id", &TIOReader::GetRunID);
    tio_reader.def_property_readonly("is_r1", &TIOReader::IsR1);
    tio_reader.def_property_readonly("camera_version", &TIOReader::GetCameraVersion);
    tio_reader.def("__str__", ReaderInfo);
    tio_reader.def("__getitem__", [](const TIOReader& reader, int64_t event_index) {
        if (event_index < 0) event_index += reader.GetNEvents();
        return GetEvent(reader, static_cast<uint32_t>(event_index));
    });
    tio_reader.def("__len__", [](const TIOReader& reader) {
        return reader.GetNEvents();
    });
    tio_reader.def("__iter__", [](const TIOReader& reader) {
        size_t end = reader.GetNEvents() - 1;
        return py::make_iterator(TIOIter(reader, 0), TIOIter(reader, end));
    }, py::keep_alive<0, 1>());
    tio_reader.def("__enter__", [&](TIOReader* reader) {
        return reader;
    });
    tio_reader.def("__exit__", [&](TIOReader* reader, py::object& exc_type, py::object& exc_val, py::object& exc_tb) {
        (void)exc_type;
        (void)exc_val;
        (void)exc_tb;
        reader->Close();
    });
}

}