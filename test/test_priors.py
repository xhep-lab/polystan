"""
Inspect priors to check transforms
==================================
"""

import arviz as az

import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm, beta, expon, cauchy

from test_examples import run_polystan_example


def test_priors():
    run_polystan_example("priors")


def plotter(x, y, name, **kwargs):
    plt.clf()
    az.plot_posterior(data, name, group="prior",
                      hdi_prob="hide", point_estimate=None, **kwargs)
    plt.plot(x, y, ls=":")
    plt.xlim(x.min(), x.max())
    plt.savefig(name + ".pdf")


def plotter2d(name):
    plt.clf()
    az.plot_kde(data["prior"][name + ".1"], data["prior"][name + ".2"])
    plt.savefig(name + ".pdf")


if __name__ == "__main__":

    run_polystan_example("priors")
    data = az.from_json("priors.json")

    # flat_prior

    x = np.linspace(5, 10, 1000)
    y = np.ones_like(x) / 5.
    plotter(x, y, "flat")

    # log_prior

    x = np.linspace(0, 3, 1000)
    y = np.ones_like(x) / 3.
    plotter(x, y, "log_", transform=np.log10)

    # std_normal_prior

    x = np.linspace(-5, 5, 1000)
    y = norm.pdf(x)
    plotter(x, y, "std_normal")

    # half_std_normal_prior

    x = np.linspace(0, 5, 1000)
    y = 2 * norm.pdf(x)
    plotter(x, y, "half_std_normal")

    # normal_prior

    x = np.linspace(-5, 25, 1000)
    y = norm.pdf(x, 10, 3)
    plotter(x, y, "normal")

    # half_normal_prior

    x = np.linspace(50, 100, 1000)
    y = 2 * norm.pdf(x, 50, 10)
    plotter(x, y, "half_normal")

    # log10normal_prior

    x = np.linspace(-90, 110, 1000)
    y = norm.pdf(x, 10, 20)
    plotter(x, y, "log10normal", transform=np.log10)

    # lognormal_prior

    x = np.linspace(-6, 14, 1000)
    y = norm.pdf(x, 4, 2)
    plotter(x, y, "lognormal", transform=np.log)

    # beta_prior

    x = np.linspace(0, 1, 1000)
    y = beta.pdf(x, 0.5, 0.5)
    plotter(x, y, "beta")

    # exponential_prior

    x = np.linspace(0, 10, 1000)
    y = expon.pdf(x, 0., 0.5)
    plotter(x, y, "exponential")

    # cauchy_prior

    x = np.linspace(-35, 25, 1000)
    y = cauchy.pdf(x, -5, 2)
    plotter(x, y, "cauchy", kind="hist", bins=np.linspace(
        x.min(), x.max(), 500), density=True)

    # half_cauchy_prior

    x = np.linspace(8, 38, 1000)
    y = 2. * cauchy.pdf(x, 8, 3)
    plotter(x, y, "half_cauchy", kind="hist", bins=np.linspace(
        x.min(), x.max(), 500), density=True)

    # multi_normal_prior

    plotter2d("multi_normal")

    # multi_normal_cholesky_prior

    plotter2d("multi_normal_cholesky")
