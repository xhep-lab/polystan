"""
Compare bridge sampling and polystan
====================================
"""

import json
import os
import sys

from examples import examples
from run_polystan import run_polystan
from run_bridge_sampling import run_bridge_sampling


if __name__ == "__main__":

    results = {}

    if sys.argv[1:]:
        examples_ = [os.path.abspath(a) for a in sys.argv[1:]]
    else:
        examples_ = examples()

    for example in examples_:
        bridge_sampling = run_bridge_sampling(example)
        polystan = run_polystan(example)

        polystan_summary = {"log evidence": polystan['sample_stats']['evidence'].data[0][0]['log evidence'],
                            "error log evidence": polystan['sample_stats']['evidence'].data[0][0]['error log evidence'],
                            "neval": polystan['sample_stats']['neval'].data[0][0]['neval']}

        print(f"# example: {example}")
        print(f"polystan: {polystan_summary}")
        print(f"bridge_sampling: {bridge_sampling}")

        name = os.path.splitext(os.path.split(example)[1])[0]
        results[name] = {"polystan": polystan_summary,
                         "bridge_sampling": bridge_sampling}

    with open("compare.json", "w", encoding="utf-8") as f:
        json.dump(results, f, indent=4)
