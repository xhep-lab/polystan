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

EXPECTED_FILE_NAME = os.path.join(CWD, "expected_logz_examples.json")
with open(EXPECTED_FILE_NAME, "r", encoding="utf-8") as expected_file:
    EXPECTED = json.load(expected_file)

SEED = 127


def make_polystan(example):
    target = os.path.join(EXAMPLE, example)
    subprocess.check_call(f"make {target}", shell=True, cwd=ROOT)


def cli_subargs(**kwargs):
    return " ".join(f"--{k}={v}" for k, v in kwargs.items())


def cli_args(**kwargs):
    return " ".join(f"{k} {cli_subargs(**v)}" for k, v in kwargs.items())


def find_data_file(example):
    data_file = f"{os.path.join(EXAMPLE, example)}.data.json"
    if os.path.isfile(data_file):
        return data_file
    return None


def run_polystan_example(example, data_file=None, **kwargs):

    make_polystan(example)

    args = {"random": {"seed": SEED}, "polychord": {
        "seed": SEED, "overwrite": 1}}

    for k, v in kwargs.items():
        args[k].update(v)

    if data_file is None:
        data_file = find_data_file(example)

    if data_file is not None:
        args["data"] = {"file": data_file}

    subprocess.check_call(
        f"./{example} {cli_args(**args)}", shell=True, cwd=EXAMPLE)

    return read_polystan_evidence(example)


def read_polystan_evidence(example):
    result_name = os.path.join(EXAMPLE, f"{example}.json")
    with open(result_name, "r", encoding="utf-8") as result_file:
        result = json.load(result_file)
    data = result["sample_stats"]["evidence"]
    data["neval"] = result["sample_stats"]["neval"]
    return data


@pytest.mark.parametrize("example", EXAMPLES)
def test_evidence(example):
    assert run_polystan_example(example)["log evidence"] == EXPECTED[example]


if __name__ == "__main__":

    expected = {}

    for example in EXAMPLES:
        expected[example] = run_polystan_example(example)["log evidence"]

    with open(EXPECTED_FILE_NAME, "w", encoding="utf-8") as expected_file:
        json.dump(expected, expected_file)
