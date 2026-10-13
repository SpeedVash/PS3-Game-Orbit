#include "fan_policy_fix48.h"
#include "raw_reader_fix48.h"
#include "ntfs_reader_fix48.h"
#include "iso_metadata_fix48.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <random>
namespace {
void partition_tables(){
    auto little=[](unsigned char* p,std::uint64_t n,unsigned bytes){for(unsigned i=0;i<bytes;++i)p[i]=static_cast<unsigned char>(n>>(i*8));};
    auto crc=[](const unsigned char* p,std::size_t n){std::uint32_t c=0xffffffffu;while(n--){c^=*p++;for(unsigned i=0;i<8;++i)c=(c>>1)^(0xedb88320u&std::uint32_t(-int(c&1)));}return ~c;};
    assert(crc(reinterpret_cast<const unsigned char*>("123456789"),9)==0xcbf43926u);
    for(unsigned sector:{512u,4096u}){
        std::vector<unsigned char> bytes(sector*3,0);auto* m=bytes.data();auto* h=m+sector;auto* t=h+sector;
        m[510]=0x55;m[511]=0xaa;m[450]=0xee;little(m+454,1,4);little(m+458,0xffffffffu,4);
        const std::uint64_t start=0x100001000ull,cap=0x200000000ull;
        t[0]=1;little(t+32,start,8);little(t+40,start+999,8);
        memcpy(h,"EFI PART",8);little(h+8,0x10000,4);little(h+12,92,4);little(h+24,1,8);little(h+32,cap-1,8);
        little(h+40,34,8);little(h+48,cap-34,8);little(h+72,2,8);little(h+80,1,4);little(h+84,128,4);little(h+88,crc(t,128),4);little(h+16,crc(h,92),4);
        auto read=[&](std::uint64_t lba,std::uint32_t n,void* out){if(lba>2 || n>3-lba)return false;memcpy(out,bytes.data()+lba*sector,n*sector);return true;};
        auto parts=OrbitNtfs48::partitions(read,sector,cap);assert(parts.size()==1 && parts[0].start==start && parts[0].sectors==1000);
        t[40]^=1;assert(OrbitNtfs48::partitions(read,sector,cap).empty());t[40]^=1;h[12]^=1;assert(OrbitNtfs48::partitions(read,sector,cap).empty());
    }
    std::vector<unsigned char> m(512,0),a(512,0),b(512,0);m[510]=a[510]=b[510]=0x55;m[511]=a[511]=b[511]=0xaa;
    m[450]=7;little(m.data()+454,100,4);little(m.data()+458,200,4);
    auto read=[&](std::uint64_t lba,std::uint32_t n,void* out){if(n!=1)return false;const auto* p=lba==0?m.data():lba==1024?a.data():lba==1524?b.data():nullptr;if(!p)return false;memcpy(out,p,512);return true;};
    auto parts=OrbitNtfs48::partitions(read,512,10000);assert(parts.size()==1 && parts[0].start==100);
    m[450]=0x0f;little(m.data()+454,1024,4);a[450]=b[450]=7;little(a.data()+454,8,4);little(a.data()+458,200,4);little(b.data()+454,8,4);little(b.data()+458,200,4);
    a[466]=b[466]=0x0f;little(a.data()+470,500,4);little(b.data()+470,500,4);
    parts=OrbitNtfs48::partitions(read,512,10000);assert(parts.size()==2 && parts[0].start==1032 && parts[1].start==1532);
    puts("PASS: MBR, bounded cyclic EBR, GPT LBAs above 2 TiB, 512/4096-byte sectors and header/table CRC corruption rejection");
}
void fan(){
    Orbit48FanPolicy p;orbit48_fan_init(&p);
    assert(orbit48_fan_tick(&p,60,60,1,0,0)==-1);
    Orbit48FanConfig cfg{ORBIT48_FAN_AUTO,68,35,85,40};
    assert(orbit48_fan_configure(&p,&cfg));
    for(unsigned i=0;i<100;++i)assert(orbit48_fan_tick(&p,50,55,1,0,0)==35);
    assert(orbit48_fan_tick(&p,55,70,1,0,0)==37);
    for(unsigned i=0;i<100;++i)assert(orbit48_fan_tick(&p,78,60,1,0,0)<=85);
    assert(p.percent==85);
    for(unsigned i=0;i<4;++i)assert(orbit48_fan_tick(&p,60,60,1,0,0)==85);
    assert(orbit48_fan_tick(&p,60,60,1,0,0)==84);
    cfg.mode=ORBIT48_FAN_MANUAL;cfg.manual_percent=40;assert(orbit48_fan_configure(&p,&cfg));
    assert(orbit48_fan_tick(&p,60,60,1,0,0)==40);
    assert(orbit48_fan_tick(&p,80,60,1,0,0)>=85);
    assert(orbit48_fan_tick(&p,60,83,1,0,0)==100);
    assert(orbit48_fan_tick(&p,60,60,1,1,0)==-2 && p.state==ORBIT48_FAN_EXTERNAL);
    assert(orbit48_fan_tick(&p,60,60,1,0,1)==-2 && p.state==ORBIT48_FAN_STARTING_GAME);
    assert(orbit48_fan_tick(&p,0,0,0,0,0)>=35);
    assert(orbit48_fan_tick(&p,0,0,0,0,0)>=35);
    assert(orbit48_fan_tick(&p,0,0,0,0,0)==-1 && p.state==ORBIT48_FAN_SENSOR_FAILURE);
    cfg.minimum_percent=0;assert(!orbit48_fan_configure(&p,&cfg));
    cfg.minimum_percent=35;cfg.target_c=90;assert(!orbit48_fan_configure(&p,&cfg));
    cfg.target_c=68;cfg.manual_percent=1;assert(!orbit48_fan_configure(&p,&cfg));
    puts("PASS: hottest CPU/RSX, automatic limits and hysteresis, manual thermal override, controller conflict, startup pause, sensor failure and config bounds");
}
void raw(){
    auto m=new Orbit48RawMap{};m->device_id=0x10300000000000aull;m->device_sectors=UINT64_C(0x1000000000);
    m->partition_lba=2048;m->sector_size=512;m->iso_size=8192;m->count=3;
    m->extents[0]={0,UINT64_C(0x100001234),5};
    m->extents[1]={2560,UINT64_C(0x100009876),3};
    m->extents[2]={4096,UINT64_C(0x100000080),8};
    assert(orbit48_validate_raw_map(m));
    struct Source {bool fail=false;std::size_t calls=0;};Source source;
    auto read=[](void* ptr,std::uint64_t lba,std::uint32_t count,void* out)->int{
        auto& s=*static_cast<Source*>(ptr);++s.calls;if(s.fail)return 0;
        auto* p=static_cast<unsigned char*>(out);
        for(unsigned i=0;i<count*512;++i)p[i]=static_cast<unsigned char>(((lba*512+i)*7+13)&255);
        return 1;
    };
    unsigned char scratch[1024],out[10000];
    for(unsigned offset=0;offset<8192;offset+=137){
        const unsigned n=std::min(723u,8192-offset);
        assert(orbit48_raw_read(m,read,&source,offset,out,n,scratch,sizeof(scratch)));
        for(unsigned i=0;i<n;++i){
            const auto pos=offset+i;unsigned part=0;while(part+1<m->count && m->extents[part+1].iso_offset<=pos)++part;
            const auto absolute=m->extents[part].device_lba*512+pos-m->extents[part].iso_offset;
            assert(out[i]==static_cast<unsigned char>((absolute*7+13)&255));
        }
    }
    assert(orbit48_raw_read(m,read,&source,8100,out,300,scratch,sizeof(scratch)));
    for(unsigned i=92;i<300;++i)assert(out[i]==0);
    assert(!orbit48_raw_read(m,read,&source,UINT64_MAX-10,out,300,scratch,sizeof(scratch)));
    source.fail=true;assert(!orbit48_raw_read(m,read,&source,0,out,512,scratch,sizeof(scratch)));
    m->extents[1].iso_offset=3000;assert(!orbit48_validate_raw_map(m));m->extents[1].iso_offset=2560;
    m->extents[0].device_lba=UINT64_MAX-1;assert(!orbit48_validate_raw_map(m));delete m;
    puts("PASS: resident raw reader, fragmented LBAs above 2 TiB, extent boundaries and arbitrary alignment, zero tail, USB failure and arithmetic bounds");
}
void malformed_ntfs(){
    std::vector<OrbitNtfs48::Run> out;
    const unsigned char runs[]={0x11,2,100,0x11,1,0xfe,0};
    assert(OrbitNtfs48::decode_runs(runs,sizeof(runs),0,1000,out));
    assert(out.size()==2 && out[0].lcn==100 && out[1].lcn==98 && out[1].vcn==2);
    const unsigned char sparse[]={0x01,2,0};
    assert(!OrbitNtfs48::decode_runs(sparse,sizeof(sparse),0,1000,out));
    const unsigned char corrupt[]={0x99,0,0};
    assert(!OrbitNtfs48::decode_runs(corrupt,sizeof(corrupt),0,1000,out));
    std::mt19937 random(48);
    for(unsigned i=0;i<5000;++i){
        std::vector<unsigned char> b(32);for(auto& c:b)c=static_cast<unsigned char>(random());
        OrbitNtfs48::decode_runs(b.data(),b.size(),0,UINT64_C(0x100000000),out);
        OrbitIso48::sfo(b);
    }
    std::vector<unsigned char> record(1024);std::memcpy(record.data(),"FILE",4);
    record[4]=48;record[6]=3;record[48]=0xaa;record[49]=0xbb;
    record[510]=record[1022]=0xaa;record[511]=record[1023]=0xbb;
    record[50]=1;record[52]=2;assert(OrbitNtfs48::apply_fixups(record,512,"FILE"));
    assert(record[510]==1 && record[1022]==2);record[510]=3;
    assert(!OrbitNtfs48::apply_fixups(record,512,"FILE"));
    puts("PASS: signed NTFS run offsets, sparse/corrupt rejection, USA validation and 5000 malformed metadata samples");
}
}
int main(){fan();raw();malformed_ntfs();partition_tables();}
