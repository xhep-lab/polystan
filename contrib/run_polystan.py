"""
Python wrapper to build & run polystan program
==============================================
"""

import os
import subprocess

import arviz as az

CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))


def make_polystan(target):
    subprocess.check_call(f"make {target}", shell=True, cwd=ROOT)


def cli_subargs(**kwargs):
    return " ".join(f"--{k}={v}" for k, v in kwargs.items())


def cli_args(**kwargs):
    return " ".join(f"{k} {cli_subargs(**v)}" for k, v in kwargs.items())


def find_data_file(target):
    data_file = f"{target}.data.json"
    if os.path.isfile(data_file):
        return data_file
    return None


def run_polystan(stan_file, data_file=None, seed=0, **kwargs):

    target = os.path.splitext(stan_file)[0]

    make_polystan(target)

    args = {
        "random": {"seed": seed},
        "polychord": {
            "seed": seed,
            "overwrite": True,
        },
    }

    for k, v in kwargs.items():
        args[k].update(v)

    if data_file is None:
        data_file = find_data_file(target)

    if data_file is not None:
        args["data"] = {"file": data_file}

    subprocess.check_call(f"{target} {cli_args(**args)}", shell=True)

    name = os.path.split(target)[1]
    result_name = f"{name}.json"
    return az.from_json(result_name)
