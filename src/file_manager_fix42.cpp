#include "file_manager_fix42.h"
#ifdef PS3_GAME_ORBIT_FIX42
#include "file_manager_io_fix42.inc"
namespace {
constexpr std::size_t MaxJobs=16384,MaxPlanBytes=8u*1024*1024;
bool within(const std::string& path,const std::string& parent){
    return path==parent||(path.size()>parent.size()&&path.compare(0,parent.size(),parent)==0&&path[parent.size()]=='/');
}
bool open_directory(const std::string& p,int& fd,void*& dir,int& error){
#ifdef __PSL1GHT__
    (void)dir;s32 handle=-1;error=sysFsOpendir(p.c_str(),&handle);fd=handle;return error==0;
#else
    (void)fd;dir=opendir(p.c_str());error=dir?0:errno;return dir;
#endif
}
bool read_directory(int fd,void* dir,std::string& name,bool& end,int& error){
#ifdef __PSL1GHT__
    (void)dir;sysFSDirent e{};u64 count=0;error=sysFsReaddir(fd,&e,&count);end=count==0;
    if(error||end)return error==0;
    name.assign(e.d_name,e.d_namlen);
#else
    (void)fd;errno=0;auto* e=readdir(static_cast<DIR*>(dir));error=e?0:errno;end=!e;
    if(error||end)return error==0;
    name=e->d_name;
#endif
    return true;
}
void close_directory(int& fd,void*& dir){
#ifdef __PSL1GHT__
    if(fd>=0)sysFsClosedir(fd);
#else
    if(dir)closedir(static_cast<DIR*>(dir));
#endif
    fd=-1;dir=nullptr;
}
bool make_directory(const std::string& path,int& error){
#ifdef __PSL1GHT__
    error=sysFsMkdir(path.c_str(),0755);return error==0;
#else
    const int r=mkdir(path.c_str(),0755);error=r?errno:0;return r==0;
#endif
}
}
bool FileManagerFix42::Snapshot::operator==(const Snapshot& o) const{
    return exists==o.exists&&regular==o.regular&&directory==o.directory&&size==o.size&&mtime==o.mtime&&ctime==o.ctime;
}
FileManagerFix42::Snapshot FileManagerFix42::snapshot(const std::string& path,int fd,int* error){
    Snapshot out;
#ifdef __PSL1GHT__
    sysFSStat s{};const int e=fd>=0?sysFsFstat(fd,&s):sysFsStat(path.c_str(),&s);if(error)*error=e;if(e)return out;
    out.regular=S_ISREG(s.mode);out.directory=S_ISDIR(s.mode);out.size=s.size<0?0:std::uint64_t(s.size);out.mtime=s.mtime;out.ctime=s.ctime;
#else
    struct stat s{};const int e=fd>=0?fstat(fd,&s):lstat(path.c_str(),&s);if(error)*error=e?errno:0;if(e)return out;
    out.regular=S_ISREG(s.st_mode);out.directory=S_ISDIR(s.st_mode);out.size=s.st_size<0?0:std::uint64_t(s.st_size);out.mtime=s.st_mtime;out.ctime=s.st_ctime;
#endif
    out.exists=true;return out;
}
FileManagerFix42::FileManagerFix42(std::string root):FileManagerFix42(std::vector<std::string>{root}){discover_usb_=root=="/dev_hdd0";}
FileManagerFix42::FileManagerFix42(std::vector<std::string> roots):roots_(std::move(roots)),buffer_(Slice){
    for(auto& p:roots_)while(p.size()>1&&p.back()=='/')p.pop_back();
}
FileManagerFix42::~FileManagerFix42(){release_scan();release_copy();}
void FileManagerFix42::discover(){
    devices_.clear();auto paths=roots_;
    if(discover_usb_)for(int n=0;n<128;++n){char p[24];std::snprintf(p,sizeof(p),"/dev_usb%03d",n);paths.emplace_back(p);}
    for(const auto& p:paths)if(snapshot(p).directory&&std::none_of(devices_.begin(),devices_.end(),[&](const Device& d){return d.path==p;})){
        const bool usb=leaf(p).rfind("dev_usb",0)==0;
        devices_.push_back({p,usb?"USB "+leaf(p).substr(7):p=="/dev_hdd0"?"HD interno":leaf(p),usb});
    }
    view_.devices=devices_;
}
const FileManagerFix42::Device* FileManagerFix42::device_for(const std::string& p) const{
    for(const auto& d:devices_)if(within(p,d.path))return &d;
    return nullptr;
}
void FileManagerFix42::open(){
    release_scan();release_copy();view_=View{};view_.active=true;marked_.clear();copied_.clear();jobs_.clear();pending_.clear();batch_active_=false;
    discover();for(int i=0;i<2;++i)view_.panels[i].path=devices_.empty()?"":devices_[i&&devices_.size()>1?1:0].path;
    view_.status="O fundo animado será pausado e voltará ao sair.";changed();
}
void FileManagerFix42::close(){release_scan();release_copy();batch_active_=false;pending_.clear();view_.active=false;changed();}
void FileManagerFix42::flags(){
    view_.marked_count=unsigned(marked_.size());
    if(marked_.empty()){view_.selection_panel=-1;view_.selection_path.clear();}
    for(auto& p:view_.panels)for(auto& e:p.entries){const auto full=p.path+"/"+e.name;e.marked=marked_.count(full);e.copied=copied_.count(full);}
}
bool FileManagerFix42::entries(const std::string& path,std::vector<Entry>& out,int& error){
    int fd=-1;void* dir=nullptr;if(!open_directory(path,fd,dir,error))return false;
    bool end=false,ok=true;std::string name;
    while(ok&&!end){
        ok=read_directory(fd,dir,name,end,error);if(!ok||end)break;
        if(!safe_name(name)||name.rfind(".orbit152_",0)==0||name.rfind(".orbit151_",0)==0)continue;
        const auto s=snapshot(path+"/"+name);
        if(s.exists)out.push_back({name,s.directory,s.regular,s.size,false,false});
    }
    close_directory(fd,dir);
    std::sort(out.begin(),out.end(),[](const Entry& a,const Entry& b){return a.directory!=b.directory?a.directory>b.directory:a.name<b.name;});
    return ok;
}
void FileManagerFix42::refresh(int index,bool keep){
    auto& p=view_.panels[index];const auto old=keep&&!p.entries.empty()?p.entries[std::size_t(p.selected)].name:"";
    p.entries.clear();p.selected=0;int error=0;
    if(p.path.empty()){for(const auto& d:devices_)p.entries.push_back({d.label,true,false,0,false,false});}
    else if(!entries(p.path,p.entries,error))view_.status="Não foi possível ler a pasta. Use R3 para escolher o dispositivo.";
    for(std::size_t i=0;i<p.entries.size();++i)if(p.entries[i].name==old)p.selected=int(i);
    flags();changed();
}
void FileManagerFix42::clear_selection(){marked_.clear();flags();changed();}
bool FileManagerFix42::bind_selection(){
    const auto& p=view_.panels[view_.active_panel];
    if(p.path.empty())return false;
    if(!marked_.empty()&&(view_.selection_panel!=view_.active_panel||view_.selection_path!=p.path)){
        view_.status="A seleção pertence ao outro painel. Use Triângulo → Limpar seleção.";changed();return false;
    }
    view_.selection_panel=view_.active_panel;view_.selection_path=p.path;return true;
}
void FileManagerFix42::mark_one(){
    const auto& p=view_.panels[view_.active_panel];if(p.entries.empty())return;const auto& e=p.entries[std::size_t(p.selected)];
    if(!e.regular&&!e.directory){view_.status="Este tipo de item não pode ser copiado.";changed();return;}
    if(!bind_selection())return;
    const auto path=p.path+"/"+e.name;
    if(marked_.count(path))marked_.erase(path);else if(marked_.size()<MaxFiles)marked_.insert(path);
    else view_.status="Limite de 4096 itens marcados atingido.";
    flags();view_.status=marked_.empty()?"Seleção removida.":"Origem fixada. Abra o destino no outro painel e use Triângulo.";changed();
}
void FileManagerFix42::mark_all(){
    if(!bind_selection())return;
    const auto& p=view_.panels[view_.active_panel];std::size_t count=0;for(const auto& e:p.entries)if(e.regular||e.directory)++count;
    if(count>MaxFiles){view_.status="Esta pasta excede 4096 itens. Marque um lote menor.";changed();return;}
    for(const auto& e:p.entries)if(e.regular||e.directory)marked_.insert(p.path+"/"+e.name);
    flags();view_.status="Arquivos e pastas da origem marcados.";changed();
}
void FileManagerFix42::navigate(const std::string& path){
    const bool cleared=!marked_.empty()&&view_.selection_panel==view_.active_panel&&path!=view_.selection_path;
    if(cleared)clear_selection();
    view_.panels[view_.active_panel].path=path;refresh(view_.active_panel);
    view_.status=cleared?"Nova pasta de origem: a seleção anterior foi limpa.":"Quadrado marca; Triângulo abre as ações.";changed();
}
void FileManagerFix42::release_scan(){close_directory(scan_fd_,scan_dir_);scanning_active_=false;}
void FileManagerFix42::release_copy(){
    close_file(source_fd_);close_file(dest_fd_);source_fd_=dest_fd_=-1;
    if(owns_temp_&&!temporary_.empty())remove_file(temporary_);
    owns_temp_=false;temporary_.clear();
}
void FileManagerFix42::cancel(const std::string& status){
    release_scan();release_copy();pending_.clear();batch_active_=false;view_.cancelled=true;view_.mode=Mode::Summary;
    refresh(0,true);refresh(1,true);view_.status=status;changed();
}
void FileManagerFix42::failure(const char* action,int error){
    ++view_.error_count;char code[32];std::snprintf(code,sizeof(code)," (0x%08x)",unsigned(error));
    RuntimeDiag::log("FILE 1.5.2: action=%s code=0x%08x source=%s destination=%s",action,unsigned(error),view_.source.c_str(),view_.destination.c_str());
    cancel(std::string(action)+(error?code:""));view_.cancelled=false;
}
bool FileManagerFix42::queue(Task task){
    if(jobs_.size()+pending_.size()>=MaxJobs||plan_bytes_+task.source.size()+task.destination.size()>MaxPlanBytes){
        failure("A pasta excede o limite de preparação. Copie subpastas em lotes menores.",0);return false;
    }
    plan_bytes_+=task.source.size()+task.destination.size();pending_.push_back(std::move(task));return true;
}
void FileManagerFix42::prepare(){
    const int origin=marked_.empty()?view_.active_panel:view_.selection_panel;
    const auto& from=view_.panels[origin];const auto& to=view_.panels[1-origin];
    if(from.path.empty()||to.path.empty()||!device_for(to.path)||!snapshot(to.path).directory){
        view_.mode=Mode::Browse;view_.status="Abra uma pasta de origem e uma pasta de destino no outro painel.";changed();return;
    }
    std::vector<std::string> paths(marked_.begin(),marked_.end());std::sort(paths.begin(),paths.end());
    if(paths.empty()&&!from.entries.empty()){const auto& e=from.entries[std::size_t(from.selected)];if(e.regular||e.directory)paths.push_back(from.path+"/"+e.name);}
    if(paths.empty()){view_.mode=Mode::Browse;view_.status="Marque arquivos ou pastas com Quadrado.";changed();return;}
    release_scan();release_copy();jobs_.clear();pending_.clear();root_items_=paths;root_ok_.assign(paths.size(),true);plan_bytes_=0;job_=0;
    view_.source_panel=origin;view_.source_directory=from.path;view_.destination_directory=to.path;
    view_.file_count=view_.folder_count=view_.directories_done=view_.copied_count=view_.skipped_count=view_.error_count=0;
    view_.selected_items=unsigned(paths.size());view_.file_index=0;view_.batch_done=view_.batch_total=view_.bytes_copied=view_.done=view_.total=0;view_.cancelled=false;
    view_.mode=Mode::Planning;view_.status="Contando arquivos e pastas. Nenhum arquivo foi gravado.";changed();
    for(std::size_t i=0;i<paths.size();++i){
        const auto& p=paths[i];const auto s=snapshot(p);const auto target=to.path+"/"+leaf(p);
        if(!device_for(p)||(!s.regular&&!s.directory)||p.size()>1023||target.size()>1023||p==target||(s.directory&&within(target,p))){
            view_.source=p;view_.destination=target;failure("Origem indisponível ou destino dentro da própria origem.",0);return;
        }
        if(!queue({p,target,i}))return;
    }
}
void FileManagerFix42::plan_step(){
    for(unsigned budget=0;budget<16&&view_.mode==Mode::Planning;++budget){
        int error=0;
        if(scanning_active_){
            std::string name;bool end=false;
            if(!read_directory(scan_fd_,scan_dir_,name,end,error)){failure("Não foi possível ler uma subpasta da origem.",error);return;}
            if(end){
                if(!(snapshot(scanning_.source)==scanning_snapshot_)){failure("Uma pasta da origem mudou durante a preparação.",0);return;}
                release_scan();continue;
            }
            if(!safe_name(name))continue;
            const auto source=scanning_.source+"/"+name,destination=scanning_.destination+"/"+name;
            if(source.size()>1023||destination.size()>1023){failure("Um caminho da pasta excede 1023 bytes.",0);return;}
            if(!queue({source,destination,scanning_.root}))return;
            continue;
        }
        if(pending_.empty()){planning_finished();return;}
        auto task=std::move(pending_.front());pending_.pop_front();const auto original=snapshot(task.source,-1,&error);
        view_.source=task.source;view_.destination=task.destination;
        if(!original.regular&&!original.directory){failure("A origem contém um item indisponível ou não suportado.",error);return;}
        const auto target=snapshot(task.destination);
        if(target.exists&&(original.regular?!target.regular:!target.directory)){
            failure("Há um arquivo e uma pasta com o mesmo nome no destino.",0);return;
        }
        if(original.regular){
            const auto* d=device_for(task.destination);
            if(!d){failure("O dispositivo de destino foi desconectado.",0);return;}
            if(d->usb&&original.size>0xffffffffULL){failure("USB FAT32: um arquivo ultrapassa o limite de 4 GB.",0);return;}
            if(view_.batch_total>std::numeric_limits<std::uint64_t>::max()-original.size){failure("O tamanho do lote excede o limite.",0);return;}
            view_.batch_total+=original.size;++view_.file_count;
        }else ++view_.folder_count;
        jobs_.push_back({task.source,task.destination,original,task.root});
        if(original.directory){
            scanning_=std::move(task);scanning_snapshot_=original;
            if(!open_directory(scanning_.source,scan_fd_,scan_dir_,error)){failure("Não foi possível abrir uma subpasta da origem.",error);return;}
            scanning_active_=true;
        }
    }
    changed();
}
void FileManagerFix42::planning_finished(){
    view_.mode=Mode::Confirm;view_.source=view_.source_directory;view_.destination=view_.destination_directory;
    view_.status="Confira a origem e o destino antes de confirmar.";changed();
}
void FileManagerFix42::start_copy(){
    view_.mode=Mode::Copy;batch_active_=true;overwrite_all_=0;job_=0;view_.file_index=0;
    view_.status="Iniciando a cópia confirmada.";changed();
}
std::string FileManagerFix42::unique(const char* role){
    const auto dir=view_.destination.substr(0,view_.destination.find_last_of('/'));
    for(unsigned i=0;i<1024;++i){const auto path=dir+"/.orbit152_"+role+"_"+std::to_string(++serial_);if(path.size()>1023)return "";if(!snapshot(path).exists)return path;}
    return "";
}
void FileManagerFix42::next(){
    if(job_==jobs_.size()){batch_finished();return;}
    const auto& j=jobs_[job_];view_.source=j.source;view_.destination=j.destination;view_.done=0;view_.total=j.original.size;
    if(!device_for(j.source)||!device_for(j.destination)||!(snapshot(j.source)==j.original)){failure("A origem mudou ou um dispositivo foi desconectado.",0);return;}
    dest_snapshot_=snapshot(j.destination);
    if(j.original.directory){
        if(dest_snapshot_.exists&&!dest_snapshot_.directory){failure("Uma pasta do destino foi substituída por outro tipo de item.",0);return;}
        int error=0;if(!dest_snapshot_.exists&&!make_directory(j.destination,error)){failure("Não foi possível criar uma pasta no destino.",error);return;}
        ++view_.directories_done;++job_;view_.status="Preparando pasta: "+leaf(j.source);changed();return;
    }
    view_.file_index=view_.copied_count+view_.skipped_count+1;
    if(dest_snapshot_.exists&&!dest_snapshot_.regular){failure("O destino deixou de ser um arquivo regular.",0);return;}
    if(dest_snapshot_.exists&&overwrite_all_==2){skip();return;}
    if(dest_snapshot_.exists&&overwrite_all_!=1){view_.mode=Mode::Overwrite;view_.overwrite_selected=1;view_.status="Já existe: "+leaf(j.destination);changed();return;}
    begin_copy();
}
void FileManagerFix42::skip(){
    const auto& j=jobs_[job_];root_ok_[j.root]=false;view_.batch_done+=j.original.size;++view_.skipped_count;++job_;
    view_.mode=Mode::Copy;changed();
}
void FileManagerFix42::begin_copy(){
    const auto& j=jobs_[job_];const auto* d=device_for(j.destination);
    if(!d||!(snapshot(j.source)==j.original)||!(snapshot(j.destination)==dest_snapshot_)){failure("Origem ou destino mudou antes da cópia.",0);return;}
    std::uint64_t free=0;const auto parent=j.destination.substr(0,j.destination.find_last_of('/'));
    if(!free_bytes(parent,d->path,free)||free<j.original.size){failure("Não há espaço disponível no destino.",0);return;}
    int error=0;source_fd_=open_file(j.source,false,error);if(source_fd_<0){failure("Não foi possível abrir a origem.",error);return;}
    source_fd_snapshot_=snapshot(j.source,source_fd_,&error);
    if(!source_fd_snapshot_.regular||source_fd_snapshot_.size!=j.original.size||!(snapshot(j.source)==j.original)){failure("Não foi possível verificar a origem aberta.",error);return;}
    for(unsigned i=0;i<1024;++i){
        temporary_=unique("tmp");if(temporary_.empty())break;dest_fd_=open_file(temporary_,true,error);
        if(dest_fd_>=0){owns_temp_=true;break;}if(!snapshot(temporary_).exists)break;
    }
    if(!owns_temp_){failure("Não foi possível criar a cópia temporária.",error);return;}
    view_.mode=Mode::Copy;view_.status="Copiando: "+leaf(j.source);changed();
}
void FileManagerFix42::finish(){
    const auto& j=jobs_[job_];
    if(!(snapshot(j.source)==j.original)||!(snapshot(j.source,source_fd_)==source_fd_snapshot_)||!(snapshot(j.destination)==dest_snapshot_)){
        failure("Origem ou destino mudou durante a cópia.",0);return;
    }
    const bool synced=sync_file(dest_fd_),closed=close_file(dest_fd_);dest_fd_=-1;
    if(!synced||!closed){failure("Falha ao gravar a cópia.",0);return;}close_file(source_fd_);source_fd_=-1;backup_.clear();
    if(dest_snapshot_.exists){
        backup_=unique("bak");if(backup_.empty()||!rename_file(j.destination,backup_)){failure("Não foi possível preservar o arquivo anterior.",0);return;}
    }
    if(!rename_file(temporary_,j.destination)){
        const bool restored=backup_.empty()||rename_file(backup_,j.destination);
        const auto message=restored?"Falha ao concluir. Arquivo anterior preservado.":"Arquivo anterior preservado em: "+backup_;
        failure(message.c_str(),0);return;
    }
    owns_temp_=false;temporary_.clear();copied_.insert(j.source);copied_.insert(j.destination);marked_.erase(j.source);
    ++view_.copied_count;view_.batch_done+=j.original.size;view_.bytes_copied+=j.original.size;++job_;
    if(!backup_.empty()&&!remove_file(backup_)){const auto message="Cópia concluída; arquivo anterior preservado em: "+backup_;failure(message.c_str(),0);return;}
    flags();view_.status="Arquivo concluído. Preparando o próximo.";changed();
}
void FileManagerFix42::batch_finished(){
    for(const auto& j:jobs_)if(j.original.directory&&!(snapshot(j.source)==j.original)){
        failure("Uma pasta da origem mudou. Confira os arquivos concluídos.",0);return;
    }
    for(std::size_t i=0;i<root_items_.size();++i)if(root_ok_[i]){
        copied_.insert(root_items_[i]);copied_.insert(view_.destination_directory+"/"+leaf(root_items_[i]));marked_.erase(root_items_[i]);
    }
    batch_active_=false;view_.mode=Mode::Summary;refresh(0,true);refresh(1,true);
    view_.status="Operação concluída.";changed();
}
void FileManagerFix42::step(){
    if(!active())return;
    if(view_.mode==Mode::Planning){plan_step();return;}
    if(!batch_active_||view_.mode!=Mode::Copy)return;
    if(source_fd_<0){next();return;}
    if(view_.done==view_.total){finish();return;}
    const auto length=std::size_t(std::min<std::uint64_t>(Slice,view_.total-view_.done));std::uint64_t n=0;int error=0;
    if(!read_file(source_fd_,buffer_.data(),length,n,error)||!n||n>length){failure("Falha na leitura; cópia interrompida.",error);return;}
    std::uint64_t at=0;while(at<n){std::uint64_t written=0;if(!write_file(dest_fd_,buffer_.data()+at,std::size_t(n-at),written,error)||!written||written>n-at){failure("Falha na gravação; cópia interrompida.",error);return;}at+=written;}
    const int before=view_.total?int(100.0*double(view_.done)/double(view_.total)):100;view_.done+=n;
    if(before!=int(100.0*double(view_.done)/double(view_.total)))changed();
}
void FileManagerFix42::update(const InputFrame& in,float dt){
    if(!active()||!in.connected)return;
    if(view_.mode==Mode::ExitConfirm){
        if(in.circle.pressed){view_.mode=exit_previous_;changed();return;}
        if(in.up.pressed||in.down.pressed){view_.exit_selected=1-view_.exit_selected;changed();}
        if(in.cross.pressed){if(view_.exit_selected)close();else{view_.mode=exit_previous_;changed();}}return;
    }
    if(in.start.pressed){
        if(batch_active_||view_.mode==Mode::Planning){exit_previous_=view_.mode;view_.mode=Mode::ExitConfirm;view_.exit_selected=0;changed();}
        else close();
        return;
    }
    if(view_.mode==Mode::Notice){if(in.circle.pressed)close();else if(in.cross.pressed){view_.mode=Mode::Browse;refresh(0);refresh(1);view_.status="Marque a origem e escolha o destino no outro painel.";changed();}return;}
    if(view_.mode==Mode::Summary){if(in.cross.pressed||in.circle.pressed){view_.mode=Mode::Browse;changed();}return;}
    if(view_.mode==Mode::Planning||view_.mode==Mode::Copy){if(in.circle.pressed)cancel("Operação cancelada. Arquivos concluídos foram preservados.");return;}
    if(view_.mode==Mode::Confirm){
        if(in.circle.pressed){view_.mode=Mode::Browse;jobs_.clear();view_.status="Cópia não iniciada.";changed();}
        else if(in.cross.pressed)start_copy();
        return;
    }
    if(view_.mode==Mode::Overwrite){
        if(in.circle.pressed){cancel("Operação cancelada. Arquivos concluídos foram preservados.");return;}
        if(in.up.pressed)view_.overwrite_selected=(view_.overwrite_selected+4)%5;
        if(in.down.pressed)view_.overwrite_selected=(view_.overwrite_selected+1)%5;
        if(in.up.pressed||in.down.pressed)changed();
        if(in.cross.pressed){const int selected=view_.overwrite_selected;
            if(selected==4)cancel("Operação cancelada.");else if(selected==1||selected==3){if(selected==3)overwrite_all_=2;skip();}
            else{if(selected==2)overwrite_all_=1;begin_copy();}}return;
    }
    if(view_.mode==Mode::Actions){
        if(in.circle.pressed||in.triangle.pressed){view_.mode=Mode::Browse;changed();return;}
        if(in.up.pressed)view_.action_selected=(view_.action_selected+3)%4;
        if(in.down.pressed)view_.action_selected=(view_.action_selected+1)%4;
        if(in.up.pressed||in.down.pressed)changed();
        if(in.cross.pressed){view_.mode=Mode::Browse;
            switch(view_.action_selected){case 0:prepare();break;case 1:mark_all();break;case 2:clear_selection();view_.status="Seleção limpa.";break;
                case 3:discover();refresh(0,true);refresh(1,true);view_.status="Pastas e dispositivos atualizados.";break;}
            changed();}return;
    }
    if(view_.mode==Mode::Devices){
        if(in.circle.pressed||in.r3.pressed){view_.mode=Mode::Browse;changed();return;}
        const int count=int(devices_.size());
        if(count){if(in.up.pressed)view_.device_selected=(view_.device_selected+count-1)%count;if(in.down.pressed)view_.device_selected=(view_.device_selected+1)%count;}
        if(in.up.pressed||in.down.pressed)changed();
        if(in.cross.pressed&&count){view_.mode=Mode::Browse;navigate(devices_[std::size_t(view_.device_selected)].path);}return;
    }
    if(view_.mode==Mode::Details){
        if(in.circle.pressed||in.select.pressed){view_.mode=Mode::Browse;changed();return;}
        if(in.up.pressed||in.l2.pressed)view_.details_page=std::max(0,view_.details_page-1);
        if(in.down.pressed||in.r2.pressed)view_.details_page=std::min(64,view_.details_page+1);
        if(in.up.pressed||in.down.pressed||in.l2.pressed||in.r2.pressed)changed();
        return;
    }
    if(in.l1.pressed||in.left.pressed){view_.active_panel=0;repeat_=0;changed();}
    if(in.r1.pressed||in.right.pressed){view_.active_panel=1;repeat_=0;changed();}
    auto& p=view_.panels[view_.active_panel];
    if(in.r3.pressed){discover();view_.device_selected=0;for(std::size_t i=0;i<devices_.size();++i)if(within(p.path,devices_[i].path))view_.device_selected=int(i);view_.mode=Mode::Devices;changed();return;}
    if(in.triangle.pressed){view_.mode=Mode::Actions;view_.action_selected=0;changed();return;}
    if(in.circle.pressed){if(p.path.empty()){close();return;}const auto* d=device_for(p.path);navigate(!d||p.path==d->path?"":p.path.substr(0,p.path.find_last_of('/')));return;}
    int delta=in.down.pressed?1:in.up.pressed?-1:0;const int held=in.down.held?1:in.up.held?-1:0;
    if(delta||held!=repeat_direction_){repeat_direction_=held;repeat_=0;}
    else if(held){repeat_+=std::clamp(dt,0.0f,.1f);if(repeat_>=.32f){delta=held;repeat_-=.085f;}}
    if(in.l2.pressed)delta=-7;
    if(in.r2.pressed)delta=7;
    if(delta&&!p.entries.empty()){p.selected=std::clamp(p.selected+delta,0,int(p.entries.size())-1);changed();}
    if(p.entries.empty())return;
    const auto e=p.entries[std::size_t(p.selected)];
    if(in.select.pressed){view_.mode=Mode::Details;view_.details_page=0;changed();return;}
    if(in.square.pressed){mark_one();return;}
    if(in.cross.pressed&&e.directory){
        if(p.path.empty()){for(const auto& d:devices_)if(d.label==e.name){navigate(d.path);break;}}
        else navigate(p.path+"/"+e.name);
    }
}
#endif

