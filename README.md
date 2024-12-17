# `PolyStan`

`PolyStan` allows you to use the [`PolyChord`](https://github.com/PolyChord) nested sampling algorithm on [`Stan`](https://mc-stan.org/docs/reference-manual/blocks.html) models. This algorithm provides posterior samples and an estimate of the Bayesian evidence (a.k.a. normalizing constant and marginal likelihood) for use in Bayesian model comparison. For background reading, see e.g., [[1](https://arxiv.org/abs/2205.15570), [2](https://arxiv.org/abs/1502.01856)].

# Quickstart

You need a few basic dependencies
```bash
sudo apt-get install make gcc g++ libopenmpi-dev git  # on Ubuntu/debian
sudo dnf install make gcc g++ openmpi openmpi-devel git && module load mpi/openmpi  # Fedora
```

You need to clone recursively to obtain PolyChord and Stan code
```bash
git clone --recursive https://github.com/andrewfowlie/polystan  # fresh clone
git submodule update --init --recursive  # update an existing clone
```
Lastly, build and run a model, e.g.,
```bash
make PS_MODEL=./examples/bernoulli.stan  # builds everything required for this model
./build/bernoulli/run data --file examples/bernoulli.data.json  # runs model
```
For further runtime options, see
```bash
./build/bernoulli/run --help  # options
./build/bernoulli/run polychord --help  # polychord options
```

# Supported Stan models

The underlying model parameters block should be defined on a unit hypercube with constraints, e.g., a 3-dimensional model
```stan
parameters {
    vector<lower=0, upper=1>[3] x;
}
```
though it needn't be a vector. E.g.,
```stan    
parameters {
    real<lower=0, upper=1> x;
    real<lower=0, upper=1> y;
    real<lower=0, upper=1> z;
}
```
would be acceptable.

You can transform them into your parameters, by e.g.,
```stan
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
```
Here we used transformation functions from `polystan.stanfunctions` to transform from the unit hypercube to parameters with a flat distribution, a normal distribution and a log-normal distribution.
