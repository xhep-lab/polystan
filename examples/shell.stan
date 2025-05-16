functions {
  #include polystan.stanfunctions
}
data {
  int N;
}
parameters {
  vector<lower=0, upper=1>[N] x;
}
transformed parameters {
  real r2 = norm2(x);
}
model {
  target += normal_lpdf(r2 | 0.25, 0.01);
}
