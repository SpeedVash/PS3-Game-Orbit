#pragma once
#ifdef ORBIT48_HOST_TEST
#include "../tests/service_host_api_fix48.h"
#else
#include <ppu-lv2.h>
#include <sys/file.h>
#include <sys/event_queue.h>
#include <stdint.h>
#include <stddef.h>

/* Public LV2/Cobra interfaces. No executable patches outside the guarded fan adapter. */
typedef struct Orbit48DeviceInfo {
    char label[32];uint32_t reserved1,reserved2;uint64_t sectors;
    uint32_t sector_size,reserved3;unsigned char reserved4[8];
} __attribute__((packed)) Orbit48DeviceInfo;
static inline int orbit48_cobra(void){uint32_t version=0;lv2syscall2(8,0x7000,(uint64_t)(uintptr_t)&version);return (int)p1==0 && version!=0;}
static inline int orbit48_hen(void){lv2syscall1(8,0x1337);return p1==0x1337;}
static inline uint64_t orbit48_time(void){lv2syscall0(145);return p1;}
static inline void orbit48_sleep(uint64_t us){lv2syscall1(141,us);}
static inline void orbit48_exit_thread(void){lv2syscall1(41,0);}
static inline int orbit48_join(uint64_t id){uint64_t result=0;lv2syscall2(44,id,(uint64_t)(uintptr_t)&result);return (int)p1;}
static inline int orbit48_thread(uint64_t* id,void(*entry)(uint64_t),int priority,uint64_t stack,const char* name){
    const uint64_t* opd=(const uint64_t*)(uintptr_t)entry;
    const uint32_t descriptor[2]={(uint32_t)opd[0],(uint32_t)opd[1]};
    lv2syscall8(52,(uint64_t)(uintptr_t)id,(uint64_t)(uintptr_t)descriptor,0,0,
        (uint64_t)priority,stack,1,(uint64_t)(uintptr_t)name);return (int)p1;
}
static inline int orbit48_storage_info(uint64_t id,Orbit48DeviceInfo* info){
    lv2syscall2(609,id,(uint64_t)(uintptr_t)info);return (int)p1;
}
static inline int orbit48_storage_open(uint64_t id,uint32_t* handle){
    lv2syscall4(600,id,0,(uint64_t)(uintptr_t)handle,0);return (int)p1;
}
static inline int orbit48_storage_close(uint32_t handle){lv2syscall1(601,handle);return (int)p1;}
static inline int orbit48_storage_read(uint32_t handle,uint64_t lba,uint32_t count,void* out){
    uint32_t read=0;lv2syscall7(602,handle,0,lba,count,(uint64_t)(uintptr_t)out,
        (uint64_t)(uintptr_t)&read,0);return (int)p1==0 && read==count;
}
static inline int orbit48_disc_types(uint32_t* real,uint32_t* effective,uint32_t* fake){
    lv2syscall4(8,0x7020,(uint64_t)(uintptr_t)real,(uint64_t)(uintptr_t)effective,
        (uint64_t)(uintptr_t)fake);return (int)p1;
}
static inline int orbit48_storage_event(uint64_t event,uint64_t param){
    lv2syscall4(8,0x7022,event,param,0x101000000000006ull);return (int)p1;
}
static inline int orbit48_eject(void){
    const int a=orbit48_storage_event(4,0);const int b=orbit48_storage_event(8,0);return a?a:b;
}
static inline int orbit48_insert(uint32_t real){
    const int a=orbit48_storage_event(7,0);const int b=orbit48_storage_event(3,(uint64_t)real<<32);return a?a:b;
}
static inline int orbit48_unmount(void){lv2syscall1(8,0x702c);return (int)p1;}
static inline int orbit48_iso(char** files,uint32_t count){
    lv2syscall3(8,0x7024,count,(uint64_t)(uintptr_t)files);return (int)p1;
}
static inline int orbit48_map(const char* old_path,const char* new_path){
    const char* old[1]={old_path};const char* target[1]={new_path};
    lv2syscall4(8,0x7964,(uint64_t)(uintptr_t)old,(uint64_t)(uintptr_t)target,1);return (int)p1;
}
static inline int orbit48_proxy(uint32_t port,uint32_t queue,uint64_t bytes){
    lv2syscall8(8,0x6808,port,queue,1,bytes,256u*1024u,0,0);return (int)p1;
}
static inline uint64_t orbit48_peek(uint64_t address){lv2syscall1(6,address);return p1;}
static inline void orbit48_poke(uint64_t address,uint64_t value){lv2syscall2(7,address,value);}
static inline int orbit48_temperature(unsigned chip,unsigned* out){
    uint32_t value=0;lv2syscall2(383,chip,(uint64_t)(uintptr_t)&value);
    if((int)p1)return 0;
    *out=value>>24;return *out>=5 && *out<=120;
}
static inline int orbit48_get_plugin(unsigned slot,char* name,char* path){
    lv2syscall5(8,0x7777,0x0047,slot,(uint64_t)(uintptr_t)name,(uint64_t)(uintptr_t)path);return (int)p1;
}
static inline uint32_t orbit48_firmware(void){lv2syscall2(8,0x7777,0x0014);return (uint32_t)p1;}
static inline int orbit48_console_type(void){uint64_t type=0;lv2syscall1(985,(uint64_t)(uintptr_t)&type);return (int)p1==0?(int)type:0;}
static inline uint32_t orbit48_pid(void){lv2syscall0(1);return (uint32_t)p1;}
static inline int orbit48_processes(uint32_t* ids){lv2syscall3(8,0x7777,0x0021,(uint64_t)(uintptr_t)ids);return (int)p1;}
static inline int orbit48_process_name(uint32_t pid,char* name){
    lv2syscall4(8,0x7777,0x0022,pid,(uint64_t)(uintptr_t)name);return (int)p1;
}
#endif
