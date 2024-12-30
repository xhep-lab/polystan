#include "polystan/metadata.hpp"

#define XSTR(s) STR(s)
#define STR(s) #s

namespace polystan {
const char* stan_file_name = XSTR(PS_STAN_FILE_NAME);
const char* stan_model_name = XSTR(PS_STAN_MODEL_NAME);
}  // end namespace polystan

#undef XSTR
#undef STR
