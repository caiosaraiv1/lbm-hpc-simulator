#include "config.h"
#include "types.h"
#include "memory.h"
#include "lbm_kernels.h"
#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <hip/hip_runtime.h>
#include <direct.h>

int setup_lattices(const SimConfig *config, LatticeSoA *mesh_host, LatticeSoA *mesh_a, LatticeSoA *mesh_b)
{
      MemoryStatus status;

      status = allocate_host_lattice(mesh_host, config->nx, config->ny);
      if (status != MEM_SUCCESS)
            goto err_host;

      status = allocate_device_lattice(mesh_a, config->nx, config->ny);
      if (status != MEM_SUCCESS)
            goto err_a;

      status = allocate_device_lattice(mesh_b, config->nx, config->ny);
      if (status != MEM_SUCCESS)
            goto err_b;

      return 0;

err_b:
      free_device_lattice(mesh_a);
err_a:
      free_host_lattice(mesh_host);
err_host:
      return 1;
}

int main(int argc, char* argv[])
{
      if (argc < 2)
      {
            fprintf(stderr, "[ERROR] Correct use: %s <path.txt>\n", argv[0]);
            return 1;
      }

      SimConfig config = {};
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

      LatticeSoA mesh_host;   // RAM para VTK
      LatticeSoA mesh_a;      // VRAM
      LatticeSoA mesh_b;      // VRAM

      int mem_status = setup_lattices(&config, &mesh_host, &mesh_a, &mesh_b);
      if (mem_status != 0)
      {
            fprintf(stderr,"[ERROR] Problem with Lattice Setup.\n");
            return 1;
      }

      char output_dir[256];
      sprintf(output_dir, "data/output/sim_%.0f_%.0f_%d",
            config.omega * 10,
            config.u_lid * 1000,
            config.max_iters);
      _mkdir(output_dir);

      LBM_Context context;
      context.config = config;
      context.lattice_in = &mesh_a;
      context.lattice_out = &mesh_b;
      context.memory_size = (config.nx * config.ny) * sizeof(real_t) * 12;

      dim3 threads(16, 16);
      dim3 blocks((config.nx + 15) / 16, (config.ny + 15) / 16);

      init_fluid_gpu<<<blocks, threads>>>(*context.lattice_in, config.nx, config.ny);
      init_fluid_gpu<<<blocks, threads>>>(*context.lattice_out, config.nx, config.ny);

      for (int i = 0; i < config.max_iters; i++)
      {
            collide_stream_gpu<<<blocks, threads>>>(*context.lattice_in, *context.lattice_out, config.nx, config.ny, config.omega);
            bounce_back_boundaries_gpu<<<blocks, threads>>>(*context.lattice_in, *context.lattice_out, config.nx, config.ny, config.u_lid);

            if (i % config.save_interval == 0)
            {
                  hipDeviceSynchronize();
                  copy_device_to_host(&mesh_host, context.lattice_out, config.nx, config.ny);

                  LBM_Context host_ctx = context;
                  host_ctx.lattice_out = &mesh_host;

                  export_vtk(output_dir, &host_ctx, i);

                  double progress = (double)i / config.max_iters * 100.0;
                  printf("\r[PROGRESS] [");

                  int bar_width = 20;
                  int pos = (progress / 100.0) * bar_width;
                  for (int j = 0; j < bar_width; ++j)
                  {
                        if (j < pos) printf("=");
                        else if (j == pos) printf(">");
                        else printf(" ");
                  }
                  printf("] %.1f%% (%d/%d)", progress, i, config.max_iters);
                  fflush(stdout);
            }

            LatticeSoA *temp;
            temp = context.lattice_in;
            context.lattice_in = context.lattice_out;
            context.lattice_out = temp;
      }

      printf("\r[PROGRESS] [====================] 100.0%% (%d/%d)\n",
      config.max_iters, config.max_iters);
      fflush(stdout);

      printf("[SUCCESS] Simulation completed successfully!\n");

      free_host_lattice(&mesh_host);
      free_device_lattice(&mesh_a);
      free_device_lattice(&mesh_b);

      return 0;
}
