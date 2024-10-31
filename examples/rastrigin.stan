functions {
  #include polystan.stanfunctions
}
data {
  int<lower=0> N;
}
transformed data {
  real norm = log(4991.21750);
}
parameters {
  vector<lower=0, upper=1>[N] x;
}
transformed parameters {
  vector[N] theta = flat_prior(x, -5.12, 5.12);
}
model {
  target += -sum(norm + square(theta) - 10. * cos(2. * pi() * theta));
}
