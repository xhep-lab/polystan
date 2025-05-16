functions {
  #include polystan.stanfunctions
}
data {
  real sigma_1;
  real sigma_2;
}
parameters {
  real<lower=0, upper=1> x;
}
transformed parameters {
  real y = flat_prior(x, -100, 100);
}
model {
  target += log_sum_exp(normal_lpdf(y | 0, sigma_2),
                        normal_lpdf(y | 0, sigma_1));
}
