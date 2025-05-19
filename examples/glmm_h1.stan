functions {
  #include polystan.stanfunctions
}
data {
  int<lower=1> n_turtles;
  array[n_turtles] int<lower=0, upper=1> survived;
  vector<lower=0>[n_turtles] weight;
  
  int<lower=1> n_clutches;
  array[n_turtles] int<lower=1, upper=n_clutches> clutch;
}
transformed data {
  real sigma_alpha = sqrt(10.0);
}
parameters {
  vector<lower=0, upper=1>[2] x_alpha;
  
  real<lower=0, upper=1> x_sigma_effect;
  vector<lower=0, upper=1>[n_clutches] x_b;
}
transformed parameters {
  vector[2] alpha = sigma_alpha * std_normal_prior(x_alpha);
  
  real sigma_effect = dagum_prior(x_sigma_effect, 1., 2., 1.);
  vector[n_clutches] effect_by_clutch = sigma_effect * std_normal_prior(x_b);
  vector[n_turtles] effect;
  for (i in 1 : n_turtles) {
    effect[i] = effect_by_clutch[clutch[i]];
  }
  
  vector[n_turtles] p = Phi(alpha[1] + alpha[2] * weight + effect);
}
model {
  target += bernoulli_lpmf(survived | p);
}
