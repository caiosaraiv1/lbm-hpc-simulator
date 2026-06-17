#include "io.h"
#include "types.h"
#include <stdio.h>

void export_vtk(LBM_Context *ctx, int step)
{
      char str_name[256];

      sprintf(str_name, "data/output/lbm_step_%05d.vtk", step);

      FILE *archive = fopen(str_name, "w");
      if (archive == NULL)
      {
            fprintf(stderr, "[ERROR] Could not open the file: %s\n", str_name);
            return;
      }

      size_t nx = ctx->config.nx;
      size_t ny = ctx->config.ny;

      fprintf(archive, "# vtk DataFile Version 3.0\n");
      fprintf(archive, "Lid-Driven Cavity LBM\n");
      fprintf(archive, "ASCII\n");
      fprintf(archive, "DATASET STRUCTURED_POINTS\n");
      fprintf(archive, "DIMENSIONS %zu %zu 1\n", nx, ny);
      fprintf(archive, "ORIGIN 0 0 0\n");
      fprintf(archive, "SPACING 1 1 1\n");
      fprintf(archive, "POINT_DATA %zu\n", nx * ny);

      fprintf(archive, "SCALARS density float 1\n");
      fprintf(archive, "LOOKUP_TABLE default\n");

      for (size_t y = 0; y < ny; y++)
      {
            for (size_t x = 0; x < nx; x++)
            {
                  size_t idx = (y * nx) + x;
                  fprintf(archive, "%f\n", ctx->lattice_out->d_rho[idx]);
            }
      }

      fprintf(archive, "VECTORS velocity float\n");
      for (size_t y = 0; y < ny; y++)
      {
            for (size_t x = 0; x < nx; x++)
            {
                  size_t idx = (y * nx) + x;
                  fprintf(archive, "%f %f 0.0\n", ctx->lattice_out->u_x[idx], ctx->lattice_out->u_y[idx]);
            }
      }

      fclose(archive);
}
