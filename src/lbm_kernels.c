#include "lbm_kernels.h"
#include "types.h"

static const real_t w[Q] = {
      (real_t)(4.0 / 9.0),
      (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0), (real_t)(1.0 / 9.0),
      (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0), (real_t)(1.0 / 36.0)
};

void init_fluid(LatticeSoA *host, int nx, int ny)
{
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

void collide_stream_cpu(LBM_Context *ctx)
{
      // Deslocamentos no eixo X para as direções de 0 a 8
      const int cx[Q] = { 0,  1,  0, -1,  0,  1, -1, -1,  1 };

      // Deslocamentos no eixo Y para as direções de 0 a 8
      const int cy[Q] = { 0,  0,  1,  0, -1,  1,  1, -1, -1 };

      size_t nx = ctx->config.nx;
      size_t ny = ctx->config.ny;
      real_t omega = ctx->config.omega;

      LatticeSoA *in = ctx->lattice_in;
      LatticeSoA *out = ctx->lattice_out;

      // Vou até n_ - 1 para ignorar as paredes e focar aonde o fluído pode pode se chocar
      for (size_t y = 1; y < ny - 1; y++)
      {
            for (size_t x = 1; x < nx - 1; x++)
            {
                  real_t local_f[Q];
                  real_t rho = 0.0;
                  real_t u_x = 0.0;
                  real_t u_y = 0.0;

                  for (int j = 0; j < Q; j++)
                  {

                        size_t x_nbr = x - cx[j]; // Inverte a direção
                        size_t y_nbr = y - cy[j];
                        size_t idx_nbr = (y_nbr * nx) + x_nbr; // Converte em indice

                        local_f[j] = in->df[j][idx_nbr]; // Descubro a densidade total da célula após a chegada dos vizinhos

                        rho += local_f[j];

                        // Calculo o sentido e a força
                        u_x += local_f[j] * cx[j];
                        u_y += local_f[j] * cy[j];
                  }

                  // Calculo da velocidade escalar, momento / massa
                  u_x /= rho;
                  u_y /= rho;

                  size_t idx_curr = (y * nx) + x;

                  out->d_rho[idx_curr] = rho;
                  out->u_x[idx_curr] = u_x;
                  out->u_y[idx_curr] = u_y;

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
                        out->df[j][idx_curr] =
                              local_f[j]                          // estado atual
                              + omega * (f_eq - local_f[j]);      // empurrão
                  }
            }
      }
}

void bounce_back_boundaries(LBM_Context *ctx)
{
      /*
      Direita: 1 (Leste), 5 (Nordeste), 8 (Sudeste)

      Esquerda: 3 (Oeste), 6 (Noroeste), 7 (Sudoeste)

      Cima: 2 (Norte), 5 (Nordeste), 6 (Noroeste)

      Baixo: 4 (Sul), 7 (Sudoeste), 8 (Sudeste)
      */

      size_t nx = ctx->config.nx;
      size_t ny = ctx->config.ny;

      LatticeSoA *in = ctx->lattice_in;
      LatticeSoA *out = ctx->lattice_out;

      // Parede Esquerda
      for (size_t y = 1; y < ny - 1; y++)
      {
            size_t idx_curr = (y * nx) + 1;
            out->df[1][idx_curr] = in->df[3][idx_curr];
            out->df[5][idx_curr] = in->df[7][idx_curr];
            out->df[8][idx_curr] = in->df[6][idx_curr];
      }

      // Parede Direita
      for (size_t y = 1; y < ny - 1; y++)
      {
            size_t x = nx - 2;
            size_t idx_curr = (y * nx) + x;
            out->df[3][idx_curr] = in->df[1][idx_curr];
            out->df[6][idx_curr] = in->df[8][idx_curr];
            out->df[7][idx_curr] = in->df[5][idx_curr];
      }

      // Parede Inferior
      for (size_t x = 1; x < nx - 1; x++)
      {
            size_t idx_curr = (1 * nx) + x;
            out->df[2][idx_curr] = in->df[4][idx_curr];
            out->df[5][idx_curr] = in->df[7][idx_curr];
            out->df[6][idx_curr] = in->df[8][idx_curr];
      }

      // Tampa Móvel (Parede Superior)
      real_t u_lid = ctx->config.u_lid;
      real_t drag = u_lid / (real_t) 6.0;

      for (size_t x = 1; x < nx - 1; x++)
      {
            size_t y = ny - 2;
            size_t idx_curr = (y * nx) + x;
            out->df[4][idx_curr] = in->df[2][idx_curr];
            out->df[7][idx_curr] = in->df[5][idx_curr] - drag;
            out->df[8][idx_curr] = in->df[6][idx_curr] + drag;
      }
}

