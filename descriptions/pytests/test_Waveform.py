import pytest
import numpy as np
from os.path import join, dirname, abspath
from sstcam.descriptions import WaveformDataPacket, Waveform


@pytest.fixture(scope="module")
def packet():
    directory = abspath(dirname(__file__))
    path = join(
        directory, "../share/sstcam/descriptions/waveform_data_packet_example.bin"
    )
    return np.fromfile(path, dtype=np.uint8)


def test_constructor(packet):
    data_packet = WaveformDataPacket(packet.size)
    data_packet.GetDataPacket()[:] = packet
    waveform = Waveform()
    print(type(data_packet))
    waveform.Associate(data_packet, 0)


def test_getters(packet):
    data_packet = WaveformDataPacket(packet.size)
    data_packet.GetDataPacket()[:] = packet
    waveform = Waveform()
    assert not waveform.IsAssociated()
    waveform.Associate(data_packet, 20)
    assert waveform.IsAssociated()

    assert not waveform.IsErrorFlagOn()
    assert waveform.GetChannelID() == 4
    assert waveform.GetASICID() == 1
    assert waveform.GetNSamples() == 128
    assert not waveform.IsZeroSuppressed()
    assert waveform.GetSample12bit(0) == 584
    assert waveform.GetSample12bit(10) == 582
    assert waveform.GetSample16bit(0) == 584
    assert waveform.GetSample16bit(10) == 8774
    assert waveform.GetPixelID() == 20


def test_setters(packet):
    data_packet = WaveformDataPacket(packet.size)
    data_packet.GetDataPacket()[:] = packet
    waveform = Waveform()
    waveform.Associate(data_packet, 20)

    assert waveform.GetSample12bit(0) != 4000
    waveform.SetSample12bit(0, 4000)
    assert waveform.GetSample12bit(0) == 4000
    assert waveform.GetSample16bit(0) == 4000

    assert waveform.GetSample16bit(0) != 65000
    waveform.SetSample16bit(0, 65000)
    assert waveform.GetSample12bit(0) != 65000  # Overflow
    assert waveform.GetSample16bit(0) == 65000


def test_last_waveform(packet):
    data_packet = WaveformDataPacket(packet.size)
    data_packet.GetDataPacket()[:] = packet
    waveform = Waveform()
    waveform.Associate(data_packet, data_packet.GetNWaveforms() - 1)
    assert waveform.GetSample12bit(0) == 649
