# polystan

Polystan allows you to use the PolyChord nested sampling algorithm on Stan models. This provides posterior samples and an estimate of the Bayesian evidence (a.k.a. normalizing constant and marginal likelihood) for use in Bayesian model comparison.

# Quickstart

## Dependencies

You need a few basic dependencies; on Ubuntu/debian

    sudo apt-get install make gcc g++ libopenmpi-dev git

on Fedora,

    sudo dnf install make gcc g++ openmpi openmpi-devel git

You need to clone recusively; either a fresh clone

    git clone --recursive https://github.com/andrewfowlie/polystan

or if you already cloned but not recursively

    git submodule update --init --recursive

Lastly, build and run a model, e.g.,

    make PS_FILE=./examples/bernoulli.stan
    ./build/bernoulli/run data --file examples/bernoulli.data.json

# Supported Stan models

The underlying model parameters block should be defined on a unit hypercube with constraints, e.g., a 3-dimensional model

  parameters {
    vector<lower=0, upper=1>[3] x;
  }

though it needn't be a vector. E.g.,

  parameters {
    real<lower=0, upper=1> x;
    real<lower=0, upper=1> y;
    real<lower=0, upper=1> z;
  }

would be acceptable.

You can transform them into your parameters, by e.g.,

  functions {
    #include polystan.stanfunctions
  }
  parameters {
    vector<lower=0, upper=1>[3] x;
  }
  transformed parameters {
    real theta = flat_prior(x[1], -5, 5);
    real phi = normal_prior(x[2], 0, 1);
    real omega = log10_normal_prior(x[3], 0, 1);
  }

Here we used transformation functions from `polystan.stanfunctions` to transform from the unit hypercube to parameters with a uniform distribution, a normal distribution and a log-normal distribution.
