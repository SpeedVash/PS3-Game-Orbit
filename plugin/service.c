#include "native_calls.h"
#include "../include/orbit_service_protocol_fix48.h"
#include "../include/fan_policy_fix48.h"
#include "../include/raw_reader_fix48.h"

/* Freestanding module: all storage is bounded and all APIs are direct syscalls. */
#ifndef ORBIT48_HOST_TEST
void* memcpy(void* out,const void* in,size_t n){unsigned char* d=out;const unsigned char* s=in;while(n--)*d++=*s++;return out;}
void* memset(void* out,int value,size_t n){unsigned char* d=out;while(n--)*d++=(unsigned char)value;return out;}
int memcmp(const void* a,const void* b,size_t n){const unsigned char* x=a;const unsigned char* y=b;while(n--){if(*x!=*y)return *x-*y;++x;++y;}return 0;}
size_t strlen(const char* s){size_t n=0;while(s[n])++n;return n;}
int strncmp(const char* a,const char* b,size_t n){
    while(n--){if((unsigned char)*a!=(unsigned char)*b)return (unsigned char)*a-(unsigned char)*b;
        if(!*a)return 0;
        ++a;++b;}return 0;
}
void* memchr(const void* input,int value,size_t n){
    const unsigned char* p=input;while(n--){if(*p==(unsigned char)value)return (void*)p;++p;}return 0;
}
#endif
static int contains(const char* s,const char* target){
    const size_t n=strlen(target),length=strlen(s);if(n>length)return 0;
    for(size_t i=0;i<=length-n;++i)if(!memcmp(s+i,target,n))return 1;
    return 0;
}
static volatile int running=0,raw_active=0,raw_stop=0,dispatch_busy=0,lock=0;
static uint64_t thermal_thread=0,dispatch_thread=0,raw_thread=0;
static uint32_t raw_port=0,raw_queue=0,raw_handle=0xffffffffu;
static uint64_t fan_get_offset=0,fan_set_offset=0;
static int fan_supported=0,is_hen=0,own_policy=0,foreign_change=0;
static uint32_t last_fan_byte=0,last_fan_mode=0;
static Orbit48FanPolicy policy;
static Orbit48FanStatus fan_status;
static Orbit48RawMap raw_map,new_map;
static unsigned char request[ORBIT48_HEADER_BYTES+ORBIT48_MAX_PAYLOAD];
static unsigned char scratch[65536] __attribute__((aligned(128)));
static char paths[ORBIT48_MAX_PARTS][ORBIT48_PATH_BYTES];
static char* path_pointers[ORBIT48_MAX_PARTS];
static void take_lock(void){while(__sync_lock_test_and_set(&lock,1))orbit48_sleep(100);}
static void release_lock(void){__sync_lock_release(&lock);}
static int read_file(const char* path,void* out,uint32_t cap,uint32_t* length){
    int fd=-1;uint64_t n=0;sysFSStat info;
    if(sysLv2FsStat(path,&info) || info.size<0 || (uint64_t)info.size>cap || sysLv2FsOpen(path,0,&fd,0,0,0))return 0;
    const int ok=!sysLv2FsRead(fd,out,(uint64_t)info.size,&n) && n==(uint64_t)info.size;
    sysLv2FsClose(fd);if(ok)*length=(uint32_t)n;return ok;
}
static int write_file(const char* path,const void* data,uint32_t length){
    char temp[ORBIT48_PATH_BYTES],backup[ORBIT48_PATH_BYTES];const size_t n=strlen(path);
    if(n+5>=sizeof(temp))return 0;
    memcpy(temp,path,n);memcpy(temp+n,".tmp",5);
    memcpy(backup,path,n);memcpy(backup+n,".old",5);
    int fd=-1;uint64_t done=0;
    if(sysLv2FsOpen(temp,0x200|0x400|1,&fd,0600,0,0))return 0;
    int ok=!sysLv2FsWrite(fd,data,length,&done) && done==length && !sysLv2FsFsync(fd);
    if(sysLv2FsClose(fd))ok=0;
    if(ok && sysLv2FsRename(temp,path)){
        sysLv2FsUnlink(backup);
        if(!sysLv2FsRename(path,backup)){
            ok=!sysLv2FsRename(temp,path);
            if(ok)sysLv2FsUnlink(backup);else sysLv2FsRename(backup,path);
        }else ok=0;
    }
    if(!ok)sysLv2FsUnlink(temp);
    return ok;
}
static void header(unsigned char* b,uint16_t command,uint64_t seq,uint32_t length,uint32_t error){
    memcpy(b,"PGOR",4);b[4]=1;b[5]=6;b[6]=(unsigned char)(command>>8);b[7]=(unsigned char)command;
    orbit48_put64(b+8,seq);orbit48_put32(b+16,length);orbit48_put32(b+20,error);
}
static void status_bytes(unsigned char* out){
    Orbit48FanStatus s;take_lock();s=fan_status;release_lock();
    orbit48_put32(out,s.config.mode);orbit48_put32(out+4,s.config.target_c);
    orbit48_put32(out+8,s.config.minimum_percent);orbit48_put32(out+12,s.config.maximum_percent);
    orbit48_put32(out+16,s.config.manual_percent);orbit48_put32(out+20,s.state);
    orbit48_put32(out+24,s.cpu_c);orbit48_put32(out+28,s.rsx_c);
    orbit48_put32(out+32,s.percent);orbit48_put32(out+36,s.native_error);
    orbit48_put32(out+40,(uint32_t)raw_active);
}
static void reply(const char* path,uint16_t command,uint64_t seq,uint32_t error,int with_status){
    unsigned char b[ORBIT48_HEADER_BYTES+44];header(b,command|0x8000u,seq,with_status?44:0,error);
    if(with_status)status_bytes(b+ORBIT48_HEADER_BYTES);
    write_file(path,b,ORBIT48_HEADER_BYTES+(with_status?44:0));
}
static int external_controller(void){
    for(unsigned slot=0;slot<7;++slot){
        char name[64],path[1024];memset(name,0,sizeof(name));memset(path,0,sizeof(path));
        if(!orbit48_get_plugin(slot,name,path)){
            name[63]=0;path[1023]=0;
            if(contains(path,"webftp_server") || contains(path,"sman.sprx") ||
               contains(name,"webMAN") || contains(name,"sMAN") || contains(path,"fan_control.sprx"))return 1;
        }
    }return 0;
}
/* Patch only the permission call at known fan API sites, then restore exact bytes. */
static int fan_patch(uint64_t offset,uint64_t* original,int* changed){
    *original=orbit48_peek(offset);*changed=0;
    const uint32_t instruction=(uint32_t)(*original>>32),next=(uint32_t)*original;
    if(instruction==0x38600001u)return 1;
    const int call=(instruction&0xfc000003u)==0x48000001u &&
        ((next&0xffff0000u)==0x2c030000u || (next&0xffff0000u)==0x2c230000u);
    const int false_return=instruction==0x38600000u && next==0x4e800020u;
    if(!call && !false_return)return 0;
    orbit48_poke(offset,0x3860000100000000ull|next);
    if(orbit48_peek(offset)!=(0x3860000100000000ull|next))return 0;
    *changed=1;return 1;
}
static int fan_restore(uint64_t offset,uint64_t original,int changed){
    if(!changed)return 1;
    if(orbit48_peek(offset)!=(0x3860000100000000ull|(uint32_t)original))return 0;
    orbit48_poke(offset,original);return orbit48_peek(offset)==original;
}
static int get_policy(unsigned* mode,unsigned* speed){
    uint64_t before;int changed;uint8_t state=0,m=0,s=0,unknown=0;
    if(!fan_supported)return 0;
    int direct;
    {lv2syscall5(409,0,(uint64_t)(uintptr_t)&state,(uint64_t)(uintptr_t)&m,
        (uint64_t)(uintptr_t)&s,(uint64_t)(uintptr_t)&unknown);direct=(int)p1;}
    if(!direct && (m==1 || m==2)){*mode=m;*speed=s;return 1;}
    if(!fan_patch(fan_get_offset,&before,&changed))return 0;
    int result;
    {lv2syscall5(409,0,(uint64_t)(uintptr_t)&state,(uint64_t)(uintptr_t)&m,
        (uint64_t)(uintptr_t)&s,(uint64_t)(uintptr_t)&unknown);result=(int)p1;}
    if(!fan_restore(fan_get_offset,before,changed)){fan_supported=0;return 0;}
    if(result || (m!=1 && m!=2))return 0;
    *mode=m;*speed=s;return 1;
}
static int verify_set(unsigned mode,unsigned speed){
    unsigned actual_mode=0,actual_speed=0;
    if(!get_policy(&actual_mode,&actual_speed) || actual_mode!=mode ||
       (mode==2 && (actual_speed>speed?actual_speed-speed:speed-actual_speed)>1)){
        if(mode==2){lv2syscall3(389,0,1,0);}
        fan_supported=0;own_policy=0;return 0;
    }
    last_fan_mode=mode;last_fan_byte=actual_speed;own_policy=mode==2;return 1;
}
static int set_policy(int percent){
    uint64_t before;int changed;
    if(!fan_supported)return 0;
    const unsigned mode=percent<0?1:2,speed=percent<0?0:((unsigned)percent*255+99)/100;
    int direct;{lv2syscall3(389,0,mode,speed);direct=(int)p1;}
    if(!direct)return verify_set(mode,speed);
    if(!fan_patch(fan_set_offset,&before,&changed))return 0;
    int result;{lv2syscall3(389,0,mode,speed);result=(int)p1;}
    if(!fan_restore(fan_set_offset,before,changed)){fan_supported=0;return 0;}
    if(result)return 0;
    return verify_set(mode,speed);
}
static int game_cooldown(uint64_t now){
    static uint32_t previous=0;static uint64_t until=0;
    if(!is_hen)return 0;
    uint32_t pids[16];memset(pids,0,sizeof(pids));
    if(orbit48_processes(pids))return 1;
    const uint32_t self=orbit48_pid();uint32_t active=0;
    for(unsigned i=0;i<16;++i)if(pids[i] && pids[i]!=self){
        char name[128];memset(name,0,sizeof(name));
        if(!orbit48_process_name(pids[i],name)){
            name[127]=0;
            if(!contains(name,"vsh") && !contains(name,"system_manager") && !contains(name,"init")){active=pids[i];break;}
        }
    }
    if(active!=previous){previous=active;until=now+15000000ull;}
    return now<until;
}
static void thermal(uint64_t unused){
    (void)unused;unsigned ticks=0;
    while(running){
        unsigned cpu=0,rsx=0;const int ok=orbit48_temperature(0,&cpu) && orbit48_temperature(1,&rsx);
        const int external=external_controller(),pause=game_cooldown(orbit48_time());
        unsigned mode=0,speed=0;int conflict=external || foreign_change;
        if(!pause && !external && own_policy && get_policy(&mode,&speed) &&
           (mode!=last_fan_mode || (speed>last_fan_byte?speed-last_fan_byte:last_fan_byte-speed)>1)){
            conflict=1;foreign_change=1;own_policy=0;
        }
        take_lock();const int wanted=orbit48_fan_tick(&policy,(int)cpu,(int)rsx,ok,conflict,pause);
        fan_status.config=policy.config;fan_status.state=policy.state;
        fan_status.cpu_c=cpu;fan_status.rsx_c=rsx;fan_status.native_error=0;
        release_lock();
        if(!external && !conflict && !pause && fan_supported && wanted>=-1){
            const unsigned desired_mode=wanted<0?1:2,desired_byte=wanted<0?0:((unsigned)wanted*255+99)/100;
            if(desired_mode!=last_fan_mode || (desired_mode==2 && desired_byte!=last_fan_byte)){
                if(!set_policy(wanted)){take_lock();fan_status.state=ORBIT48_FAN_UNSUPPORTED;fan_status.native_error=ORBIT48_FAN_UNAVAILABLE;release_lock();}
            }
        }
        if(!fan_supported){take_lock();fan_status.state=ORBIT48_FAN_UNSUPPORTED;release_lock();}
        if(!pause && !external && !conflict && get_policy(&mode,&speed)){take_lock();fan_status.percent=(speed*100+127)/255;release_lock();}
        if(!(ticks++%2))reply(ORBIT48_STATUS_PATH,ORBIT48_STATUS,orbit48_time(),0,1);
        for(unsigned i=0;i<20 && running;++i)orbit48_sleep(50000);
    }
    if(own_policy && !external_controller())set_policy(-1);
    orbit48_exit_thread();
}
static int raw_read_sector(void* context,uint64_t lba,uint32_t count,void* out){
    (void)context;return orbit48_storage_read(raw_handle,lba,count,out);
}
static void raw_worker(uint64_t unused){
    (void)unused;
    while(running && !raw_stop){
        sys_event_t event;const int result=sysEventQueueReceive(raw_queue,&event,100000);
        if(result==(int)0x80010005)continue; // ETIMEDOUT
        if(result)break;
        int status=(int)0x80010009;
        if(event.data_1==0){
            void* destination=(void*)(uintptr_t)(uint32_t)(event.data_3>>32);
            const uint32_t amount=(uint32_t)event.data_3;
            if(orbit48_raw_read(&raw_map,raw_read_sector,0,event.data_2,destination,amount,scratch,sizeof(scratch)))status=0;
        }
        for(unsigned retry=0;retry<50 && running && !raw_stop;++retry){
            const int sent=sysEventPortSend(raw_port,(uint64_t)(uint32_t)status,0,0);
            if(!sent)break;
            orbit48_sleep(1000);
        }
    }
    raw_active=0;orbit48_exit_thread();
}
static void stop_raw(void){
    if(raw_thread){raw_stop=1;orbit48_unmount();orbit48_join(raw_thread);raw_thread=0;}
    if(raw_queue){sysEventQueueDestroy(raw_queue,SYS_EVENT_QUEUE_FORCE_DESTROY);raw_queue=0;}
    if(raw_port){sysEventPortDisconnect(raw_port);sysEventPortDestroy(raw_port);raw_port=0;}
    if(raw_handle!=0xffffffffu){orbit48_storage_close(raw_handle);raw_handle=0xffffffffu;}
    raw_active=0;
}
static int valid_id(const char* s){
    for(unsigned i=0;i<9;++i)if(i<4?(s[i]<'A'||s[i]>'Z'):(s[i]<'0'||s[i]>'9'))return 0;
    return s[9]==0;
}
static int valid_path(const char* path,int folder){
    if(strncmp(path,"/dev_hdd0/",10) && !(strlen(path)>12 && !strncmp(path,"/dev_usb",8) &&
       path[8]>='0' && path[8]<='9' && path[9]>='0' && path[9]<='9' && path[10]>='0' && path[10]<='9' && path[11]=='/'))return 0;
    const char* relative=path+(!strncmp(path,"/dev_hdd0/",10)?10:12);
    if(folder){if(strncmp(relative,"GAMES/",6) && strncmp(relative,"GAMEZ/",6))return 0;}
    else if(strncmp(relative,"PS3ISO/",7))return 0;
    for(const char* p=path;*p;++p)if((unsigned char)*p<32 || *p=='\\' || *p==':')return 0;
    if(contains(path,"/../") || contains(path,"/./") || contains(path,"//"))return 0;
    const size_t n=strlen(path);if(n<1 || path[n-1]=='/' || (n>=3 && !memcmp(path+n-3,"/..",3)))return 0;
    return 1;
}
static int cancelled(uint64_t sequence){
    unsigned char b[8];uint32_t n=0;return read_file(ORBIT48_CANCEL_PATH,b,sizeof(b),&n) && n==8 && orbit48_be64(b)==sequence;
}
static int parse_native(const unsigned char* b,uint32_t n,int* folder,char id[10],uint32_t* count){
    if(n<18)return 0;
    *folder=(int)orbit48_be32(b);*count=orbit48_be32(b+4);
    if(*folder<0 || *folder>1 || !*count || *count>ORBIT48_MAX_PARTS || (*folder && *count!=1))return 0;
    memcpy(id,b+8,10);if(id[9] || (*folder && !valid_id(id)))return 0;
    if(id[0] && !valid_id(id))return 0;
    uint32_t at=18;uint64_t total=0;
    for(unsigned i=0;i<*count;++i){
        if(at>n || n-at<4)return 0;
        const uint32_t bytes=orbit48_be32(b+at);at+=4;
        if(!bytes || bytes>=ORBIT48_PATH_BYTES || bytes>n-at || memchr(b+at,0,bytes))return 0;
        memcpy(paths[i],b+at,bytes);paths[i][bytes]=0;at+=bytes;
        if(!valid_path(paths[i],*folder))return 0;
        path_pointers[i]=paths[i];
        sysFSStat info;if(sysLv2FsStat(paths[i],&info) ||
           ((info.mode&0170000)!=(*folder?0040000:0100000)) || (!*folder && (info.size<=0 || info.size%512)))return 0;
        if(!*folder){if((uint64_t)info.size>UINT64_MAX-total)return 0;total+=(uint64_t)info.size;}
    }return at==n && (*folder || total%2048==0);
}
static int mount_native(const unsigned char* b,uint32_t n,uint64_t sequence){
    int folder=0;uint32_t count=0;char id[10];if(!parse_native(b,n,&folder,id,&count))return ORBIT48_BAD_PATH;
    if(cancelled(sequence))return ORBIT48_CANCELLED;
    uint32_t real=0,effective=0,fake=0;if(orbit48_disc_types(&real,&effective,&fake))return ORBIT48_NO_COBRA;
    if(folder){
        char sfo[ORBIT48_PATH_BYTES];const size_t len=strlen(paths[0]);
        if(len+20>=sizeof(sfo))return ORBIT48_BAD_PATH;
        memcpy(sfo,paths[0],len);memcpy(sfo+len,"/PS3_GAME/PARAM.SFO",20);
        sysFSStat stat;if(sysLv2FsStat(sfo,&stat) || !stat.size)return ORBIT48_BAD_IMAGE;
        if(sysLv2FsStat(ORBIT48_BLANK_PATH,&stat) || stat.size!=131072)return ORBIT48_BAD_IMAGE;
        uint32_t bytes=0;
        if(!read_file(ORBIT48_BLANK_PATH,request,sizeof(request),&bytes) || bytes!=131072 ||
           memcmp(request+0x800,"PlayStation3",12) || memcmp(request+0x8001,"CD001",5))return ORBIT48_BAD_IMAGE;
        memcpy(request+0x810,id,4);request[0x814]='-';memcpy(request+0x815,id+4,5);
        if(!write_file(ORBIT48_BLANK_PATH,request,bytes))return ORBIT48_BAD_IMAGE;
    }
    orbit48_eject();stop_raw();orbit48_unmount();
    orbit48_map("/dev_bdvd",0);orbit48_map("//dev_bdvd",0);orbit48_map("/app_home",0);
    int result=0;
    if(folder){
        result=orbit48_map("/dev_bdvd",paths[0]);
        if(!result)result=orbit48_map("//dev_bdvd",paths[0]);
        if(!result){char* blank=(char*)ORBIT48_BLANK_PATH;result=orbit48_iso(&blank,1);}
    }else result=orbit48_iso(path_pointers,count);
    if(!result)result=orbit48_map("/dev_bdvd/PS3_UPDATE",ORBIT48_EMPTY_PATH);
    if(!result)result=orbit48_insert(real);
    if(result){
        orbit48_unmount();orbit48_map("/dev_bdvd",0);orbit48_map("//dev_bdvd",0);
        orbit48_map("/dev_bdvd/PS3_UPDATE",0);orbit48_insert(real);
    }return result;
}
static int parse_raw(const unsigned char* b,uint32_t n){
    if(n<48)return 0;
    new_map.device_id=orbit48_be64(b);new_map.partition_lba=orbit48_be64(b+8);
    new_map.volume_serial=orbit48_be64(b+16);new_map.device_sectors=orbit48_be64(b+24);
    new_map.iso_size=orbit48_be64(b+32);new_map.sector_size=orbit48_be32(b+40);new_map.count=orbit48_be32(b+44);
    if(new_map.count>ORBIT48_MAX_EXTENTS || n!=48+new_map.count*24)return 0;
    for(unsigned i=0;i<new_map.count;++i){
        const unsigned char* p=b+48+i*24;new_map.extents[i].iso_offset=orbit48_be64(p);
        new_map.extents[i].device_lba=orbit48_be64(p+8);new_map.extents[i].sector_count=orbit48_be64(p+16);
    }
    if(new_map.device_id<0x10300000000000aull || new_map.device_id>0x103000000000098ull)return 0;
    return orbit48_validate_raw_map(&new_map);
}
static uint64_t little64(const unsigned char* b){uint64_t v=0;for(unsigned i=0;i<8;++i)v|=(uint64_t)b[i]<<(i*8);return v;}
static int mount_raw(const unsigned char* b,uint32_t n,uint64_t sequence){
    if(!parse_raw(b,n))return ORBIT48_BAD_REQUEST;
    Orbit48DeviceInfo info;memset(&info,0,sizeof(info));
    if(orbit48_storage_info(new_map.device_id,&info) || info.sectors!=new_map.device_sectors ||
       info.sector_size!=new_map.sector_size)return ORBIT48_USB_CHANGED;
    uint32_t handle=0xffffffffu;if(orbit48_storage_open(new_map.device_id,&handle))return ORBIT48_USB_CHANGED;
    unsigned char boot[4096] __attribute__((aligned(128)));
    const int valid=orbit48_storage_read(handle,new_map.partition_lba,1,boot) &&
        !memcmp(boot+3,"NTFS    ",8) && little64(boot+72)==new_map.volume_serial;
    if(!valid || cancelled(sequence)){orbit48_storage_close(handle);return valid?ORBIT48_CANCELLED:ORBIT48_USB_CHANGED;}
    uint32_t real=0,effective=0,fake=0;
    if(orbit48_disc_types(&real,&effective,&fake)){orbit48_storage_close(handle);return ORBIT48_NO_COBRA;}
    orbit48_eject();stop_raw();orbit48_unmount();orbit48_map("/dev_bdvd",0);orbit48_map("//dev_bdvd",0);orbit48_map("/app_home",0);
    raw_handle=handle;raw_map=new_map;raw_stop=0;
    sys_event_queue_attr_t attr={SYS_EVENT_QUEUE_FIFO,SYS_EVENT_QUEUE_PPU,{'O','R','B','I','T','4','8',0}};
    int result=sysEventPortCreate(&raw_port,SYS_EVENT_PORT_LOCAL,SYS_EVENT_PORT_NO_NAME);
    if(!result)result=sysEventQueueCreate(&raw_queue,&attr,SYS_EVENT_QUEUE_KEY_LOCAL,1);
    if(!result){raw_active=1;result=orbit48_thread(&raw_thread,raw_worker,1300,65536,"Orbit NTFS");}
    if(!result)result=orbit48_proxy(raw_port,raw_queue,raw_map.iso_size);
    if(!result)result=orbit48_map("/dev_bdvd/PS3_UPDATE",ORBIT48_EMPTY_PATH);
    if(!result)result=orbit48_insert(real);
    if(result){stop_raw();orbit48_unmount();orbit48_map("/dev_bdvd/PS3_UPDATE",0);orbit48_insert(real);}
    return result;
}
static void dispatch_request(const char* input,const char* output,int mount){
    uint32_t n=0;if(!read_file(input,request,sizeof(request),&n))return;
    if(n<ORBIT48_HEADER_BYTES || memcmp(request,"PGOR",4) || request[4]!=1 || request[5]!=6){sysLv2FsUnlink(input);return;}
    const uint16_t command=(uint16_t)((unsigned)request[6]*256+request[7]);const uint64_t seq=orbit48_be64(request+8);
    const uint32_t length=orbit48_be32(request+16);int error=ORBIT48_BAD_REQUEST;int with_status=!mount;
    if(length==n-ORBIT48_HEADER_BYTES && !orbit48_be32(request+20)){
        const unsigned char* payload=request+ORBIT48_HEADER_BYTES;
        if(!mount && command==ORBIT48_STATUS && !length)error=0;
        else if(!mount && (command==ORBIT48_CONFIGURE || command==ORBIT48_RESTORE_SYSCON)){
            Orbit48FanConfig config;
            if(command==ORBIT48_RESTORE_SYSCON && !length){take_lock();config=policy.config;release_lock();config.mode=0;error=0;}
            else if(command==ORBIT48_CONFIGURE && length==20){
                config.mode=orbit48_be32(payload);config.target_c=orbit48_be32(payload+4);
                config.minimum_percent=orbit48_be32(payload+8);config.maximum_percent=orbit48_be32(payload+12);
                config.manual_percent=orbit48_be32(payload+16);error=orbit48_valid_fan_config(&config)?0:ORBIT48_BAD_REQUEST;
            }
            if(!error){
                if(external_controller())error=ORBIT48_FAN_CONFLICT;
                else if(!fan_supported)error=ORBIT48_FAN_UNAVAILABLE;
                else{take_lock();orbit48_fan_configure(&policy,&config);fan_status.config=config;own_policy=0;foreign_change=0;release_lock();}
            }
        }else if(mount && (command==ORBIT48_MOUNT_NATIVE || command==ORBIT48_MOUNT_NTFS)){
            dispatch_busy=1;error=command==ORBIT48_MOUNT_NATIVE?mount_native(payload,length,seq):mount_raw(payload,length,seq);dispatch_busy=0;
        }
    }
    reply(output,command,seq,(uint32_t)error,with_status);sysLv2FsUnlink(input);
}
static void dispatcher(uint64_t unused){
    (void)unused;while(running){dispatch_request(ORBIT48_CONTROL_REQUEST,ORBIT48_CONTROL_REPLY,0);
        dispatch_request(ORBIT48_MOUNT_REQUEST,ORBIT48_MOUNT_REPLY,1);orbit48_sleep(100000);}
    orbit48_exit_thread();
}
int orbit_service_start(uint64_t size,const void* argument){
    (void)size;(void)argument;
    if(running || !orbit48_cobra())return 1;
    is_hen=orbit48_hen();orbit48_fan_init(&policy);memset(&fan_status,0,sizeof(fan_status));
    fan_status.config=policy.config;
    const uint32_t fw=orbit48_firmware()&0xffffu;const int type=orbit48_console_type();
    if(fw>=0x0455 && fw<=0x0492 && (type==1 || type==3)){
        fan_get_offset=type==1?0x8000000000009e38ull:0x8000000000009eb8ull;
        fan_set_offset=type==1?0x800000000000a334ull:0x800000000000a3b4ull;
        fan_supported=1;
    }
    running=1;sysLv2FsUnlink(ORBIT48_CONTROL_REPLY);sysLv2FsUnlink(ORBIT48_MOUNT_REPLY);
    int result=orbit48_thread(&thermal_thread,thermal,1500,65536,"Orbit Fan");
    if(!result)result=orbit48_thread(&dispatch_thread,dispatcher,1700,65536,"Orbit Service");
    if(result){running=0;if(thermal_thread)orbit48_join(thermal_thread);thermal_thread=0;return 1;}
    return 0;
}
int orbit_service_stop(uint64_t size,const void* argument){
    (void)size;(void)argument;if(raw_active || dispatch_busy)return 1;
    running=0;if(dispatch_thread)orbit48_join(dispatch_thread);if(thermal_thread)orbit48_join(thermal_thread);
    dispatch_thread=thermal_thread=0;stop_raw();sysLv2FsUnlink(ORBIT48_STATUS_PATH);return 0;
}
