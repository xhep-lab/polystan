functions {
  #include polystan.stanfunctions
}
parameters {
  real<lower=0, upper=1> x;
}
transformed parameters {
  real y = flat_prior(x, -100, 100);
}
model {
  target += log_sum_exp(std_normal_lpdf(y), normal_lpdf(y | 0, 50));
}
