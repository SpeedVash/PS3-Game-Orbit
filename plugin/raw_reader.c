#include "../include/raw_reader_fix48.h"
static void copy_bytes(unsigned char* out,const unsigned char* in,uint32_t n){while(n--)*out++=*in++;}
static void zero_bytes(unsigned char* out,uint32_t n){while(n--)*out++=0;}
int orbit48_validate_raw_map(const Orbit48RawMap* m){
    if(!m || !m->count || m->count>ORBIT48_MAX_EXTENTS || m->sector_size<512 ||
       m->sector_size>4096 || (m->sector_size&(m->sector_size-1)) ||
       !m->iso_size || m->iso_size%2048 || m->partition_lba>=m->device_sectors ||
       m->extents[0].iso_offset)return 0;
    for(uint32_t i=0;i<m->count;++i){
        const Orbit48Extent* e=&m->extents[i];
        if(!e->sector_count || e->device_lba<m->partition_lba ||
           e->device_lba>=m->device_sectors || e->sector_count>m->device_sectors-e->device_lba ||
           e->iso_offset>=m->iso_size || e->sector_count>UINT64_MAX/m->sector_size)return 0;
        const uint64_t capacity=e->sector_count*m->sector_size;
        const uint64_t limit=i+1<m->count?m->extents[i+1].iso_offset:m->iso_size;
        if(limit<=e->iso_offset || limit-e->iso_offset>capacity)return 0;
    }return 1;
}
int orbit48_raw_read(const Orbit48RawMap* m,Orbit48SectorRead read,void* context,
    uint64_t offset,void* output,uint32_t size,void* scratch,uint32_t scratch_size){
    if(!read || !output || !scratch || !m || !m->count || scratch_size<m->sector_size ||
       size>256u*1024u || offset>UINT64_MAX-size)return 0;
    unsigned char* out=(unsigned char*)output;
    if(offset>=m->iso_size){zero_bytes(out,size);return 1;}
    const uint32_t real=(uint32_t)((m->iso_size-offset)<size?(m->iso_size-offset):size);
    if(real<size)zero_bytes(out+real,size-real);
    size=real;
    uint32_t index=0;
    while(index+1<m->count && m->extents[index+1].iso_offset<=offset)++index;
    while(size){
        if(index>=m->count)return 0;
        const Orbit48Extent* e=&m->extents[index];
        const uint64_t limit=index+1<m->count?m->extents[index+1].iso_offset:m->iso_size;
        if(offset<e->iso_offset || offset>=limit)return 0;
        const uint64_t delta=offset-e->iso_offset;
        const uint32_t skip=(uint32_t)(delta%m->sector_size);
        uint32_t amount=size;
        if(amount>scratch_size-skip)amount=scratch_size-skip;
        if((uint64_t)amount>limit-offset)amount=(uint32_t)(limit-offset);
        const uint32_t count=(amount+skip+m->sector_size-1)/m->sector_size;
        const uint64_t sector=delta/m->sector_size;
        if(sector>=e->sector_count || count>e->sector_count-sector ||
           !read(context,e->device_lba+sector,count,scratch))return 0;
        copy_bytes(out,(const unsigned char*)scratch+skip,amount);
        out+=amount;offset+=amount;size-=amount;
        if(offset==limit)++index;
    }return 1;
}
