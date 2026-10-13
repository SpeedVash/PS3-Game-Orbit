#pragma once
#include <stdint.h>
#include <stddef.h>

/* Own resident service. Wire values are big endian; no native pointers on RPC. */
#define ORBIT48_PROTOCOL_VERSION 0x0106u
#define ORBIT48_MAX_PAYLOAD (128u * 1024u)
#define ORBIT48_MAX_EXTENTS 4096u
#define ORBIT48_MAX_PARTS 64u
#define ORBIT48_PATH_BYTES 1024u
#ifndef ORBIT48_RUNTIME_DIR
#define ORBIT48_RUNTIME_DIR "/dev_hdd0/game/PGORBT301/USRDIR"
#endif
#define ORBIT48_PLUGIN_PATH ORBIT48_RUNTIME_DIR "/orbit_service.sprx"
#define ORBIT48_BLANK_PATH ORBIT48_RUNTIME_DIR "/orbit_folder.iso"
#define ORBIT48_EMPTY_PATH ORBIT48_RUNTIME_DIR "/empty"
#define ORBIT48_SETTINGS_PATH ORBIT48_RUNTIME_DIR "/service.cfg"
#define ORBIT48_STATUS_PATH ORBIT48_RUNTIME_DIR "/service.status"
#define ORBIT48_CONTROL_REQUEST ORBIT48_RUNTIME_DIR "/control.req"
#define ORBIT48_CONTROL_REPLY ORBIT48_RUNTIME_DIR "/control.reply"
#define ORBIT48_MOUNT_REQUEST ORBIT48_RUNTIME_DIR "/mount.req"
#define ORBIT48_MOUNT_REPLY ORBIT48_RUNTIME_DIR "/mount.reply"
#define ORBIT48_CANCEL_PATH ORBIT48_RUNTIME_DIR "/mount.cancel"
#define ORBIT48_HEADER_BYTES 24u
enum Orbit48Error { ORBIT48_OK=0, ORBIT48_BAD_REQUEST=0x1601,
    ORBIT48_NO_COBRA=0x1602, ORBIT48_BAD_PATH=0x1603, ORBIT48_BAD_IMAGE=0x1604,
    ORBIT48_USB_CHANGED=0x1605, ORBIT48_BUSY=0x1606, ORBIT48_CANCELLED=0x1607,
    ORBIT48_FAN_CONFLICT=0x1608, ORBIT48_FAN_UNAVAILABLE=0x1609 };

enum Orbit48Command { ORBIT48_STATUS=1, ORBIT48_CONFIGURE=2,
    ORBIT48_MOUNT_NATIVE=3, ORBIT48_MOUNT_NTFS=4, ORBIT48_RESTORE_SYSCON=5 };
enum Orbit48FanMode { ORBIT48_FAN_SYSCON=0, ORBIT48_FAN_AUTO=1, ORBIT48_FAN_MANUAL=2 };
enum Orbit48FanState { ORBIT48_FAN_READY=0, ORBIT48_FAN_EXTERNAL=1,
    ORBIT48_FAN_UNSUPPORTED=2, ORBIT48_FAN_SENSOR_FAILURE=3,
    ORBIT48_FAN_STARTING_GAME=4, ORBIT48_FAN_EMERGENCY=5 };
typedef struct Orbit48FanConfig {
    uint32_t mode, target_c, minimum_percent, maximum_percent, manual_percent;
} Orbit48FanConfig;
typedef struct Orbit48FanStatus {
    Orbit48FanConfig config;
    uint32_t state, cpu_c, rsx_c, percent, native_error;
} Orbit48FanStatus;
typedef struct Orbit48Extent {
    uint64_t iso_offset, device_lba, sector_count;
} Orbit48Extent;
typedef struct Orbit48RawMap {
    uint64_t device_id, partition_lba, volume_serial, device_sectors, iso_size;
    uint32_t sector_size, count;
    Orbit48Extent extents[ORBIT48_MAX_EXTENTS];
} Orbit48RawMap;

static inline uint32_t orbit48_be32(const unsigned char* p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static inline uint64_t orbit48_be64(const unsigned char* p) {
    return ((uint64_t)orbit48_be32(p)<<32)|orbit48_be32(p+4);
}
static inline void orbit48_put32(unsigned char* p,uint32_t v) {
    p[0]=(unsigned char)(v>>24);p[1]=(unsigned char)(v>>16);
    p[2]=(unsigned char)(v>>8);p[3]=(unsigned char)v;
}
static inline void orbit48_put64(unsigned char* p,uint64_t v) {
    orbit48_put32(p,(uint32_t)(v>>32));orbit48_put32(p+4,(uint32_t)v);
}
static inline int orbit48_valid_fan_config(const Orbit48FanConfig* c) {
    return c && c->mode<=ORBIT48_FAN_MANUAL && c->target_c>=60 && c->target_c<=75 &&
        c->minimum_percent>=35 && c->minimum_percent<=70 &&
        c->maximum_percent>=c->minimum_percent && c->maximum_percent<=100 &&
        c->manual_percent>=35 && c->manual_percent<=100;
}
