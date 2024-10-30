functions {
  #include polystan.stanfunctions
}
transformed data {
  real norm = -log(0.4071069421432255);
}
parameters {
  vector<lower=0, upper=1>[2] x;
}
transformed parameters {
  vector[2] theta = flat_prior(x, -5, 5);
}
model {
    target += norm - square(square(theta[1]) + theta[2] - 11) - square(theta[1] + square(theta[2]) - 7); 
}
