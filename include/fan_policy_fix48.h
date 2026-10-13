#pragma once
#include "orbit_service_protocol_fix48.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct Orbit48FanPolicy {
    Orbit48FanConfig config;
    uint32_t percent, failures, cool_ticks, state;
} Orbit48FanPolicy;
void orbit48_fan_init(Orbit48FanPolicy* p);
int orbit48_fan_configure(Orbit48FanPolicy* p,const Orbit48FanConfig* c);
/* Returns -1 for SYSCON, or the requested percentage. Manual also has thermal override. */
int orbit48_fan_tick(Orbit48FanPolicy* p,int cpu,int rsx,int readings_ok,int external,int cooling_pause);
#ifdef __cplusplus
}
#endif
