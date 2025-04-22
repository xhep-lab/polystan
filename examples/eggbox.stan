functions {
  #include polystan.stanfunctions
}
data {
  int<lower=0> N;
}
parameters {
  vector<lower=0, upper=1>[N] x;
}
transformed parameters {
  vector[N] theta = flat_prior(x, 0, 31.4159);
}
model {
  target += -pow(2 + prod(cos(0.5 * theta)), 5);
}
