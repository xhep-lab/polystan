functions {
  #include polystan.stanfunctions
}
data {
  int<lower=0> N;
}
transformed data {
  real l = -5;
  real u = 5;
  real norm = N * log(u - l);
}
parameters {
  vector<lower=0, upper=1>[N] x;
}
transformed parameters {
  vector[N] theta = flat_prior(x, l, u);
}
model {
  target += std_normal_lpdf(theta);
  target += norm;
}
