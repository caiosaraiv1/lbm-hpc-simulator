#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

typedef enum
{
      CONFIG_SUCCESS,
      CONFIG_ERROR_FILE_NOT_FOUND,
      CONFIG_ERROR_INVALID_VALUE
} ConfigStatus;

ConfigStatus load_config(const char* file_path, SimConfig* config);

#endif
