#pragma once
#ifdef PS3_GAME_ORBIT_FIX41
#include "v14_case_mesh.h"
enum class GameCaseStyleFix41 { Clear=0, Platinum=1, GreatestHits=2 };
namespace CaseStyleFix41 {
const char* name(GameCaseStyleFix41 style);
bool applies(const V14MeshPart& part,GameCaseStyleFix41 style,unsigned plastic_pass);
std::array<float,4> color(GameCaseStyleFix41 style);
}
#endif
