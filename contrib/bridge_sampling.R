# run bridge sampling with Stan MCMC chains on Stan models

dir.create(Sys.getenv("R_LIBS_USER"), recursive = TRUE)
.libPaths(Sys.getenv("R_LIBS_USER"))

if (!require("pacman")) install.packages("pacman")
pacman::p_load("bridgesampling", "rstan", "jsonlite")


bridge_sampling = function(target, iter, warmup, chains, seed, headers) {
  set.seed(seed)
  rstan_options(auto_write = TRUE)
  wd <- getwd()
  setwd(headers)

  stanfit <- stan(file = paste(target, ".stan", sep = ""),
                  data = fromJSON(paste(target, ".data.json", sep = "")),
                  iter = iter,
                  warmup = warmup,
                  chains = chains,
                  seed = seed)
        
  setwd(wd)
        
  sampler_params <- get_sampler_params(stanfit, inc_warmup = TRUE)
  mcmc_neval <- sum(do.call(rbind, sampler_params)[,'n_leapfrog__'])

  result <- bridge_sampler(stanfit)
  bs_neval <- length(result$q11) + length(result$q21)
  neval <- mcmc_neval + bs_neval
  logz <- result$logml
  err_logz <- sqrt(error_measures(result)$re2)
  return(c(logz, err_logz, neval))
}
