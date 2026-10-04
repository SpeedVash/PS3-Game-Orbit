#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX33
#include "runtime_diag.h"
#include "cover_orientation_fix28.h"
#ifndef PS3_GAME_ORBIT_FIX34
void RsxRendererV10::release_inspection_art(InspectionArt& art) {
    if(stage1_ && art.texture.gpu_ptr) stage1_->release_cover(art.texture);
    art={};
}
void RsxRendererV10::load_inspection_art(InspectionArt& art,int gi,const GameEntry& game,
                                        const std::string& path,GameCoverKind kind) {
    if(art.game_index==gi && art.game_path==game.path && art.path==path && art.attempted) return;
    release_inspection_art(art);art.game_index=gi;art.game_path=game.path;art.path=path;art.attempted=true;
    if(path.empty()) return;
    const auto image=load_cover_file(path,kind);
    if(!image.valid() || !stage1_->prepare_cover(image,art.texture,default_cover_orientation_fix30(image))) {
        RuntimeDiag::log("ART 1.3 fallback: game=%d kind=%s path=%s error=%s",gi,
            kind==GameCoverKind::FullCover ? "INSIDE" : "DISC",path.c_str(),stage1_->last_error().c_str());
        return;
    }
    RuntimeDiag::log("ART 1.3 ready: game=%d kind=%s path=%s size=%dx%d",gi,
        kind==GameCoverKind::FullCover ? "INSIDE" : "DISC",path.c_str(),art.texture.width,art.texture.height);
}
bool RsxRendererV10::sync_inspection_art(const CoverflowState& state) {
    if(!ready_ || !stage1_) {last_error_="Renderer not ready for inspection artwork";return false;}
    const auto* game=current_game(state);const int gi=game ? state.visible[std::size_t(state.selected)] : -1;
    if(!game || inside_art_.game_index!=gi || inside_art_.game_path!=game->path) release_inspection_art(inside_art_);
    if(!game || disc_art_.game_index!=gi || disc_art_.game_path!=game->path) release_inspection_art(disc_art_);
    if(!game || (!state.inspection_target && state.inspection_phase==0)) return true;
    load_inspection_art(inside_art_,gi,*game,game->inside_cover_path,GameCoverKind::FullCover);
    load_inspection_art(disc_art_,gi,*game,game->disc_art_path,GameCoverKind::FrontOnly);
    return true;
}
bool RsxRendererV10::has_inside_texture(int gi) const {return inside_art_.game_index==gi && inside_art_.texture.uploaded;}
bool RsxRendererV10::has_disc_art_texture(int gi) const {return disc_art_.game_index==gi && disc_art_.texture.uploaded;}
#endif
const GpuTextureStage1* RsxRendererV10::texture_for_art_role(V14MeshPart::TextureRole role,int gi) const {
    switch(role) {
    case V14MeshPart::TextureRole::Cover:return texture_for_game(gi);
    case V14MeshPart::TextureRole::Inside:return has_inside_texture(gi) ?
#ifdef PS3_GAME_ORBIT_FIX34
        &inside_cache_.entries.at(gi).texture : nullptr;
#else
        &inside_art_.texture : nullptr;
#endif
    case V14MeshPart::TextureRole::DiscLabel:return has_disc_art_texture(gi) ?
#ifdef PS3_GAME_ORBIT_FIX34
        &disc_cache_.entries.at(gi).texture : &disc_label_texture_;
#else
        &disc_art_.texture : &disc_label_texture_;
#endif
    case V14MeshPart::TextureRole::DiscBack:return &disc_back_texture_;
    default:return nullptr;
    }
}
#endif
