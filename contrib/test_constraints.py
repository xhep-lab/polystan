"""
Test parameter constraints
==========================

We require parameters to be constrained between 0 and 1.
"""

import os
import subprocess

import pytest
from run_polystan import run_polystan
from examples import search_programs

CWD = os.path.dirname(os.path.realpath(__file__))
CONSTRAINT_EXAMPLES = os.path.join(CWD, "constraint_examples")


ALL = search_programs(CONSTRAINT_EXAMPLES)
PASS = [os.path.join(CONSTRAINT_EXAMPLES, "right.stan")]
FAIL = [f for f in ALL if f not in PASS]


@pytest.mark.parametrize("example", PASS)
def test_constraints_pass(example):
    run_polystan(example)


@pytest.mark.parametrize("example", FAIL)
def test_constraints_fail(example):
    with pytest.raises(subprocess.CalledProcessError):
        run_polystan(example)
