#include "orbit_service_client_fix48.h"
#include "iso_metadata_fix48.h"
#include <atomic>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
using Bytes=std::vector<unsigned char>;
static void delay(unsigned n){std::this_thread::sleep_for(std::chrono::milliseconds(n));}
static Bytes read(const std::string& p){std::ifstream f(p,std::ios::binary);return Bytes(std::istreambuf_iterator<char>(f),{});}
static void write(const std::string& p,const Bytes& b){const auto temp=p+".server";{std::ofstream f(temp,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));}assert(!std::rename(temp.c_str(),p.c_str()));}
static Bytes response(const Bytes& b,unsigned error=0){
    Bytes out(24+44,0);std::copy_n(b.data(),24,out.data());out[6]|=0x80;orbit48_put32(out.data()+16,44);orbit48_put32(out.data()+20,error);
    const auto c=OrbitService48::defaults();orbit48_put32(out.data()+24,c.mode);orbit48_put32(out.data()+28,c.target_c);orbit48_put32(out.data()+32,c.minimum_percent);orbit48_put32(out.data()+36,c.maximum_percent);orbit48_put32(out.data()+40,c.manual_percent);orbit48_put32(out.data()+48,65);orbit48_put32(out.data()+52,67);orbit48_put32(out.data()+56,40);return out;
}
int main(int argc,char** argv){
    assert(argc==3);const std::string work=argv[1];const auto fixture=read(argv[2]);assert(fixture.size()==131072);
    const auto first=work+"/Game.iso.0",second=work+"/Game.iso.1",third=work+"/Game.iso.2";
    write(first,Bytes(fixture.begin(),fixture.begin()+65536));write(second,Bytes(fixture.begin()+65536,fixture.end()));
    std::vector<std::string> parts;std::string why;assert(OrbitIso48::native_parts(first,parts,why) && parts.size()==2);
    assert(OrbitIso48::native_metadata(parts).id=="BLES00001");
    assert(!std::rename(second.c_str(),third.c_str()));assert(!OrbitIso48::native_parts(first,parts,why));assert(!std::rename(third.c_str(),second.c_str()));
    const auto incomplete=work+"/Incomplete.iso";write(incomplete,Bytes(512,0));assert(!OrbitIso48::native_parts(incomplete,parts,why));
    GameEntry game;game.path=first;game.format=GameFormat::ISO;game.title_id="BLUS00002";
    assert(OrbitService48::native_payload(game,why).empty());game.title_id="BLES00001";assert(!OrbitService48::native_payload(game,why).empty());
    const auto settings=work+"/config";auto c=OrbitService48::defaults();assert(OrbitService48::save_settings(settings,c));c.mode=2;c.manual_percent=55;assert(OrbitService48::save_settings(settings,c));
    write(settings,Bytes{1,2,3});Orbit48FanConfig recovered{};assert(OrbitService48::load_settings(settings,recovered) && recovered.mode==1);
    std::atomic<bool> stop{false};std::atomic<unsigned> mount_count{0};
    std::thread server([&]{
        while(!stop.load()){
            for(bool mount:{false,true}){
                const std::string input=mount?ORBIT48_MOUNT_REQUEST:ORBIT48_CONTROL_REQUEST,output=mount?ORBIT48_MOUNT_REPLY:ORBIT48_CONTROL_REPLY;
                auto b=read(input);if(b.size()<24)continue;std::remove(input.c_str());auto out=response(b);
                if(orbit48_be64(b.data()+8)==100){auto stale=out;orbit48_put64(stale.data()+8,99);write(output,stale);delay(35);auto malformed=out;orbit48_put32(malformed.data()+16,100);write(output,malformed);delay(35);}
                if(mount){++mount_count;for(unsigned i=0;i<100 && read(ORBIT48_CANCEL_PATH).size()!=8;++i)delay(2);
                    assert(orbit48_be64(read(ORBIT48_CANCEL_PATH).data())==orbit48_be64(b.data()+8));orbit48_put32(out.data()+20,ORBIT48_CANCELLED);
                }else if(b[7]==ORBIT48_CONFIGURE && b.size()==44)std::copy(b.begin()+24,b.end(),out.begin()+24);
                write(output,out);
            }delay(2);
        }
    });
    std::uint32_t error=0;Bytes out;
    assert(OrbitService48::exchange(false,ORBIT48_STATUS,{},100,error,out,{},500) && !error && out.size()==44);
    assert(OrbitService48::exchange(true,ORBIT48_MOUNT_NATIVE,{},101,error,out,[]{return true;},500) && error==ORBIT48_CANCELLED && mount_count==1);
    assert(!OrbitService48::exchange(false,ORBIT48_STATUS,Bytes(ORBIT48_MAX_PAYLOAD+1),102,error,out,{},500) && error==ORBIT48_BAD_REQUEST);
    OrbitServiceClientFix48 client;c.manual_percent=57;assert(client.configure(c)==1);
    assert(client.start());OrbitService48::Snapshot snapshot;bool ready=false;
    for(unsigned i=0;i<500;++i){if(client.take(snapshot) && snapshot.ready){ready=true;break;}delay(2);}
    assert(ready && snapshot.status.config.manual_percent==57);
    std::uint64_t revision=0;for(unsigned i=0;i<25;++i){c.manual_percent=35+i;revision=client.configure(c);}
    bool applied=false;for(unsigned i=0;i<1000;++i){if(client.take(snapshot) && snapshot.revision>=revision && snapshot.status.config.manual_percent==59){applied=true;break;}delay(2);}assert(applied);
    c.manual_percent=70;client.configure(c);client.stop();
    Orbit48FanConfig final_config{};assert(OrbitService48::load_settings(ORBIT48_SETTINGS_PATH,final_config));assert(final_config.manual_percent==70);
    stop.store(true);server.join();
    assert(!OrbitService48::exchange(false,ORBIT48_STATUS,{},999,error,out,{},75) && error==ORBIT48_BUSY);
    puts("PASS: native split ISO bytes/metadata, missing part rejection, ID mismatch and configuration backup recovery");
    puts("PASS: actual mailbox client ignores stale/malformed replies; cancel token, bounded payload, timeout, 25 rapid config revisions and last setting saved on immediate shutdown");
}
