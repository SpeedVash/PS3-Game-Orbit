#include "ntfs_reader_fix48.h"
#ifdef PS3_GAME_ORBIT_FIX48
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <unordered_set>

namespace OrbitNtfs48 {
namespace {
using Bytes=std::vector<unsigned char>;
constexpr std::uint64_t RefMask=0x0000ffffffffffffull;
std::uint16_t le16(const unsigned char* p){return std::uint16_t(p[0]|(unsigned(p[1])<<8));}
std::uint32_t le32(const unsigned char* p){return std::uint32_t(le16(p))|(std::uint32_t(le16(p+2))<<16);}
std::uint64_t le64(const unsigned char* p){return std::uint64_t(le32(p))|(std::uint64_t(le32(p+4))<<32);}
bool range(std::size_t off,std::size_t size,std::size_t limit){return off<=limit && size<=limit-off;}
bool power2(std::uint32_t n){return n && !(n&(n-1));}
bool add(std::uint64_t a,std::uint64_t b,std::uint64_t& out){out=a+b;return out>=a;}
bool multiply(std::uint64_t a,std::uint64_t b,std::uint64_t& out){
    if(b && a>std::numeric_limits<std::uint64_t>::max()/b)return false;
    out=a*b;return true;
}
std::uint32_t crc(const unsigned char* p,std::size_t n){
    std::uint32_t c=~0u;for(std::size_t i=0;i<n;++i){c^=p[i];for(int k=0;k<8;++k)c=(c>>1)^((c&1)?0xedb88320u:0);}
    return ~c;
}
bool ntfs_boot(const Bytes& b){return b.size()>=512 && !std::memcmp(b.data()+3,"NTFS    ",8) && b[510]==0x55 && b[511]==0xaa;}
std::string utf16(const unsigned char* p,std::size_t units){
    std::string out;
    for(std::size_t i=0;i<units;++i){
        std::uint32_t c=le16(p+i*2);
        if(c>=0xd800 && c<=0xdbff){
            if(++i>=units)return {};
            const auto low=le16(p+i*2);if(low<0xdc00 || low>0xdfff)return {};
            c=0x10000+((c-0xd800)<<10)+(low-0xdc00);
        }else if(c>=0xdc00 && c<=0xdfff)return {};
        if(c<32 || c=='/' || c=='\\' || !c)return {};
        if(c<0x80)out+=char(c);
        else if(c<0x800){out+=char(0xc0|(c>>6));out+=char(0x80|(c&63));}
        else if(c<0x10000){out+=char(0xe0|(c>>12));out+=char(0x80|((c>>6)&63));out+=char(0x80|(c&63));}
        else{out+=char(0xf0|(c>>18));out+=char(0x80|((c>>12)&63));out+=char(0x80|((c>>6)&63));out+=char(0x80|(c&63));}
    }
    return out;
}
struct Attribute {const unsigned char* p=nullptr;std::size_t size=0;std::uint32_t type=0;std::string name;};
bool attributes(const Bytes& record,std::vector<Attribute>& out){
    if(record.size()<48)return false;
    const auto used=le32(record.data()+24);if(used>record.size() || used<48)return false;
    std::size_t at=le16(record.data()+20);
    if(at<48 || at%8)return false;
    while(range(at,4,used)){
        const auto type=le32(record.data()+at);if(type==0xffffffff)return true;
        if(!range(at,24,used))return false;
        const auto* p=record.data()+at;const auto n=le32(p+4);
        if(n<24 || n%8 || !range(at,n,used) || p[8]>1)return false;
        const std::size_t no=le16(p+10),nl=p[9];
        if(nl && (no<16 || !range(no,nl*2,n)))return false;
        const auto name=nl?utf16(p+no,nl):std::string{};
        if(nl && name.empty())return false;
        out.push_back({p,n,type,name});at+=n;
        if(out.size()>4096)return false;
    }
    return false;
}
bool append_attribute(const Attribute& a,std::uint32_t cluster,std::uint64_t clusters,Stream& s){
    const auto* p=a.p;
    if(le16(p+12)&0xc001)return false; // compressed, EFS or sparse: never expose raw garbage.
    if(!p[8]){
        if(s.nonresident || !s.resident.empty() || a.size<24)return false;
        const auto n=le32(p+16);const auto off=le16(p+20);if(off<24 || !range(off,n,a.size))return false;
        s.resident.assign(p+off,p+off+n);s.size=s.initialized=n;return true;
    }
    if(a.size<64 || !s.resident.empty())return false;
    const auto vcn=le64(p+16),last=le64(p+24);const auto offset=le16(p+32);
    if(offset<64 || offset>=a.size || last<vcn || last==std::numeric_limits<std::uint64_t>::max())return false;
    if(!vcn){s.size=le64(p+48);s.initialized=le64(p+56);if(s.initialized>s.size)return false;}
    std::vector<Run> runs;
    if(!decode_runs(p+offset,a.size-offset,vcn,clusters,runs) || runs.empty())return false;
    if(runs.back().vcn+runs.back().clusters!=last+1)return false;
    std::uint64_t bound=0;if(!multiply(last+1,cluster,bound))return false;
    s.nonresident=true;s.runs.insert(s.runs.end(),runs.begin(),runs.end());return true;
}
}
bool equal_ascii_ci(const std::string& a,const std::string& b){
    if(a.size()!=b.size())return false;
    for(std::size_t i=0;i<a.size();++i){
        unsigned char x=a[i],y=b[i];if(x>='a' && x<='z')x-=32;if(y>='a' && y<='z')y-=32;if(x!=y)return false;
    }return true;
}
bool decode_runs(const unsigned char* p,std::size_t n,std::uint64_t vcn,std::uint64_t cap,std::vector<Run>& out){
    std::size_t at=0;std::int64_t lcn=0;out.clear();
    while(at<n){
        const unsigned h=p[at++];if(!h)return !out.empty();
        const unsigned lb=h&15,ob=h>>4;
        if(!lb || lb>8 || !ob || ob>8 || !range(at,lb+ob,n))return false;
        std::uint64_t count=0,delta=0;for(unsigned k=0;k<lb;++k)count|=std::uint64_t(p[at+k])<<(k*8);
        at+=lb;for(unsigned k=0;k<ob;++k)delta|=std::uint64_t(p[at+k])<<(k*8);
        if(ob<8 && (p[at+ob-1]&0x80))delta|=(~std::uint64_t(0))<<(ob*8);
        at+=ob;const auto signed_delta=static_cast<std::int64_t>(delta);
        if((signed_delta>0 && lcn>std::numeric_limits<std::int64_t>::max()-signed_delta) ||
           (signed_delta<0 && lcn<std::numeric_limits<std::int64_t>::min()-signed_delta))return false;
        lcn+=signed_delta;
        if(lcn<0 || !count || std::uint64_t(lcn)>=cap || count>cap-std::uint64_t(lcn) ||
           count>std::numeric_limits<std::uint64_t>::max()-vcn || out.size()>=ORBIT48_MAX_EXTENTS)return false;
        out.push_back({vcn,std::uint64_t(lcn),count});vcn+=count;
    }return false;
}
bool apply_fixups(Bytes& b,std::uint32_t sector,const char* signature){
    if(b.size()<8 || !power2(sector) || sector<512 || b.size()%sector || std::memcmp(b.data(),signature,4))return false;
    const auto off=le16(b.data()+4),count=le16(b.data()+6);
    if(count!=b.size()/sector+1 || off<8 || !range(off,std::size_t(count)*2,b.size()))return false;
    const auto usn=le16(b.data()+off);
    for(std::size_t i=1;i<count;++i)if(le16(b.data()+i*sector-2)!=usn)return false;
    for(std::size_t i=1;i<count;++i){b[i*sector-2]=b[off+i*2];b[i*sector-1]=b[off+i*2+1];}
    return true;
}
std::vector<Partition> partitions(SectorReader read,std::uint32_t sector,std::uint64_t cap){
    std::vector<Partition> out;Bytes boot(sector);
    if(!power2(sector) || sector<512 || sector>4096 || !cap || !read(0,1,boot.data()))return out;
    if(ntfs_boot(boot)){out.push_back({0,cap});return out;}
    if(boot[510]!=0x55 || boot[511]!=0xaa)return out;
    auto insert=[&](std::uint64_t start,std::uint64_t count){
        if(start>=cap || !count || count>cap-start)return;
        if(std::none_of(out.begin(),out.end(),[&](const Partition& p){return p.start==start;}))out.push_back({start,count});
    };
    bool gpt=false;
    for(unsigned i=0;i<4;++i){
        const auto* p=boot.data()+446+i*16;const auto start=le32(p+8),count=le32(p+12);
        if(p[4]==0xee){gpt=true;continue;}
        if(p[4]==7)insert(start,count);
        if(p[4]==5 || p[4]==0x0f || p[4]==0x85){
            const std::uint64_t base=start;std::uint64_t current=base;std::unordered_set<std::uint64_t> visited;
            for(unsigned depth=0;depth<128 && current<cap && visited.insert(current).second;++depth){
                Bytes ebr(sector);if(!read(current,1,ebr.data()) || ebr[510]!=0x55 || ebr[511]!=0xaa)break;
                const auto* first=ebr.data()+446;std::uint64_t pos=0;
                if(first[4]==7 && add(current,le32(first+8),pos))insert(pos,le32(first+12));
                const auto* next=first+16;if(!next[4] || !le32(next+8) || !add(base,le32(next+8),current))break;
            }
        }
    }
    if(gpt){
        Bytes h(sector);if(cap<2 || !read(1,1,h.data()) || std::memcmp(h.data(),"EFI PART",8))return {};
        const auto n=le32(h.data()+12),checksum=le32(h.data()+16);
        if(n<92 || n>sector)return {};
        std::fill(h.begin()+16,h.begin()+20,0);if(crc(h.data(),n)!=checksum)return {};
        const auto table=le64(h.data()+72);const auto count=le32(h.data()+80),entrysize=le32(h.data()+84);
        if(!count || count>4096 || entrysize<128 || entrysize>512 || entrysize%8)return {};
        const std::uint64_t bytes=std::uint64_t(count)*entrysize,sectors=(bytes+sector-1)/sector;
        if(table>=cap || sectors>cap-table)return {};
        Bytes entries(std::size_t(sectors*sector));if(!read(table,std::uint32_t(sectors),entries.data()))return {};
        if(crc(entries.data(),std::size_t(bytes))!=le32(h.data()+88))return {};
        out.clear();
        for(unsigned i=0;i<count;++i){
            const auto* p=entries.data()+std::size_t(i)*entrysize;bool empty=true;
            for(unsigned k=0;k<16;++k)if(p[k])empty=false;
            if(empty)continue;
            const auto first=le64(p+32),last=le64(p+40);if(last>=first && last<cap)insert(first,last-first+1);
        }
    }return out;
}
Volume::Volume(SectorReader read,std::uint32_t sector,std::uint64_t sectors,Partition part):
    reader_(std::move(read)),sector_size_(sector),device_sectors_(sectors),part_(part){}
bool Volume::bytes(std::uint64_t offset,void* out,std::size_t n){
    if(offset>volume_bytes_ || n>volume_bytes_-offset)return fail("NTFS: leitura fora do volume.");
    auto* dst=static_cast<unsigned char*>(out);Bytes buffer(64*1024);
    while(n){
        const auto relative=offset/sector_size_;const auto skip=std::size_t(offset%sector_size_);
        const auto amount=std::min(n,buffer.size()-skip);
        const auto sectors=std::uint32_t((amount+skip+sector_size_-1)/sector_size_);
        if(relative>=part_.sectors || sectors>part_.sectors-relative || !reader_(part_.start+relative,sectors,buffer.data()))
            return fail("NTFS: USB indisponível ou leitura incompleta.");
        std::memcpy(dst,buffer.data()+skip,amount);dst+=amount;offset+=amount;n-=amount;
    }return true;
}
bool Volume::open(){
    if(!power2(sector_size_) || sector_size_<512 || sector_size_>4096 || !part_.sectors ||
       part_.start>=device_sectors_ || part_.sectors>device_sectors_-part_.start ||
       !multiply(part_.sectors,sector_size_,volume_bytes_))return fail("NTFS: partição inválida.");
    Bytes b(sector_size_);if(!reader_(part_.start,1,b.data()) || !ntfs_boot(b))return fail("USB sem volume NTFS.");
    const auto spc=b[13];
    if(le16(b.data()+11)!=sector_size_ || !power2(spc) || spc>128 || le64(b.data()+40)>part_.sectors)
        return fail("NTFS: geometria não suportada.");
    cluster_size_=sector_size_*spc;serial_=le64(b.data()+72);
    const auto mft_lcn=le64(b.data()+48);
    const auto signed_record=static_cast<std::int8_t>(b[64]),signed_index=static_cast<std::int8_t>(b[68]);
    auto block_size=[&](int v)->std::uint32_t{
        if(v<0)return -v<20?std::uint32_t(1)<<(-v):0;
        return v>0 && std::uint64_t(v)*cluster_size_<=65536?std::uint32_t(v)*cluster_size_:0;
    };
    record_size_=block_size(signed_record);index_size_=block_size(signed_index);
    if(record_size_<sector_size_ || record_size_>65536 || !power2(record_size_) ||
       index_size_<sector_size_ || index_size_>65536 || !power2(index_size_) ||
       mft_lcn>=volume_bytes_/cluster_size_)return fail("NTFS: tamanho dos registros inválido.");
    Bytes first(record_size_);std::uint64_t offset=0;
    if(!multiply(mft_lcn,cluster_size_,offset) || !bytes(offset,first.data(),first.size()) ||
       !apply_fixups(first,sector_size_,"FILE") || !(le16(first.data()+22)&1))return fail("NTFS: MFT inválida.");
    if(!attribute_stream(0,first,0x80,"",mft_,false) || !mft_.nonresident)return fail("NTFS: MFT sem mapa de dados.");
    Stream complete;if(!attribute_stream(0,first,0x80,"",complete,true))return false;mft_=std::move(complete);
    return true;
}
bool Volume::read(const Stream& s,std::uint64_t offset,void* out,std::size_t n){
    if(offset>s.size || n>s.size-offset)return fail("NTFS: leitura além do arquivo.");
    if(!s.nonresident){
        if(s.resident.size()!=s.size)return fail("NTFS: atributo residente inválido.");
        if(n)std::memcpy(out,s.resident.data()+std::size_t(offset),n);
        return true;
    }
    auto* dst=static_cast<unsigned char*>(out);
    while(n){
        if(offset>=s.initialized){std::memset(dst,0,n);return true;}
        const auto vcn=offset/cluster_size_;const auto it=std::find_if(s.runs.begin(),s.runs.end(),
            [&](const Run& r){return vcn>=r.vcn && vcn-r.vcn<r.clusters;});
        if(it==s.runs.end())return fail("NTFS: mapa incompleto.");
        std::uint64_t start=0,available=0;if(!multiply(it->lcn,cluster_size_,start) ||
           !multiply(it->clusters,cluster_size_,available))return fail("NTFS: endereço inválido.");
        const auto within=offset-it->vcn*cluster_size_;if(within>=available)return false;
        const auto count=std::size_t(std::min<std::uint64_t>(n,std::min(available-within,s.initialized-offset)));
        if(!add(start,within,start) || !bytes(start,dst,count))return false;
        dst+=count;offset+=count;n-=count;
    }return true;
}
bool Volume::record(std::uint64_t ref,Bytes& out){
    const auto index=ref&RefMask;std::uint64_t offset=0;
    if(!multiply(index,record_size_,offset) || offset>=mft_.initialized || record_size_>mft_.initialized-offset)
        return fail("NTFS: referência fora da MFT.");
    out.resize(record_size_);
    if(!read(mft_,offset,out.data(),out.size()) || !apply_fixups(out,sector_size_,"FILE") || !(le16(out.data()+22)&1))
        return fail("NTFS: registro inválido.");
    const auto seq=std::uint16_t(ref>>48);if(seq && seq!=le16(out.data()+16))return fail("NTFS: referência antiga.");
    return true;
}
bool Volume::attribute_stream(std::uint64_t ref,const Bytes& rec,std::uint32_t type,
    const std::string& name,Stream& stream,bool extensions){
    stream={};std::vector<Attribute> attrs;if(!attributes(rec,attrs))return fail("NTFS: atributos inválidos.");
    bool found=false;
    for(const auto& a:attrs)if(a.type==type && a.name==name){
        if(!append_attribute(a,cluster_size_,volume_bytes_/cluster_size_,stream))return fail("NTFS: atributo comprimido, esparso, cifrado ou inválido.");
        found=true;
    }
    if(extensions){
        Stream list;bool has_list=false;
        for(const auto& a:attrs)if(a.type==0x20){
            if(!append_attribute(a,cluster_size_,volume_bytes_/cluster_size_,list))return fail("NTFS: lista de atributos inválida.");
            has_list=true;
        }
        if(has_list){
            if(list.size>1024*1024)return fail("NTFS: lista de atributos excede o limite.");
            Bytes b(std::size_t(list.size));if(!read(list,0,b.data(),b.size()))return false;
            std::unordered_set<std::uint64_t> seen;seen.insert(ref&RefMask);
            for(std::size_t at=0;at<b.size();){
                if(!range(at,26,b.size()))return fail("NTFS: lista truncada.");
                const auto* p=b.data()+at;const auto length=le16(p+4);
                if(length<26 || !range(at,length,b.size()) || !range(p[7],std::size_t(p[6])*2,length))return fail("NTFS: entrada de atributo inválida.");
                const auto target=le64(p+16),reference=target&RefMask;
                const auto attribute_name=p[6]?utf16(p+p[7],p[6]):std::string{};
                if(le32(p)==type && attribute_name==name && seen.insert(reference).second){
                    if(seen.size()>64)return fail("NTFS: arquivo fragmentado excede o limite de registros.");
                    Bytes extra;if(!record(target,extra))return false;
                    if((le64(extra.data()+32)&RefMask)!=(ref&RefMask))return fail("NTFS: extensão de outro arquivo.");
                    std::vector<Attribute> more;if(!attributes(extra,more))return fail("NTFS: extensão inválida.");
                    for(const auto& a:more)if(a.type==type && a.name==name){
                        if(!append_attribute(a,cluster_size_,volume_bytes_/cluster_size_,stream))return fail("NTFS: extensão não suportada.");
                        found=true;
                    }
                }at+=length;
            }
        }
    }
    if(!found)return fail("NTFS: atributo não encontrado.");
    if(stream.nonresident){
        std::sort(stream.runs.begin(),stream.runs.end(),[](const Run& a,const Run& b){return a.vcn<b.vcn;});
        std::uint64_t end=0;
        for(const auto& r:stream.runs){if(r.vcn!=end || !add(end,r.clusters,end))return fail("NTFS: segmentos sobrepostos ou ausentes.");}
        std::uint64_t total=0;if(!multiply(end,cluster_size_,total) || (extensions && total<stream.size))return fail("NTFS: mapa menor que o arquivo.");
    }return true;
}
bool Volume::index_entries(const unsigned char* p,std::size_t n,std::size_t header,std::vector<Entry>& entries){
    if(!range(header,16,n))return fail("NTFS: índice truncado.");
    const auto begin=le32(p+header),used=le32(p+header+4),allocated=le32(p+header+8);
    if(begin<16 || used<begin || used>allocated || !range(header,used,n))return fail("NTFS: limites do índice inválidos.");
    std::size_t at=header+begin,end=header+used;
    while(range(at,16,end)){
        const auto* e=p+at;const auto length=le16(e+8),key=le16(e+10),flags=le16(e+12);
        if(length<16 || length%8 || !range(at,length,end))return fail("NTFS: entrada de índice inválida.");
        if(flags&2)return true;
        if(key<66 || !range(16,key,length) || (flags&1 && length<24))return fail("NTFS: nome de arquivo inválido.");
        const auto* f=e+16;const auto units=f[64],space=f[65];
        if(!range(66,std::size_t(units)*2,key))return fail("NTFS: nome truncado.");
        if(space!=2){
            const auto name=utf16(f+66,units);
            if(!name.empty() && name!="." && name!=".."){
                if(entries.size()>=16384)return fail("NTFS: diretório excede 16384 entradas.");
                entries.push_back({name,le64(e),(le32(f+56)&0x10000000)!=0});
            }
        }at+=length;
    }return fail("NTFS: índice sem terminador.");
}
bool Volume::list(std::uint64_t ref,std::vector<Entry>& entries){
    entries.clear();Bytes rec;if(!record(ref,rec) || !(le16(rec.data()+22)&2))return fail("NTFS: não é uma pasta.");
    Stream root;if(!attribute_stream(ref,rec,0x90,"$I30",root) || root.nonresident || root.size<32)return fail("NTFS: raiz do índice inválida.");
    if(le32(root.resident.data())!=0x30 || le32(root.resident.data()+8)!=index_size_ ||
       !index_entries(root.resident.data(),root.resident.size(),16,entries))return false;
    if(!(root.resident[28]&1))return true;
    Stream allocation,bitmap;
    if(!attribute_stream(ref,rec,0xa0,"$I30",allocation) ||
       !attribute_stream(ref,rec,0xb0,"$I30",bitmap))return false;
    const auto blocks=allocation.size/index_size_;
    if(allocation.size%index_size_ || blocks>65536 || bitmap.size<(blocks+7)/8 || bitmap.size>1024*1024)
        return fail("NTFS: índice excede o limite.");
    Bytes bits(std::size_t(bitmap.size)),index(index_size_);if(!read(bitmap,0,bits.data(),bits.size()))return false;
    for(std::uint64_t i=0;i<blocks;++i)if(bits[std::size_t(i/8)]&(1u<<(i%8))){
        if(!read(allocation,i*index_size_,index.data(),index.size()) || !apply_fixups(index,sector_size_,"INDX") ||
           !index_entries(index.data(),index.size(),24,entries))return false;
    }
    std::sort(entries.begin(),entries.end(),[](const Entry& a,const Entry& b){return a.name<b.name;});
    entries.erase(std::unique(entries.begin(),entries.end(),[](const Entry& a,const Entry& b){return a.reference==b.reference && equal_ascii_ci(a.name,b.name);}),entries.end());
    return true;
}
bool Volume::find(const std::string& path,std::uint64_t& ref){
    ref=5;std::size_t at=0;unsigned depth=0;
    while(at<path.size()){
        while(at<path.size() && path[at]=='/')++at;
        if(at==path.size())break;
        const auto slash=path.find('/',at);const auto name=path.substr(at,slash==std::string::npos?path.size()-at:slash-at);
        if(name=="." || name==".." || name.size()>1020 || ++depth>16)return fail("NTFS: caminho inválido.");
        std::vector<Entry> entries;if(!list(ref,entries))return false;
        const auto found=std::find_if(entries.begin(),entries.end(),[&](const Entry& e){return equal_ascii_ci(e.name,name);});
        if(found==entries.end())return fail("NTFS: caminho não encontrado.");
        ref=found->reference;at=slash==std::string::npos?path.size():slash+1;
    }return true;
}
bool Volume::file(std::uint64_t ref,Stream& out){
    Bytes rec;if(!record(ref,rec) || (le16(rec.data()+22)&2))return fail("NTFS: não é um arquivo.");
    std::vector<Attribute> attrs;if(!attributes(rec,attrs))return false;
    if(std::any_of(attrs.begin(),attrs.end(),[](const Attribute& a){return a.type==0xc0;}))return fail("NTFS: link/reparse point não suportado.");
    return attribute_stream(ref,rec,0x80,"",out);
}
bool Volume::append_map(const Stream& s,std::uint64_t base,Orbit48RawMap& map){
    if(!s.nonresident || !s.size || s.initialized!=s.size)return fail("NTFS: ISO residente ou incompleta.");
    std::uint64_t final=0;if(!add(base,s.size,final))return fail("NTFS: tamanho inválido.");
    for(const auto& run:s.runs){
        std::uint64_t offset=0,amount=0,lba=0;
        if(!multiply(run.vcn,cluster_size_,offset) || !multiply(run.clusters,cluster_size_,amount) ||
           !multiply(run.lcn,cluster_size_/sector_size_,lba) || !add(lba,part_.start,lba))return false;
        if(offset>=s.size)break;
        amount=std::min(amount,s.size-offset);
        const auto count=(amount+sector_size_-1)/sector_size_;
        if(map.count>=ORBIT48_MAX_EXTENTS || lba>=device_sectors_ || count>device_sectors_-lba ||
           !add(base,offset,offset))return fail("NTFS: fragmentação excede o limite.");
        map.extents[map.count++]={offset,lba,count};
    }
    map.iso_size=final;map.device_sectors=device_sectors_;map.sector_size=sector_size_;
    map.partition_lba=part_.start;map.volume_serial=serial_;return true;
}
}
#endif
