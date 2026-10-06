#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX32
#include <cmath>
#include <array>
#include "library_pair_fix28.h"

// Validate every case range before emitting any RSX command. Articulation
// changes the range count, but never permits missing or duplicate surfaces.
bool validate_native_plan_fix32(const V10FramePlan& plan,const V14CaseMesh& mesh) {
    if(
#ifndef PS3_GAME_ORBIT_FIX37
       plan.packets.empty() ||
#endif
       mesh.parts.empty() || mesh.parts.size()>64 ||
       plan.packets.size()>LibraryPairFix28::MaxCases*6+
#ifdef PS3_GAME_ORBIT_FIX37
       64
#else
       32
#endif
       ) return false;
    struct Range {int game=-1;bool open=false;std::array<unsigned char,64> counts{};};
    std::array<Range,LibraryPairFix28::MaxCases> ranges{};
    std::size_t range_count=0;
    for(const auto& p:plan.packets) {
        if(p.game_index<0 || p.mesh_part>=mesh.parts.size() || p.plastic_pass>2 ||
           !std::isfinite(p.alpha) || !std::isfinite(p.visibility) ||
           p.alpha<0 || p.alpha>1 || p.visibility<0 || p.visibility>1) return false;
        for(float v:p.mvp.m) if(!std::isfinite(v)) return false;
        for(float v:p.model.m) if(!std::isfinite(v)) return false;
        const auto& part=mesh.parts[p.mesh_part];
        const bool glass=mesh.jfx && (part.material==V14MeshPart::Material::ClearPlastic ||
                                     part.material==V14MeshPart::Material::DiscGlass);
        if(glass ? p.plastic_pass==0 : p.plastic_pass!=0) return false;
        const bool open=part.joint!=V14MeshPart::Joint::Closed;
        if(open && !p.selected) return false;
        std::size_t ri=0;while(ri<range_count && ranges[ri].game!=p.game_index) ++ri;
        if(ri==range_count) {
            if(range_count==ranges.size()) return false;
            ranges[ri].game=p.game_index;ranges[ri].open=open;++range_count;
        } else if(ranges[ri].open!=open) return false;
        auto& counts=ranges[ri].counts;
        const auto bit=static_cast<unsigned char>(1u<<p.plastic_pass);
        if(counts[p.mesh_part]&bit) return false;
        counts[p.mesh_part]|=bit;
    }
    unsigned articulated=0;
    for(std::size_t ri=0;ri<range_count;++ri) {
        const bool open=ranges[ri].open;if(open) ++articulated;
        for(std::size_t i=0;i<mesh.parts.size();++i) {
            const auto& part=mesh.parts[i];
            const bool expected=(part.joint!=V14MeshPart::Joint::Closed)==open;
            const bool glass=mesh.jfx && (part.material==V14MeshPart::Material::ClearPlastic ||
                                         part.material==V14MeshPart::Material::DiscGlass);
            if(ranges[ri].counts[i]!=(expected ? (glass ? 6 : 1) : 0)) return false;
        }
    }
    return articulated<=1;
}
#endif
