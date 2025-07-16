"""
Test parameter constraints
==========================

We require parameters to be constrained between 0 and 1.
"""

import os
import subprocess

import pytest
from polystan import run_polystan
from examples import search_programs

CWD = os.path.dirname(os.path.realpath(__file__))
CONSTRAINT_EXAMPLES = os.path.join(CWD, "constraint_examples")


ALL = search_programs(CONSTRAINT_EXAMPLES)
PASS = [os.path.join(CONSTRAINT_EXAMPLES, "right.stan")]
FAIL = [f for f in ALL if f not in PASS]

TEST_SETTINGS = {"no-feedback": True, "no-write": True,
                 "no-derived": True, "nlive": 10, "num-repeats": 1}


@pytest.mark.parametrize("example", PASS)
def test_constraints_pass(example):
    run_polystan(example, polychord=TEST_SETTINGS)


@pytest.mark.parametrize("example", FAIL)
def test_constraints_fail(example):
    with pytest.raises(subprocess.CalledProcessError):
        run_polystan(example, polychord=TEST_SETTINGS)
