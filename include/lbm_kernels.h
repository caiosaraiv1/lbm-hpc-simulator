#ifndef LBM_KERNELS_H
#define LBM_KERNELS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

__global__ void init_fluid_gpu(LatticeSoA in, size_t nx, size_t ny);
__global__ void collide_stream_gpu(LatticeSoA in, LatticeSoA out, size_t nx, size_t ny,real_t omega);
__global__ void bounce_back_boundaries_gpu(LatticeSoA in, LatticeSoA out, size_t nx, size_t ny, real_t u_lid);

#ifdef __cplusplus
}
#endif

#endif

