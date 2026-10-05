#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX34
#include "runtime_diag.h"
#include "cover_orientation_fix28.h"
#include <algorithm>
#include <limits>

void RsxRendererV10::clear_inspection_cache(InspectionArtCache& cache) {
    for(auto& kv:cache.entries) if(stage1_ && kv.second.texture.gpu_ptr)
        stage1_->release_cover(kv.second.texture);
    cache={};
}

bool RsxRendererV10::load_cached_inspection_art(InspectionArtCache& cache,int gi,const GameEntry& game,
                                               const std::string& path,GameCoverKind kind) {
    auto found=cache.entries.find(gi);
    if(found!=cache.entries.end()) {
        auto& art=found->second;
        if(art.game_path==game.path && art.path==path && art.texture.uploaded) {
            art.stamp=++cache.clock;++cache.hits;return true;
        }
        cache.bytes-=std::min(cache.bytes,gpu_texture_storage_bytes(art.texture));
        if(art.texture.gpu_ptr) stage1_->release_cover(art.texture);
        cache.entries.erase(found);
    }
    if(path.empty()) return false;
    const auto identity=std::make_pair(game.path,path);
    const auto failed=cache.failed.find(gi);
    if(failed!=cache.failed.end() && failed->second==identity) return false;
    ++cache.misses;
    const auto image=load_cover_file(path,kind);
    if(!image.valid()) {
        cache.failed[gi]=identity;
        RuntimeDiag::log("ART 1.3.1 fallback: game=%d kind=%s path=%s error=Unreadable or unsupported image",gi,
            kind==GameCoverKind::FullCover ? "INSIDE" : "DISC",path.c_str());
        return false;
    }
    // Bound the cache before allocating. Only the selected game has open
    // interior/disc surfaces; main calls this after the preceding frame ACK.
    constexpr std::size_t WorstTextureBytes=1024u*1024u*4u;
    while(cache.entries.size()>=MaxResidentInspectionArt || cache.bytes+WorstTextureBytes>InspectionArtBudgetBytes) {
        auto victim=cache.entries.end();auto oldest=std::numeric_limits<std::uint64_t>::max();
        for(auto it=cache.entries.begin();it!=cache.entries.end();++it)
            if(it->second.stamp<oldest) {victim=it;oldest=it->second.stamp;}
        if(victim==cache.entries.end()) return false;
        auto& art=victim->second;
        cache.bytes-=std::min(cache.bytes,gpu_texture_storage_bytes(art.texture));
        if(art.texture.gpu_ptr) stage1_->release_cover(art.texture);
        cache.entries.erase(victim);++cache.evictions;
    }
    InspectionArt fresh;fresh.game_index=gi;fresh.game_path=game.path;fresh.path=path;fresh.attempted=true;
    if(!stage1_->prepare_cover(image,fresh.texture,default_cover_orientation_fix30(image))) {
        cache.failed[gi]=identity;
        if(fresh.texture.gpu_ptr) stage1_->release_cover(fresh.texture);
        RuntimeDiag::log("ART 1.3.1 fallback: game=%d kind=%s path=%s error=%s",gi,
            kind==GameCoverKind::FullCover ? "INSIDE" : "DISC",path.c_str(),
            image.valid() ? stage1_->last_error().c_str() : "Unreadable or unsupported image");
        return false;
    }
    fresh.stamp=++cache.clock;
    cache.bytes+=gpu_texture_storage_bytes(fresh.texture);
    cache.failed.erase(gi);
    const int width=fresh.texture.width,height=fresh.texture.height;
    cache.entries.emplace(gi,std::move(fresh));
    RuntimeDiag::log("ART 1.3.1 ready: game=%d kind=%s path=%s size=%dx%d resident=%u bytes=%u",gi,
        kind==GameCoverKind::FullCover ? "INSIDE" : "DISC",path.c_str(),width,height,
        unsigned(cache.entries.size()),unsigned(cache.bytes));
    return true;
}

