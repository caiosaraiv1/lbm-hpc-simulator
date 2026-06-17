#ifndef LBM_KERNELS_H
#define LBM_KERNELS_H

#include "types.h"

void init_fluid(LatticeSoA *host, int nx, int ny);
void collide_stream_cpu(LBM_Context *ctx);
void bounce_back_boundaries(LBM_Context *ctx);
void compute_macroscopic_cpu(LBM_Context *ctx);

#endif
