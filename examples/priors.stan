functions {
  #include polystan.stanfunctions
}
parameters {
  real<lower=0, upper=1> x;
  vector<lower=0, upper=1>[2] y;
}
transformed parameters {
  real flat = flat_prior(x, 5, 10);
  real log_ = log_prior(x, 1, 1000);
  real std_normal = std_normal_prior(x);
  real half_std_normal = half_std_normal_prior(x);
  real normal = normal_prior(x, 10, 3);
  real half_normal = half_normal_prior(x, 50, 10);
  vector[2] multi_normal = multi_normal_prior(y, [2, 3]',
                                              [[1, 0.5], [0.5, 0.5]]);
  vector[2] multi_normal_cholesky = multi_normal_cholesky_prior(y, [2, 3]',
                                                                [[1, 0],
                                                                 [0.5, 0.5]]);
  real log10normal = log10normal_prior(x, 10, 20);
  real lognormal = lognormal_prior(x, 4, 2);
  real beta = beta_prior(x, 0.5, 0.5);
  real exponential = exponential_prior(x, 2);
  real cauchy = cauchy_prior(x, -5, 2);
  real half_cauchy = half_cauchy_prior(x, 8, 3);
  real dagum = dagum_prior(x, 1, 1, 1);
  real beta_prime = beta_prime_prior(x, 2, 2);
}
