"""
Run bridgesampling
==================
"""

import os
import rpy2.robjects as ro

CWD = os.path.dirname(os.path.realpath(__file__))
SCRIPT = os.path.join(CWD, "bridge_sampling.R")
HEADERS = os.path.normpath(os.path.join(CWD, "..", "stanfunctions"))


def run_bridge_sampling(target, iter_=20_000, warmup=5_000, chains=4, seed=0):
    r = ro.r
    r.source(SCRIPT)
    data = list(r.bridge_sampling(target, iter_, warmup, chains, seed, HEADERS))
    data[-1] = int(data[-1])
    keys = ["log evidence", "error log evidence", "neval"]
    return dict(zip(keys, data))
