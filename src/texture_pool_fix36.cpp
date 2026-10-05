#include "rsx_stage1.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include <cstdlib>
#ifdef __PSL1GHT__
#include <rsx/rsx.h>
#endif
void RsxStage1::clear_texture_pool(){
    for(const auto& block:texture_pool_){
#ifdef __PSL1GHT__
        rsxFree(block.ptr);
#else
        std::free(block.ptr);
#endif
    }
    texture_pool_.clear();pooled_texture_bytes_=0;
}
void* RsxStage1::acquire_texture_buffer(std::size_t bytes,std::size_t& allocation_bytes){
    auto best=texture_pool_.end();
    for(auto it=texture_pool_.begin();it!=texture_pool_.end();++it)if(it->bytes>=bytes && (best==texture_pool_.end() || it->bytes<best->bytes))best=it;
    if(best!=texture_pool_.end()){const auto block=*best;pooled_texture_bytes_-=block.bytes;texture_pool_.erase(best);allocation_bytes=block.bytes;++texture_buffer_reuses_;return block.ptr;}
    const auto allocate=[&](){
#ifdef __PSL1GHT__
        return rsxMemalign(128,static_cast<u32>(bytes));
#else
        void* ptr=nullptr;return posix_memalign(&ptr,128,bytes)==0?ptr:nullptr;
#endif
    };
    auto* ptr=allocate();if(!ptr && !texture_pool_.empty()){clear_texture_pool();ptr=allocate();}
    if(ptr){allocation_bytes=bytes;++texture_buffer_allocations_;}return ptr;
}
#endif
