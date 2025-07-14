"""
Test example programs
=====================
"""

import pytest

from run_polystan import run_polystan
from examples import examples


TEST_SETTINGS = {"no-feedback": True, "no-write": True,
                 "no-derived": True, "nlive": 1000, "write-stats": True}


def read_evidence(data):
    return data['sample_stats']['evidence'].data[0][0]['log evidence']


@pytest.mark.parametrize("example", examples())
def test_evidence(example, snapshot):
    data = run_polystan(example, polychord=TEST_SETTINGS)
    assert read_evidence(data) == snapshot
