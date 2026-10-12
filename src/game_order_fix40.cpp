#include "game_order_fix40.h"
#ifdef PS3_GAME_ORBIT_FIX40
#include "file_store_fix38.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cstdio>
#include <unistd.h>
namespace {
std::string fold(const std::string& s){
    std::string result;
    for(std::size_t i=0;i<s.size();++i){
        unsigned char c=s[i];
        if(c==0xc3 && i+1<s.size()){
            const unsigned char b=s[i+1];char a=0;
            if((b>=0x80&&b<=0x85)||(b>=0xa0&&b<=0xa5))a='a';
            else if(b==0x87||b==0xa7)a='c';
            else if((b>=0x88&&b<=0x8b)||(b>=0xa8&&b<=0xab))a='e';
            else if((b>=0x8c&&b<=0x8f)||(b>=0xac&&b<=0xaf))a='i';
            else if(b==0x91||b==0xb1)a='n';
            else if((b>=0x92&&b<=0x96)||(b>=0xb2&&b<=0xb6)||b==0x98||b==0xb8)a='o';
            else if((b>=0x99&&b<=0x9c)||(b>=0xb9&&b<=0xbc))a='u';
            else if(b==0x9d||b==0xbd||b==0xbf)a='y';
            if(a){result+=a;++i;continue;}
        }
        result+=char(c>='A'&&c<='Z'?c+32:c);
    }
    return result;
}
bool valid(const std::string& s){return !s.empty()&&s[0]=='/'&&s.size()<=1024&&s.find('\0')==std::string::npos;}
std::uint32_t hash(const std::string& s){std::uint32_t h=2166136261u;for(unsigned char c:s){h^=c;h*=16777619u;}return h;}
int unhex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;}
}
namespace GameOrderFix40 {
const char* name(GameSortFix40 m){return m==GameSortFix40::Custom?"Custom":m==GameSortFix40::Reverse?"Z até A":"A até Z";}
void sort_visible(std::vector<int>& v,const std::vector<GameEntry>& g,GameSortFix40 m,const std::vector<std::string>& order){
    std::unordered_map<std::string,std::size_t> ranks;
    if(m==GameSortFix40::Custom)for(std::size_t i=0;i<order.size();++i)ranks.emplace(order[i],i);
    std::stable_sort(v.begin(),v.end(),[&](int a,int b){
        if(m==GameSortFix40::Custom){const auto x=ranks.find(g[a].identity_path()),y=ranks.find(g[b].identity_path());
            const auto ra=x==ranks.end()?order.size():x->second,rb=y==ranks.end()?order.size():y->second;
            if(ra!=rb)return ra<rb;
        }
        const auto x=fold(g[a].title),y=fold(g[b].title);
        if(x!=y)return m==GameSortFix40::Reverse?x>y:x<y;
        return g[a].identity_path()<g[b].identity_path();
    });
}
void reconcile(std::vector<std::string>& order,const std::vector<GameEntry>& games){
    std::unordered_set<std::string> seen;std::vector<std::string> clean;
    for(const auto& p:order)if(seen.insert(p).second)clean.push_back(p);
    std::vector<int> alpha;for(std::size_t i=0;i<games.size();++i)alpha.push_back(int(i));
    sort_visible(alpha,games,GameSortFix40::Alphabetical,{});
    for(int i:alpha)if(seen.insert(games[i].identity_path()).second)clean.push_back(games[i].identity_path());
    order=std::move(clean);
}
bool move_visible(std::vector<std::string>& order,const std::vector<GameEntry>& games,const std::vector<int>& visible,int from,int to){
    if(from<0||to<0||from>=int(visible.size())||to>=int(visible.size()))return false;
    reconcile(order,games);
    std::unordered_set<std::string> included;std::vector<std::string> values;std::vector<std::size_t> slots;
    for(int i:visible){if(i<0||i>=int(games.size()))return false;included.insert(games[i].identity_path());values.push_back(games[i].identity_path());}
    for(std::size_t i=0;i<order.size();++i)if(included.count(order[i]))slots.push_back(i);
    if(slots.size()!=values.size())return false;
    const auto moving=values[from];values.erase(values.begin()+from);values.insert(values.begin()+to,moving);
    for(std::size_t i=0;i<slots.size();++i)order[slots[i]]=values[i];
    return true;
}
bool Saved::save(const std::string& path) const {
    if(int(mode)<0||int(mode)>2||paths.size()>16000)return false;
    std::string body="PS3GO_ORDER_1\n"+std::to_string(int(mode))+"\n";
    static constexpr char digits[]="0123456789abcdef";std::unordered_set<std::string> seen;
    for(const auto& p:paths){if(!valid(p)||!seen.insert(p).second)return false;for(unsigned char c:p){body+=digits[c>>4];body+=digits[c&15];}body+='\n';}
    if(body.size()>8u*1024*1024-18)return false;
    char checksum[32];std::snprintf(checksum,sizeof(checksum),"CHECKSUM %08x\n",unsigned(hash(body)));body+=checksum;
    const auto temp=path+".tmp";FILE* f=std::fopen(temp.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(body.data(),1,body.size(),f)==body.size();
    if(std::fflush(f)!=0)ok=false;
    if(fsync(fileno(f))!=0)ok=false;
    if(std::fclose(f)!=0)ok=false;
    if(ok)ok=FileStoreFix38::commit(temp,path);
    if(!ok)std::remove(temp.c_str());
    return ok;
}
bool Saved::load(const std::string& path){
    FILE* f=FileStoreFix38::open_read(path);if(!f)return false;
    bool ok=std::fseek(f,0,SEEK_END)==0;const auto length=std::ftell(f);
    if(length<34||length>8*1024*1024)ok=false;
    std::string s;if(ok){s.resize(std::size_t(length));std::rewind(f);ok=std::fread(s.data(),1,s.size(),f)==s.size();}
    std::fclose(f);if(!ok)return false;
    const std::size_t footer=s.size()-18;
    if(s.compare(0,14,"PS3GO_ORDER_1\n")||s[14]<'0'||s[14]>'2'||s[15]!='\n'||s.compare(footer,9,"CHECKSUM ")||s.back()!='\n')return false;
    std::uint32_t expected=0;for(std::size_t i=footer+9;i<footer+17;++i){const int n=unhex(s[i]);if(n<0)return false;expected=(expected<<4)|unsigned(n);}
    if(hash(s.substr(0,footer))!=expected)return false;
    Saved candidate;candidate.mode=static_cast<GameSortFix40>(s[14]-'0');std::unordered_set<std::string> seen;
    std::size_t at=16;while(at<footer){const auto end=s.find('\n',at);if(end==std::string::npos||end>=footer||(end-at)%2)return false;
        std::string p;for(std::size_t i=at;i<end;i+=2){const int a=unhex(s[i]),b=unhex(s[i+1]);if(a<0||b<0)return false;p+=char(a*16+b);}
        if(!valid(p)||!seen.insert(p).second||candidate.paths.size()>=16000)return false;
        candidate.paths.push_back(std::move(p));at=end+1;
    }
    if(at!=footer)return false;
    *this=std::move(candidate);return true;
}
}
#endif
