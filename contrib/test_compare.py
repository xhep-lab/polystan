"""
Compare bridge sampling and polystan
====================================
"""
import pytest

from examples import examples
from run_polystan import run_polystan
from run_bridge_sampling import run_bridge_sampling


POLYCHORD_SETTINGS = {"no-feedback": True,
                      "no-derived": True, "nlive": 1000, "write-stats": True}
BRIDGE_SAMPLING_SETTINGS = {"iter_": 20_000, "warmup": 5_000, "chains": 4}


def compare(example):
    bridge_sampling = run_bridge_sampling(example, **BRIDGE_SAMPLING_SETTINGS)
    polystan = run_polystan(example, polychord=POLYCHORD_SETTINGS)

    polystan_summary = {"log evidence": polystan['sample_stats']['evidence'].data[0][0]['log evidence'],
                        "error log evidence": polystan['sample_stats']['evidence'].data[0][0]['error log evidence'],
                        "neval": polystan['sample_stats']['neval'].data[0][0]['neval']}

    return {"polystan": polystan_summary, "bridge_sampling": bridge_sampling}


@pytest.mark.parametrize("example", examples())
def test_compare(example, snapshot):
    assert compare(example) == snapshot


if __name__ == "__main__":
    for e in examples():
        print(compare(e))
