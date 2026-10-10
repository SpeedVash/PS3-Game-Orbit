#include "file_manager_fix41.h"
#ifdef PS3_GAME_ORBIT_FIX41
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
bool FileManagerFix41::Snapshot::operator==(const Snapshot& o) const{return exists==o.exists&&regular==o.regular&&directory==o.directory&&size==o.size&&mtime==o.mtime&&ctime==o.ctime;}
FileManagerFix41::Snapshot FileManagerFix41::snapshot(const std::string& path,int fd,int* error){
    Snapshot out;int e=0;
#ifdef __PSL1GHT__
    sysFSStat s{};e=fd>=0?sysFsFstat(fd,&s):sysFsStat(path.c_str(),&s);
    if(error)*error=e;
    if(e)return out;
    out.regular=S_ISREG(s.mode);out.directory=S_ISDIR(s.mode);out.size=s.size<0?0:std::uint64_t(s.size);out.mtime=s.mtime;out.ctime=s.ctime;
#else
    struct stat s{};e=fd>=0?fstat(fd,&s):lstat(path.c_str(),&s);
    if(error)*error=e?errno:0;
    if(e)return out;
    out.regular=S_ISREG(s.st_mode);out.directory=S_ISDIR(s.st_mode);out.size=s.st_size<0?0:std::uint64_t(s.st_size);out.mtime=s.st_mtime;out.ctime=s.st_ctime;
#endif
    out.exists=true;return out;
}
FileManagerFix41::FileManagerFix41(std::string root):FileManagerFix41(std::vector<std::string>{root}){discover_usb_=root=="/dev_hdd0";}
FileManagerFix41::FileManagerFix41(std::vector<std::string> roots):roots_(std::move(roots)),buffer_(Slice){for(auto& p:roots_)while(p.size()>1&&p.back()=='/')p.pop_back();}
FileManagerFix41::~FileManagerFix41(){release_copy();}
void FileManagerFix41::discover(){
    devices_.clear();auto roots=roots_;
    if(discover_usb_)for(int n=0;n<128;++n){char p[24];std::snprintf(p,sizeof(p),"/dev_usb%03d",n);roots.emplace_back(p);}
    for(const auto& p:roots)if(snapshot(p).directory&&std::none_of(devices_.begin(),devices_.end(),[&](const Device& d){return d.path==p;})){
        const bool usb=leaf(p).rfind("dev_usb",0)==0;
        devices_.push_back({p,usb?"USB "+leaf(p).substr(7):p=="/dev_hdd0"?"HD interno":leaf(p),usb});
    }
}
const FileManagerFix41::Device* FileManagerFix41::device_for(const std::string& p) const{
    for(const auto& d:devices_)if(p==d.path||(p.size()>d.path.size()&&p.compare(0,d.path.size(),d.path)==0&&p[d.path.size()]=='/'))return &d;
    return nullptr;
}
void FileManagerFix41::open(){
    release_copy();view_=View{};view_.active=true;view_.mode=Mode::Notice;marked_.clear();copied_.clear();jobs_.clear();batch_active_=false;
    discover();for(int i=0;i<2;++i)view_.panels[i].path=devices_.empty()?"":devices_[i&&devices_.size()>1?1:0].path;
    view_.status="O fundo animado será pausado e voltará ao sair.";changed();
}
void FileManagerFix41::close(){release_copy();batch_active_=false;view_.active=false;changed();}
void FileManagerFix41::flags(){
    view_.marked_count=unsigned(marked_.size());
    for(auto& p:view_.panels)for(auto& e:p.entries){const auto full=p.path+"/"+e.name;e.marked=marked_.count(full);e.copied=copied_.count(full);}
}
void FileManagerFix41::refresh(int index,bool keep){
    auto& p=view_.panels[index];const std::string previous=keep&&!p.entries.empty()?p.entries[std::size_t(p.selected)].name:"";
    p.entries.clear();p.selected=0;bool ok=false;
    if(p.path.empty()){
        discover();for(const auto& d:devices_)p.entries.push_back({d.label,true,false,0,false,false});ok=true;
    }else{
        auto add=[&](const std::string& name){if(!safe_name(name)||p.path.size()+name.size()+1>1023||name.rfind(".orbit151_",0)==0)return;
            const auto s=snapshot(p.path+"/"+name);if(s.exists)p.entries.push_back({name,s.directory,s.regular,s.size,false,false});};
#ifdef __PSL1GHT__
        s32 fd=-1;if(sysFsOpendir(p.path.c_str(),&fd)==0){ok=true;sysFSDirent entry{};u64 n=0;
            while(sysFsReaddir(fd,&entry,&n)==0&&n)add(std::string(entry.d_name,entry.d_namlen));
            sysFsClosedir(fd);}
#else
        if(DIR* dir=opendir(p.path.c_str())){ok=true;while(auto* e=readdir(dir))add(e->d_name);closedir(dir);}
#endif
        std::sort(p.entries.begin(),p.entries.end(),[](const Entry& a,const Entry& b){return a.directory!=b.directory?a.directory>b.directory:a.name<b.name;});
    }
    for(std::size_t i=0;i<p.entries.size();++i)if(p.entries[i].name==previous)p.selected=int(i);
    if(!ok)view_.status="Não foi possível ler a pasta. R3 para atualizar os dispositivos.";
    flags();changed();
}
void FileManagerFix41::release_copy(){
    close_file(source_fd_);close_file(dest_fd_);source_fd_=dest_fd_=-1;
    if(owns_temp_&&!temporary_.empty())remove_file(temporary_);
    owns_temp_=false;temporary_.clear();
}
void FileManagerFix41::cancel(const std::string& status){release_copy();batch_active_=false;view_.mode=Mode::Browse;
    if(active()){refresh(0,true);refresh(1,true);}view_.status=status;changed();}
