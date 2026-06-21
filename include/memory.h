#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum
{
      MEM_SUCCESS,
      MEM_ERROR_HOST_ALLOCATION,
      MEM_ERROR_DEVICE_ALLOCATION,
      MEM_ERROR_INVALID_ARGUMENTS
} MemoryStatus;

MemoryStatus allocate_host_lattice(LatticeSoA *lattice, int nx, int ny);
void free_host_lattice(LatticeSoA *lattice);
MemoryStatus allocate_device_lattice(LatticeSoA *lattice, int nx, int ny);
void free_device_lattice(LatticeSoA *lattice);
void copy_device_to_host(LatticeSoA *host, LatticeSoA *device, int nx, int ny);

#ifdef __cplusplus
}
#endif

#endif
