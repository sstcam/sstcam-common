from sstcam.io import TIOReader
import numpy as np
import pytest
import gc


@pytest.fixture(scope="module")
def single_tm_r0():
    return TIOReader("../share/sstcam/io/targetmodule_r0.tio")


@pytest.fixture(scope="module")
def single_tm_r1():
    return TIOReader("../share/sstcam/io/targetmodule_r1.tio")


@pytest.fixture(scope="module")
def camera_r1():
    return TIOReader("../share/sstcam/io/chec_r1.tio")


def test_packet_lifetime(single_tm_r0):
    e = single_tm_r0[0]
    p = e.packets
    assert p[0].GetTACK() == 4426394938696
    del e
    gc.collect()
    assert p[0].GetTACK() == 4426394938696


@pytest.mark.benchmark(warmup=True)
def test_benchmark_get_samples(benchmark, camera_r1):
    def get_waveform():
        event = camera_r1[0]
        return event.get_array()

    result = benchmark(get_waveform)
    assert result.shape == (camera_r1.n_pixels, camera_r1.n_samples)
    assert benchmark.stats.stats.mean < 400


def test_event_type(single_tm_r0, single_tm_r1):
    assert single_tm_r0[0].__class__.__name__ == "WaveformEventR0"
    assert single_tm_r1[0].__class__.__name__ == "WaveformEventR1"


def test_context_manager():
    with TIOReader("../share/sstcam/io/chec_r1.tio") as r:
        assert r.is_open
        assert "chec_r1.tio" in r.path
    assert not r.is_open


def test_properties(single_tm_r0, single_tm_r1, camera_r1):
    assert "targetmodule_r0.tio" in single_tm_r0.path
    assert single_tm_r0.n_events == 8
    assert single_tm_r0.is_r1 is False
    assert single_tm_r0.scale == 1
    assert single_tm_r0.offset == 0
    assert single_tm_r0.n_packets_per_event == 64
    assert single_tm_r0.first_active_module_slot == 0
    assert single_tm_r0.n_pixels == 64
    assert single_tm_r0.n_samples == 128

    assert "targetmodule_r1.tio" in single_tm_r1.path
    assert single_tm_r1.n_events == 8
    assert single_tm_r1.is_r1 is True
    assert single_tm_r1.scale == pytest.approx(13.6)
    assert single_tm_r1.offset == 700
    assert single_tm_r1.n_packets_per_event == 64
    assert single_tm_r1.first_active_module_slot == 0
    assert single_tm_r1.n_pixels == 64
    assert single_tm_r1.n_samples == 128

    assert "chec_r1.tio" in camera_r1.path
    assert camera_r1.n_events == 8
    assert camera_r1.is_r1 is True
    assert camera_r1.scale == pytest.approx(23.4)
    assert camera_r1.offset == 300
    assert camera_r1.n_packets_per_event == 64
    assert camera_r1.first_active_module_slot == 0
    assert camera_r1.n_pixels == 2048
    assert camera_r1.n_samples == 128


def test_str(single_tm_r1):
    assert len(str(single_tm_r1)) > 0


def test_len(single_tm_r1):
    assert len(single_tm_r1) == single_tm_r1.n_events


def test_iter(single_tm_r1):
    tack = []
    for iev, event in enumerate(single_tm_r1):
        tack.append(event.tack)
        samples_from_getitem = single_tm_r1[iev].get_array()
        samples_from_iter = event.get_array()
        np.testing.assert_equal(samples_from_getitem, samples_from_iter)
    assert len(tack) == len(set(tack))


def test_getitem(single_tm_r1):
    n_events = single_tm_r1.n_events
    with pytest.raises(RuntimeError):
        single_tm_r1[n_events]

    event_0 = single_tm_r1[n_events-2]
    event_1 = single_tm_r1[-2]
    assert event_0.tack == event_1.tack
    samples_0 = single_tm_r1[n_events-2].get_array()
    samples_1 = single_tm_r1[-2].get_array()
    np.testing.assert_equal(samples_0, samples_1)


@pytest.fixture(params=["single_tm_r0", "single_tm_r1", "camera_r1"])
def reader(request):
    return request.getfixturevalue(request.param)


def get_target_io_event(reader, iev):
    if reader.fR1:
        samples = np.zeros((reader.fNPixels, reader.fNSamples), np.float32)
        result = reader.GetR1Event(iev, samples)
    else:
        samples = np.zeros((reader.fNPixels, reader.fNSamples), np.uint16)
        result = reader.GetR0Event(iev, samples)
    keys = ("first_cell_id", "stale", "missing_packets", "tack", "cpu_sec", "cpu_ns")
    result = dict(zip(keys, result))
    return samples, result


def test_vs_targetio(reader):
    try:
        from target_io import WaveformArrayReader
    except ImportError:
        pytest.skip("TargetIO is not installed")
        return

    target_io_reader = WaveformArrayReader(reader.path)
    assert reader.is_r1 is target_io_reader.fR1
    if reader.is_r1:
        np.testing.assert_allclose(reader.scale, target_io_reader.fScale)
        np.testing.assert_allclose(reader.offset, target_io_reader.fOffset)
    assert reader.n_events == target_io_reader.fNEvents
    assert reader.n_pixels == target_io_reader.fNPixels
    assert reader.n_samples == target_io_reader.fNSamples

    for iev in range(reader.n_events):
        event = reader[iev]
        samples = event.get_array()
        target_io_event, target_io_info = get_target_io_event(target_io_reader, iev)

        assert event.first_cell_id == target_io_info["first_cell_id"]
        assert event.stale == target_io_info["stale"]
        assert event.missing_packets == target_io_info["missing_packets"]
        assert event.tack == target_io_info["tack"]
        assert event.cpu_sec == target_io_info["cpu_sec"]
        assert event.cpu_ns == target_io_info["cpu_ns"]

        np.testing.assert_allclose(samples, target_io_event)
