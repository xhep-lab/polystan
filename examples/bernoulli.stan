functions {
  #include polystan.stanfunctions
}
data {
  int<lower=0> N;
  array[N] int<lower=0, upper=1> y;
}
parameters {
  real<lower=0, upper=1> x;
}
transformed parameters {
  real theta = beta_prior(x, 1, 1);
  real logit_theta = logit(theta);
}
model {
  y ~ bernoulli(theta);
}
generated quantities {
  int y_sim = bernoulli_rng(theta);
}
