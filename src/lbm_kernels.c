#include "lbm_kernels.h"
#include "types.h"

void init_fluid(LatticeSoA *host, int nx, int ny)
{
      const real_t w[Q] = {
            (real_t)(4.0 / 9.0),
            (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0),
            (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0)
      };

      size_t total_cells = (size_t) nx * ny;
      for (size_t i = 0; i < total_cells; i++)
      {
            host->d_rho[i] = 1.0;
            host->u_x[i] = 0.0;
            host->u_y[i] = 0.0;

            for (int j = 0; j < Q; j++)
            {
                  host->df[j][i] = w[j] * host->d_rho[i];
            }
      }
}
