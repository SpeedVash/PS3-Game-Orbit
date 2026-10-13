#include "mount_operation_fix29.h"
#ifdef PS3_GAME_ORBIT_FIX47
#include "physical_disc_fix47.h"
#endif

bool MountOperationFix29::start(const GameEntry& game,std::uint64_t now){
    if(busy()) return false;
    path_=game.path;expected_id_=game.title_id;deadline_=now+20000;
#ifdef PS3_GAME_ORBIT_FIX47
    if(game.source==GameSource::Disc){
        const auto current=read_disc_sample_fix47(game.path);
        if(!current.present || current.game.title_id!=game.title_id){
            state_=State::Failed;status_="O disco foi removido ou trocado. Aguarde a biblioteca atualizar.";return false;
        }
        state_=State::Confirmed;exit_at_=now+1200;
        status_="Disco "+game.title_id+" pronto. Voltando ao XMB...";return true;
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX48
    if(!client_.start(game,now)){state_=State::Failed;status_=client_.error();return false;}
    state_=State::WaitingResponse;status_="Orbit: preparando e montando o jogo...";return true;
#else
    if(!client_.start(path_,now)){state_=State::Failed;status_=client_.error();return false;}
    state_=State::WaitingResponse;status_="webMAN: enviando pedido de montagem...";return true;
#endif
}
void MountOperationFix29::update(std::uint64_t now,const std::string& id){
    if(state_==State::WaitingResponse){
        client_.update(now);
#ifdef PS3_GAME_ORBIT_FIX48
        if(client_.state()==NativeMountFix48::State::Failed){state_=State::Failed;status_=client_.error();return;}
        if(client_.state()!=NativeMountFix48::State::Accepted)return;
        deadline_=now+10000;
#else
        if(client_.state()==WebmanMountFix29::State::Failed){state_=State::Failed;status_=client_.error();return;}
        if(client_.state()!=WebmanMountFix29::State::Accepted) return;
#endif
        if(expected_id_.empty()){
            state_=State::Unverified;status_="Pedido aceito; sem ID. O: sair e conferir disco no XMB";return;
        }
        state_=State::WaitingDisc;status_=
#ifdef PS3_GAME_ORBIT_FIX48
            "Orbit montou; conferindo o disco "+
#else
            "webMAN respondeu; aguardando o disco "+
#endif
            expected_id_;
    }
    if(state_!=State::WaitingDisc) return;
    if(id==expected_id_){state_=State::Confirmed;exit_at_=now+1200;status_="Disco "+id+" pronto. Voltando ao XMB...";return;}
    if(now>=deadline_){state_=State::Failed;status_=
#ifdef PS3_GAME_ORBIT_FIX48
        "O disco montado não foi confirmado. X: tentar";
#else
        "webMAN respondeu; disco nao confirmado. X: tentar";
#endif
    }
}
