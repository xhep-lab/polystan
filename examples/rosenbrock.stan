functions {
  #include polystan.stanfunctions
  
  real det(int n, real b) {
    return abs(-2 * b * recur(n - 1, b) - 16 * b * b * recur(n - 2, b));
  }
  
  real recur(int n, real b) {
    if (n <= 0) {
      return 0;
    }
    
    if (n == 1) {
      return 1;
    }
    
    return (-2 - 10 * b) * recur(n - 1, b) - 16 * b * b * recur(n - 2, b);
  }
}
data {
  int<lower=0> N;
  real a;
  real b;
}
transformed data {
  real normalisation = -0.5 * log(pow(pi(), N) / det(N, b));
}
parameters {
  vector<lower=0, upper=1>[N] u;
}
transformed parameters {
  vector[N] x = flat_prior(u, -5, 5);
}
model {
  target += normalisation;
  for (i in 1 : N - 1) {
    target += -b * pow(x[i + 1] - pow(x[i], 2), 2) - pow(a - x[i], 2);
  }
}
