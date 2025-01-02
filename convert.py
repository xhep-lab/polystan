"""
Script to ease writing PolyStan models
======================================
"""

import io
import json
import subprocess
import sys
from contextlib import redirect_stdout

from cmdstanpy import cmdstan_path, format_stan_file

TYPES = ["real", "vector", "matrix"]


def stan_format(file_name):
    f = io.StringIO()

    with redirect_stdout(f):
        format_stan_file(file_name)

    return f.getvalue().strip()


def parameter_names(file_name):
    str_ = subprocess.check_output([f"{cmdstan_path()}/bin/stanc", "--info", file_name])
    names = json.loads(str_)["parameters"].keys()
    return {f"u{i + 1}": k for i, k in enumerate(names)}


def make_physical(map_, str_):
    for k, v in map_.items():
        str_ = str_.replace(v, f"{v} = INVERSE_TRANSFORM({k})")
    return str_


def make_unit(map_, str_):
    for k, v in map_.items():
        str_ = str_.replace(v, k)

    for t in TYPES:
        str_ = str_.replace(t, f"{t}<lower=0, upper=1>")

    return str_


def find_block(src, block):
    started = False

    for line in src.split("\n"):

        if started and line == "}":
            return

        if started:
            yield line

        if line.startswith(f"{block} {{"):
            started = True


def before_block(src, block):
    for line in src.split("\n"):
        if line.startswith(f"{block} {{"):
            return
        yield line


def after_block(src, block):
    started = False
    ended = False
    for line in src.split("\n"):
        if ended:
            yield line

        if line.startswith(f"{block} {{"):
            started = True

        if started and line == "}":
            ended = True


def convert_parameters(src, map_):

    yield "parameters {"

    for line in find_block(src, "parameters"):
        line = make_unit(map_, line)
        yield line

    yield "}"


def convert_transformed_parameters(src, map_):

    yield "transformed parameters {"

    for line in find_block(src, "parameters"):
        yield make_physical(map_, line)

    yield from find_block(src, "transformed parameters")

    yield "}"


if __name__ == "__main__":
    file_name = sys.argv[1]

    src = stan_format(file_name)
    map_ = parameter_names(file_name)

    for line in before_block(src, "parameters"):
        print(line)

    for line in convert_parameters(src, map_):
        print(line)

    for line in convert_transformed_parameters(src, map_):
        print(line)

    for line in after_block(src, "transformed parameters"):
        print(line)
