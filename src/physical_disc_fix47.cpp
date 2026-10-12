#include "physical_disc_fix47.h"
#ifdef PS3_GAME_ORBIT_FIX47
#include "metadata_utils.h"
#include "cover_resolver.h"
#include <algorithm>
#include <sys/stat.h>
#ifdef __PSL1GHT__
#include <ppu-lv2.h>
#endif

namespace {
#ifdef __PSL1GHT__
bool ps3_type(std::uint32_t type){return type==0xff70 || type==0xff71;}
#endif
bool directory(const std::string& path){struct stat s{};return stat(path.c_str(),&s)==0 && S_ISDIR(s.st_mode);}
std::string normalize_id(std::string id){
    if(id.size()!=9)return {};
    for(std::size_t i=0;i<id.size();++i){
        auto& c=id[i];if(c>='a' && c<='z')c=char(c-'a'+'A');
        if(i<4 ? c<'A' || c>'Z' : c<'0' || c>'9')return {};
    }
    return id;
}
}
DiscAccessFix47 probe_disc_access_fix47(){
#ifdef __PSL1GHT__
    // Read-only Cobra APIs: raw device type and emulation state. Do not mount,
    // unmount, authenticate or send fake insertion/ejection events.
    // References: aldostools/webMAN-MOD/cobra/{storage_ext.h,storage.h,syscall8.h}.
    std::uint32_t real=0,effective=0,fake=0;
    std::int32_t result=0;
    {lv2syscall4(8,0x7020,reinterpret_cast<std::uintptr_t>(&real),
        reinterpret_cast<std::uintptr_t>(&effective),reinterpret_cast<std::uintptr_t>(&fake));
        result=static_cast<std::int32_t>(p1);}
    if(result!=0)return DiscAccessFix47::Unknown;
    if(!ps3_type(real) || !ps3_type(effective))return DiscAccessFix47::None;
    struct EmulationState {std::int32_t size=1032,mode=0;char first_file[1024]{};} state;
    static_assert(sizeof(state)==1032,"Cobra emulation-state layout");
    {lv2syscall2(8,0x7023,reinterpret_cast<std::uintptr_t>(&state));result=static_cast<std::int32_t>(p1);}
    if(result!=0)return DiscAccessFix47::Unknown;
    if(state.mode!=0)return DiscAccessFix47::Emulated;
    // A folder mount can redirect /dev_bdvd while real and effective types
    // still describe the inserted disc. Query mappings without altering them.
    // PS3Xploit/PS3HEN payload/main.c: GET_MAP_PATH(index, old_path, new_path).
    bool mapping_api=false;
    for(unsigned index=0;index<32;++index){
        char old_path[2048]{},new_path[2048]{};
        {lv2syscall4(8,0x7967,index,reinterpret_cast<std::uintptr_t>(old_path),
            reinterpret_cast<std::uintptr_t>(new_path));result=static_cast<std::int32_t>(p1);}
        if(result!=0)continue;
        mapping_api=true;old_path[sizeof(old_path)-1]=0;new_path[sizeof(new_path)-1]=0;
        std::string mapped=old_path;
        while(mapped.size()>1 && mapped[0]=='/' && mapped[1]=='/')mapped.erase(0,1);
        while(mapped.size()>1 && mapped.back()=='/')mapped.pop_back();
        if((mapped=="/dev_bdvd" || mapped=="/dev_bdvd/PS3_GAME" || mapped=="/dev_bdvd/PS3_GAME/PARAM.SFO") &&
           new_path[0] && mapped!=new_path)return DiscAccessFix47::Emulated;
    }
    return mapping_api ? DiscAccessFix47::Physical : DiscAccessFix47::Unknown;
#else
    return DiscAccessFix47::Unknown;
#endif
}
DiscSampleFix47 read_disc_sample_fix47(const std::string& root,DiscProbeFix47 probe,const std::string& covers){
    DiscSampleFix47 result;
    if(!probe || root.empty())return result;
    const auto access=probe();
    if(access==DiscAccessFix47::None || access==DiscAccessFix47::Emulated || !directory(root+"/PS3_GAME"))return result;
    const auto sfo=root+"/PS3_GAME/PARAM.SFO";
    const auto id=normalize_id(read_param_sfo_string(sfo,"TITLE_ID"));
    const auto category=read_param_sfo_string(sfo,"CATEGORY");
    if(id.empty() || (!category.empty() && category!="DG"))return result;
    auto title=read_param_sfo_string(sfo,"TITLE");
    if(title.size()>512)title.resize(512);
    GameEntry game;game.title=title.empty()?id:title;game.title_id=id;game.path=root;
    game.library_key="/dev_bdvd/"+id;game.source=GameSource::Disc;game.format=GameFormat::Disc;
    game.physical_disc=access==DiscAccessFix47::Physical;
    CoverResolver resolver(covers);const auto cover=resolver.resolve(game);
    game.cover_path=cover.path;game.cover_kind=cover.path.empty()?GameCoverKind::None:
        cover.is_full_cover?GameCoverKind::FullCover:GameCoverKind::FrontOnly;
    resolver.resolve_inspection_art(game);
    // A tray change while reading must not publish mixed title/artwork data.
    const auto after=probe();
    if(after==DiscAccessFix47::None || after==DiscAccessFix47::Emulated ||
       after!=access || normalize_id(read_param_sfo_string(sfo,"TITLE_ID"))!=id)return result;
    result.present=true;result.game=std::move(game);return result;
}
bool same_disc_sample_fix47(const DiscSampleFix47& a,const DiscSampleFix47& b){
    return a.present==b.present && (!a.present ||
        (a.game.title_id==b.game.title_id && a.game.physical_disc==b.game.physical_disc));
}
DiscCatalogChangeFix47 sync_disc_catalog_fix47(std::vector<GameEntry>& games,const DiscSampleFix47& sample){
    const auto disc=std::find_if(games.begin(),games.end(),[](const GameEntry& g){return g.source==GameSource::Disc;});
    DiscSampleFix47 previous;if(disc!=games.end()){previous.present=true;previous.game=*disc;}
    if(same_disc_sample_fix47(previous,sample))return {};
    DiscCatalogChangeFix47 change;change.changed=true;
    if(disc!=games.end()){
        change.index=int(disc-games.begin());
        if(sample.present)*disc=sample.game;
        else{change.preserves_indices=disc+1==games.end();games.erase(disc);}
    }else if(sample.present){change.index=int(games.size());games.push_back(sample.game);}
    return change;
}
#endif
