"""
Script to ease writing PolyStan models
======================================
"""

import io
import sys
import re
from contextlib import redirect_stdout

from cmdstanpy import format_stan_file, compilation


def stan_format(file_name):
    f = io.StringIO()

    with redirect_stdout(f):
        format_stan_file(file_name)

    return f.getvalue().strip()


def find_parameter_names(file_name):
    data = compilation.src_info(file_name, compilation.CompilerOptions())
    return data["parameters"]


def make_physical(parameter_names, line):
    for p in parameter_names:
        line = line.replace(f" {p};", f" {p} = INVERSE_TRANSFORM(unit_{p});")
    return line


def make_unit(parameter_names, line):
    line = re.sub(r'<.*?>', "", line)  # remove any existing constraint

    for p in parameter_names:
        line = line.replace(f" {p};", f"<lower=0, upper=1> unit_{p};")

    return line


def split(src):
    before, after = src.split("parameters {\n", 1)
    parameters, after = after.split("\n}", 1)

    try:
        _, after = src.split("transformed parameters {\n", 1)
        transformed, after = after.split("\n}", 1)
    except ValueError:
        transformed = str()

    return before, parameters, transformed, after


def convert_file(file_name):

    src = stan_format(file_name)
    parameter_names = find_parameter_names(file_name)
    before, parameters, transformed, after = split(src)

    yield from before.splitlines()

    yield "parameters {"

    for line in parameters.splitlines():
        yield make_unit(parameter_names, line)

    yield "}"

    yield "transformed parameters {"

    for line in parameters.splitlines():
        yield make_physical(parameter_names, line)

    yield from transformed.splitlines()

    yield "}"

    yield from after.splitlines()



if __name__ == "__main__":
    file_name = sys.argv[1]
    for line in convert_file(file_name):
        print(line)
