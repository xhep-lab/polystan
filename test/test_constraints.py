"""
Test parameter constraints
==========================

We require parameters to be constrained between 0 and 1.
"""

import os
import subprocess

import pytest
from test_examples import run_polystan_example

CWD = os.path.dirname(os.path.realpath(__file__))
CONSTRAINT_EXAMPLES = os.path.join(CWD, "constraint_examples")


ALL = [
    os.path.join(CONSTRAINT_EXAMPLES, os.path.splitext(n)[0])
    for n in os.listdir(CONSTRAINT_EXAMPLES)
    if n.endswith(".stan")
]
PASS = [os.path.join(CONSTRAINT_EXAMPLES, "right")]
FAIL = [f for f in ALL if f not in PASS]


@pytest.mark.parametrize("example", PASS)
def test_constraints_pass(example):
    run_polystan_example(example)


@pytest.mark.parametrize("example", FAIL)
def test_constraints_fail(example):
    with pytest.raises(subprocess.CalledProcessError):
        run_polystan_example(example)
