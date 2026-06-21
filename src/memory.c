#include "types.h"
#include "memory.h"
#include <stdlib.h>
#include <hip/hip_runtime_api.h>

#ifdef _WIN32
#include <malloc.h>
#endif

MemoryStatus allocate_host_lattice(LatticeSoA *lattice, int nx, int ny)
{
      if (lattice == NULL || nx <= 0 || ny <= 0)
            return MEM_ERROR_INVALID_ARGUMENTS;

      size_t total_bytes = (nx * ny) * sizeof(real_t);
      int error = 0;

      for (int i = 0; i < Q; i++)
      {
            lattice->df[i] = (real_t*)_aligned_malloc(total_bytes, 64);
            if (lattice->df[i] == NULL) error = 1;
      }

      lattice->d_rho = (real_t*)_aligned_malloc(total_bytes, 64);
      if (lattice->d_rho == NULL) error = 1;

      lattice->u_x = (real_t*)_aligned_malloc(total_bytes, 64);
      if (lattice->u_x == NULL) error = 1;

      lattice->u_y = (real_t*)_aligned_malloc(total_bytes, 64);
      if (lattice->u_y == NULL) error = 1;

      if (error != 0)
      {
            free_host_lattice(lattice);
            return MEM_ERROR_HOST_ALLOCATION;
      }

      return MEM_SUCCESS;
}

void free_host_lattice(LatticeSoA *lattice)
{
      if (lattice == NULL) return;

      for (int i = 0; i < Q; i++)
      {
            _aligned_free(lattice->df[i]);
            lattice->df[i] = NULL;
      }

      _aligned_free(lattice->d_rho);
      _aligned_free(lattice->u_x);
      _aligned_free(lattice->u_y);

      lattice->d_rho = NULL;
      lattice->u_x = NULL;
      lattice->u_y = NULL;
}

MemoryStatus allocate_device_lattice(LatticeSoA *lattice, int nx, int ny)
{
      if (lattice == NULL || nx <= 0 || ny <= 0)
            return MEM_ERROR_INVALID_ARGUMENTS;

      size_t total_bytes = (nx * ny) * sizeof(real_t);
      int error = 0;

      for (int i = 0; i < Q; i++)
            error = error || (hipMalloc((void**)&lattice->df[i], total_bytes) != hipSuccess);

      error = error || (hipMalloc((void**)&lattice->d_rho, total_bytes) != hipSuccess);
      error = error || (hipMalloc((void**)&lattice->u_x, total_bytes) != hipSuccess);
      error = error || (hipMalloc((void**)&lattice->u_y, total_bytes) != hipSuccess);

      if (error != 0)
      {
            free_device_lattice(lattice);
            return MEM_ERROR_DEVICE_ALLOCATION;
      }

      return MEM_SUCCESS;
}

void free_device_lattice(LatticeSoA *lattice)
{
      if (lattice == NULL) return;

      for (int i = 0; i < Q; i++)
      {
            hipFree(lattice->df[i]);
            lattice->df[i] = NULL;
      }

      hipFree(lattice->d_rho);
      hipFree(lattice->u_x);
      hipFree(lattice->u_y);

      lattice->d_rho = NULL;
      lattice->u_x = NULL;
      lattice->u_y = NULL;
}

void copy_device_to_host(LatticeSoA *host, LatticeSoA *device, int nx, int ny)
{
      size_t total_bytes = (nx * ny) * sizeof(real_t);

      for (int i = 0; i < Q; i++)
      {
            hipMemcpy(host->df[i], device->df[i], total_bytes, hipMemcpyDeviceToHost);
      }
      hipMemcpy(host->d_rho, device->d_rho, total_bytes, hipMemcpyDeviceToHost);
      hipMemcpy(host->u_x, device->u_x, total_bytes, hipMemcpyDeviceToHost);
      hipMemcpy(host->u_y, device->u_y, total_bytes, hipMemcpyDeviceToHost);
}
