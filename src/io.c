#include "io.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Converte um float Little-Endian para Big-Endian
static inline float swap_endian_float(float val) {
    // Aponta para o endereço do float, mas lê como um int de 32 bits
    uint32_t bits = *(uint32_t*)&val;

    // Fatiamos os 4 bytes e invertemos as posições com Bitwise Shift e máscaras OR
    bits = ((bits << 24) & 0xFF000000) |
           ((bits <<  8) & 0x00FF0000) |
           ((bits >>  8) & 0x0000FF00) |
           ((bits >> 24) & 0x000000FF);

    // Devolvemos a memória invertida lendo como float novamente
    return *(float*)&bits;
}

void export_vtk(const char* base_dir, LBM_Context *ctx, int step)
{
      char filename[512];

      sprintf(filename, "%s/lbm_step_%05d.vtk", base_dir, step);

      FILE *archive = fopen(filename, "wb");
      if (archive == NULL)
      {
            fprintf(stderr, "[ERROR] Could not open the file: %s\n", filename);
            return;
      }

      size_t nx = ctx->config.nx;
      size_t ny = ctx->config.ny;
      size_t n_points = nx * ny;

      fprintf(archive, "# vtk DataFile Version 3.0\n");
      fprintf(archive, "Lid-Driven Cavity LBM\n");
      fprintf(archive, "BINARY\n");
      fprintf(archive, "DATASET STRUCTURED_POINTS\n");
      fprintf(archive, "DIMENSIONS %zu %zu 1\n", nx, ny);
      fprintf(archive, "ORIGIN 0 0 0\n");
      fprintf(archive, "SPACING 1 1 1\n");
      fprintf(archive, "POINT_DATA %zu\n", n_points);

      real_t *temp = (real_t *)malloc(n_points * sizeof(real_t));
      for (size_t y = 0; y < ny; y++)
      {
            for (size_t x = 0; x < nx; x++)
            {
                  size_t idx = (y * nx) + x;
                  temp[idx] = (real_t)swap_endian_float(ctx->lattice_out->d_rho[idx]);
            }
      }

      fprintf(archive, "SCALARS density float 1\n");
      fprintf(archive, "LOOKUP_TABLE default\n");
      fwrite(temp, sizeof(real_t), n_points, archive);

      free(temp);

      temp = (real_t *)malloc(n_points * 3 * sizeof(real_t));

      for (size_t y = 0; y < ny; y++)
      {
            for (size_t x = 0; x < nx; x++)
            {
                  size_t idx = (y * nx) + x;
                  size_t v_idx = idx * 3;

                  temp[v_idx] = (real_t)swap_endian_float(ctx->lattice_out->u_x[idx]);
                  temp[v_idx + 1] = (real_t)swap_endian_float(ctx->lattice_out->u_y[idx]);
                  temp[v_idx + 2] = (real_t)swap_endian_float(0.0f);
            }
      }

      fprintf(archive, "VECTORS velocity float\n");
      fwrite(temp, sizeof(real_t), n_points * 3, archive);

      free(temp);

      fclose(archive);
}
