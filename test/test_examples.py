"""
Python wrapper around calls for tests
=====================================
"""

import json
import os
import subprocess

import pytest

CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))

EXAMPLE = os.path.join(ROOT, "examples")

EXAMPLES = [
    os.path.join(EXAMPLE, os.path.splitext(n)[0])
    for n in os.listdir(EXAMPLE)
    if n.endswith(".stan")
]

EXPECTED_FILE_NAME = os.path.join(CWD, "expected_logz_examples.json")
with open(EXPECTED_FILE_NAME, "r", encoding="utf-8") as expected_file:
    EXPECTED = json.load(expected_file)

SEED = 127


def make_polystan(example):
    subprocess.check_call(f"make {example}", shell=True, cwd=ROOT)


def cli_subargs(**kwargs):
    return " ".join(f"--{k}={v}" for k, v in kwargs.items())


def cli_args(**kwargs):
    return " ".join(f"{k} {cli_subargs(**v)}" for k, v in kwargs.items())


def find_data_file(example):
    data_file = f"{example}.data.json"
    if os.path.isfile(data_file):
        return data_file
    return None


def run_polystan_example(example, data_file=None, seed=None, **kwargs):

    make_polystan(example)

    if seed is None:
        seed = SEED

    args = {
        "random": {"seed": seed},
        "polychord": {
            "seed": seed,
            "overwrite": True,
            "no-write": True,
            "no-derived": True,
            "write-stats": True,
        },
    }

    for k, v in kwargs.items():
        args[k].update(v)

    if data_file is None:
        data_file = find_data_file(example)

    if data_file is not None:
        args["data"] = {"file": data_file}

    subprocess.check_call(f"{example} {cli_args(**args)}", shell=True)

    return read_polystan_evidence(example)


def read_polystan_evidence(example):
    name = os.path.split(example)[1]
    result_name = f"{name}.json"
    with open(result_name, "r", encoding="utf-8") as result_file:
        result = json.load(result_file)
    data = result["sample_stats"]["evidence"]
    data["neval"] = result["sample_stats"]["neval"]
    return data


@pytest.mark.parametrize("example", EXAMPLES)
def test_evidence(example):
    name = os.path.split(example)[1]
    assert run_polystan_example(example)["log evidence"] == EXPECTED[name]


if __name__ == "__main__":

    expected = {}

    for example in EXAMPLES:
        name = os.path.split(example)[1]
        expected[name] = run_polystan_example(example)["log evidence"]

    with open(EXPECTED_FILE_NAME, "w", encoding="utf-8") as expected_file:
        json.dump(expected, expected_file)
