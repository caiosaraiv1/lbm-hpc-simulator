#include "config.h"
#include "types.h"
#include "memory.h"
#include "lbm_kernels.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
      if (argc < 2)
      {
            fprintf(stderr, "[ERROR] Correct use: %s <path.txt>\n", argv[0]);
            return 1;
      }

      SimConfig config = {0};
      ConfigStatus status = load_config(argv[1], &config);

      switch (status)
      {
            case CONFIG_SUCCESS:
                  printf("=========================================\n");
                  printf("     CONFIGURATION LOADED SUCCESSFULLY   \n");
                  printf("=========================================\n");
                  printf("Grid Size         : %d x %d\n", config.nx, config.ny);
                  printf("Maximum Iterations: %d\n", config.max_iters);
                  printf("Output Interval   : %d\n", config.save_interval);
                  printf("Omega             : %f\n", config.omega);
                  printf("Lid Velocity      : %f\n", config.u_lid);
                  printf("=========================================\n");
                  break;
            case CONFIG_ERROR_FILE_NOT_FOUND:
                  fprintf(stderr, "[ERROR] Configuration file not found: '%s'\n", argv[1]);
                  return 1;
            case CONFIG_ERROR_INVALID_VALUE:
                  fprintf(stderr, "[ERROR] Invalid physical or computational parameters in: '%s'\n", argv[1]);
                  fprintf(stderr, "Check the restrictions (nx/ny > 2, 0 < omega < 2, u_lid <= 0.10)\n");
                  return 1;
            default:
                  fprintf(stderr,"[CRITICAL ERROR] Unknown status code detected.\n");
                  return 1;
      }

      LatticeSoA mesh_a;
      LatticeSoA mesh_b;

      MemoryStatus status_a = allocate_host_lattice(&mesh_a, config.nx, config.ny);
      if (status_a != MEM_SUCCESS) return 1;

      MemoryStatus status_b = allocate_host_lattice(&mesh_b, config.nx, config.ny);
      if (status_b != MEM_SUCCESS)
      {
            free_host_lattice(&mesh_a);
            return 1;
      }

      init_fluid(&mesh_a, config.nx, config.ny);
      LBM_Context context;
      context.config = config;
      context.lattice_in = &mesh_a;
      context.lattice_out = &mesh_b;
      context.memory_size = (config.nx * config.ny) * sizeof(real_t) * 12;

      for (int i = 0; i < config.max_iters; i++)
      {
            collide_stream_cpu(&context);
            bounce_back_boundaries(&context);

            if (i % config.save_interval == 0)
            {
                  printf("I = %d\n", i);
                  // copy_device_to_host();
            }

            LatticeSoA *temp;
            temp = context.lattice_in;
            context.lattice_in = context.lattice_out;
            context.lattice_out = temp;
      }
      free_host_lattice(&mesh_a);
      free_host_lattice(&mesh_b);

      return 0;
}
