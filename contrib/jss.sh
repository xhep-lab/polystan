# This is a guide to reproducing examples from the paper on a
# Ubuntu/debian system. These commands are tested by a CI job here
# https://github.com/xhep-lab/polystan/actions

# First, we install necessary dependencies for polystan

sudo apt-get install git make gcc gfortran libopenmpi-dev

# We also install some dependencies for our R comparisons

sudo apt install python3-dev python3-pip python3-pytest libtirpc-dev r-base-dev

# We clone the source code recursively to obtain the code and submodules

git clone --recursive https://github.com/xhep-lab/polystan

# We can now compile a simple example

cd polystan
make examples/bernoulli

# and run the resulting executable

./examples/bernoulli data --file examples/bernoulli.data.json

# Now we reproduce results from the paper. This requires some Python 
# dependencies. We will use a virtual environment

sudo apt install python3 python3-venv
python3 -m venv .venv
source .venv/bin/activate
pip install -r contrib/requirements.txt

# Now we reproduce figure 2 from a Python script

python3 contrib/test_disaster.py

# This creates "disaster.pdf". Take a look

xdg-open disaster.pdf

# Now we reproduce comparisons of bridge sampling and polystan that appear in
# Section 6 Examples from a Python script. This command may take some time

python3 contrib/test_compare.py

# You should see results from polystan and bridge sampling printed to the
# screen

# Lastly, we can run the unit tests. Amongst other things, this checks
# all results in the paper. This may take some time

python3 -m pytest .
