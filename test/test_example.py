"""
Python wrapper around calls for tests
=====================================
"""

import os
import json
import subprocess

import pytest


CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))

EXAMPLE = os.path.join(ROOT, "examples")
BUILD = os.path.join(ROOT, "build")

EXAMPLES = [os.path.splitext(n)[0]
            for n in os.listdir(EXAMPLE) if n.endswith(".stan")]

EXPECTED_FILE_NAME = os.path.join(CWD, "expected.json")
with open(EXPECTED_FILE_NAME, "r", encoding="utf-8") as expected_file:
    EXPECTED = json.load(expected_file)

SEED = 127


def make_polystan(example):
    stan_file = f"{os.path.join(EXAMPLE, example)}.stan"
    subprocess.check_call(f"make PS_MODEL={stan_file}", shell=True, cwd=ROOT)


def cli_subargs(**kwargs):
    return " ".join(f"--{k}={v}" for k, v in kwargs.items())


def cli_args(**kwargs):
    return " ".join(f"{k} {cli_subargs(**v)}" for k, v in kwargs.items())


def run_cli(example):
    args = {"random": {"seed": SEED}, "polychord": {
        "seed": SEED, "overwrite": 1}}

    data_file = f"{os.path.join(EXAMPLE, example)}.data.json"
    if os.path.isfile(data_file):
        args["data"] = {"file": data_file}

    return subprocess.check_call(f"./run {cli_args(**args)}", shell=True, cwd=os.path.join(BUILD, example))


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


if __name__ == "__main__":

    expected = {}

    for example in EXAMPLES:
        make_polystan(example)
        run_cli(example)
        expected[example] = read_evidence(example)

    with open(EXPECTED_FILE_NAME, "w", encoding="utf-8") as expected_file:
        json.dump(expected, expected_file)
