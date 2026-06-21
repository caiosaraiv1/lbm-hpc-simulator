#include "lbm_kernels.h"
#include "types.h"

#include <hip/hip_runtime.h>

__constant__ real_t w[Q] = {
      (real_t)(4.0 / 9.0),
      (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0),
      (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0)
};

// Deslocamentos no eixo X para as direções de 0 a 8
/*extern "C" */
__constant__ int cx[Q] = { 0,  1,  0, -1,  0,  1, -1, -1,  1 };
// Deslocamentos no eixo Y para as direções de 0 a 8
__constant__ int cy[Q] = { 0,  0,  1,  0, -1,  1,  1, -1, -1 };

extern "C" __global__ void init_fluid_gpu(
      LatticeSoA in,
      size_t nx, size_t ny
)
{
      size_t x = (blockIdx.x * blockDim.x) + threadIdx.x;
      size_t y = (blockIdx.y * blockDim.y) + threadIdx.y;

      if (x >= nx || y >= ny) return;

      size_t idx = (y * nx) + x;

      in.d_rho[idx] = 1.0;
      in.u_x[idx] = 0.0;
      in.u_y[idx] = 0.0;

      for (int j = 0; j < Q; j++)
      {
            in.df[j][idx] = w[j] * in.d_rho[idx];
      }
}

extern "C" __global__ void collide_stream_gpu(
      LatticeSoA in,
      LatticeSoA out,
      size_t nx, size_t ny,
      real_t omega)
{
      size_t x = (blockIdx.x * blockDim.x) + threadIdx.x;
      size_t y = (blockIdx.y * blockDim.y) + threadIdx.y;

      if (
            x >= nx || y >= ny ||
            x <= 0 || x == nx-1 ||
            y <= 0 || y == ny - 1
      ) return;

      real_t local_f[Q];
      real_t rho = 0.0;
      real_t u_x = 0.0;
      real_t u_y = 0.0;

      for (int j = 0; j < Q; j++)
      {

            size_t x_nbr = x - cx[j]; // Inverte a direção
            size_t y_nbr = y - cy[j];
            size_t idx_nbr = (y_nbr * nx) + x_nbr; // Converte em indice

            local_f[j] = in.df[j][idx_nbr]; // Descubro a densidade total da célula após a chegada dos vizinhos

            rho += local_f[j];

            // Calculo o sentido e a força
            u_x += local_f[j] * cx[j];
            u_y += local_f[j] * cy[j];
      }

      // Calculo da velocidade escalar, momento / massa
      u_x /= rho;
      u_y /= rho;

      size_t idx_curr = (y * nx) + x;

      out.d_rho[idx_curr] = rho;
      out.u_x[idx_curr] = u_x;
      out.u_y[idx_curr] = u_y;

      // Energica cinética
      real_t u_sq = (u_x * u_x) + (u_y * u_y);

      for (int j = 0; j < Q; j++)
      {
            real_t cu = (cx[j] * u_x) + (cy[j] * u_y); // alinhamento da direção j com o fluxo
            // Distribuição de equilíbrio Maxwell-Boltzmann
            // Representa como o fluido estaria se não houvesse perturbações
            real_t f_eq = w[j] * rho * (
                  (real_t)1.0
                  + (real_t)3.0 * cu            // termo linear
                  +  (real_t)4.5 * (cu * cu)    // termo quadrático (restaura pressão)
                  -  (real_t)1.5 * u_sq);       // correção cinética
            out.df[j][idx_curr] =
                  local_f[j]                          // estado atual
                  + omega * (f_eq - local_f[j]);      // empurrão
      }
}

extern "C" __global__ void bounce_back_boundaries_gpu(
      LatticeSoA in,
      LatticeSoA out,
      size_t nx, size_t ny,
      real_t u_lid)
{
      size_t x = (blockIdx.x * blockDim.x) + threadIdx.x;
      size_t y = (blockIdx.y * blockDim.y) + threadIdx.y;

      if (x >= nx || y >= ny) return;

      size_t idx_curr = (y * nx) + x;

      // ----------------------------------------------------------------
      // EIXO X (Paredes Verticais)
      // ----------------------------------------------------------------

      if (x == 1) { // Sou a Parede Esquerda?
            out.df[1][idx_curr] = in.df[3][idx_curr];
            out.df[5][idx_curr] = in.df[7][idx_curr];
            out.df[8][idx_curr] = in.df[6][idx_curr];
      }
      else if (x == nx - 2) { // Sou a Parede Direita?
            out.df[3][idx_curr] = in.df[1][idx_curr];
            out.df[6][idx_curr] = in.df[8][idx_curr];
            out.df[7][idx_curr] = in.df[5][idx_curr];
      }

      // ----------------------------------------------------------------
      // EIXO Y (Paredes Horizontais)
      // ----------------------------------------------------------------

      if (y == 1) { // Sou a Parede Inferior?
            out.df[2][idx_curr] = in.df[4][idx_curr];
            out.df[5][idx_curr] = in.df[7][idx_curr];
            out.df[6][idx_curr] = in.df[8][idx_curr];
      }
      else if (y == ny - 2) { // Sou a Tampa Móvel (Superior)?
            real_t drag = u_lid / (real_t) 6.0;

            out.df[4][idx_curr] = in.df[2][idx_curr];
            out.df[7][idx_curr] = in.df[5][idx_curr] - drag;
            out.df[8][idx_curr] = in.df[6][idx_curr] + drag;
      }
}
