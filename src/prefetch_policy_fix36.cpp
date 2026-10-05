#include "prefetch_policy_fix36.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include <algorithm>
namespace PrefetchPolicyFix36 {
std::vector<int> candidates(const CoverflowState& s){
    std::vector<int> out;const int n=int(s.visible.size());
    if(n<2 || s.selected<0 || s.selected>=n)return out;
    const int forward=s.layout==OrbitLayout::List?2:s.layout==OrbitLayout::Classic?3:7;
    const int behind=s.layout==OrbitLayout::List?1:s.layout==OrbitLayout::Classic?2:7;
    const int direction=s.navigation_direction<0?-1:1;
    out.reserve(std::min(n-1,forward+behind));
    for(int d=1;d<=forward;++d)for(int sign:{direction,-direction}){
        if(sign==-direction && d>behind)continue;
        int vi=(s.selected+d*sign)%n;if(vi<0)vi+=n;
        const int gi=s.visible[std::size_t(vi)];
        if(vi!=s.selected && std::find(out.begin(),out.end(),gi)==out.end())out.push_back(gi);
    }
    return out;
}
}
#endif