void FileManagerFix41::failure(const char* action,int error){
    char code[32];std::snprintf(code,sizeof(code)," (0x%08x)",unsigned(error));
    RuntimeDiag::log("FILE 1.5.1: action=%s code=0x%08x source=%s destination=%s",action,unsigned(error),view_.source.c_str(),view_.destination.c_str());
    cancel(std::string(action)+(error?code:""));
}
std::string FileManagerFix41::unique(const char* role){
    const auto dir=view_.destination.substr(0,view_.destination.find_last_of('/'));
    for(unsigned i=0;i<1024;++i){const auto p=dir+"/.orbit151_"+role+"_"+std::to_string(++serial_);if(p.size()>1023)return "";if(!snapshot(p).exists)return p;}
    return "";
}
void FileManagerFix41::start_batch(){
    const auto& from=view_.panels[view_.active_panel];const auto& to=view_.panels[1-view_.active_panel];
    if(from.path.empty()||to.path.empty()||!device_for(to.path)||!snapshot(to.path).directory){view_.status="Abra a pasta de origem e a pasta de destino no outro painel.";changed();return;}
    std::vector<std::string> paths(marked_.begin(),marked_.end());std::sort(paths.begin(),paths.end());
    if(paths.empty()&&!from.entries.empty()){const auto& e=from.entries[std::size_t(from.selected)];if(e.regular)paths.push_back(from.path+"/"+e.name);}
    if(paths.empty()){view_.status="Marque arquivos com Quadrado ou selecione um arquivo.";changed();return;}
    jobs_.clear();std::unordered_set<std::string> targets;std::uint64_t total=0;
    for(const auto& p:paths){const auto s=snapshot(p);const auto name=leaf(p);const auto dest=to.path+"/"+name;
        if(!device_for(p)||!s.regular||!safe_name(name)||dest.size()>1023||p==dest){view_.status="Origem indisponível ou destino igual à origem: "+name;changed();return;}
        if(!targets.insert(dest).second){view_.status="Há nomes repetidos na seleção. Copie cada pasta separadamente.";changed();return;}
        if(total>std::numeric_limits<std::uint64_t>::max()-s.size){view_.status="Seleção excede o tamanho permitido.";changed();return;}total+=s.size;
        jobs_.push_back({p,dest,s});
    }
    job_=0;overwrite_all_=0;batch_active_=true;view_.mode=Mode::Copy;view_.batch_done=0;view_.batch_total=total;
    view_.file_count=unsigned(jobs_.size());view_.copied_count=view_.skipped_count=0;next();
}
void FileManagerFix41::batch_finished(){
    batch_active_=false;view_.mode=Mode::Browse;refresh(0,true);refresh(1,true);
    view_.status=std::to_string(view_.copied_count)+" arquivo(s) copiado(s), "+std::to_string(view_.skipped_count)+" ignorado(s).";changed();
}
void FileManagerFix41::next(){
    if(job_>=jobs_.size()){batch_finished();return;}
    const auto& j=jobs_[job_];view_.source=j.source;view_.destination=j.destination;view_.total=j.source_snapshot.size;view_.done=0;view_.file_index=unsigned(job_+1);
    if(!(snapshot(j.source)==j.source_snapshot)){cancel("A origem mudou ou foi desconectada. Marque novamente.");return;}
    dest_snapshot_=snapshot(j.destination);
    if(dest_snapshot_.exists&&!dest_snapshot_.regular){cancel("Já existe uma pasta ou outro tipo de item no destino.");return;}
    if(dest_snapshot_.exists&&overwrite_all_==2){skip();return;}
    if(dest_snapshot_.exists&&overwrite_all_!=1){view_.mode=Mode::Overwrite;view_.overwrite_selected=0;view_.status="Arquivo existente: "+leaf(j.destination);changed();return;}
    begin_copy();
}
void FileManagerFix41::skip(){view_.batch_done+=jobs_[job_].source_snapshot.size;++view_.skipped_count;++job_;view_.mode=Mode::Copy;changed();}
void FileManagerFix41::begin_copy(){
    const auto& j=jobs_[job_];const auto* device=device_for(j.destination);
    if(!device||!(snapshot(j.source)==j.source_snapshot)||!(snapshot(j.destination)==dest_snapshot_)){cancel("Origem ou destino mudou. Marque novamente.");return;}
    if(device->usb&&j.source_snapshot.size>0xffffffffULL){cancel("USB FAT32: este arquivo ultrapassa o limite de 4 GB.");return;}
    const auto directory=j.destination.substr(0,j.destination.find_last_of('/'));std::uint64_t free=0;
    if(!free_bytes(directory,device->path,free)||free<j.source_snapshot.size){cancel("Não há espaço disponível no dispositivo de destino.");return;}
    int error=0;source_fd_=open_file(j.source,false,error);if(source_fd_<0){failure("Não foi possível abrir a origem.",error);return;}
    source_fd_snapshot_=snapshot(j.source,source_fd_,&error);
    if(!source_fd_snapshot_.exists){failure("Não foi possível verificar a origem aberta.",error);return;}
    if(!source_fd_snapshot_.regular||source_fd_snapshot_.size!=j.source_snapshot.size||!(snapshot(j.source)==j.source_snapshot)){
        cancel("A origem mudou durante a abertura.");return;
    }
    // Path and descriptor timestamps have separate baselines. A successful
    // open must not be reported as an open failure because they differ.
    for(unsigned i=0;i<1024;++i){temporary_=unique("tmp");if(temporary_.empty())break;dest_fd_=open_file(temporary_,true,error);
        if(dest_fd_>=0){owns_temp_=true;break;}if(!snapshot(temporary_).exists)break;}
    if(!owns_temp_){failure("Não foi possível criar a cópia temporária.",error);return;}
    view_.mode=Mode::Copy;view_.status="Copiando "+leaf(j.source);changed();
}
void FileManagerFix41::finish(){
    const auto& j=jobs_[job_];
    if(!(snapshot(j.source)==j.source_snapshot)||!(snapshot(j.source,source_fd_)==source_fd_snapshot_)||!(snapshot(j.destination)==dest_snapshot_)){
        cancel("Origem ou destino mudou. Cópia cancelada.");return;
    }
    const bool synced=sync_file(dest_fd_),closed=close_file(dest_fd_);dest_fd_=-1;
    if(!synced||!closed){cancel("Falha ao gravar a cópia.");return;}close_file(source_fd_);source_fd_=-1;backup_.clear();
    if(dest_snapshot_.exists){backup_=unique("bak");if(backup_.empty()||!rename_file(j.destination,backup_)){cancel("Não foi possível preservar o arquivo anterior.");return;}}
    if(!rename_file(temporary_,j.destination)){const bool restored=backup_.empty()||rename_file(backup_,j.destination);
        cancel(restored?"Falha ao concluir. Arquivo anterior preservado.":"Arquivo anterior preservado em: "+backup_);return;}
    owns_temp_=false;temporary_.clear();
    if(!backup_.empty()&&!remove_file(backup_)){cancel("Cópia concluída; arquivo anterior preservado em: "+backup_);return;}
    copied_.insert(j.source);copied_.insert(j.destination);marked_.erase(j.source);++view_.copied_count;view_.batch_done+=j.source_snapshot.size;++job_;
    refresh(0,true);refresh(1,true);view_.mode=Mode::Copy;view_.status="Arquivo copiado. Preparando o próximo...";changed();
    if(job_==jobs_.size())batch_finished();
}
void FileManagerFix41::step(){
    if(!active()||!batch_active_||view_.mode==Mode::Overwrite)return;
    if(source_fd_<0){next();return;}if(view_.done==view_.total){finish();return;}
    const auto length=std::size_t(std::min<std::uint64_t>(Slice,view_.total-view_.done));std::uint64_t n=0;int error=0;
    if(!read_file(source_fd_,buffer_.data(),length,n,error)||!n||n>length){failure("Falha na leitura; cópia cancelada.",error);return;}
    std::uint64_t at=0;while(at<n){std::uint64_t written=0;
        if(!write_file(dest_fd_,buffer_.data()+at,std::size_t(n-at),written,error)||!written||written>n-at){failure("Falha na gravação; cópia cancelada.",error);return;}at+=written;}
    const auto before=view_.total?100.0*double(view_.done)/double(view_.total):100.0;view_.done+=n;
    if(int(before)!=int(100.0*double(view_.done)/double(view_.total)))changed();
}
void FileManagerFix41::update(const InputFrame& in,float dt){
    if(!active()||!in.connected)return;
    if(in.start.pressed){close();return;}
    if(view_.mode==Mode::Notice){if(in.circle.pressed)close();else if(in.cross.pressed){view_.mode=Mode::Browse;refresh(0);refresh(1);view_.status="Quadrado marca; Triângulo copia para o outro painel.";changed();}return;}
    if(view_.mode==Mode::Overwrite){
        if(in.circle.pressed){cancel("Lote cancelado. Arquivos já concluídos permanecem copiados.");return;}
        if(in.up.pressed)view_.overwrite_selected=(view_.overwrite_selected+4)%5;
        if(in.down.pressed)view_.overwrite_selected=(view_.overwrite_selected+1)%5;
        if(in.up.pressed||in.down.pressed)changed();
        if(in.cross.pressed){const int selected=view_.overwrite_selected;
            if(selected==4)cancel("Lote cancelado.");else if(selected==1||selected==3){if(selected==3)overwrite_all_=2;skip();}
            else{if(selected==2)overwrite_all_=1;begin_copy();}}
        return;
    }
    if(batch_active_){if(in.circle.pressed)cancel("Lote cancelado. Arquivos já concluídos permanecem copiados.");return;}
    if(in.left.pressed||in.l2.pressed){view_.active_panel=0;changed();}
    if(in.right.pressed||in.r2.pressed){view_.active_panel=1;changed();}
    auto& p=view_.panels[view_.active_panel];
    if(in.r3.pressed){discover();refresh(0,true);refresh(1,true);view_.status="Pastas e dispositivos atualizados.";changed();return;}
    if(in.circle.pressed){if(p.path.empty()){close();return;}const auto* device=device_for(p.path);
        p.path=!device||p.path==device->path?"":p.path.substr(0,p.path.find_last_of('/'));refresh(view_.active_panel);return;}
    int delta=in.down.pressed?1:in.up.pressed?-1:0;const int held=in.down.held?1:in.up.held?-1:0;
    if(delta||held!=repeat_direction_){repeat_direction_=held;repeat_=0;}else if(held){repeat_+=std::clamp(dt,0.0f,.1f);if(repeat_>=.32f){delta=held;repeat_-=.085f;}}
    if(in.r1.pressed)delta=8;
    if(in.l1.pressed)delta=-8;
    if(delta&&!p.entries.empty()){p.selected=std::clamp(p.selected+delta,0,int(p.entries.size())-1);changed();}
    if(in.triangle.pressed){start_batch();return;}
    if(in.select.pressed&&!p.path.empty()){
        const bool all=std::all_of(p.entries.begin(),p.entries.end(),[](const Entry& e){return !e.regular||e.marked;});
        for(const auto& e:p.entries)if(e.regular){const auto path=p.path+"/"+e.name;if(all)marked_.erase(path);else if(marked_.size()<MaxFiles)marked_.insert(path);}
        flags();view_.status=all?"Seleção da pasta removida.":"Arquivos da pasta selecionados.";changed();return;
    }
    if(p.entries.empty())return;
    const auto e=p.entries[std::size_t(p.selected)];
    if(in.cross.pressed&&e.directory){
        if(p.path.empty()){discover();for(const auto& d:devices_)if(d.label==e.name){p.path=d.path;break;}}
        else p.path+="/"+e.name;
        refresh(view_.active_panel);return;
    }
    if(in.square.pressed){if(!e.regular)view_.status="Marque arquivos; pastas servem para navegar.";
        else{const auto full=p.path+"/"+e.name;if(marked_.count(full))marked_.erase(full);else if(marked_.size()<MaxFiles)marked_.insert(full);
            view_.status="Triângulo copia a seleção para a pasta do outro painel.";flags();}changed();}
}
#endif
