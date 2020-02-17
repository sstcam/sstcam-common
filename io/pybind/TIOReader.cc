// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/TIOReader.h"
#include <pybind11/pybind11.h>
namespace py = pybind11;


namespace sstcam {
namespace io {

namespace py = pybind11;

void tio_reader(py::module &m) {
    py::class_<TIOReader> tio_reader(m, "TIOReader");
    tio_reader.def(py::init<std::string>());
    tio_reader.def("Close", &TIOReader::Close);
    tio_reader.def("IsOpen", &TIOReader::IsOpen);
    tio_reader.def("GetPath", &TIOReader::GetPath);
    tio_reader.def("GetNEvents", &TIOReader::GetNEvents);
    tio_reader.def("GetRunID", &TIOReader::GetRunID);
    tio_reader.def("IsR1", &TIOReader::IsR1);
    tio_reader.def("GetCameraVersion", &TIOReader::GetCameraVersion);
//    tio_reader.def("GetRunHeader", &TIOReader::GetRunHeader);
    tio_reader.def("GetEventR0", &TIOReader::GetEventR0);
    tio_reader.def("GetEventR1", &TIOReader::GetEventR1);
}

}}