#include "file_manager_fix40.h"
#if defined(PS3_GAME_ORBIT_FIX40) && !defined(PS3_GAME_ORBIT_FIX41)
#include <algorithm>
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
constexpr std::size_t Slice=256u*1024;
bool safe_name(const std::string& s){return !s.empty()&&s!="."&&s!=".."&&s.find('/')==std::string::npos&&s.find('\0')==std::string::npos;}
int open_file(const std::string& p,bool writing){
#ifdef __PSL1GHT__
    s32 fd=-1;const s32 flags=writing?SYS_O_WRONLY|SYS_O_CREAT|SYS_O_EXCL:SYS_O_RDONLY;
    return sysLv2FsOpen(p.c_str(),flags,&fd,0644,nullptr,0)==0?fd:-1;
#else
    int flags=writing?O_WRONLY|O_CREAT|O_EXCL:O_RDONLY;
#ifdef O_NOFOLLOW
    flags|=O_NOFOLLOW;
#endif
    return ::open(p.c_str(),flags,0644);
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
bool read_file(int fd,void* p,std::size_t length,std::uint64_t& count){
#ifdef __PSL1GHT__
    u64 n=0;const auto e=sysFsRead(fd,p,length,&n);count=n;return e==0;
#else
    const auto n=::read(fd,p,length);count=n<0?0:std::uint64_t(n);return n>=0;
#endif
}
bool write_file(int fd,const void* p,std::size_t length,std::uint64_t& count){
#ifdef __PSL1GHT__
    u64 n=0;const auto e=sysFsWrite(fd,p,length,&n);count=n;return e==0;
#else
    const auto n=::write(fd,p,length);count=n<0?0:std::uint64_t(n);return n>=0;
#endif
}
bool rename_file(const std::string& from,const std::string& to){
#ifdef __PSL1GHT__
    return sysLv2FsRename(from.c_str(),to.c_str())==0;
#else
    return ::rename(from.c_str(),to.c_str())==0;
#endif
}
bool remove_file(const std::string& path){
#ifdef __PSL1GHT__
    return sysLv2FsUnlink(path.c_str())==0;
#else
    return ::unlink(path.c_str())==0;
#endif
}
bool free_bytes(const std::string& path,std::uint64_t& bytes){
#ifdef __PSL1GHT__
    (void)path;u32 block=0;u64 blocks=0;if(sysFsGetFreeSize("/dev_hdd0",&block,&blocks)!=0||!block)return false;
    if(blocks>std::numeric_limits<std::uint64_t>::max()/block)return false;
    bytes=blocks*block;
#else
    struct statvfs s{};if(statvfs(path.c_str(),&s)!=0)return false;
    bytes=std::uint64_t(s.f_bavail)*s.f_frsize;
#endif
    return true;
}
}
bool FileManagerFix40::Snapshot::operator==(const Snapshot& o) const{return exists==o.exists&&regular==o.regular&&directory==o.directory&&size==o.size&&mtime==o.mtime&&ctime==o.ctime;}
FileManagerFix40::Snapshot FileManagerFix40::snapshot(const std::string& path,int fd){
    Snapshot out;
#ifdef __PSL1GHT__
    sysFSStat s{};if((fd>=0?sysFsFstat(fd,&s):sysFsStat(path.c_str(),&s))!=0)return out;
    out.regular=S_ISREG(s.mode);out.directory=S_ISDIR(s.mode);out.size=s.size<0?0:std::uint64_t(s.size);out.mtime=s.mtime;out.ctime=s.ctime;
#else
    struct stat s{};if((fd>=0?fstat(fd,&s):lstat(path.c_str(),&s))!=0)return out;
    out.regular=S_ISREG(s.st_mode);out.directory=S_ISDIR(s.st_mode);out.size=s.st_size<0?0:std::uint64_t(s.st_size);out.mtime=s.st_mtime;out.ctime=s.st_ctime;
#endif
    out.exists=true;return out;
}
FileManagerFix40::FileManagerFix40(std::string root):root_(std::move(root)),buffer_(Slice){while(root_.size()>1&&root_.back()=='/')root_.pop_back();}
FileManagerFix40::~FileManagerFix40(){cancel("");}
void FileManagerFix40::open(){cancel("");view_=View{};view_.active=true;view_.path=root_;view_.mode=Mode::Notice;view_.status="O fundo animado será pausado neste modo e voltará ao sair.";changed();}
void FileManagerFix40::close(){cancel("");view_.active=false;changed();}
void FileManagerFix40::refresh(){
    view_.entries.clear();view_.selected=0;bool ok=false;
    auto add=[&](const std::string& name){if(!safe_name(name)||view_.path.size()+name.size()+1>1023)return;const auto s=snapshot(view_.path+"/"+name);if(s.exists)view_.entries.push_back({name,s.directory,s.regular,s.size});};
#ifdef __PSL1GHT__
    s32 fd=-1;if(sysFsOpendir(view_.path.c_str(),&fd)==0){ok=true;sysFSDirent entry{};u64 n=0;
        while(sysFsReaddir(fd,&entry,&n)==0&&n){add(std::string(entry.d_name,entry.d_namlen));}
        sysFsClosedir(fd);
    }
#else
    if(DIR* dir=opendir(view_.path.c_str())){ok=true;while(auto* e=readdir(dir))add(e->d_name);closedir(dir);}
#endif
    std::sort(view_.entries.begin(),view_.entries.end(),[](const Entry& a,const Entry& b){return a.directory!=b.directory?a.directory>b.directory:a.name<b.name;});
    view_.status=ok?(view_.entries.empty()?"Pasta vazia.":"Selecione uma pasta ou arquivo."):"Não foi possível ler esta pasta.";changed();
}
void FileManagerFix40::cancel(const std::string& status){
    close_file(source_fd_);close_file(dest_fd_);source_fd_=dest_fd_=-1;
    if(owns_temp_&&!temporary_.empty())remove_file(temporary_);
    owns_temp_=false;temporary_.clear();view_.mode=Mode::Browse;view_.status=status;changed();
}
std::string FileManagerFix40::unique(const char* role){
    for(unsigned i=0;i<1024;++i){const auto p=view_.path+"/.orbit15_"+role+"_"+std::to_string(++serial_);if(p.size()>1023)return "";if(!snapshot(p).exists)return p;}
    return "";
}
void FileManagerFix40::paste(){
    if(view_.clipboard.empty()){view_.status="Copie um arquivo com Quadrado.";changed();return;}
    source_=view_.clipboard;const auto split=source_.find_last_of('/');const auto name=source_.substr(split+1);
    view_.destination=view_.path+"/"+name;
    source_snapshot_=snapshot(source_);dest_snapshot_=snapshot(view_.destination);
    if(!safe_name(name)||view_.destination.size()>1023||source_==view_.destination){view_.status="Escolha outra pasta para colar.";changed();return;}
    if(!source_snapshot_.regular){view_.status="O arquivo de origem não está disponível.";changed();return;}
    if(dest_snapshot_.exists&&!dest_snapshot_.regular){view_.status="Destino não é um arquivo regular.";changed();return;}
    if(dest_snapshot_.exists){view_.mode=Mode::Overwrite;view_.status="Já existe um arquivo com este nome. Substituir?";changed();return;}
    begin_copy();
}
void FileManagerFix40::begin_copy(){
    if(!(snapshot(source_)==source_snapshot_)||!(snapshot(view_.destination)==dest_snapshot_)){cancel("Origem ou destino mudou. Copie novamente.");return;}
    std::uint64_t free=0;if(!free_bytes(view_.path,free)||free<source_snapshot_.size){cancel("Não há espaço disponível para a cópia temporária.");return;}
    source_fd_=open_file(source_,false);
    if(source_fd_<0||!(snapshot(source_,source_fd_)==source_snapshot_)){cancel("Não foi possível abrir a origem.");return;}
    for(unsigned i=0;i<1024;++i){temporary_=unique("tmp");if(temporary_.empty())break;dest_fd_=open_file(temporary_,true);if(dest_fd_>=0){owns_temp_=true;break;}if(!snapshot(temporary_).exists)break;}
    if(!owns_temp_){cancel("Não foi possível criar a cópia temporária.");return;}
    view_.total=source_snapshot_.size;view_.done=0;view_.mode=Mode::Copy;view_.status="Copiando arquivo...";changed();
}
void FileManagerFix40::finish(){
    if(!(snapshot(source_)==source_snapshot_)||!(snapshot(source_,source_fd_)==source_snapshot_)||!(snapshot(view_.destination)==dest_snapshot_)){cancel("Origem ou destino mudou. Cópia cancelada.");return;}
    const bool synced=sync_file(dest_fd_);const bool closed=close_file(dest_fd_);dest_fd_=-1;
    if(!synced||!closed){cancel("Falha ao gravar a cópia.");return;}
    close_file(source_fd_);source_fd_=-1;backup_.clear();
    if(dest_snapshot_.exists){backup_=unique("bak");if(backup_.empty()||!rename_file(view_.destination,backup_)){cancel("Não foi possível preservar o arquivo anterior.");return;}}
    if(!rename_file(temporary_,view_.destination)){
        const bool restored=backup_.empty()||rename_file(backup_,view_.destination);
        cancel(restored?"Falha ao concluir. Arquivo anterior preservado.":"Falha ao concluir. Arquivo anterior em: "+backup_);return;
    }
    owns_temp_=false;temporary_.clear();const bool cleanup=backup_.empty()||remove_file(backup_);
    view_.mode=Mode::Browse;refresh();view_.status=cleanup?"Arquivo copiado com sucesso.":"Cópia concluída. Arquivo anterior em: "+backup_;changed();
}
void FileManagerFix40::step(){
    if(!active()||!copying())return;
    if(view_.done==view_.total){finish();return;}
    const auto length=std::size_t(std::min<std::uint64_t>(Slice,view_.total-view_.done));std::uint64_t n=0;
    if(!read_file(source_fd_,buffer_.data(),length,n)||!n||n>length){cancel("Falha na leitura. Cópia cancelada.");return;}
    std::uint64_t at=0;while(at<n){std::uint64_t wrote=0;if(!write_file(dest_fd_,buffer_.data()+at,std::size_t(n-at),wrote)||!wrote||wrote>n-at){cancel("Falha na gravação. Cópia cancelada.");return;}at+=wrote;}
    const auto percent=view_.total?100.0*double(view_.done)/double(view_.total):100.0;view_.done+=n;
    if(int(percent)!=int(100.0*double(view_.done)/double(view_.total)))changed();
}
void FileManagerFix40::update(const InputFrame& in,float dt){
    if(!active()||!in.connected)return;
    if(view_.mode==Mode::Notice){if(in.start.pressed||in.circle.pressed)close();else if(in.cross.pressed){view_.mode=Mode::Browse;refresh();}return;}
    if(copying()){if(in.circle.pressed||in.start.pressed){cancel("Cópia cancelada. Origem e destino anterior preservados.");if(in.start.pressed)close();}return;}
    if(view_.mode==Mode::Overwrite){if(in.start.pressed){close();return;}if(in.circle.pressed){view_.mode=Mode::Browse;view_.status="Substituição cancelada.";changed();}else if(in.cross.pressed)begin_copy();return;}
    if(in.start.pressed){close();return;}
    if(in.circle.pressed){if(view_.path==root_)close();else{view_.path=view_.path.substr(0,view_.path.find_last_of('/'));refresh();}return;}
    int delta=in.down.pressed?1:in.up.pressed?-1:0;
    const int held=in.down.held?1:in.up.held?-1:0;
    if(delta||held!=repeat_direction_){repeat_direction_=held;repeat_=0;}
    else if(held){repeat_+=std::clamp(dt,0.0f,.1f);if(repeat_>=.32f){delta=held;repeat_-=.085f;}}
    if(in.r1.pressed)delta=9;
    if(in.l1.pressed)delta=-9;
    if(delta&&!view_.entries.empty()){view_.selected=std::clamp(view_.selected+delta,0,int(view_.entries.size())-1);changed();}
    if(in.triangle.pressed){paste();return;}
    if(view_.entries.empty())return;
    const auto entry=view_.entries[std::size_t(view_.selected)];
    if(in.cross.pressed&&entry.directory){view_.path+="/"+entry.name;refresh();}
    else if(in.square.pressed){view_.status=entry.regular?"Arquivo copiado para a seleção. Triângulo para colar.":"Copiar pastas não está disponível nesta versão.";if(entry.regular)view_.clipboard=view_.path+"/"+entry.name;changed();}
}
#endif
