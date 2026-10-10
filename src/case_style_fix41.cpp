#include "case_style_fix41.h"
#ifdef PS3_GAME_ORBIT_FIX41
namespace CaseStyleFix41 {
const char* name(GameCaseStyleFix41 s){return s==GameCaseStyleFix41::Platinum?"Platinum (cinza)":s==GameCaseStyleFix41::GreatestHits?"Greatest Hits (vermelha)":"Padrão (transparente)";}
bool applies(const V14MeshPart& p,GameCaseStyleFix41 s,unsigned pass){
    return s!=GameCaseStyleFix41::Clear&&p.external_plastic&&p.material==V14MeshPart::Material::ClearPlastic&&pass==2;
}
std::array<float,4> color(GameCaseStyleFix41 s){return s==GameCaseStyleFix41::GreatestHits?std::array<float,4>{{.72f,.035f,.05f,1.f}}:std::array<float,4>{{.56f,.57f,.58f,1.f}};}
}
#endif
