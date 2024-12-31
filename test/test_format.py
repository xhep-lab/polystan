"""
Python wrapper around calls for tests
=====================================
"""

import os
import subprocess

import pytest


CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))

FORMAT = os.path.join(CWD, "format")
BUILD = os.path.join(ROOT, "build")


ALL = [os.path.splitext(n)[0] for n in os.listdir(FORMAT) if n.endswith(".stan")]
PASS = ["right"]
FAIL = [f for f in ALL if f not in PASS]


def build_and_run(example):
    target = os.path.join(FORMAT, example)
    subprocess.check_call(f"make {target}", shell=True, cwd=ROOT)
    subprocess.check_call(f"./{example}", shell=True, cwd=FORMAT)


@pytest.mark.parametrize("example", PASS)
def test_bounds_pass(example):
    build_and_run(example)


@pytest.mark.parametrize("example", FAIL)
def test_bounds_fail(example):
    with pytest.raises(subprocess.CalledProcessError):
        build_and_run(example)
