"""
Test example programs
=====================
"""

import os
import pytest

from run_polystan import run_polystan
from examples import examples


TEST_SETTINGS = {"no-feedback": True, "no-write": True,
                 "no-derived": True, "nlive": 1000, "write-stats": True}

EXAMPLES = {os.path.basename(e): e for e in examples()}


@pytest.mark.parametrize("example", EXAMPLES.keys())
def test_evidence(example, snapshot):
    example = EXAMPLES[example]
    data = run_polystan(example, polychord=TEST_SETTINGS)
    assert data['sample_stats']['evidence'].data[0][0]['log evidence'] == snapshot
