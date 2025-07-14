"""
Find example programs
=====================
"""

import os


CWD = os.path.dirname(os.path.realpath(__file__))
ROOT = os.path.normpath(os.path.join(CWD, ".."))
EXAMPLE_DIR = os.path.join(ROOT, "examples")


def search_programs(dir_):
    return [
        os.path.join(dir_, n)
        for n in os.listdir(dir_)
        if n.endswith(".stan")
    ]


def examples():
    return search_programs(EXAMPLE_DIR)


def example(name):
    stan_file = os.path.join(EXAMPLE_DIR, name)
    assert os.path.isfile(stan_file)
    return stan_file
