"""
Reproduce figure from Stan docs
===============================

https://mc-stan.org/docs/stan-users-guide/img/s-discrete-posterior.png
"""

import os

import arviz as az
import matplotlib.pyplot as plt
import numpy as np
from test_examples import EXAMPLE, run_polystan_example


plt.style.use('ggplot')
plt.rcParams.update({'font.size': 12, 'font.family': 'Liberation Sans'})
target = os.path.join(EXAMPLE, "disaster")


# fetch data
data = az.from_json("disaster.json")
shift = data["posterior"]["save_s"].to_numpy().astype(int).flatten()
shift += 1851

# histogram data
factor = 4000 / len(shift)
bins = np.arange(shift.min() - 0.5, shift.max() + 0.5)
counts, bins = np.histogram(shift, bins=bins)

plt.hist(bins[:-1], bins, weights=factor * counts, rwidth=1, edgecolor="black", linewidth=1)

# ticks
plt.xticks([1885, 1890, 1895, 1900])
plt.xlim(1881.25, 1901.25)
plt.yticks([0, 250, 500, 750])
plt.ylim(-50, None)

# labels
plt.xlabel("year", color="black")
plt.ylabel("frequency in 4000 draws", color="black")

# finalize
plt.gca().set_box_aspect(1)
plt.tight_layout()
plt.savefig("disaster.pdf")
