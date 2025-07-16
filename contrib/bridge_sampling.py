"""
Run bridge sampling
===================
"""

import os
import rpy2.robjects

CWD = os.path.dirname(os.path.realpath(__file__))
SCRIPT = os.path.join(CWD, "bridge_sampling.R")
HEADERS = os.path.normpath(os.path.join(CWD, "..", "stanfunctions"))

R_OBJECT = rpy2.robjects.r
R_OBJECT.source(SCRIPT)
BRIDGE_SAMPLING = R_OBJECT.bridge_sampling


def run_bridge_sampling(stan_file, iter_=20_000, warmup=5_000, chains=4, seed=0):
    target = os.path.splitext(stan_file)[0]
    data = list(BRIDGE_SAMPLING(
        target, iter_, warmup, chains, seed, HEADERS))
    data[-1] = int(data[-1])
    keys = ["log evidence", "error log evidence", "neval"]
    return dict(zip(keys, data))
