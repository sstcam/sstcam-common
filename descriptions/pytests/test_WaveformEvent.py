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
        event.get_array()

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.add_packet_shared(packet)
    assert event.packets[0] == packet
    waveforms = event.get_array()
    assert (waveforms[:32] > 0).all()
    assert waveforms.dtype == np.uint16


def test_waveform_event_r1(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22
    event = WaveformEventR1(n_packets_per_event, n_pixels, first_active_module_slot)
    with pytest.raises(RuntimeError):
        event.get_array()

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.add_packet_shared(packet)
    assert event.packets[0] == packet
    waveforms = event.get_array()
    assert (waveforms[:32] > 0).all()
    assert waveforms.dtype == np.float32


def test_scale_offset(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22

    event = WaveformEventR1(n_packets_per_event, n_pixels, first_active_module_slot)
    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.add_packet_shared(packet)
    waveforms_1 = event.get_array()
    assert (waveforms_1[:32] > 0).all()

    scale = 10
    offset = 5
    event = WaveformEventR1(n_packets_per_event, n_pixels,
                            first_active_module_slot, 20, 30, scale, offset)
    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.add_packet_shared(packet)
    waveforms_2 = event.get_array()
    assert (waveforms_2[:32] > 0).all()

    np.testing.assert_equal((waveforms_1[:32] / scale) - offset, waveforms_2[:32])


def test_get_event_metadata(packet_array):
    n_packets_per_event = 1
    packet_size = 8276
    n_pixels = 64
    first_active_module_slot = 22

    event = WaveformEventR1(n_packets_per_event, n_pixels,
                            first_active_module_slot, 20, 30, 40, 50)

    assert event.cpu_sec == 20
    assert event.cpu_ns == 30
    assert event.scale == 40
    assert event.offset == 50

    packet = WaveformDataPacket(packet_size)
    packet.GetDataPacket()[:] = packet_array
    event.add_packet_shared(packet)
    assert event.first_cell_id == 1448
