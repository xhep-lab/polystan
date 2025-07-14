"""
Test priors to check transforms
===============================
"""

import arviz as az
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import beta, betaprime, cauchy, expon, mielke, norm
import pytest

from run_polystan import run_polystan
from examples import example


PLOT_SETTINGS = {
    "no-derived": False,
    "nlive": 1,
    "nprior": 10000,
    "write-prior": True,
}


TARGET = example("priors.stan")
DATA = run_polystan(TARGET, polychord=PLOT_SETTINGS)


def plot_1d(data, x, y, name, **kwargs):
    fig, ax = plt.subplots()

    az.plot_posterior(
        data, name, group="prior", hdi_prob="hide", point_estimate=None, ax=ax, **kwargs
    )
    ax.plot(x, y, ls=":")
    ax.set_xlim(x.min(), x.max())
    return fig


def plot_2d(data, name, **kwargs):
    fig, ax = plt.subplots()
    az.plot_kde(data["prior"][name + ".1"],
                data["prior"][name + ".2"], ax=ax, **kwargs)
    return fig


@pytest.mark.mpl_image_compare
def test_flat_prior():
    x = np.linspace(5, 10, 1000)
    y = np.ones_like(x) / 5.0
    return plot_1d(DATA, x, y, "flat")


@pytest.mark.mpl_image_compare
def test_log_prior():
    x = np.linspace(0, 3, 1000)
    y = np.ones_like(x) / 3.0
    return plot_1d(DATA, x, y, "log_", transform=np.log10)


@pytest.mark.mpl_image_compare
def test_std_normal_prior():
    x = np.linspace(-5, 5, 1000)
    y = norm.pdf(x)
    return plot_1d(DATA, x, y, "std_normal")


@pytest.mark.mpl_image_compare
def test_half_std_normal_prior():
    x = np.linspace(0, 5, 1000)
    y = 2 * norm.pdf(x)
    return plot_1d(DATA, x, y, "half_std_normal")


@pytest.mark.mpl_image_compare
def test_normal_prior():
    x = np.linspace(-5, 25, 1000)
    y = norm.pdf(x, 10, 3)
    return plot_1d(DATA, x, y, "normal")


@pytest.mark.mpl_image_compare
def test_half_normal_prior():
    x = np.linspace(50, 100, 1000)
    y = 2 * norm.pdf(x, 50, 10)
    return plot_1d(DATA, x, y, "half_normal")


@pytest.mark.mpl_image_compare
def test_log10normal_prior():
    x = np.linspace(-90, 110, 1000)
    y = norm.pdf(x, 10, 20)
    return plot_1d(DATA, x, y, "log10normal", transform=np.log10)


@pytest.mark.mpl_image_compare
def test_lognormal_prior():
    x = np.linspace(-6, 14, 1000)
    y = norm.pdf(x, 4, 2)
    return plot_1d(DATA, x, y, "lognormal", transform=np.log)


@pytest.mark.mpl_image_compare
def test_beta_prior():
    x = np.linspace(0, 1, 1000)
    y = beta.pdf(x, 0.5, 0.5)
    return plot_1d(DATA, x, y, "beta")


@pytest.mark.mpl_image_compare
def test_beta_prime_prior():

    x = np.linspace(0, 10, 1000)
    y = betaprime.pdf(x, 2, 2)
    return plot_1d(DATA,
                   x,
                   y,
                   "beta_prime",
                   kind="hist",
                   bins=np.linspace(x.min(), x.max(), 500),
                   density=True,
                   )


@pytest.mark.mpl_image_compare
def test_dagum_prior():

    def dagum(p, a, b):
        return mielke(a * p, a, scale=b)

    x = np.linspace(0, 10, 1000)
    y = dagum(1, 1, 1).pdf(x)
    return plot_1d(DATA,
                   x,
                   y,
                   "dagum",
                   kind="hist",
                   bins=np.linspace(x.min(), x.max(), 500),
                   density=True,
                   )


@pytest.mark.mpl_image_compare
def test_exponential_prior():
    x = np.linspace(0, 10, 1000)
    y = expon.pdf(x, 0.0, 0.5)
    return plot_1d(DATA, x, y, "exponential")


@pytest.mark.mpl_image_compare
def test_cauchy_prior():
    x = np.linspace(-35, 25, 1000)
    y = cauchy.pdf(x, -5, 2)
    return plot_1d(DATA,
                   x,
                   y,
                   "cauchy",
                   kind="hist",
                   bins=np.linspace(x.min(), x.max(), 500),
                   density=True,
                   )


@pytest.mark.mpl_image_compare
def test_half_cauchy_prior():
    x = np.linspace(8, 38, 1000)
    y = 2.0 * cauchy.pdf(x, 8, 3)
    return plot_1d(DATA,
                   x,
                   y,
                   "half_cauchy",
                   kind="hist",
                   bins=np.linspace(x.min(), x.max(), 500),
                   density=True,
                   )


@pytest.mark.mpl_image_compare
def test_multi_normal_prior():
    return plot_2d(DATA, "multi_normal")


@pytest.mark.mpl_image_compare
def test_multi_normal_cholesky_prior():
    return plot_2d(DATA, "multi_normal_cholesky")
