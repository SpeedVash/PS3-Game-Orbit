#pragma once
#include "orbit_service_protocol_fix48.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef int (*Orbit48SectorRead)(void* context,uint64_t lba,uint32_t count,void* out);
int orbit48_validate_raw_map(const Orbit48RawMap* map);
int orbit48_raw_read(const Orbit48RawMap* map,Orbit48SectorRead read,void* context,
    uint64_t offset,void* out,uint32_t size,void* scratch,uint32_t scratch_size);
#ifdef __cplusplus
}
#endif
