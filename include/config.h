#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum
{
      CONFIG_SUCCESS,
      CONFIG_ERROR_FILE_NOT_FOUND,
      CONFIG_ERROR_INVALID_VALUE
} ConfigStatus;

ConfigStatus load_config(const char* file_path, SimConfig* config);

#ifdef __cplusplus
}
#endif

#endif
