<h1 align="center">
 🌀 PolyStan
</h1>

<div align="center">
<i>PolyChord nested sampling on Stan models. </i>
</div>
<br>

PolyStan allows you to use the [PolyChord](https://github.com/PolyChord) nested sampling algorithm on [Stan](https://mc-stan.org/docs/reference-manual/blocks.html) models. This algorithm provides posterior samples and an estimate of the Bayesian evidence (a.k.a. normalizing constant and marginal likelihood) for use in Bayesian model comparison. For background reading, see e.g., [[1](https://arxiv.org/abs/2205.15570), [2](https://arxiv.org/abs/1502.01856)].

## Install

You need a few basic dependencies
```bash
sudo apt-get install git make gcc gfortran libopenmpi-dev  # on Ubuntu/debian
sudo dnf install git make gcc gfortran openmpi-devel && module load mpi/openmpi-$(uname -m)  # Fedora
```

You need to clone recursively to obtain PolyChord and Stan code
```bash
git clone --recursive https://github.com/xhep-lab/polystan  # fresh clone
git submodule update --init --recursive  # update an existing clone
```
Lastly, you can build and run a model, e.g.,
```bash
cd polystan
make examples/bernoulli  # builds everything required for this model
./examples/bernoulli data --file examples/bernoulli.data.json  # runs model
```
## Run

After building a model, try
```bash
./examples/bernoulli data --file examples/bernoulli.data.json  # runs model
```
For further runtime options, see
```bash
./examples/bernoulli --help  # options
./examples/bernoulli polychord --help  # polychord options
```

## Python interface

You can install a thin Python wrapper
```bash
pip install .
```
To build and run programs inside Python,
```python
from polystan import run_polystan

data = run_polystan("examples/bernoulli.stan")
```

## Outputs

The log evidence and error are printed to the screen, and saved to disk alongside other outputs.

PolyChord's native CSV outputs are explained in the [here](https://github.com/PolyChord/PolyChordLite/tree/master?tab=readme-ov-file#output-files). In addition to those, PolyStan organizes outputs into a JSON file that follows the [arViz InferenceData schema](https://python.arviz.org/en/latest/schema/schema.html). This can be easily read and used by other programs, e.g.,
```python
import arviz as az
data = az.from_json('bernoulli.json')
```
For a complete workflow, including plotting, see [EXAMPLE.md](EXAMPLE.md).

## Supported Stan models

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

## License and citations

If you use PolyStan, you must agree to the PolyChord [LICENSE](https://github.com/PolyChord/PolyChordLite/blob/master/LICENCE) and cite `\cite{Handley:2015fda,Handley:2015vkr,Roualdes2023,MCStan}`

```bibtex
@article{Handley:2015fda,
    title = "{PolyChord: nested sampling for cosmology}",
    author = "Handley, W. J. and Hobson, M. P. and Lasenby, A. N.",
    year = "2015",
    journal = "Mon. Not. Roy. Astron. Soc.",
    volume = "450",
    number = "1",
    pages = "L61--L65",
    doi = "10.1093/mnrasl/slv047",
    eprint = "1502.01856",
    archiveprefix = "arXiv",
    primaryclass = "astro-ph.CO"
}
@article{Handley:2015vkr,
    title = "{polychord: next-generation nested sampling}",
    author = "Handley, W. J. and Hobson, M. P. and Lasenby, A. N.",
    year = "2015",
    journal = "Mon. Not. Roy. Astron. Soc.",
    volume = "453",
    number = "4",
    pages = "4385--4399",
    doi = "10.1093/mnras/stv1911",
    eprint = "1506.00171",
    archiveprefix = "arXiv",
    primaryclass = "astro-ph.IM"
}
@article{Roualdes2023,
    title = {BridgeStan: Efficient in-memory access to the methods of a Stan model},
    author = {Roualdes, Edward A. and Ward, Brian and Carpenter, Bob and Seyboldt, Adrian and Axen, Seth D.},
    year = {2023},
    month = jul,
    journal = {J. Open Source Softw.},
    publisher = {The Open Journal},
    volume = {8},
    number = {87},
    pages = {5236},
    doi = {10.21105/joss.05236},
    issn = {2475-9066}
}
@misc{MCStan,
    title = {Stan Modeling Language User's Guide and Reference Manual},
    author = {Stan Development Team},
    year = 2025,
    url = {https://mc-stan.org}
}
```
