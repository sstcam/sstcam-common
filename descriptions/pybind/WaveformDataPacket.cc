// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformDataPacket.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

namespace sstcam {
namespace descriptions {

namespace py = pybind11;

py::array_t<uint8_t> GetDataPacket(const WaveformDataPacket& packet) {
    return py::array_t<uint8_t>(packet.GetPacketSize(), packet.GetDataPacket(), py::cast(packet));
}

void waveform_data_packet(py::module &m) {
    py::class_<WaveformDataPacket> datapacket(m, "WaveformDataPacket");
    datapacket.def(py::init<size_t>());
    datapacket.def("GetDataPacket", &GetDataPacket, py::return_value_policy::reference_internal);
    datapacket.def("GetPacketSize", &WaveformDataPacket::GetPacketSize);
    datapacket.def("GetNWaveforms", &WaveformDataPacket::GetNWaveforms);
    datapacket.def("GetRegisterSetup", &WaveformDataPacket::GetRegisterSetup);
    datapacket.def("IsLastSubPacket", &WaveformDataPacket::IsLastSubPacket);
    datapacket.def("IsFirstSubPacket", &WaveformDataPacket::IsFirstSubPacket);
    datapacket.def("GetNBuffers", &WaveformDataPacket::GetNBuffers);
    datapacket.def("GetTACK", &WaveformDataPacket::GetTACK);
    datapacket.def("GetEventNumber", &WaveformDataPacket::GetEventNumber);
    datapacket.def("GetSlotID", &WaveformDataPacket::GetSlotID);
    datapacket.def("GetDetectorID", &WaveformDataPacket::GetDetectorID);
    datapacket.def("GetEventSequenceNumber", &WaveformDataPacket::GetEventSequenceNumber);
    datapacket.def("GetDetectorUniqueTag", &WaveformDataPacket::GetDetectorUniqueTag);
    datapacket.def("GetColumn", &WaveformDataPacket::GetColumn);
    datapacket.def("GetStaleBit", &WaveformDataPacket::GetStaleBit);
    datapacket.def("IsZeroSupressionEnabled", &WaveformDataPacket::IsZeroSupressionEnabled);
    datapacket.def("GetRow", &WaveformDataPacket::GetRow);
    datapacket.def("GetBlockPhase", &WaveformDataPacket::GetBlockPhase);
    datapacket.def("GetCRC", &WaveformDataPacket::GetCRC);
    datapacket.def("GetMBZ", &WaveformDataPacket::GetMBZ);
    datapacket.def("IsTimeout", &WaveformDataPacket::IsTimeout);
    datapacket.def("IsError", &WaveformDataPacket::IsError);
    datapacket.def("GetWaveformSamplesNBlocks", &WaveformDataPacket::GetWaveformSamplesNBlocks);
    datapacket.def("GetWaveformSamplesNBytes", &WaveformDataPacket::GetWaveformSamplesNBytes);
    datapacket.def("GetWaveformNSamples", &WaveformDataPacket::GetWaveformNSamples);
    datapacket.def("GetWaveformNBytes", &WaveformDataPacket::GetWaveformNBytes);
    datapacket.def("GetWaveformStart", &WaveformDataPacket::GetWaveformStart);
    datapacket.def("GetPacketNBytes", &WaveformDataPacket::GetPacketNBytes);
    datapacket.def("GetFirstCellID", &WaveformDataPacket::GetFirstCellID);
    datapacket.def("CalculatePacketSizeBytes", &WaveformDataPacket::CalculatePacketSizeBytes);
    datapacket.def("CalculateCellID", &WaveformDataPacket::CalculateCellID);
    datapacket.def("CalculateRowColumnBlockPhase", &WaveformDataPacket::CalculateRowColumnBlockPhase);
    datapacket.def("IsValid", &WaveformDataPacket::IsValid);
}

}  // namespace descriptions
}  // namespace sstcam