bool RsxRendererV10::sync_inspection_art(const CoverflowState& state) {
    if(!ready_ || !stage1_) {last_error_="Renderer not ready for inspection artwork";return false;}
    const auto* game=current_game(state);
    if(!game || (!state.inspection_target && state.inspection_phase==0)) return true;
#ifdef PS3_GAME_ORBIT_FIX36
    prefetch_inspection_art(state);
#else
    const int gi=state.visible[std::size_t(state.selected)];
    load_cached_inspection_art(inside_cache_,gi,*game,game->inside_cover_path,GameCoverKind::FullCover);
    load_cached_inspection_art(disc_cache_,gi,*game,game->disc_art_path,GameCoverKind::FrontOnly);
#endif
    return true; // Missing optional art uses the existing neutral/default surfaces.
}

#ifdef PS3_GAME_ORBIT_FIX36
bool RsxRendererV10::prefetch_inspection_art(const CoverflowState& state){
    if(!ready_ || !stage1_)return false;
    const auto* game=current_game(state);
    if(!game)return false;
    const int gi=state.visible[std::size_t(state.selected)];
    const auto attempt=[&](InspectionArtCache& cache,const std::string& path,GameCoverKind kind){
        const auto found=cache.entries.find(gi);
        if(path.empty()){
            if(found!=cache.entries.end()){
                cache.bytes-=std::min(cache.bytes,gpu_texture_storage_bytes(found->second.texture));
                stage1_->release_cover(found->second.texture);
                cache.entries.erase(found);
            }
            cache.failed.erase(gi);
            return false;
        }
        if(found!=cache.entries.end() && found->second.game_path==game->path && found->second.path==path && found->second.texture.uploaded){
            found->second.stamp=++cache.clock;
            ++cache.hits;
            return false;
        }
        const auto failed=cache.failed.find(gi);if(failed!=cache.failed.end() && failed->second==std::make_pair(game->path,path))return false;
        load_cached_inspection_art(cache,gi,*game,path,kind);return true;
    };
    if(attempt(inside_cache_,game->inside_cover_path,GameCoverKind::FullCover))return true;
    return attempt(disc_cache_,game->disc_art_path,GameCoverKind::FrontOnly);
}
#endif
bool RsxRendererV10::has_inside_texture(int gi) const {
    const auto found=inside_cache_.entries.find(gi);
    return found!=inside_cache_.entries.end() && found->second.texture.uploaded;
}
bool RsxRendererV10::has_disc_art_texture(int gi) const {
    const auto found=disc_cache_.entries.find(gi);
    return found!=disc_cache_.entries.end() && found->second.texture.uploaded;
}
#endif

#ifdef PS3_GAME_ORBIT_FIX35
void RsxRendererV10::invalidate_game_art(int gi
#ifdef PS3_GAME_ORBIT_FIX36
    ,unsigned mask
#endif
){
#ifdef PS3_GAME_ORBIT_FIX36
    if(mask&1){auto cover=covers_.find(gi);if(cover!=covers_.end()){release_cover_record(cover->second);covers_.erase(cover);}failed_gpu_covers_.erase(gi);}
    for(const auto& pair:{std::make_pair(&inside_cache_,2u),std::make_pair(&disc_cache_,4u)}){
        if(!(mask&pair.second))continue;
        auto* cache=pair.first;
#else
    auto cover=covers_.find(gi);if(cover!=covers_.end()){release_cover_record(cover->second);covers_.erase(cover);}failed_gpu_covers_.erase(gi);
    for(auto* cache:{&inside_cache_,&disc_cache_}){
#endif
        auto it=cache->entries.find(gi);if(it!=cache->entries.end()){const auto bytes=gpu_texture_storage_bytes(it->second.texture);stage1_->release_cover(it->second.texture);cache->bytes-=std::min(cache->bytes,bytes);cache->entries.erase(it);}cache->failed.erase(gi);
    }
}
#endif
