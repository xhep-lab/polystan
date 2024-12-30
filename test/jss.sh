# build everything required for this model

make ./examples/bernoulli.stan  

# run model with fixed seeds

./examples/bernoulli data --file examples/bernoulli.data.json random --seed=11 polychord --seed=12 --overwrite

# check evidence estimate etc

cat chains/bernoulli.stats

# check log evidence value

log_evidence=$(grep -Po '"log evidence": .*?[^\\],' bernoulli.json)

expected='"log evidence": -6.186195373535156,'

echo "$log_evidence" versus "$expected"

if [ "$log_evidence" != "$expected" ]; then
  exit 1
fi
