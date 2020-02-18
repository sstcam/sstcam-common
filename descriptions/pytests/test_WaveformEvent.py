import pytest
import numpy as np
from os.path import join, dirname, abspath
from sstcam.descriptions import WaveformRunHeader, WaveformEventR0, WaveformEventR1


@pytest.fixture(scope="module")
def packet():
    directory = abspath(dirname(__file__))
    path = join(
        directory, "../share/sstcam/descriptions/waveform_data_packet_example.bin"
    )
    return np.fromfile(path, dtype=np.uint8)


def test_waveform_event_r0(packet):
    header = WaveformRunHeader(1, 8276, {22}, 128)
    event = WaveformEventR0(header)
    waveforms = event.GetWaveforms()
    assert waveforms.dtype == np.uint16
    assert (waveforms == 0).all()
    event.packets[0].GetDataPacket()[:] = packet
    waveforms = event.GetWaveforms()
    assert (waveforms[:32] > 0).all()


def test_waveform_event_r1(packet):
    header = WaveformRunHeader(1, 8276, {22}, 128, True, 10, 5)
    event = WaveformEventR1(header)
    waveforms = event.GetWaveforms()
    assert waveforms.dtype == np.float32
    assert (waveforms == 0).all()
    event.packets[0].GetDataPacket()[:] = packet
    waveforms = event.GetWaveforms()
    assert (waveforms[:32] > 0).all()


def test_scale_offset(packet):
    header = WaveformRunHeader(1, 8276, {22}, 128, True)
    event = WaveformEventR1(header)
    event.packets[0].GetDataPacket()[:] = packet
    waveforms_1 = event.GetWaveforms()
    assert (waveforms_1[:32] > 0).all()

    scale = 10
    offset = 5
    header = WaveformRunHeader(1, 8276, {22}, 128, True, scale, offset)
    event = WaveformEventR1(header)
    event.packets[0].GetDataPacket()[:] = packet
    waveforms_2 = event.GetWaveforms()
    assert (waveforms_2[:32] > 0).all()

    np.testing.assert_equal((waveforms_1[:32] / scale) - offset, waveforms_2[:32])


def test_set_event_header_from_packets():
    header = WaveformRunHeader(1, 8276, {22}, 128)
    event = WaveformEventR0(header)
    event.packets[0].GetDataPacket()[:] = packet
    assert event.first_cell_id == 0
    event.SetEventHeaderFromPackets()
    assert event.first_cell_id == 1448
