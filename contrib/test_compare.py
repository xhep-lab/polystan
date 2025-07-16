"""
Compare bridge sampling and polystan
====================================
"""

import os
import sigfig
import pytest

from examples import examples
from polystan import run_polystan
from bridge_sampling import run_bridge_sampling


POLYCHORD_SETTINGS = {"no-feedback": True,
                      "no-derived": True, "nlive": 1000, "write-stats": True}
BRIDGE_SAMPLING_SETTINGS = {"iter_": 20_000, "warmup": 5_000, "chains": 4}

EXAMPLES = {os.path.basename(e): e for e in examples()}


@pytest.mark.parametrize("example", EXAMPLES.keys())
def test_bridge_sampling(example, snapshot):
    example = EXAMPLES[example]
    bridge_sampling = run_bridge_sampling(example, **BRIDGE_SAMPLING_SETTINGS)
    rounded = {k: sigfig.round(v, sigfigs=6) for k, v in bridge_sampling.items()}
    assert rounded == snapshot


@pytest.mark.parametrize("example", EXAMPLES.keys())
def test_polystan(example, snapshot):
    example = EXAMPLES[example]
    polystan = run_polystan(example, polychord=POLYCHORD_SETTINGS)
    polystan_summary = {"log evidence": polystan['sample_stats']['evidence'].data[0][0]['log evidence'],
                        "error log evidence": polystan['sample_stats']['evidence'].data[0][0]['error log evidence'],
                        "neval": polystan['sample_stats']['neval'].data[0][0]['neval']}
    rounded = {k: sigfig.round(v, sigfigs=6) for k, v in polystan_summary.items()}
    assert rounded == snapshot
