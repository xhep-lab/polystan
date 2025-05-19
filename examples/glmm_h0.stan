functions {
  #include polystan.stanfunctions
}
data {
  int<lower=1> n_turtles;
  array[n_turtles] int<lower=0, upper=1> survived;
  vector<lower=0>[n_turtles] weight;
}
transformed data {
  real sigma_alpha = sqrt(10.0);
}
parameters {
  vector<lower=0, upper=1>[2] x_alpha;
}
transformed parameters {
  vector[2] alpha = sigma_alpha * std_normal_prior(x_alpha);
  vector[n_turtles] p = Phi(alpha[1] + alpha[2] * weight);
}
model {
  target += bernoulli_lpmf(survived | p);
}
