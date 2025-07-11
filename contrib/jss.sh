# First, we install necessary dependencies for building on Ubuntu/debian:

sudo apt-get install git make gcc gfortran libopenmpi-dev

# We clone the source code recursively to obtain the code and submodules:

git clone --recursive https://github.com/xhep-lab/polystan

# We can now compile a simple example:

cd polystan
make examples/bernoulli

# and run the resulting executable:

./examples/bernoulli data --file examples/bernoulli.data.json

# Now we reproduce results from the paper. This requires some Python 
# dependencies. We will use a virtual environment:

sudo apt install python3 python3-venv
python3 -m venv .venv
source .venv/bin/activate
pip install -r contrib/requirements.txt

# Now we reproduce figure 2 by creating "disaster.pdf" from a Python script

python3 contrib/plot_disaster.py

# Now we reproduce comparisons of bridge sampling and polystan that appear in
# Section 6 Examples from a Python script

python3 contrib/compare.py

# This creates a data file "compare.json"

# Lastly, to run the unit tests

python3 -m pytest .
