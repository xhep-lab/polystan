# This is a guide to reproducing examples from the paper on a
# Ubuntu/debian system. These commands are tested by a CI job here
# https://github.com/xhep-lab/polystan/actions

# First, we install necessary dependencies for polystan

sudo apt-get install git make gcc gfortran libopenmpi-dev

# We reproduce the examples in the paper using Python scripts
# so require some Python dependencies

sudo apt install python3 python3-pip python3-venv python3-pytest

# To compare pdf figures to those expected, we require

sudo apt install ghostscript

# We also install some dependencies for our comparisons to bridge sampling
# which is in R

sudo apt install libtirpc-dev r-base-dev

# We clone the source code recursively to obtain the code and submodules

git clone --recursive https://github.com/xhep-lab/polystan

# As a preliminary check, we can now compile a simple example

cd polystan
make examples/bernoulli

# and run the resulting executable

./examples/bernoulli data --file examples/bernoulli.data.json

# Now we reproduce results from the paper. This requires some Python 
# dependencies. We will use a virtual environment

python3 -m venv .venv
source .venv/bin/activate

# and install the Python dependencies there

pip install .

# Now we verify figure 2 from a Python script

pytest -k test_disaster_fig .

# This verifies that one can reproduce figure 2. Take a look at

xdg-open contrib/baseline/test_disaster_fig.pdf

# Now we verify the data in the results section. This may take some time
# e.g., about 10 minutes on my system

pytest -k test_compare .

# This verifies that you can reproduce the data. To see the data,

cat contrib/__snapshot__/test_compare.ambr
