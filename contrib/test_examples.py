"""
Python wrapper around calls for tests
=====================================
"""

import json
import os

import pytest

from run_polystan import run_polystan
from examples import examples


CWD = os.path.dirname(os.path.realpath(__file__))
EXPECTED_FILE_NAME = os.path.join(CWD, "expected_logz_examples.json")


def read_evidence(data):
    return data['sample_stats']['evidence'].data[0][0]['log evidence']


def read_expected():
    with open(EXPECTED_FILE_NAME, "r", encoding="utf-8") as expected_file:
        return json.load(expected_file)


def write_expected():
    expected = {}

    for example in examples():
        name = os.path.split(example)[1]
        expected[name] = read_evidence(run_polystan(example))

    with open(EXPECTED_FILE_NAME, "w", encoding="utf-8") as expected_file:
        json.dump(expected, expected_file)


@pytest.mark.parametrize("example", examples())
def test_evidence(example):
    name = os.path.split(example)[1]
    data = run_polystan(example)
    assert read_evidence(data) == read_expected()[name]


if __name__ == "__main__":
    write_expected()
