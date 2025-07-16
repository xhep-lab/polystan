"""
Test example programs
=====================
"""

import os
import sigfig
import pytest

from polystan import run_polystan
from examples import examples


TEST_SETTINGS = {"no-feedback": True, "no-write": True,
                 "no-derived": True, "nlive": 1000, "write-stats": True}

EXAMPLES = {os.path.basename(e): e for e in examples()}


@pytest.mark.parametrize("example", EXAMPLES.keys())
def test_evidence(example, snapshot):
    example = EXAMPLES[example]
    data = run_polystan(example, polychord=TEST_SETTINGS)
    log_evidence = data['sample_stats']['evidence'].data[0][0]['log evidence']
    rounded = sigfig.round(log_evidence, sigfigs=6)
    assert rounded == snapshot
