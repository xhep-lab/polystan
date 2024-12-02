"""
Python wrapper around calls for tests
=====================================
"""

import os
import json
import subprocess
from contextlib import chdir

import pytest


CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))
EXAMPLE = os.path.join(ROOT, "examples")
BUILD = os.path.join(ROOT, "build")
EXAMPLES = [os.path.splitext(n)[0]
            for n in os.listdir(EXAMPLE) if n.endswith(".stan")]
EXPECTED = {"bernoulli": -6.232039928436279,
            "himmelblau": -4.707243919372559,
            "eggbox": -4.178520202636719,
            "rastrigin": -4.548888206481934}
SEED = 127


def make_polystan(example):
    stan_file = f"{os.path.join(EXAMPLE, example)}.stan"
    with chdir(ROOT):
        subprocess.check_call(f"make PS_MODEL={stan_file}", shell=True)


def cli_subargs(**kwargs):
    return " ".join(f"--{k}={v}" for k, v in kwargs.items())


def cli_args(**kwargs):
    return " ".join(f"{k} {cli_subargs(**v)}" for k, v in kwargs.items())


def run_cli(example):
    args = {"random": {"seed": SEED}, "polychord": {"seed": SEED}}

    data_file = f"{os.path.join(EXAMPLE, example)}.data.json"
    if os.path.isfile(data_file):
        args["data"] = {"file": data_file}

    with chdir(os.path.join(BUILD, example)):
        return subprocess.check_call(f"./run {cli_args(**args)}", shell=True)


def read_evidence(example):
    result_name = os.path.join(BUILD, example, f"{example}.json")
    with open(result_name, "r", encoding="utf-8") as result_file:
        result = json.load(result_file)
    return result["evidence"]["log evidence"]


@pytest.mark.parametrize("example", EXAMPLES)
def test_evidence(example):
    make_polystan(example)
    run_cli(example)
    assert read_evidence(example) == EXPECTED[example]
