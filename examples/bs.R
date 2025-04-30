# run bridgesampling with Stan MCMC chains on Stan models

# install.packages("bridgesampling")
# install.packages("rstan")
# install.packages("jsonlite")

library("bridgesampling")
library("rstan")
library("jsonlite")

headers <- list.files("../stanfunctions", "*.stanfunctions")
file.copy(headers, ".")
set.seed(1)
rstan_options(auto_write = TRUE)

args <- commandArgs(TRUE)
iter <- 100000
warmup <- 50000
chains <- 4

stanfit <- stan(file = paste(args[1], ".stan", sep = ""),
                data = fromJSON(paste(args[1], ".data.json", sep = "")),
                iter = iter,
                warmup = warmup,
                chains = chains,
                seed = 1)
                
sampler_params <- get_sampler_params(stanfit, inc_warmup = TRUE)
mcmc_neval <- sum(do.call(rbind, sampler_params)[,'n_leapfrog__'])

result <- bridge_sampler(stanfit)
bs_neval <- length(result$q11) + length(result$q21)
neval <- mcmc_neval + bs_neval

print(neval)
print(result$logml)
print(sqrt(error_measures(result)$re2))
