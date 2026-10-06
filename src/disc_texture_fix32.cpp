#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX32
#include "project_identity.h"
bool RsxRendererV10::prepare_disc_textures() {
#ifdef __PSL1GHT__
    const std::string base="/dev_hdd0/game/PGORBT301/USRDIR/";
#else
    const std::string base="pkgfiles/USRDIR/";
#endif
    const auto label=load_cover_file(base+"ORBIT_DISC_LABEL.png",GameCoverKind::FrontOnly);
    const auto back=load_cover_file(base+"ORBIT_DISC_BACK.png",GameCoverKind::FrontOnly);
    const bool label_ready=
#ifdef PS3_GAME_ORBIT_FIX38
        stage1_->prepare_disc_artwork(label,disc_label_texture_);
#else
        stage1_->prepare_cover(label,disc_label_texture_);
#endif
    if(!label_ready || !stage1_->prepare_cover(back,disc_back_texture_)) {
        last_error_=stage1_->last_error();return false;
    }
    return true;
}
#endif
