#include "mount_operation_fix29.h"

bool MountOperationFix29::start(const GameEntry& game,std::uint64_t now){
    if(busy()) return false;
    path_=game.path;expected_id_=game.title_id;deadline_=now+20000;
    if(!client_.start(path_,now)){state_=State::Failed;status_=client_.error();return false;}
    state_=State::WaitingResponse;status_="webMAN: enviando pedido de montagem...";return true;
}
void MountOperationFix29::update(std::uint64_t now,const std::string& id){
    if(state_==State::WaitingResponse){
        client_.update(now);
        if(client_.state()==WebmanMountFix29::State::Failed){state_=State::Failed;status_=client_.error();return;}
        if(client_.state()!=WebmanMountFix29::State::Accepted) return;
        if(expected_id_.empty()){
            state_=State::Unverified;status_="Pedido aceito; sem ID. O: sair e conferir disco no XMB";return;
        }
        state_=State::WaitingDisc;status_="webMAN respondeu; aguardando o disco "+expected_id_;
    }
    if(state_!=State::WaitingDisc) return;
    if(id==expected_id_){state_=State::Confirmed;exit_at_=now+1200;status_="Disco "+id+" pronto. Voltando ao XMB...";return;}
    if(now>=deadline_){state_=State::Failed;status_="webMAN respondeu; disco nao confirmado. X: tentar";}
}
