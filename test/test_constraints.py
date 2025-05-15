"""
Test parameter constraints
==========================

We require parameters to be constrained between 0 and 1.
"""

import os
import subprocess

import pytest


CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))

CONSTRAINT_EXAMPLES = os.path.join(CWD, "constraint_examples")
BUILD = os.path.join(ROOT, "build")


ALL = [os.path.splitext(n)[0]
       for n in os.listdir(CONSTRAINT_EXAMPLES) if n.endswith(".stan")]
PASS = ["right"]
FAIL = [f for f in ALL if f not in PASS]


def build_and_run(example):
    target = os.path.join(CONSTRAINT_EXAMPLES, example)
    subprocess.check_call(f"make {target}", shell=True, cwd=ROOT)
    subprocess.check_call(f"./{example}", shell=True, cwd=CONSTRAINT_EXAMPLES)


@pytest.mark.parametrize("example", PASS)
def test_constraints_pass(example):
    build_and_run(example)


@pytest.mark.parametrize("example", FAIL)
def test_constraints_fail(example):
    with pytest.raises(subprocess.CalledProcessError):
        build_and_run(example)
