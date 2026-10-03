#include <cstdio>
#include <string>
#include <vector>
#include "library_scanner.h"
#include "cover_resolver.h"
#include "cover_cache.h"
#include "controller.h"
#include "safe_boot.h"
#include "full_cover_case_fix26.h"
#include "rsx_renderer_v10.h"
#include "runtime_diag.h"
#include "ps3_lifecycle.h"
#include "project_identity.h"
#include "rsx_command_stream_fix25.h"
#include "library_browser_fix28.h"
#include "library_pair_fix28.h"
#include "cover_orientation_fix28.h"
#include "jfx_case_fix29.h"
#include "preferences_fix29.h"
#include "mount_operation_fix29.h"
#include "metadata_utils.h"
#include <unordered_set>
#ifdef __PSL1GHT__
#include <unistd.h>
#include <lv2/systime.h>
#include <rsx/rsx.h>
static bool hold_diagnostic_frame(Ps3Lifecycle& lifecycle,const char* tag,s64 duration_us) {
    const s64 start=sysGetSystemTime();
    const u64 vblank_before=gcmGetVBlankCount();
    RuntimeDiag::log("HOLD %s: begin; duration_us=%lld",tag,static_cast<long long>(duration_us));
    while(sysGetSystemTime()-start<duration_us){
        lifecycle.pump();
        if(lifecycle.exit_requested()) return false;
        usleep(10000);
    }
    RuntimeDiag::log("HOLD %s: complete; elapsed_us=%lld vblank_delta=%llu",tag,
                     static_cast<long long>(sysGetSystemTime()-start),
                     static_cast<unsigned long long>(gcmGetVBlankCount()-vblank_before));
    return true;
}
#endif

static std::vector<GameEntry> scan_catalog(){
    RuntimeDiag::log("LIBRARY SCAN: begin");
    LibraryScanner scanner;CoverResolver covers;
    auto games=scanner.scan_ps3();
    for(auto& g:games){
        const auto cover=covers.resolve(g);g.cover_path=cover.path;
        g.cover_kind=cover.path.empty() ? GameCoverKind::None :
                     (cover.is_full_cover ? GameCoverKind::FullCover : GameCoverKind::FrontOnly);
    }
    RuntimeDiag::log("LIBRARY SCAN: complete; games=%u covers=%s",unsigned(games.size()),covers.global_dir().c_str());
    return games;
}

