#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void export_vtk(const char* base_dir, LBM_Context *ctx, int step);

#ifdef __cplusplus
}
#endif

#endif
