functions {
  #include polystan.stanfunctions
}
data {
  real<lower=0> r_e;
  real<lower=0> r_l;
  int<lower=1> T;
  array[T] int<lower=0> D;
}
parameters {
  vector<lower=0, upper=1>[3] x;
}
transformed parameters {
  real e = exponential_prior(x[1], r_e);
  real l = exponential_prior(x[2], r_l);
}
model {
  int s = flat_prior(x[3], T);
  vector[T] rate = rep_vector(l, T);
  rate[1 : s] = rep_vector(e, s);
  D ~ poisson(rate);
}
generated quantities {
  int save_s = flat_prior(x[3], T);
}
