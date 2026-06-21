#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define Q 9

#include <stddef.h>

#define PRECISION_SINGLE 1

#if PRECISION_SINGLE
      typedef float real_t;
#else
      typedef double real_t;
#endif

typedef struct
{
      int nx;
      int ny;
      int max_iters;
      int save_interval;
      real_t omega;
      real_t u_lid;
} SimConfig;

typedef struct
{
      real_t *df[Q];
      real_t *d_rho;
      real_t *u_x;
      real_t *u_y;
} LatticeSoA;

typedef struct
{
      SimConfig config;
      LatticeSoA *lattice_in;
      LatticeSoA *lattice_out;
      size_t memory_size;
} LBM_Context;

#ifdef __cplusplus
}
#endif

#endif
