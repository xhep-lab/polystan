"""
Inspect priors to check transforms
==================================
"""

import arviz as az
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import beta, betaprime, cauchy, expon, mielke, norm

from run_polystan import run_polystan
from examples import example


def plotter(data, x, y, name, **kwargs):
    plt.clf()
    az.plot_posterior(
        data, name, group="prior", hdi_prob="hide", point_estimate=None, **kwargs
    )
    plt.plot(x, y, ls=":")
    plt.xlim(x.min(), x.max())
    plt.savefig(name + ".pdf")


def plotter2d(data, name, **kwargs):
    plt.clf()
    az.plot_kde(data["prior"][name + ".1"],
                data["prior"][name + ".2"], **kwargs)
    plt.savefig(name + ".pdf")


if __name__ == "__main__":

    target = example("priors")
    data = run_polystan(
        target,
        polychord={
            "no-derived": False,
            "nlive": 1,
            "nprior": 10000,
            "write-prior": True,
        },
    )

    print(az.summary(data, group="prior"))

    # flat_prior

    x = np.linspace(5, 10, 1000)
    y = np.ones_like(x) / 5.0
    plotter(data, x, y, "flat")

    # log_prior

    x = np.linspace(0, 3, 1000)
    y = np.ones_like(x) / 3.0
    plotter(data, x, y, "log_", transform=np.log10)

    # std_normal_prior

    x = np.linspace(-5, 5, 1000)
    y = norm.pdf(x)
    plotter(data, x, y, "std_normal")

    # half_std_normal_prior

    x = np.linspace(0, 5, 1000)
    y = 2 * norm.pdf(x)
    plotter(data, x, y, "half_std_normal")

    # normal_prior

    x = np.linspace(-5, 25, 1000)
    y = norm.pdf(x, 10, 3)
    plotter(data, x, y, "normal")

    # half_normal_prior

    x = np.linspace(50, 100, 1000)
    y = 2 * norm.pdf(x, 50, 10)
    plotter(data, x, y, "half_normal")

    # log10normal_prior

    x = np.linspace(-90, 110, 1000)
    y = norm.pdf(x, 10, 20)
    plotter(data, x, y, "log10normal", transform=np.log10)

    # lognormal_prior

    x = np.linspace(-6, 14, 1000)
    y = norm.pdf(x, 4, 2)
    plotter(data, x, y, "lognormal", transform=np.log)

    # beta_prior

    x = np.linspace(0, 1, 1000)
    y = beta.pdf(x, 0.5, 0.5)
    plotter(data, x, y, "beta")

    # beta_prime_prior

    x = np.linspace(0, 10, 1000)
    y = betaprime.pdf(x, 2, 2)
    plotter(data,
            x,
            y,
            "beta_prime",
            kind="hist",
            bins=np.linspace(x.min(), x.max(), 500),
            density=True,
            )

    # dagum prior

    def dagum(p, a, b):
        return mielke(a * p, a, scale=b)

    x = np.linspace(0, 10, 1000)
    y = dagum(1, 1, 1).pdf(x)
    plotter(data,
            x,
            y,
            "dagum",
            kind="hist",
            bins=np.linspace(x.min(), x.max(), 500),
            density=True,
            )

    # exponential_prior

    x = np.linspace(0, 10, 1000)
    y = expon.pdf(x, 0.0, 0.5)
    plotter(data, x, y, "exponential")

    # cauchy_prior

    x = np.linspace(-35, 25, 1000)
    y = cauchy.pdf(x, -5, 2)
    plotter(data,
            x,
            y,
            "cauchy",
            kind="hist",
            bins=np.linspace(x.min(), x.max(), 500),
            density=True,
            )

    # half_cauchy_prior

    x = np.linspace(8, 38, 1000)
    y = 2.0 * cauchy.pdf(x, 8, 3)
    plotter(data,
            x,
            y,
            "half_cauchy",
            kind="hist",
            bins=np.linspace(x.min(), x.max(), 500),
            density=True,
            )

    # multi_normal_prior

    plotter2d(data, "multi_normal")

    # multi_normal_cholesky_prior

    plotter2d(data, "multi_normal_cholesky")
