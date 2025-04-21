<h1 align="center">
Bernoulli
</h1>

<div align="center">
<i>Build, run and analyse results from Bernoulli model. </i>
</div>
<br>

## Build & run

```bash
make examples/bernoulli  # builds everything required for this model
./examples/bernoulli data --file examples/bernoulli.data.json  # runs model
```

This produces data from polystan
```
bernoulli.json
bernoulli.toml
```
and inside the chains folder data from polychord
```
bernoulli.prior_info
bernoulli.resume
bernoulli.stats
bernoulli_dead-birth.txt
bernoulli_dead.txt
bernoulli_equal_weights.txt
bernoulli_phys_live-birth.txt
bernoulli_phys_live.txt
bernoulli_prior.txt
```

For future reference, you can re-run with identical settings by
```
./examples/bernoulli --from-toml bernoulli.toml
```

## Analyse

Now we can examine the results. E.g.,
```
cat chains/bernoulli.stats 
```
shows us the evidence estimate and error.

For plotting, we can read the samples in the json file. Plot e.g., with arviz,
```python
import arviz as az
import matplotlib.pyplot as plt

id = az.from_json('bernoulli.json')

az.plot_density(id)
plt.show()
```
