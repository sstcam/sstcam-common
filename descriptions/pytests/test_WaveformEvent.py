import pytest
import numpy as np
from os.path import join, dirname, abspath
from sstcam.descriptions import WaveformEventR0, WaveformEventR1, WaveformDataPacket


@pytest.fixture(scope="module")
def packet_array():
    directory = abspath(dirname(__file__))
    path = join(
        directory, "../share/sstcam/descriptions/waveform_data_packet_example.bin"
    )
    return np.fromfile(path, dtype=np.uint8)


def test_waveform_event_r0(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22
    event = WaveformEventR0(n_packets_per_event, n_pixels, first_active_module_slot)
    with pytest.raises(RuntimeError):
        event.GetWaveforms()

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.AddPacketShared(packet)
    assert event.GetPackets()[0] == packet
    waveforms = event.GetWaveforms()
    assert (waveforms[:32] > 0).all()


def test_waveform_event_r1(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22
    event = WaveformEventR1(n_packets_per_event, n_pixels, first_active_module_slot)
    with pytest.raises(RuntimeError):
        event.GetWaveforms()

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.AddPacketShared(packet)
    assert event.GetPackets()[0] == packet
    waveforms = event.GetWaveforms()
    assert (waveforms[:32] > 0).all()


def test_scale_offset(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22

    event = WaveformEventR1(n_packets_per_event, n_pixels, first_active_module_slot)
    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.AddPacketShared(packet)
    waveforms_1 = event.GetWaveforms()
    assert (waveforms_1[:32] > 0).all()

    scale = 10
    offset = 5
    event = WaveformEventR1(n_packets_per_event, n_pixels,
                            first_active_module_slot, 20, 30, scale, offset)
    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.AddPacketShared(packet)
    waveforms_2 = event.GetWaveforms()
    assert (waveforms_2[:32] > 0).all()

    np.testing.assert_equal((waveforms_1[:32] / scale) - offset, waveforms_2[:32])


def test_get_event_metadata(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22

    event = WaveformEventR1(n_packets_per_event, n_pixels,
                            first_active_module_slot, 20, 30, 40, 50)

    assert event.GetCPUTimeSecond() == 20
    assert event.GetCPUTimeNanosecond() == 30
    assert event.GetScale() == 40
    assert event.GetOffset() == 50

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.AddPacketShared(packet)
    assert event.GetFirstCellID() == 1448
