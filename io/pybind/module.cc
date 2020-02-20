// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include <pybind11/pybind11.h>
#include "sstcam_common_version.h"
namespace sstcam {
namespace io {

namespace py = pybind11;

void tio_reader(py::module &m);

PYBIND11_MODULE(sstcam_io, m) {
    m.def("_get_version",&getSSTCamCommonGitVersion);
    tio_reader(m);
}

}
}
