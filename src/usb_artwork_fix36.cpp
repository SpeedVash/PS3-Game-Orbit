#include "game_tools_fix35.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include "cover_resolver.h"
#include "image_decode.h"
#include "cover_limits_fix31.h"
#include "runtime_diag.h"
#include <array>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
namespace GameToolsFix35 {
namespace {
bool regular(const std::string& p){struct stat st{};return stat(p.c_str(),&st)==0 && S_ISREG(st.st_mode);}
bool exists(const std::string& p){struct stat st{};return stat(p.c_str(),&st)==0;}
bool valid_id(const std::string& id){if(id.size()!=9)return false;for(unsigned i=0;i<9;++i)if(i<4?id[i]<'A'||id[i]>'Z':id[i]<'0'||id[i]>'9')return false;return true;}
bool safe_stem(const std::string& stem){if(stem.empty() || stem.size()>236 || stem=="." || stem=="..")return false;for(unsigned char c:stem)if(c<32 || c==127 || c=='/' || c=='\\')return false;return true;}
bool stage_file(const std::string& path,const std::vector<std::uint8_t>& data){
    auto* f=std::fopen(path.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(data.data(),1,data.size(),f)==data.size() && std::fflush(f)==0;
    if(ok)ok=fsync(fileno(f))==0;
    if(std::fclose(f)!=0)ok=false;
    return ok;
}
}
ImportResult import_usb(const GameEntry& game,const std::vector<std::string>& roots,const std::string& covers){
    if(!game.title_id.empty() && !valid_id(game.title_id))return {false,"ID do jogo inválida."};
    std::vector<std::string> keys;if(valid_id(game.title_id))keys.push_back(game.title_id);
    if(game.format==GameFormat::ISO){const auto stem=CoverResolver::iso_stem(game.path);if(safe_stem(stem) && std::find(keys.begin(),keys.end(),stem)==keys.end())keys.push_back(stem);}
    if(keys.empty())return {false,"Use a ID do jogo ou o nome exato da ISO."};
    const auto target_key=keys.front();
    struct Role{const char* suffix;const char* label;GameCoverKind kind;unsigned bit;};
    const std::array<Role,3> roles{{{"","capa",GameCoverKind::FullCover,1},{"_INSIDE","interior",GameCoverKind::FullCover,2},{"_DISC","disco",GameCoverKind::FrontOnly,4}}};
    const std::array<const char*,6> exts{{".jpg",".jpeg",".png",".JPG",".JPEG",".PNG"}};
    DecodedImageRGBA decoded;unsigned mask=0,failed=0;bool any=false;
    for(const auto& role:roles){
        std::string source,extension;
        for(const auto& root:roots){for(const auto& key:keys){for(const auto* ext:exts){const auto path=root+"/PS3COVERS/"+key+role.suffix+ext;if(regular(path)){source=path;extension=ext;break;}}if(!source.empty())break;}if(!source.empty())break;}
        if(source.empty())continue;
        any=true;
        const auto image=load_cover_file(source,role.kind);std::string error;
        if(!image.valid() || image.width>8192 || image.height>8192 || std::size_t(image.width)*image.height>CoverLimitsFix31::MaximumDecodePixels || !decode_cover_rgba(image,decoded,error)){
            ++failed;RuntimeDiag::log("USB 1.3.3 rejected: role=%s path=%s error=%s",role.label,source.c_str(),error.c_str());continue;
        }
        if(mkdir(covers.c_str(),0777)!=0 && errno!=EEXIST){++failed;continue;}
        for(char& c:extension)if(c>='A' && c<='Z')c=char(c-'A'+'a');
        const auto base=covers+"/"+target_key+role.suffix,dest=base+extension,temp=dest+".orbit-new";
        if(exists(temp)){++failed;continue;}
        if(!stage_file(temp,image.encoded)){std::remove(temp.c_str());++failed;continue;}
        std::vector<std::pair<std::string,std::string>> backups;bool ok=true;
        for(const auto* ext:exts){const auto old=base+ext;if(!exists(old))continue;const auto backup=old+".orbit-old";if(exists(backup) || !regular(old) || std::rename(old.c_str(),backup.c_str())!=0){ok=false;break;}backups.emplace_back(old,backup);}
        if(ok)ok=std::rename(temp.c_str(),dest.c_str())==0;
        if(!ok){for(auto it=backups.rbegin();it!=backups.rend();++it)std::rename(it->second.c_str(),it->first.c_str());std::remove(temp.c_str());++failed;continue;}
        for(const auto& pair:backups)std::remove(pair.second.c_str());
        mask|=role.bit;
        RuntimeDiag::log("USB 1.3.3 updated: role=%s source=%s target=%s",role.label,source.c_str(),dest.c_str());
    }
    if(!any)return {false,"Nenhuma imagem encontrada em USB/PS3COVERS."};
    if(!mask)return {false,"As imagens encontradas não puderam ser atualizadas."};
    const unsigned count=unsigned(bool(mask&1))+unsigned(bool(mask&2))+unsigned(bool(mask&4));
    std::string message=std::to_string(count)+(count==1?" imagem atualizada.":" imagens atualizadas.");if(failed)message+=" "+std::to_string(failed)+" arquivo(s) com erro.";
    return {true,message,mask};
}
}
#endif
