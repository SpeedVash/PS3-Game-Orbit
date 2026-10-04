#!/usr/bin/env python3
"""Run actual native decoder and OSK code with controlled SDK responses."""
from pathlib import Path
import subprocess, tempfile
ROOT=Path(__file__).resolve().parent.parent
headers={
'sysutil/osk.h':r'''#pragma once
#include <cstdint>
using sys_mem_container_t=std::uint32_t;
enum oskInputFieldResult {OSK_OK,OSK_CANCELED,OSK_ABORT,OSK_NO_TEXT};
enum oskInputDevice {OSK_DEVICE_PAD,OSK_DEVICE_KB};
constexpr int OSK_PANEL_TYPE_PORTUGUESE=64,OSK_PANEL_TYPE_ENGLISH=2,OSK_PANEL_TYPE_NUMERAL=0x08000000,OSK_PROHIBIT_RETURN=2,OSK_FULLKEY_PANEL=2;
struct oskPoint {float x,y;};
struct oskParam {std::uint32_t allowedPanels,firstViewPanel;oskPoint controlPoint;int prohibitFlags;};
struct oskInputFieldInfo {std::uint16_t *message,*startText;int maxLength;};
struct oskCallbackReturnParam {oskInputFieldResult res;int len;std::uint16_t* str;};
int oskLoadAsync(sys_mem_container_t,oskParam*,oskInputFieldInfo*);
int oskUnloadAsync(oskCallbackReturnParam*);int oskAbort();
int oskSetInitialInputDevice(oskInputDevice);int oskSetKeyLayoutOption(std::uint32_t);
''',
'sys/memory.h':r'''#pragma once
#include <sysutil/osk.h>
int sysMemContainerCreate(sys_mem_container_t*,std::uint32_t);int sysMemContainerDestroy(sys_mem_container_t);
''',
'sysutil/sysutil.h':r'''#pragma once
#include <cstdint>
constexpr int SYSUTIL_EVENT_SLOT1=1;
constexpr std::uint64_t SYSUTIL_OSK_DONE=0x0503,SYSUTIL_OSK_UNLOADED=0x0504;
using Callback=void(*)(std::uint64_t,std::uint64_t,void*);
int sysUtilRegisterCallback(int,Callback,void*);int sysUtilUnregisterCallback(int);int sysUtilCheckCallback();
''',
'sysmodule/sysmodule.h':r'''#pragma once
#include <cstdint>
using s32=std::int32_t;using u32=std::uint32_t;
enum sysModuleId {SYSMODULE_PNGDEC,SYSMODULE_JPGDEC};
s32 sysModuleLoad(sysModuleId);s32 sysModuleUnload(sysModuleId);
''',
'pngdec/pngdec.h':r'''#pragma once
#include <sysmodule/sysmodule.h>
constexpr int PNGDEC_ERROR_OK=0;
struct pngData {void* bmp_out;u32 width,height,pitch;};
s32 pngLoadFromBuffer(const void*,u32,pngData*);
''',
'jpgdec/jpgdec.h':r'''#pragma once
#include <sysmodule/sysmodule.h>
constexpr int JPGDEC_ERROR_OK=0;
struct jpgData {void* bmp_out;u32 width,height,pitch;};
s32 jpgLoadFromBuffer(const void*,u32,jpgData*);
''',
'lv2/systime.h':r'''#pragma once
#include <cstdint>
std::uint64_t sysGetSystemTime();
'''
}
source=(ROOT/'src/game_tools_fix35.cpp').read_text()
# Use the real Unicode conversion and validation, without the unrelated importer.
unicode=source[source.index('std::vector<std::uint16_t> utf16('):source.index('bool Names::set(')]
test=r'''
#include <cassert>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "rename_keyboard_fix35.h"
#include "game_tools_fix35.h"
#include "image_decode.h"
#include "performance_fix35.h"
#include <sysutil/sysutil.h>
#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>
#include <jpgdec/jpgdec.h>
namespace RuntimeDiag {void log(const char*,...) {}}
static unsigned loads[2],unloads[2];static int decoder_failure=-1,decode_rc;
std::uint64_t sysGetSystemTime(){static std::uint64_t t;return t+=20;}
s32 sysModuleLoad(sysModuleId id){++loads[id];return id==decoder_failure ? -1 : 0;}
s32 sysModuleUnload(sysModuleId id){++unloads[id];return 0;}
template<class T> static int bitmap(T* data){
 data->width=2;data->height=1;data->pitch=12;data->bmp_out=std::malloc(12);
 const unsigned char bytes[12]={255,100,20,8,127,33,44,55,0,0,0,0};std::memcpy(data->bmp_out,bytes,12);return decode_rc;
}
s32 pngLoadFromBuffer(const void*,u32,pngData* out){return bitmap(out);}
s32 jpgLoadFromBuffer(const void*,u32,jpgData* out){return bitmap(out);}
static Callback callback;static unsigned created,destroyed,registered,unregistered,load_calls,unload_calls;
static int failure,unload_failure;static bool container,dialog,pending_abort;
static oskInputFieldInfo retained;static oskCallbackReturnParam* output;
int sysMemContainerCreate(sys_mem_container_t* id,std::uint32_t size){assert(size==(4u<<20));if(failure==1)return -1;assert(!container);*id=9;container=true;++created;return 0;}
int sysMemContainerDestroy(sys_mem_container_t id){assert(id==9 && container && !dialog);container=false;++destroyed;return 0;}
int sysUtilRegisterCallback(int slot,Callback cb,void*){assert(slot==1 && !callback);if(failure==2)return -1;callback=cb;++registered;return 0;}
int sysUtilUnregisterCallback(int slot){assert(slot==1 && callback);callback=nullptr;++unregistered;return 0;}
int oskSetInitialInputDevice(oskInputDevice d){assert(d==OSK_DEVICE_PAD);return 0;}
int oskSetKeyLayoutOption(std::uint32_t k){assert(k==OSK_FULLKEY_PANEL);return 0;}
int oskLoadAsync(sys_mem_container_t id,oskParam* p,oskInputFieldInfo* info){
 assert(id==9 && container && callback && info->maxLength==96 && p->prohibitFlags==OSK_PROHIBIT_RETURN);
 ++load_calls;if(failure==3)return -1;retained=*info;dialog=true;return 0;
}
int oskUnloadAsync(oskCallbackReturnParam* value){assert(container && dialog);++unload_calls;if(unload_failure-- >0)return -1;output=value;return 0;}
int oskAbort(){assert(dialog);pending_abort=true;return 0;}
int sysUtilCheckCallback(){
 if(pending_abort && callback){if(!output)callback(SYSUTIL_OSK_DONE,0,nullptr);else{output->res=OSK_ABORT;dialog=false;callback(SYSUTIL_OSK_UNLOADED,0,nullptr);pending_abort=false;}}return 0;
}
static void finish(RenameKeyboardFix35& k,oskInputFieldResult res,const std::string& title){
 assert(retained.startText && retained.message && k.busy());
 callback(SYSUTIL_OSK_DONE,0,nullptr);assert(!output);k.poll();assert(output && k.busy() && container);
 const auto chars=GameToolsFix35::utf16(title);std::copy(chars.begin(),chars.end(),output->str);output->str[chars.size()]=0;output->res=res;
 dialog=false;callback(SYSUTIL_OSK_UNLOADED,0,nullptr);k.poll();output=nullptr;assert(!k.busy() && !container && !callback);
}
int main(){
 assert(init_image_decoders_fix35());assert(loads[0]==1 && loads[1]==1);
 CoverImage image;image.width=2;image.height=1;image.encoded={1};image.format=CoverFileFormat::PNG;
 DecodedImageARGB argb;DecodedImageRGBA rgba;std::string error;
 for(int i=0;i<12;++i){image.format=i%2 ? CoverFileFormat::PNG : CoverFileFormat::JPEG;
  assert(decode_cover_argb_fix35(image,argb,error));assert(argb.pitch==12 && argb.argb[0]==255 && argb.argb[1]==100 && argb.argb[4]==127 && argb.argb[7]==55);
  assert(decode_cover_rgba(image,rgba,error));assert(rgba.pitch==8 && rgba.rgba[0]==100 && rgba.rgba[3]==255 && rgba.rgba[4]==33 && rgba.rgba[7]==127);
 }
 assert(loads[0]==1 && loads[1]==1 && !unloads[0] && !unloads[1]);decode_rc=-1;
 assert(!decode_cover_argb_fix35(image,argb,error) && !argb.valid());decode_rc=0;
 shutdown_image_decoders_fix35();shutdown_image_decoders_fix35();assert(unloads[0]==1 && unloads[1]==1);
 decoder_failure=0;assert(!init_image_decoders_fix35());decoder_failure=-1;
 assert(decode_cover_argb_fix35(image,argb,error));assert(loads[0]==3 && loads[1]==2);shutdown_image_decoders_fix35();
 puts("PASS: actual native PNG/JPEG module reuse, direct ARGB bytes including padded pitch/alpha, RGBA fallback, failed decode cleanup, load retry and one unload per module");
 RenameKeyboardFix35 keyboard;std::string title;bool accepted;
 for(failure=1;failure<=3;++failure){assert(!keyboard.begin("Original") && !keyboard.busy() && !container && !callback);}failure=0;
 assert(keyboard.begin("Ação") && !keyboard.begin("Other"));assert(GameToolsFix35::utf8(retained.startText,97)=="Ação");finish(keyboard,OSK_OK,"Edição brasileira");
 assert(keyboard.take_result(title,accepted) && accepted && title=="Edição brasileira" && !keyboard.take_result(title,accepted));
 assert(keyboard.begin("Original"));finish(keyboard,OSK_CANCELED,"");assert(keyboard.take_result(title,accepted) && !accepted);
 assert(keyboard.begin("Original"));finish(keyboard,OSK_OK,"   ");assert(keyboard.take_result(title,accepted) && !accepted);
 assert(keyboard.begin("Original"));callback(SYSUTIL_OSK_DONE,0,nullptr);unload_failure=1;keyboard.poll();assert(keyboard.busy() && !output && container);keyboard.poll();assert(output);
 output->res=OSK_CANCELED;dialog=false;callback(SYSUTIL_OSK_UNLOADED,0,nullptr);keyboard.poll();output=nullptr;assert(!keyboard.busy() && !container);
 assert(keyboard.begin("Original"));keyboard.shutdown();output=nullptr;assert(!keyboard.busy() && !container && !callback);
 assert(created==destroyed && registered==unregistered && load_calls>0 && unload_calls>0);
 puts("PASS: actual native OSK UTF-16 buffers remain live until unload; success/cancel/invalid text, allocation/register/load failure, unload retry, abort and independent callback slot cleanup");
}
'''
with tempfile.TemporaryDirectory(prefix='orbit35-services-') as directory:
 p=Path(directory)
 for name,body in headers.items():
  file=p/name;file.parent.mkdir(parents=True,exist_ok=True);file.write_text(body)
 cpp=p/'test.cpp';cpp.write_text(test.replace('int main(){','namespace GameToolsFix35 {\n'+unicode+'}\nint main(){'))
 exe=p/'test'
 subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-D__PSL1GHT__=1','-DPS3_GAME_ORBIT_FIX35=1','-I'+str(p),'-I'+str(ROOT/'include'),str(cpp),*[str(ROOT/'src'/s) for s in ('rename_keyboard_fix35.cpp','image_decode.cpp','performance_fix35.cpp')],'-o',str(exe)],check=True,timeout=40)
 subprocess.run([str(exe)],check=True,timeout=15)
