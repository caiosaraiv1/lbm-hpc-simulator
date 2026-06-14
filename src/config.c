#include "config.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ConfigStatus load_config(const char* file_path, SimConfig* config)
{
      FILE *config_file;
      config_file = fopen(file_path, "r");

      if (config_file == NULL)
            return CONFIG_ERROR_FILE_NOT_FOUND;

      char buffer[256];
      char key[32];
      char value[32];

      while (fgets(buffer, sizeof(buffer), config_file) != NULL)
      {
            if (buffer[0] == '#' || buffer[0] == ' ' || buffer[0] == '\n') continue;

            if (sscanf(buffer, "%s = %s", key, value) == 2)
            {
                  if (strcmp(key, "nx") == 0)
                        config->nx = atoi(value);
                  else if (strcmp(key, "ny") == 0)
                        config->ny = atoi(value);
                  else if (strcmp(key, "max_iters") == 0)
                        config->max_iters = atoi(value);
                  else if (strcmp(key, "save_interval") == 0)
                        config->save_interval = atoi(value);
                  else if (strcmp(key, "omega") == 0)
                  {
                        #if PRECISION_SINGLE
                              sscanf(value, "%f", &config->omega);
                        #else
                              sscanf(value, "%lf", &config->omega);
                        #endif
                  }
                  else if(strcmp(key, "u_lid") == 0)
                  {
                         #if PRECISION_SINGLE
                              sscanf(value, "%f", &config->u_lid);
                        #else
                              sscanf(value, "%lf", &config->u_lid);
                        #endif
                  }
                  else
                  {
                        fclose(config_file);
                        return CONFIG_ERROR_INVALID_VALUE;
                  }
            }
      }

      fclose(config_file);

      if    ((config->nx <= 2 || config->ny <= 2) ||
            (config->max_iters < 1 || config->save_interval < 1) ||
            (config->save_interval > config->max_iters) ||
            (config->omega <= 0.0 || config->omega >= 2.0) ||
            (config->u_lid <= 0.0 || config->u_lid > 0.105)
      ) return CONFIG_ERROR_INVALID_VALUE;

      return CONFIG_SUCCESS;
}