int main(){
    RuntimeDiag::init();
#ifdef __PSL1GHT__
    if(!RuntimeDiag::available()) return 20;
#endif
    RuntimeDiag::log("BOOT 00: %s",ProjectIdentity::DisplayTitle);
    RuntimeDiag::log("Log: %s",RuntimeDiag::log_path().c_str());
#ifdef __PSL1GHT__
    RuntimeDiag::log("BUILD: GCC %s; compiled %s %s; ps3aqua toolchain=961fddac01337f18da08f4471d558a7a5b0d9af2 SDK=af9d3d964c8faa69abce4961a269f9582e05a33f",__VERSION__,__DATE__,__TIME__);
#endif
    Ps3Lifecycle lifecycle;Controller controller;RsxStage1 rsx;
    const bool lifecycle_ready=lifecycle.init(),controller_ready=controller.init();
    RuntimeDiag::log("BOOT 01: lifecycle=%s",lifecycle_ready ? "OK" : "FAILED");
    RuntimeDiag::log("BOOT 02: controller=%s",controller_ready ? "OK" : "FAILED");
    RuntimeDiag::log("BOOT 03: PS3 Game Orbit FIX30; approved JFX model, compact UI, startup image and offscreen calibration");
    const bool rsx_ready=rsx.init();
    RuntimeDiag::log("BOOT 04: rsx_stage1=%s error=%s",rsx_ready ? "OK" : "FAILED",rsx.last_error().c_str());
#ifdef PS3_SP_LOADER_FIX29
    const auto mesh=JfxCaseFix29::build();
#else
    const auto mesh=build_full_cover_case_fix26(5);
#endif
    unsigned model_triangles=0;for(const auto& part:mesh.parts) model_triangles+=part.indices.size()/3;
    RuntimeDiag::log("COVER MODEL: jfx=%u original_triangles=%u gpu_parts=%u draws_per_case=6 neutral_plastic_rgba=1,1,1,0.14",
                     unsigned(mesh.jfx),model_triangles,unsigned(mesh.parts.size()));
    RsxRendererV10 renderer;
    const bool renderer_ready=rsx_ready && renderer.init(rsx,mesh,false);
    RuntimeDiag::log("BOOT 06: renderer=%s error=%s",renderer_ready ? "OK" : "FAILED",renderer.last_error().c_str());
    CoverCache cache(3);
    LibraryBrowserFix28 browser(ProjectIdentity::SafeBootCoverPath,ProjectIdentity::SecondDemoCoverPath);
#ifdef __PSL1GHT__
    int result=(!lifecycle_ready || !controller_ready) ? 21 : renderer_ready ? 0 : 22;
    bool warmup_ok=false,fifo_ok=false,user_exit=false,time_limit=false;
    RsxCommandStreamFix25 stream;
    if(result==0){
        warmup_ok=renderer.run_cpu_framebuffer_probe();
        if(!warmup_ok) result=23;
        else if(!hold_diagnostic_frame(lifecycle,"ORBIT_STARTUP",200000)) user_exit=true;
    }
    if(result==0 && !user_exit){
        auto diagnostic=make_safe_boot_state(ProjectIdentity::SafeBootCoverPath);
        diagnostic.center_yaw_deg=28;diagnostic.center_pitch_deg=-5;
        if(!renderer.sync_visible_covers(diagnostic,cache,0) || renderer.gpu_cover_count()!=1) result=26;
        else{
            fifo_ok=stream.init(rsx);
            if(!fifo_ok) result=24;
            bool front_valid=false;
            for(unsigned attempt=0;attempt<2 && result==0;++attempt){
                renderer.set_front_face_clockwise(attempt==1);
                renderer.set_diagnostic_view("CULL_FRONT",V14Surface::CoverFront);
                if(!stream.begin_frame()){result=29;break;}
                if(!renderer.render(diagnostic,mesh,0)){result=27;break;}
                if(!stream.complete_frame()){result=30;break;}
                const auto green=renderer.front_green_samples(mesh);
                RuntimeDiag::log("CULL VERIFY: attempt=%u clockwise=%u front_green=%u/6 accepted=%u",attempt,attempt,green,unsigned(green>=4));
                if(green>=4){front_valid=true;break;}
            }
            if(result==0 && !front_valid) result=31;
        }
    }
    if(result==0 && !user_exit && !renderer.prepare_orbit_background()) result=33;
    if(result==0 && !user_exit){
        // The last calibration frame is acknowledged before any disk reads,
        // texture releases/uploads or HUD geometry changes.
        browser.replace_catalog(scan_catalog());
#ifdef PS3_SP_LOADER_FIX29
        PreferencesFix29 preferences;std::string preferences_path=ProjectIdentity::PreferencesPath;
        bool preferences_loaded=preferences.load(preferences_path);
        if(!preferences_loaded){
            preferences_loaded=preferences.load(ProjectIdentity::FallbackPreferencesPath);
            if(preferences_loaded) preferences_path=ProjectIdentity::FallbackPreferencesPath;
        }
#ifdef PS3_GAME_ORBIT_FIX30
        if(!preferences_loaded){
            preferences_loaded=preferences.load(ProjectIdentity::LegacyPreferencesPath);
            if(!preferences_loaded) preferences_loaded=preferences.load(ProjectIdentity::LegacyFallbackPreferencesPath);
            RuntimeDiag::log("PREFERENCES MIGRATION: legacy=%u target=%s",unsigned(preferences_loaded),preferences_path.c_str());
        }
#endif
        auto restored=browser.state().games;preferences.apply(restored);browser.replace_catalog(std::move(restored));
        if(preferences_loaded) browser.restore_selection(preferences.filter(),preferences.selected_path());
        RuntimeDiag::log("PREFERENCES: loaded=%u path=%s",unsigned(preferences_loaded),preferences_path.c_str());
        MountOperationFix29 mounting;
        bool preferences_dirty=false,mounted_exit=false;
        s64 save_at=0,next_disc_check=0;
        std::string mounted_id;auto previous_mount_state=mounting.state();
        auto save_preferences=[&](){
            preferences.capture(browser.state());bool saved=preferences.save(preferences_path);
            if(!saved && preferences_path!=ProjectIdentity::FallbackPreferencesPath){
                saved=preferences.save(ProjectIdentity::FallbackPreferencesPath);
                if(saved) preferences_path=ProjectIdentity::FallbackPreferencesPath;
            }
            RuntimeDiag::log("PREFERENCES: saved=%u path=%s",unsigned(saved),preferences_path.c_str());
            preferences_dirty=!saved;
        };
#endif
        std::string prepared_key,cover_status;
        std::unordered_set<std::string> failed_covers;
        const s64 start=sysGetSystemTime();s64 previous=start,last_report=start;
        RuntimeDiag::log("CONTROLS: left/right browse; right stick rotate; L1/R1 filters; triangle favorite; X mount; UP rotation; L2/R2 zoom; R3 reset; SELECT help; START rescan; circle exit; artwork=normal full wrap; no manual orientation");
        while(result==0){
            lifecycle.pump();
            if(lifecycle.exit_requested()){user_exit=true;break;}
            const s64 now=sysGetSystemTime();
#ifndef PS3_SP_LOADER_FIX29
            if(now-start>=300000000){time_limit=true;break;}
#endif
            const auto command=browser.update(controller.poll(),float(now-previous)/1000000.0f);previous=now;
            if(command.request_exit){user_exit=true;break;}
#ifdef PS3_SP_LOADER_FIX29
            if(command.mount_selected){
                if(const auto* target=current_game(browser.state())){
                    save_preferences();mounted_id.clear();next_disc_check=0;
                    const bool started=mounting.start(*target,std::uint64_t(now/1000));
                    RuntimeDiag::log("MOUNT REQUEST: started=%u title=%s id=%s path=%s",unsigned(started),target->title.c_str(),target->title_id.c_str(),target->path.c_str());
                }
            }
            if(mounting.needs_disc_check() && now>=next_disc_check){
                mounted_id=read_param_sfo_string("/dev_bdvd/PS3_GAME/PARAM.SFO","TITLE_ID");
                next_disc_check=now+250000;
            }
            mounting.update(std::uint64_t(now/1000),mounted_id);
            browser.set_operation_status(mounting.status(),mounting.busy());
            if(mounting.state()!=previous_mount_state){
                RuntimeDiag::log("MOUNT STATUS: state=%u http=%d path=%s mounted_id=%s text=%s",unsigned(mounting.state()),mounting.http_status(),mounting.path().c_str(),mounted_id.c_str(),mounting.status().c_str());
                previous_mount_state=mounting.state();
            }
            if(mounting.exit_ready(std::uint64_t(now/1000))){mounted_exit=true;break;}
            if(browser.take_preferences_changed()){preferences_dirty=true;save_at=now+700000;}
            if(preferences_dirty && now>=save_at){save_preferences();save_at=now+2000000;}
#endif
            if(command.rescan_library){
#ifdef PS3_SP_LOADER_FIX29
                preferences.capture(browser.state());
#endif
                // Previous frame already completed: keep old GPU resources alive
                // until this point, then force a fresh decode on the new catalog.
                renderer.clear_cover_textures();cache.clear();
                browser.replace_catalog(scan_catalog());prepared_key.clear();failed_covers.clear();
#ifdef PS3_SP_LOADER_FIX29
                auto rescanned=browser.state().games;preferences.apply(rescanned);browser.replace_catalog(std::move(rescanned));
#endif
            }
            auto draw=browser.render_state();
            auto* selected=current_game(draw);
            if(!selected){result=28;break;}
            const int index=draw.visible[draw.selected];
            const auto poses=LibraryPairFix28::poses(draw,1);
            std::string key;
            for(const auto& pose:poses){
                const auto& game=draw.games[pose.game_index];
                key+=std::to_string(pose.game_index)+"\n"+game.path+"\n"+game.cover_path+"\n"+
                     std::to_string(static_cast<int>(game.cover_kind))+"\n"+std::to_string(game.cover_orientation)+"\n";
            }
            for(auto& game:draw.games) if(failed_covers.count(game.path)) game.cover_path.clear();
            if(key!=prepared_key){
                if(!renderer.sync_visible_covers(draw,cache,1)){result=26;break;}
                for(const auto& pose:poses){
                    auto& game=draw.games[pose.game_index];
                    const auto* image=cache.get_or_load(game);
                    RuntimeDiag::log("LIBRARY COVER: game=%d selected=%u title=%s path=%s kind=%u dimensions=%dx%d exif=%u manual=%u full=%u uploaded=%u",
                                     pose.game_index,unsigned(pose.selected),game.title.c_str(),game.cover_path.c_str(),
                                     unsigned(game.cover_kind),image ? image->width : 0,image ? image->height : 0,
                                     image ? read_cover_orientation_fix28(*image) : 1,game.cover_orientation,
                                     unsigned(renderer.has_full_cover_texture(pose.game_index)),unsigned(renderer.has_cover_texture(pose.game_index)));
                    if(!game.cover_path.empty() && !renderer.has_cover_texture(pose.game_index)){
                        failed_covers.insert(game.path);game.cover_path.clear();
                    }
                }
                cover_status=failed_covers.count(selected->path) ? "CAPA INVALIDA" :
                             !renderer.has_cover_texture(index) ? "SEM CAPA" :
                             renderer.has_full_cover_texture(index) ? "CAPA INTEIRA" : "CAPA FRONTAL";
                if(browser.state().games.empty()) cover_status="BIBLIOTECA VAZIA";
                prepared_key=key;
                RuntimeDiag::log("LIBRARY SELECT: visible=%u total=%u index=%d title=%s path=%s cover=%s status=%s cases=%u gpu_covers=%u",
                                 unsigned(browser.state().visible.size()),unsigned(browser.state().games.size()),index,
                                 selected->title.c_str(),selected->path.c_str(),selected->cover_path.c_str(),cover_status.c_str(),unsigned(poses.size()),unsigned(renderer.gpu_cover_count()));
            }
            // All changing resources are prepared before begin_frame, after the
            // previous GET/REF/backend-label completion. No allocations in draw.
            if(!renderer.set_library_hud(browser.hud_lines(cover_status))){result=32;break;}
            if(!stream.begin_frame()){result=29;break;}
            const auto yaw=draw.center_yaw_deg;
            const auto surface=yaw>45 && yaw<135 ? V14Surface::CoverSpine :
                               yaw>=90 && yaw<=270 ? V14Surface::CoverBack : V14Surface::CoverFront;
            renderer.set_diagnostic_view("LIBRARY",surface);
            if(!renderer.render(draw,mesh,1,browser.scale())){result=27;break;}
            if(!stream.complete_frame()){result=30;break;}
            const auto& stats=renderer.last_stats();
            if(stats.draw_calls!=poses.size()*6 || stats.hud_draw_calls!=1){result=28;break;}
#ifdef PS3_GAME_ORBIT_FIX30
            if(stats.background_draw_calls!=1){result=28;break;}
#endif
            if(stream.completed_frames()<=3 || now-last_report>=1000000){
                RuntimeDiag::log("LIBRARY FRAME: frames=%u switches=%u selected=%d visible=%u yaw=%.2f pitch=%.2f scale=%.3f auto=%u case_draws=%u hud_draws=%u background_draws=%u gpu_covers=%u",
                                 stream.completed_frames(),stream.switches(),browser.state().selected,unsigned(browser.state().visible.size()),
                                 double(yaw),double(draw.center_pitch_deg),double(browser.scale()),unsigned(browser.automatic()),
                                 unsigned(stats.draw_calls),unsigned(stats.hud_draw_calls),unsigned(stats.background_draw_calls),unsigned(renderer.gpu_cover_count()));
                last_report=now;
            }
        }
#ifdef PS3_SP_LOADER_FIX29
        mounting.cancel();save_preferences();
        RuntimeDiag::log("MOUNT EXIT: confirmed=%u",unsigned(mounted_exit));
#endif
    }
    RuntimeDiag::log("DIAG 69: result=%d warmup=%s fifo=%s frames=%u switches=%u games=%u user_exit=%u time_limit=%u renderer_error=%s fifo_error=%s",
                     result,warmup_ok ? "OK" : "FAILED",fifo_ok ? "OK" : "not completed",stream.completed_frames(),stream.switches(),
                     unsigned(browser.state().games.size()),unsigned(user_exit),unsigned(time_limit),renderer.last_error().c_str(),stream.last_error().c_str());
    controller.shutdown();lifecycle.shutdown();
    RuntimeDiag::log("EXIT: result=%d; return to XMB; RSX allocations retained for process teardown",result);
    RuntimeDiag::shutdown();return result;
#else
    browser.replace_catalog(scan_catalog());
    auto draw=browser.render_state();
    if(renderer_ready){
        renderer.prepare_orbit_background();
        renderer.sync_visible_covers(draw,cache,1);
        renderer.set_library_hud(browser.hud_lines("HOST"));renderer.render(draw,mesh,1,browser.scale());
    }
    const auto& stats=renderer.last_stats();
    std::printf("Host FIX28: games=%zu case_draws=%zu hud_draws=%zu\n",browser.state().games.size(),stats.draw_calls,stats.hud_draw_calls);
    renderer.shutdown();rsx.shutdown();controller.shutdown();lifecycle.shutdown();RuntimeDiag::shutdown();return 0;
#endif
}
