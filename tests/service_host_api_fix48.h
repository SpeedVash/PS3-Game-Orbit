#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef struct {int64_t size;uint32_t mode;} sysFSStat;
typedef struct {uint64_t source,data_1,data_2,data_3;} sys_event_t;
typedef struct {uint32_t protocol,type;char name[8];} sys_event_queue_attr_t;
typedef struct {char label[32];uint32_t reserved1,reserved2;uint64_t sectors;uint32_t sector_size,reserved3;unsigned char reserved4[8];} Orbit48DeviceInfo;
#define SYS_EVENT_QUEUE_FORCE_DESTROY 1
#define SYS_EVENT_QUEUE_FIFO 1
#define SYS_EVENT_QUEUE_PPU 1
#define SYS_EVENT_PORT_LOCAL 1
#define SYS_EVENT_PORT_NO_NAME 0
#define SYS_EVENT_QUEUE_KEY_LOCAL 0
int sysLv2FsStat(const char*,sysFSStat*);
int sysLv2FsOpen(const char*,int,int*,int,void*,uint64_t);
int sysLv2FsRead(int,void*,uint64_t,uint64_t*);
int sysLv2FsWrite(int,const void*,uint64_t,uint64_t*);
int sysLv2FsClose(int);
int sysLv2FsFsync(int);
int sysLv2FsRename(const char*,const char*);
int sysLv2FsUnlink(const char*);
int sysEventQueueReceive(uint32_t,sys_event_t*,uint64_t);
int sysEventPortSend(uint32_t,uint64_t,uint64_t,uint64_t);
int sysEventQueueDestroy(uint32_t,int);
int sysEventPortDisconnect(uint32_t);
int sysEventPortDestroy(uint32_t);
int sysEventPortCreate(uint32_t*,int,uint64_t);
int sysEventQueueCreate(uint32_t*,const sys_event_queue_attr_t*,uint64_t,int);
int orbit48_cobra(void),orbit48_hen(void),orbit48_console_type(void);
uint64_t orbit48_time(void),orbit48_peek(uint64_t);
void orbit48_poke(uint64_t,uint64_t),orbit48_sleep(uint64_t),orbit48_exit_thread(void);
int orbit48_join(uint64_t),orbit48_thread(uint64_t*,void(*)(uint64_t),int,uint64_t,const char*);
int orbit48_storage_info(uint64_t,Orbit48DeviceInfo*),orbit48_storage_open(uint64_t,uint32_t*);
int orbit48_storage_close(uint32_t),orbit48_storage_read(uint32_t,uint64_t,uint32_t,void*);
int orbit48_disc_types(uint32_t*,uint32_t*,uint32_t*),orbit48_eject(void),orbit48_insert(uint32_t);
int orbit48_unmount(void),orbit48_iso(char**,uint32_t),orbit48_map(const char*,const char*);
int orbit48_proxy(uint32_t,uint32_t,uint64_t),orbit48_temperature(unsigned,unsigned*);
int orbit48_get_plugin(unsigned,char*,char*),orbit48_processes(uint32_t*),orbit48_process_name(uint32_t,char*);
uint32_t orbit48_firmware(void),orbit48_pid(void);
extern int64_t p1;
int64_t mock_fan_get(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
int64_t mock_fan_set(uint64_t,uint64_t,uint64_t);
#define lv2syscall5(n,a,b,c,d,e) (p1=mock_fan_get(a,b,c,d,e))
#define lv2syscall3(n,a,b,c) (p1=mock_fan_set(a,b,c))
