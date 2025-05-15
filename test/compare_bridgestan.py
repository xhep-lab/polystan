"""
Compare bridgesampling and polystan
===================================
"""

import subprocess

from test_example import EXAMPLE, EXAMPLES, find_data_file, run_polystan_example


class parse_r:
    @staticmethod
    def __class_getitem__(type):
        return lambda line: type(str(line).split("]", 1)[-1].strip(" '"))


def read_r_evidence(result):
    neval_line, logz_line, err_line = result.splitlines()[-3:]
    logz = parse_r[float](logz_line)
    err = parse_r[float](err_line)
    neval = parse_r[int](neval_line)
    return {"log evidence": logz, "error log evidence": err, "neval": neval}


def run_r_example(example, data_file=None):
    if data_file is None:
        data_file = find_data_file(example)
    result = subprocess.check_output(f"Rscript bs.R {example}", shell=True, cwd=EXAMPLE)
    return read_r_evidence(result)


def run_ps_bs(example):
    data_file = find_data_file(example)
    ps = run_polystan_example(example, data_file)
    bs = run_r_example(example, data_file)
    return ps, bs


if __name__ == "__main__":

    for example in EXAMPLES:
        ps, bs = run_ps_bs(example)
        print(f"# example: {example}")
        print(f"polystan: {ps}")
        print(f"bridgesampling: {bs}")
