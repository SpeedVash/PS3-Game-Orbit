#pragma once
#include "orbit_service_protocol_fix48.h"
#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace OrbitNtfs48 {
using SectorReader=std::function<bool(std::uint64_t,std::uint32_t,void*)>;
struct Partition {std::uint64_t start=0,sectors=0;};
struct Run {std::uint64_t vcn=0,lcn=0,clusters=0;};
struct Stream {
    std::vector<unsigned char> resident;
    std::vector<Run> runs;
    std::uint64_t size=0,initialized=0;
    bool nonresident=false;
};
struct Entry {std::string name;std::uint64_t reference=0;bool directory=false;};
bool decode_runs(const unsigned char* p,std::size_t n,std::uint64_t first_vcn,
    std::uint64_t volume_clusters,std::vector<Run>& out);
bool apply_fixups(std::vector<unsigned char>& bytes,std::uint32_t sector_size,const char* signature);
std::vector<Partition> partitions(SectorReader read,std::uint32_t sector_size,std::uint64_t sectors);
class Volume {
public:
    Volume(SectorReader read,std::uint32_t sector_size,std::uint64_t device_sectors,Partition part);
    bool open();
    bool list(std::uint64_t reference,std::vector<Entry>& entries);
    bool find(const std::string& path,std::uint64_t& reference);
    bool file(std::uint64_t reference,Stream& stream);
    bool read(const Stream& stream,std::uint64_t offset,void* out,std::size_t bytes);
    bool append_map(const Stream& stream,std::uint64_t base,Orbit48RawMap& map);
    std::uint64_t serial() const{return serial_;}
    std::uint64_t device_sectors() const{return device_sectors_;}
    std::uint32_t sector_size() const{return sector_size_;}
    std::uint64_t partition_lba() const{return part_.start;}
    const std::string& error() const{return error_;}
private:
    bool bytes(std::uint64_t offset,void* out,std::size_t n);
    bool record(std::uint64_t reference,std::vector<unsigned char>& out);
    bool attribute_stream(std::uint64_t reference,const std::vector<unsigned char>& rec,
        std::uint32_t type,const std::string& name,Stream& stream,bool extensions=true);
    bool index_entries(const unsigned char* p,std::size_t size,std::size_t header,
        std::vector<Entry>& entries);
    bool fail(const char* text){error_=text;return false;}
    SectorReader reader_;std::uint32_t sector_size_,cluster_size_=0,record_size_=0,index_size_=0;
    std::uint64_t device_sectors_,serial_=0,volume_bytes_=0;Partition part_;Stream mft_;
    std::string error_;
};
bool equal_ascii_ci(const std::string& a,const std::string& b);
}
