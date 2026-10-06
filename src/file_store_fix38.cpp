#include "file_store_fix38.h"
#ifdef PS3_GAME_ORBIT_FIX38
#include <cerrno>
#include <sys/stat.h>
namespace FileStoreFix38 {
namespace {
bool regular(const std::string& path){struct stat st{};return stat(path.c_str(),&st)==0 && S_ISREG(st.st_mode);}
}
FILE* open_read(const std::string& path){
    auto* file=std::fopen(path.c_str(),"rb");
    return file ? file : std::fopen((path+".bak").c_str(),"rb");
}
bool commit(const std::string& temporary,const std::string& path){
    const auto backup=path+".bak";
    bool moved=false;
    if(regular(path)){
        if(std::remove(backup.c_str())!=0 && errno!=ENOENT)return false;
        if(std::rename(path.c_str(),backup.c_str())!=0)return false;
        moved=true;
    }
    if(std::rename(temporary.c_str(),path.c_str())==0)return true;
    const int error=errno;
    if(moved || (!regular(path) && regular(backup)))std::rename(backup.c_str(),path.c_str());
    errno=error;return false;
}
}
#endif
