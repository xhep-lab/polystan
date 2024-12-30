#ifndef POLYSTAN_VERSION_HPP_
#define POLYSTAN_VERSION_HPP_

#define MAJOR 1
#define MINOR 0
#define PATCH 0

#define XSTR(s) STR(s)
#define STR(s) #s
namespace polystan {
const char version[] = XSTR(MAJOR) "." XSTR(MINOR) "." XSTR(PATCH);
const char stan_file_name[] = XSTR(PS_STAN_FILE_NAME);
const char stan_model_name[] = XSTR(PS_STAN_MODEL_NAME);
const char polychord_version[] = XSTR(PS_POLYCHORD_VERSION);
}  // end namespace polystan
#undef XSTR
#undef STR

#endif  // POLYSTAN_VERSION_HPP_
