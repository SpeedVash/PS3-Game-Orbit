#include "ntfs_reader_fix48.h"
#include "ntfs_catalog_fix48.h"
#include "raw_reader_fix48.h"
#include <cstdio>
#include <cassert>
#include <vector>
#include <cstring>
int main(int argc,char** argv){
    assert(argc==3);FILE* f=fopen(argv[1],"rb");assert(f);fseeko(f,0,SEEK_END);const auto size=ftello(f);rewind(f);
    auto read=[&](std::uint64_t sector,std::uint32_t n,void* out){
        return fseeko(f,off_t(sector*512),SEEK_SET)==0 && fread(out,512,n,f)==n;
    };
    OrbitNtfs48::Volume volume(read,512,std::uint64_t(size/512),{0,std::uint64_t(size/512)});
    if(!volume.open()){puts(volume.error().c_str());return 2;}
    std::uint64_t ref=0;if(!volume.find("/PS3ISO/BLES00001_fixture.iso",ref)){puts(volume.error().c_str());return 3;}
    OrbitNtfs48::Stream file;if(!volume.file(ref,file)){puts(volume.error().c_str());return 4;}
    std::vector<unsigned char> bytes(std::size_t(file.size));if(!volume.read(file,0,bytes.data(),bytes.size()))return 5;
    FILE* iso=fopen(argv[2],"rb");assert(iso);std::vector<unsigned char> expected(bytes.size());assert(fread(expected.data(),1,expected.size(),iso)==expected.size());fclose(iso);
    assert(bytes==expected);
    auto map=new Orbit48RawMap{};map->device_id=0x10300000000000aull;assert(volume.append_map(file,0,*map));
    assert(orbit48_validate_raw_map(map));
    unsigned char scratch[65536];std::vector<unsigned char> copy(bytes.size());
    struct Context {decltype(read)* reader;};Context context{&read};
    auto raw=[](void* c,std::uint64_t lba,std::uint32_t n,void* out)->int{return (*static_cast<Context*>(c)->reader)(lba,n,out);};
    for(std::size_t i=0;i<copy.size();i+=7777){
        const auto n=std::min<std::size_t>(7777,copy.size()-i);assert(orbit48_raw_read(map,raw,&context,i,copy.data()+i,unsigned(n),scratch,sizeof(scratch)));
    }assert(copy==expected);
    std::vector<GameEntry> games;OrbitNtfsCatalog48::scan_volume(volume,0,0,map->device_id,games);
    if(games.size()!=1){puts(volume.error().c_str());return 6;}
    assert(games[0].title_id=="BLES00001");assert(games[0].title=="Orbit fixture");
    std::uint64_t dir=0;assert(volume.find("/PS3ISO",dir));std::vector<OrbitNtfs48::Entry> entries;assert(volume.list(dir,entries));assert(entries.size()==121);
    puts("PASS: real NTFS-3G volume, allocated directory index with 121 entries, ISO metadata, all raw-map bytes including unaligned reads");delete map;fclose(f);
}
