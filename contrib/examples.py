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
        os.path.join(dir_, os.path.splitext(n)[0])
        for n in os.listdir(dir_)
        if n.endswith(".stan")
    ]


def examples():
    return search_programs(EXAMPLE_DIR)


def example(name):
    target = os.path.join(EXAMPLE_DIR, name)
    stan_file = f"{target}.stan"
    assert os.path.isfile(stan_file)
    return target
