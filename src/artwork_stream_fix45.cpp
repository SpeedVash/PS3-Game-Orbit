#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX45
#include "library_pair_fix28.h"
#include "prefetch_policy_fix36.h"
#include "runtime_diag.h"
#include <algorithm>
#include <limits>

bool RsxRendererV10::start_streamed_artwork(){
    if(!ready_ || !stage1_){last_error_="Renderer not ready for artwork streaming";return false;}
    if(streamed_artwork_active())return true;
    artwork_loader_.reset(new ArtworkLoaderFix45);
    if(!artwork_loader_->start()){last_error_="Could not start artwork worker";artwork_loader_.reset();return false;}
    RuntimeDiag::log("ARTWORK 1.5.5: worker started; cache=15/15/15 ready=2 upload_slice=262144");
    return true;
}
void RsxRendererV10::stop_streamed_artwork(){
    if(!artwork_loader_)return;
    artwork_loader_->stop();
    RuntimeDiag::log("ARTWORK 1.5.5: worker joined; decoded=%llu stale=%llu peak_ready=%u",
        static_cast<unsigned long long>(artwork_loader_->decoded()),
        static_cast<unsigned long long>(artwork_loader_->discarded()),unsigned(artwork_loader_->peak_ready()));
    artwork_loader_.reset();
    // Native main retains RSX allocations until process teardown, including a
    // possible unfinished upload. shutdown()/clear safely release it on host.
}
void RsxRendererV10::cancel_streamed_artwork(){
    if(artwork_loader_)artwork_loader_->cancel();
    if(stage1_)stage1_->release_cover(pending_artwork_texture_);
    pending_artwork_.reset();pending_artwork_copied_=0;last_artwork_upload_bytes_=0;
}
std::vector<ArtworkRequestFix45> RsxRendererV10::artwork_requests(const CoverflowState& state) const {
    std::vector<ArtworkRequestFix45> jobs;jobs.reserve(ArtworkLoaderFix45::MaxRequests);
    if(!current_game(state))return jobs;
    const auto epoch=artwork_loader_?artwork_loader_->generation():0;
    const auto poses=LibraryPairFix28::poses(state,1);
    const auto cover_limit=std::max(std::size_t(15),poses.size());std::size_t cover_requests=0;
    const auto append=[&](int gi,ArtworkRoleFix45 role){
        if(gi<0 || gi>=int(state.games.size()))return;
        const auto& game=state.games[std::size_t(gi)];
        ArtworkRequestFix45 job;job.game_index=gi;job.game_path=game.path;job.role=role;
        job.path=role==ArtworkRoleFix45::Cover?game.cover_path:
            role==ArtworkRoleFix45::Inside?game.inside_cover_path:game.disc_art_path;
        job.kind=role==ArtworkRoleFix45::Cover?game.cover_kind:
            role==ArtworkRoleFix45::Inside?GameCoverKind::FullCover:GameCoverKind::FrontOnly;
        job.orientation=role==ArtworkRoleFix45::Cover?game.cover_orientation:1;job.generation=epoch;
        if(!job.path.empty() && std::find(jobs.begin(),jobs.end(),job)==jobs.end()){
            // Fading cases can consume most slots. Do not repeatedly decode
            // additional neighbors that cannot coexist with their pins.
            if(role==ArtworkRoleFix45::Cover && cover_requests>=cover_limit)return;
            if(role==ArtworkRoleFix45::Cover)++cover_requests;
            jobs.push_back(std::move(job));
        }
    };
    const int selected=state.visible[std::size_t(state.selected)];
    append(selected,ArtworkRoleFix45::Cover);
    if(state.inspection_target || state.inspection_phase>0){append(selected,ArtworkRoleFix45::Inside);append(selected,ArtworkRoleFix45::Disc);}
    // Visible and fading exterior textures precede background inspection work.
    for(const auto& pose:poses)append(pose.game_index,ArtworkRoleFix45::Cover);
    const auto neighbors=PrefetchPolicyFix36::candidates(state);
    for(int gi:neighbors)append(gi,ArtworkRoleFix45::Cover);
    append(selected,ArtworkRoleFix45::Inside);append(selected,ArtworkRoleFix45::Disc);
    for(int gi:neighbors){append(gi,ArtworkRoleFix45::Inside);append(gi,ArtworkRoleFix45::Disc);}
    return jobs;
}
bool RsxRendererV10::streamed_cover_failed(int gi,const GameEntry& game) const {
    const auto found=streamed_failed_covers_.find(gi);
    return found!=streamed_failed_covers_.end() && found->second==std::make_pair(game.path,game.cover_path);
}
bool RsxRendererV10::artwork_complete(const ArtworkRequestFix45& request) const {
    if(request.role==ArtworkRoleFix45::Cover){
        const auto found=covers_.find(request.game_index);
        if(found!=covers_.end()){
            const auto& art=found->second;
            if(art.game_path==request.game_path && art.path==request.path && art.kind==request.kind &&
               art.orientation==request.orientation && art.texture.uploaded)return true;
        }
        const auto failed=streamed_failed_covers_.find(request.game_index);
        return failed!=streamed_failed_covers_.end() && failed->second==std::make_pair(request.game_path,request.path);
    }
    const auto& cache=request.role==ArtworkRoleFix45::Inside?inside_cache_:disc_cache_;
    const auto found=cache.entries.find(request.game_index);
    if(found!=cache.entries.end() && found->second.game_path==request.game_path &&
       found->second.path==request.path && found->second.texture.uploaded)return true;
    const auto failed=cache.failed.find(request.game_index);
    return failed!=cache.failed.end() && failed->second==std::make_pair(request.game_path,request.path);
}
std::pair<std::size_t,std::size_t> RsxRendererV10::streamed_preload_progress(const CoverflowState& state) const {
    const auto jobs=artwork_requests(state);std::size_t done=0;
    for(const auto& job:jobs)if(artwork_complete(job))++done;
    return {done,jobs.size()};
}
bool RsxRendererV10::reserve_streamed_artwork(const ArtworkRequestFix45& request,const std::unordered_set<int>& pinned){
    constexpr std::size_t WorstTextureBytes=4u<<20;
    if(request.role==ArtworkRoleFix45::Cover)return reserve_cover_cache(pinned,WorstTextureBytes);
    auto& cache=request.role==ArtworkRoleFix45::Inside?inside_cache_:disc_cache_;
    const auto limit=std::max(resident_limit_,inspection_cache_pins_.size());
    while(cache.entries.size()>=limit || cache.bytes+WorstTextureBytes>InspectionArtBudgetBytes){
        auto victim=cache.entries.end();auto oldest=std::numeric_limits<std::uint64_t>::max();
        for(auto it=cache.entries.begin();it!=cache.entries.end();++it){
            if(inspection_cache_pins_.count(it->first))continue;
            const bool preferred=preferred_covers_.count(it->first)!=0;
            const bool victim_preferred=victim!=cache.entries.end() && preferred_covers_.count(victim->first)!=0;
            if(victim!=cache.entries.end() && preferred!=victim_preferred){
                if(!preferred){victim=it;oldest=it->second.stamp;}continue;
            }
            if(it->second.stamp<oldest){victim=it;oldest=it->second.stamp;}
        }
        if(victim==cache.entries.end())return false;
        cache.bytes-=std::min(cache.bytes,gpu_texture_storage_bytes(victim->second.texture));
        stage1_->release_cover(victim->second.texture);cache.entries.erase(victim);++cache.evictions;
    }
    return true;
}
void RsxRendererV10::fail_streamed_artwork(const ArtworkRequestFix45& request,const std::string& error){
    const auto identity=std::make_pair(request.game_path,request.path);
    if(request.role==ArtworkRoleFix45::Cover){
        streamed_failed_covers_[request.game_index]=identity;failed_gpu_covers_.insert(request.game_index);++gpu_cache_misses_;
    }else{
        auto& cache=request.role==ArtworkRoleFix45::Inside?inside_cache_:disc_cache_;
        cache.failed[request.game_index]=identity;++cache.misses;
    }
    RuntimeDiag::log("ARTWORK 1.5.5 fallback: game=%d role=%u path=%s error=%s",
        request.game_index,unsigned(request.role),request.path.c_str(),error.c_str());
}
void RsxRendererV10::commit_streamed_artwork(){
    const auto& request=pending_artwork_->request;
    if(request.role==ArtworkRoleFix45::Cover){
        auto old=covers_.find(request.game_index);
        if(old!=covers_.end()){release_cover_record(old->second);covers_.erase(old);}
        GpuCoverRecord art;art.game_path=request.game_path;art.path=request.path;art.kind=request.kind;
        art.orientation=request.orientation;art.texture=pending_artwork_texture_;art.stamp=++gpu_cache_clock_;
        gpu_cache_bytes_+=gpu_texture_storage_bytes(art.texture);++gpu_cache_misses_;
        covers_.emplace(request.game_index,std::move(art));failed_gpu_covers_.erase(request.game_index);
        streamed_failed_covers_.erase(request.game_index);
    }else{
        auto& cache=request.role==ArtworkRoleFix45::Inside?inside_cache_:disc_cache_;
        auto old=cache.entries.find(request.game_index);
        if(old!=cache.entries.end()){
            cache.bytes-=std::min(cache.bytes,gpu_texture_storage_bytes(old->second.texture));
            stage1_->release_cover(old->second.texture);cache.entries.erase(old);
        }
        InspectionArt art;art.game_index=request.game_index;art.game_path=request.game_path;
        art.path=request.path;art.texture=pending_artwork_texture_;art.attempted=true;art.stamp=++cache.clock;
        cache.bytes+=gpu_texture_storage_bytes(art.texture);++cache.misses;
        cache.entries.emplace(request.game_index,std::move(art));cache.failed.erase(request.game_index);
    }
    pending_artwork_texture_={};pending_artwork_.reset();pending_artwork_copied_=0;
}
bool RsxRendererV10::service_streamed_artwork(const CoverflowState& state){
    last_artwork_upload_bytes_=0;
    if(!ready_ || !stage1_ || !streamed_artwork_active()){last_error_="Artwork worker unavailable";return false;}
    const auto desired=artwork_requests(state);
    // Empty view (e.g. file manager) pauses preparation without touching cache.
    if(pending_artwork_ && (std::find(desired.begin(),desired.end(),pending_artwork_->request)==desired.end() ||
                            artwork_complete(pending_artwork_->request))){
        stage1_->release_cover(pending_artwork_texture_);pending_artwork_.reset();pending_artwork_copied_=0;
    }
    std::vector<ArtworkRequestFix45> missing;missing.reserve(desired.size());
    for(const auto& job:desired)if(!artwork_complete(job) && (!pending_artwork_ || !(job==pending_artwork_->request)))missing.push_back(job);
    artwork_loader_->request(std::move(missing));
    if(!pending_artwork_)pending_artwork_=artwork_loader_->take();
    if(!pending_artwork_)return true;
    const auto& request=pending_artwork_->request;
    if(std::find(desired.begin(),desired.end(),request)==desired.end() || artwork_complete(request)){
        pending_artwork_.reset();return true;
    }
    if(!pending_artwork_->valid()){
        fail_streamed_artwork(request,pending_artwork_->error);pending_artwork_.reset();return true;
    }
    if(!pending_artwork_texture_.gpu_ptr){
        std::unordered_set<int> pinned;
        for(const auto& pose:LibraryPairFix28::poses(state,1))pinned.insert(pose.game_index);
        if(!reserve_streamed_artwork(request,pinned))return true;
        if(!stage1_->begin_artwork_upload(*pending_artwork_,pending_artwork_texture_)){
            fail_streamed_artwork(request,stage1_->last_error());pending_artwork_.reset();return true;
        }
        pending_artwork_copied_=0;
    }
    last_artwork_upload_bytes_=stage1_->advance_artwork_upload(*pending_artwork_,pending_artwork_texture_,
                                                             pending_artwork_copied_,ArtworkUploadSliceBytes);
    if(pending_artwork_texture_.uploaded)commit_streamed_artwork();
    return true;
}
#endif
