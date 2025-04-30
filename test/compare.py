"""
Compare bridgesampling and polystan
===================================
"""

from test_example import run_polystan_example, run_r_example, find_data_file, EXAMPLES


def run_ps_bs(example):
    data_file = find_data_file(example)
    ps = run_polystan_example(example, data_file)
    bs = run_r_example(example, data_file)

    print(f"example: {example}")
    print(f"polystan: {ps}")
    print(f"bridgesampling: {bs}")


for example in EXAMPLES:
    run_ps_bs(example)
