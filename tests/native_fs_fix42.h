#pragma once
// Host implementation of the sysFs ABI for exercising the actual native
// file-manager branch. This does not emulate a console or an RSX driver.
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <unordered_map>
#include <string>
#include <cstdint>
using s32=std::int32_t;using u32=std::uint32_t;using u64=std::uint64_t;
constexpr s32 SYS_O_RDONLY=0,SYS_O_WRONLY=1,SYS_O_CREAT=64,SYS_O_EXCL=128;
struct sysFSStat {s32 mode;std::int64_t mtime,ctime,size;};
struct sysFSDirent {unsigned char d_namlen;char d_name[1024];};
inline unsigned native_opens=0,native_reads=0,native_fstats=0,native_mkdirs=0;inline std::string native_free_device;
inline std::unordered_map<int,DIR*> native_dirs;inline int native_next_dir=10000;
inline s32 sysFsOpen(const char* p,s32 flags,s32* fd,const void*,u64){
    ++native_opens;if(std::string(p).find("blockopen.bin")!=std::string::npos)return s32(0x80010009u);
    const int result=::open(p,flags,0644);*fd=result;return result<0?errno:0;
}
inline s32 sysFsClose(s32 fd){return ::close(fd);}
inline s32 sysFsRead(s32 fd,void* p,u64 n,u64* got){++native_reads;const auto r=::read(fd,p,n);*got=r<0?0:u64(r);return r<0?errno:0;}
inline s32 sysFsWrite(s32 fd,const void* p,u64 n,u64* got){const auto r=::write(fd,p,n);*got=r<0?0:u64(r);return r<0?errno:0;}
inline void native_stat_fill(const struct stat& from,sysFSStat* to){to->mode=from.st_mode;to->size=from.st_size;to->mtime=from.st_mtime;to->ctime=from.st_ctime;}
inline s32 sysFsStat(const char* p,sysFSStat* to){struct stat s{};if(lstat(p,&s))return errno;native_stat_fill(s,to);return 0;}
inline s32 sysFsFstat(s32 fd,sysFSStat* to){++native_fstats;struct stat s{};if(fstat(fd,&s))return errno;native_stat_fill(s,to);
    // Deliberately separate descriptor timestamps from path timestamps.
    to->mtime+=123;to->ctime+=123;return 0;
}
inline s32 sysFsOpendir(const char* p,s32* fd){DIR* d=opendir(p);if(!d)return errno;*fd=++native_next_dir;native_dirs[*fd]=d;return 0;}
inline s32 sysFsReaddir(s32 fd,sysFSDirent* to,u64* count){auto* e=readdir(native_dirs.at(fd));*count=e?1:0;if(e){to->d_namlen=std::strlen(e->d_name);std::strcpy(to->d_name,e->d_name);}return 0;}
inline s32 sysFsClosedir(s32 fd){const int r=closedir(native_dirs.at(fd));native_dirs.erase(fd);return r;}
inline s32 sysFsGetFreeSize(const char* root,u32* block,u64* blocks){native_free_device=root;struct statvfs s{};if(statvfs(root,&s))return errno;*block=s.f_frsize;*blocks=s.f_bavail;return 0;}
inline s32 sysLv2FsFsync(s32 fd){return ::fsync(fd);}
inline s32 sysLv2FsRename(const char* a,const char* b){return ::rename(a,b);}
inline s32 sysLv2FsUnlink(const char* p){return ::unlink(p);}

inline s32 sysFsMkdir(const char* p,s32 mode){++native_mkdirs;return ::mkdir(p,mode);}
