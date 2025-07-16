"""
Reproduce figure from Stan docs
===============================

https://mc-stan.org/docs/stan-users-guide/img/s-discrete-posterior.png
"""

import matplotlib.pyplot as plt
import numpy as np
import pytest

from polystan import run_polystan
from examples import example


PLOT_SETTINGS = {"no-feedback": True, "nlive": 1000, "write-samples": True}


@pytest.mark.mpl_image_compare(savefig_kwargs={"format": "pdf"})
def test_disaster_fig():

    # styling
    plt.style.use('ggplot')
    plt.rcParams.update({'font.size': 12, 'font.family': 'Liberation Sans'})

    # make figure
    fig, ax = plt.subplots(layout="constrained")
    ax.set_box_aspect(1)

    # make data
    target = example("disaster.stan")
    data = run_polystan(target, polychord=PLOT_SETTINGS)

    # fetch data
    shift = data["posterior"]["save_s"].to_numpy().astype(int).flatten()
    shift += 1851

    # histogram data
    factor = 4000 / len(shift)
    bins = np.arange(shift.min() - 0.5, shift.max() + 0.5)
    counts, bins = np.histogram(shift, bins=bins)
    ax.hist(bins[:-1], bins, weights=factor * counts,
            rwidth=1, edgecolor="black", linewidth=1)

    # ticks
    ax.set_xticks([1885, 1890, 1895, 1900])
    ax.set_xlim(1881.25, 1901.25)
    ax.set_yticks([0, 250, 500, 750])
    ax.set_ylim(-50)

    # labels
    ax.set_xlabel("year", color="black")
    ax.set_ylabel("frequency in 4000 draws", color="black")

    return fig
