#include "rename_keyboard_fix35.h"
#ifdef PS3_GAME_ORBIT_FIX35
#include "game_tools_fix35.h"
#include "runtime_diag.h"
#include <algorithm>
#ifdef __PSL1GHT__
#include <sys/memory.h>
#include <sysutil/sysutil.h>
#include <unistd.h>
RenameKeyboardFix35* RenameKeyboardFix35::instance_=nullptr;
void RenameKeyboardFix35::callback(std::uint64_t status,std::uint64_t,void*){
    if(!instance_)return;
    if(status==SYSUTIL_OSK_DONE)instance_->done_=true;
    if(status==SYSUTIL_OSK_UNLOADED)instance_->unloaded_=true;
}
#endif
bool RenameKeyboardFix35::begin(const std::string& title){
    if(busy_)return false;
    ready_=accepted_=done_=unloading_=unloaded_=false;title_.clear();
#ifdef __PSL1GHT__
    initial_.fill(0);output_.fill(0);prompt_.fill(0);
    auto initial=GameToolsFix35::utf16(title);if(initial.size()>96)initial.resize(96);
    if(!initial.empty() && initial.back()>=0xd800 && initial.back()<=0xdbff)initial.pop_back();
    std::copy(initial.begin(),initial.end(),initial_.begin());
    const auto prompt=GameToolsFix35::utf16("Alterar nome do jogo");std::copy(prompt.begin(),prompt.end(),prompt_.begin());
    if(sysMemContainerCreate(&container_,4u<<20)!=0)return false;
    container_created_=true;instance_=this;
    if(sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT1,callback,nullptr)!=0){sysMemContainerDestroy(container_);container_created_=false;instance_=nullptr;return false;}
    registered_=true;
    oskParam param{};param.allowedPanels=OSK_PANEL_TYPE_PORTUGUESE|OSK_PANEL_TYPE_ENGLISH|OSK_PANEL_TYPE_NUMERAL;
    param.firstViewPanel=OSK_PANEL_TYPE_PORTUGUESE;param.prohibitFlags=OSK_PROHIBIT_RETURN;
    param.controlPoint={0,0};
    oskInputFieldInfo info{};info.message=prompt_.data();info.startText=initial_.data();info.maxLength=96;
    oskSetInitialInputDevice(OSK_DEVICE_PAD);oskSetKeyLayoutOption(OSK_FULLKEY_PANEL);
    result_={};result_.res=OSK_CANCELED;result_.len=96;result_.str=output_.data();
    if(oskLoadAsync(container_,&param,&info)!=0){sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);registered_=false;sysMemContainerDestroy(container_);container_created_=false;instance_=nullptr;return false;}
    busy_=true;return true;
#else
    (void)title;return false;
#endif
}
void RenameKeyboardFix35::poll(){
#ifdef __PSL1GHT__
    if(!busy_)return;
    if(done_ && !unloading_){
        const auto rc=oskUnloadAsync(&result_);
        if(rc!=0){RuntimeDiag::log("OSK: unload failed rc=%d",int(rc));return;}
        unloading_=true;
    }
    if(unloaded_){
        accepted_=result_.res==OSK_OK;title_=accepted_ ? GameToolsFix35::utf8(output_.data(),96) : "";
        accepted_=accepted_ && GameToolsFix35::valid_title(title_);
        if(registered_)sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);
        registered_=false;
        if(container_created_)sysMemContainerDestroy(container_);
        container_created_=false;instance_=nullptr;
        busy_=false;ready_=true;
    }
#endif
}
bool RenameKeyboardFix35::take_result(std::string& title,bool& accepted){if(!ready_)return false;ready_=false;title=title_;accepted=accepted_;return true;}
void RenameKeyboardFix35::shutdown(){
#ifdef __PSL1GHT__
    if(busy_){oskAbort();for(unsigned i=0;i<2500 && busy_;++i){sysUtilCheckCallback();poll();if(busy_)usleep(2000);}}
    // A still-active OSK retains its buffers/container until process teardown.
    // Do not destroy a memory container the system dialog may still access.
    if(registered_){sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);registered_=false;}
    instance_=nullptr;
#endif
}
#endif
