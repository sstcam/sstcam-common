import pytest
import numpy as np
from os.path import join, dirname, abspath
from sstcam.descriptions import WaveformDataPacket


@pytest.fixture(scope="module")
def packet():
    directory = abspath(dirname(__file__))
    path = join(
        directory, "../share/sstcam/descriptions/waveform_data_packet_example.bin"
    )
    return np.fromfile(path, dtype=np.uint8)


def test_constructor(packet):
    data_packet = WaveformDataPacket(packet.size)
    assert data_packet.GetPacketSize() == packet.size

    data_packet.GetDataPacket()[:] = packet
    np.testing.assert_equal(packet, data_packet.GetDataPacket())


def test_getters(packet):
    data_packet = WaveformDataPacket(packet.size)
    data_packet.GetDataPacket()[:] = packet

    assert packet[0] == 32
    assert data_packet.GetDataPacket()[0] == 32

    assert data_packet.GetNWaveforms() == 32
    assert data_packet.GetRegisterSetup() == 0
    assert not data_packet.IsLastSubPacket()
    assert data_packet.IsFirstSubPacket()
    assert data_packet.GetNBuffers() == 8
    assert data_packet.GetTACK() == 2165717354592
    assert data_packet.GetEventNumber() == 18528
    assert data_packet.GetSlotID() == 22
    assert data_packet.GetDetectorID() == 22
    assert data_packet.GetEventSequenceNumber() == 236
    assert data_packet.GetDetectorUniqueTag() == 160
    assert data_packet.GetColumn() == 5
    assert data_packet.GetStaleBit() == 0
    assert data_packet.IsZeroSupressionEnabled() == 0
    assert data_packet.GetRow() == 5
    assert data_packet.GetBlockPhase() == 8
    assert data_packet.GetCRC() == 38078
    assert data_packet.GetMBZ() == 0
    assert not data_packet.IsTimeout()
    assert not data_packet.IsError()

    assert data_packet.GetWaveformSamplesNBlocks() == 4
    assert data_packet.GetWaveformSamplesNBytes() == 256
    assert data_packet.GetWaveformNSamples() == 128
    assert data_packet.GetWaveformNBytes() == 258
    assert data_packet.GetWaveformStart(0) == 16
    assert data_packet.GetPacketNBytes() == 8276


def test_same_memory():
    data_packet = WaveformDataPacket(8276)
    values = data_packet.GetDataPacket()
    assert values[0] == 0

    values[0] = 12
    assert data_packet.GetDataPacket()[0] == 12
