#define ORBIT48_HOST_TEST 1
#include <assert.h>
#include <stdio.h>
#include "../plugin/service.c"

typedef struct {char path[1024];unsigned char bytes[ORBIT48_MAX_PAYLOAD+24];uint32_t size,mode;int used;} File;
static File files[20];
static unsigned ejects,inserts,unmounts,maps,opens,pokes,peek_count;
static int mount_error,external_plugin,direct_denied,readback_bad;
static unsigned hw_mode=1,hw_speed=100;
static uint64_t get_code=0x480000012c030000ull,set_code=0x480000012c230000ull;
static uint64_t clock_value;
static uint32_t active_pid;
static uint64_t device_serial=123,device_sectors=100000;
int64_t p1;
static int find_file(const char* path){for(int i=0;i<20;++i)if(files[i].used && !strcmp(files[i].path,path))return i;return -1;}
static int put_file(const char* path,const void* bytes,unsigned n,uint32_t mode){
    int at=find_file(path);if(at<0)for(at=0;at<20 && files[at].used;++at){}
    assert(at<20 && n<=sizeof(files[at].bytes));strcpy(files[at].path,path);files[at].used=1;files[at].mode=mode;files[at].size=n;
    if(n && bytes)memcpy(files[at].bytes,bytes,n);
    return at;
}
int sysLv2FsStat(const char* p,sysFSStat* s){const int at=find_file(p);if(at<0)return -1;s->size=files[at].size;s->mode=files[at].mode;return 0;}
int sysLv2FsOpen(const char* p,int flags,int* fd,int mode,void* a,uint64_t n){
    (void)mode;(void)a;(void)n;int at=find_file(p);if(flags&0x200)at=put_file(p,0,0,0100000);if(at<0)return -1;*fd=at;return 0;
}
int sysLv2FsRead(int f,void* out,uint64_t n,uint64_t* got){assert(n<=files[f].size);memcpy(out,files[f].bytes,n);*got=n;return 0;}
int sysLv2FsWrite(int f,const void* b,uint64_t n,uint64_t* done){assert(n<=sizeof(files[f].bytes));memcpy(files[f].bytes,b,n);files[f].size=(unsigned)n;*done=n;return 0;}
int sysLv2FsClose(int f){(void)f;return 0;}int sysLv2FsFsync(int f){(void)f;return 0;}
int sysLv2FsUnlink(const char* p){const int at=find_file(p);if(at<0)return -1;files[at].used=0;return 0;}
int sysLv2FsRename(const char* a,const char* b){const int at=find_file(a);if(at<0 || find_file(b)>=0)return -1;strcpy(files[at].path,b);return 0;}
int sysEventQueueReceive(uint32_t a,sys_event_t* b,uint64_t c){(void)a;(void)b;(void)c;return -1;}
int sysEventPortSend(uint32_t a,uint64_t b,uint64_t c,uint64_t d){(void)a;(void)b;(void)c;(void)d;return 0;}
int sysEventQueueDestroy(uint32_t a,int b){(void)a;(void)b;return 0;}int sysEventPortDisconnect(uint32_t a){(void)a;return 0;}
int sysEventPortDestroy(uint32_t a){(void)a;return 0;}
int sysEventPortCreate(uint32_t* a,int b,uint64_t c){(void)b;(void)c;*a=1;return 0;}
int sysEventQueueCreate(uint32_t* a,const sys_event_queue_attr_t* b,uint64_t c,int d){(void)b;(void)c;(void)d;*a=2;return 0;}
int orbit48_cobra(void){return 1;}int orbit48_hen(void){return 1;}int orbit48_console_type(void){return 1;}
uint64_t orbit48_time(void){return clock_value;}void orbit48_sleep(uint64_t n){clock_value+=n;}void orbit48_exit_thread(void){}
int orbit48_join(uint64_t a){(void)a;return 0;}int orbit48_thread(uint64_t* a,void(*b)(uint64_t),int c,uint64_t d,const char* e){(void)b;(void)c;(void)d;(void)e;*a=9;return 0;}
uint32_t orbit48_firmware(void){return 0x0492;}uint32_t orbit48_pid(void){return 1;}
int orbit48_processes(uint32_t* p){p[0]=active_pid;return 0;}
int orbit48_process_name(uint32_t p,char* n){(void)p;strcpy(n,"game");return 0;}
int orbit48_get_plugin(unsigned s,char* n,char* p){if(external_plugin && s==2){strcpy(n,"webMAN MOD");strcpy(p,"/dev_hdd0/webftp_server.sprx");}return 0;}
int orbit48_temperature(unsigned a,unsigned* b){(void)a;*b=68;return 1;}
uint64_t orbit48_peek(uint64_t a){++peek_count;return a==fan_get_offset?get_code:set_code;}
void orbit48_poke(uint64_t a,uint64_t b){++pokes;if(a==fan_get_offset)get_code=b;else set_code=b;}
int64_t mock_fan_get(uint64_t a,uint64_t state,uint64_t mode,uint64_t speed,uint64_t unknown){
    (void)a;(void)state;(void)unknown;if(direct_denied && (uint32_t)(get_code>>32)!=0x38600001)return -1;
    *(uint8_t*)(uintptr_t)mode=(uint8_t)hw_mode;*(uint8_t*)(uintptr_t)speed=(uint8_t)(hw_speed+(readback_bad?5:0));return 0;
}
int64_t mock_fan_set(uint64_t a,uint64_t mode,uint64_t speed){(void)a;if(direct_denied && (uint32_t)(set_code>>32)!=0x38600001)return -1;hw_mode=(unsigned)mode;hw_speed=(unsigned)speed;return 0;}
int orbit48_storage_info(uint64_t a,Orbit48DeviceInfo* b){(void)a;b->sectors=device_sectors;b->sector_size=512;return 0;}
int orbit48_storage_open(uint64_t a,uint32_t* h){(void)a;++opens;*h=3;return 0;}int orbit48_storage_close(uint32_t h){(void)h;return 0;}
int orbit48_storage_read(uint32_t h,uint64_t lba,uint32_t n,void* out){(void)h;(void)lba;memset(out,0,n*512);unsigned char* b=out;memcpy(b+3,"NTFS    ",8);for(unsigned i=0;i<8;++i)b[72+i]=(unsigned char)(device_serial>>(8*i));return 1;}
int orbit48_disc_types(uint32_t* a,uint32_t* b,uint32_t* c){*a=0x10;*b=*c=0;return 0;}
int orbit48_eject(void){++ejects;return 0;}int orbit48_insert(uint32_t a){(void)a;++inserts;return 0;}int orbit48_unmount(void){++unmounts;return 0;}
int orbit48_iso(char** a,uint32_t b){(void)a;(void)b;return mount_error;}int orbit48_map(const char* a,const char* b){(void)a;(void)b;++maps;return 0;}
int orbit48_proxy(uint32_t a,uint32_t b,uint64_t c){(void)a;(void)b;(void)c;return mount_error;}
static unsigned native(unsigned char* b,const char* path,int folder){memset(b,0,2048);orbit48_put32(b,(unsigned)folder);orbit48_put32(b+4,1);memcpy(b+8,"BLES00001",9);const unsigned n=(unsigned)strlen(path);orbit48_put32(b+18,n);memcpy(b+22,path,n);return 22+n;}
static void request_config(Orbit48FanConfig config){unsigned char b[44];header(b,ORBIT48_CONFIGURE,55,20,0);orbit48_put32(b+24,config.mode);orbit48_put32(b+28,config.target_c);orbit48_put32(b+32,config.minimum_percent);orbit48_put32(b+36,config.maximum_percent);orbit48_put32(b+40,config.manual_percent);put_file(ORBIT48_CONTROL_REQUEST,b,44,0100000);dispatch_request(ORBIT48_CONTROL_REQUEST,ORBIT48_CONTROL_REPLY,0);}
int main(void){
    unsigned char b[2048];unsigned n=native(b,"/dev_hdd0/PS3ISO/test.iso",0);put_file("/dev_hdd0/PS3ISO/test.iso",0,2048,0100000);
    assert(mount_native(b,n,1)==0 && ejects==1 && inserts==1);unsigned old=ejects;
    n=native(b,"/dev_hdd0/PS3ISO/../bad.iso",0);assert(mount_native(b,n,2)==ORBIT48_BAD_PATH && ejects==old);
    n=native(b,"/dev_hdd0/PS3ISO/test.iso",0);b[17]=1;assert(mount_native(b,n,2)==ORBIT48_BAD_PATH && ejects==old);
    n=native(b,"/dev_hdd0/PS3ISO/test.iso",0);unsigned char token[8];orbit48_put64(token,9);put_file(ORBIT48_CANCEL_PATH,token,8,0100000);
    assert(mount_native(b,n,9)==ORBIT48_CANCELLED && ejects==old);sysLv2FsUnlink(ORBIT48_CANCEL_PATH);
    mount_error=-2;assert(mount_native(b,n,1)==-2 && inserts==2 && unmounts>=3);mount_error=0;
    n=native(b,"/dev_usb000/GAMES/test",1);put_file("/dev_usb000/GAMES/test",0,0,0040000);put_file("/dev_usb000/GAMES/test/PS3_GAME/PARAM.SFO",0,100,0100000);old=ejects;
    assert(mount_native(b,n,1)==ORBIT48_BAD_IMAGE && ejects==old);
    unsigned char raw[72]={0};orbit48_put64(raw,0x10300000000000aull);orbit48_put64(raw+16,123);orbit48_put64(raw+24,100000);orbit48_put64(raw+32,2048);orbit48_put32(raw+40,512);orbit48_put32(raw+44,1);orbit48_put64(raw+56,50);orbit48_put64(raw+64,4);
    device_serial=124;assert(mount_raw(raw,72,1)==ORBIT48_USB_CHANGED && ejects==old);device_serial=123;device_sectors=99999;
    assert(mount_raw(raw,72,1)==ORBIT48_USB_CHANGED && ejects==old);device_sectors=100000;orbit48_put64(raw+64,0);
    assert(mount_raw(raw,72,1)==ORBIT48_BAD_REQUEST && ejects==old);
    fan_supported=1;fan_get_offset=1;fan_set_offset=2;direct_denied=1;const uint64_t original_get=get_code,original_set=set_code;
    assert(set_policy(50) && hw_mode==2 && get_code==original_get && set_code==original_set && pokes==4);
    set_code=0xdeadbeef00000000ull;unsigned before=pokes;assert(!set_policy(60) && pokes==before);set_code=original_set;
    direct_denied=0;before=peek_count;assert(set_policy(-1) && hw_mode==1 && peek_count==before);
    readback_bad=1;assert(!set_policy(40) && !fan_supported && hw_mode==1);readback_bad=0;fan_supported=1;
    orbit48_fan_init(&policy);fan_status.config=policy.config;Orbit48FanConfig config={1,68,35,85,40};request_config(config);assert(policy.config.mode==1);
    config.manual_percent=0;request_config(config);assert(policy.config.manual_percent==40);
    external_plugin=1;config.mode=2;config.manual_percent=55;request_config(config);assert(policy.config.mode==1);assert(external_controller());external_plugin=0;
    is_hen=1;active_pid=12;assert(game_cooldown(100));assert(game_cooldown(14999999));assert(!game_cooldown(15000100));active_pid=13;assert(game_cooldown(16000000));
    raw_active=1;assert(orbit_service_stop(0,0)==1);raw_active=0;
    puts("PASS: actual resident dispatcher, validation before eject, cancellation, changed USB, mount failure cleanup, config conflicts");
    puts("PASS: guarded fan adapter, exact instruction restoration, unknown-code refusal, speed readback and HEN process cooldown (simulated APIs)");
}
