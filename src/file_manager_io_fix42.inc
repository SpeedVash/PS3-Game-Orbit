#include "runtime_diag.h"
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <limits>
#include <sys/stat.h>
#ifdef __PSL1GHT__
#include <sys/file.h>
#include <lv2/sysfs.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/statvfs.h>
#endif
namespace {
constexpr std::size_t Slice=256u*1024,MaxFiles=4096;
bool safe_name(const std::string& s){return !s.empty()&&s!="."&&s!=".."&&s.find('/')==std::string::npos&&s.find('\0')==std::string::npos;}
std::string leaf(const std::string& p){return p.substr(p.find_last_of('/')+1);}
int open_file(const std::string& p,bool writing,int& error){
#ifdef __PSL1GHT__
    // Use the same sysFs descriptor API as read/write/fstat/close. Do not
    // conflate an open error with a pathname-versus-descriptor stat mismatch.
    s32 fd=-1;error=sysFsOpen(p.c_str(),writing?SYS_O_WRONLY|SYS_O_CREAT|SYS_O_EXCL:SYS_O_RDONLY,&fd,nullptr,0);
    return error==0?fd:-1;
#else
    int flags=writing?O_WRONLY|O_CREAT|O_EXCL:O_RDONLY;
#ifdef O_NOFOLLOW
    flags|=O_NOFOLLOW;
#endif
    const int fd=::open(p.c_str(),flags,0644);error=fd<0?errno:0;return fd;
#endif
}
bool close_file(int fd){if(fd<0)return true;
#ifdef __PSL1GHT__
    return sysFsClose(fd)==0;
#else
    return ::close(fd)==0;
#endif
}
bool sync_file(int fd){
#ifdef __PSL1GHT__
    return sysLv2FsFsync(fd)==0;
#else
    return ::fsync(fd)==0;
#endif
}
bool read_file(int fd,void* p,std::size_t length,std::uint64_t& count,int& error){
#ifdef __PSL1GHT__
    u64 n=0;error=sysFsRead(fd,p,length,&n);count=n;return error==0;
#else
    const auto n=::read(fd,p,length);error=n<0?errno:0;count=n<0?0:std::uint64_t(n);return n>=0;
#endif
}
bool write_file(int fd,const void* p,std::size_t length,std::uint64_t& count,int& error){
#ifdef __PSL1GHT__
    u64 n=0;error=sysFsWrite(fd,p,length,&n);count=n;return error==0;
#else
    const auto n=::write(fd,p,length);error=n<0?errno:0;count=n<0?0:std::uint64_t(n);return n>=0;
#endif
}
bool rename_file(const std::string& a,const std::string& b){
#ifdef __PSL1GHT__
    return sysLv2FsRename(a.c_str(),b.c_str())==0;
#else
    return ::rename(a.c_str(),b.c_str())==0;
#endif
}
bool remove_file(const std::string& p){
#ifdef __PSL1GHT__
    return sysLv2FsUnlink(p.c_str())==0;
#else
    return ::unlink(p.c_str())==0;
#endif
}
bool free_bytes(const std::string& directory,const std::string& device,std::uint64_t& bytes){
#ifdef __PSL1GHT__
    (void)directory;u32 block=0;u64 blocks=0;
    if(sysFsGetFreeSize(device.c_str(),&block,&blocks)!=0||!block||blocks>std::numeric_limits<std::uint64_t>::max()/block)return false;
    bytes=blocks*block;
#else
    (void)device;struct statvfs s{};if(statvfs(directory.c_str(),&s)!=0)return false;
    if(s.f_frsize&&std::uint64_t(s.f_bavail)>std::numeric_limits<std::uint64_t>::max()/s.f_frsize)return false;
    bytes=std::uint64_t(s.f_bavail)*s.f_frsize;
#endif
    return true;
}
}
