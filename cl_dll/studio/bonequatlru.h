#pragma once

namespace cl {

	void BoneQuatLru_StudioCalcBoneBatch(vec3_t pos[], vec4_t q[], int frame, float s, int numbones, const mstudiobone_t* pbone, const mstudioanim_t* panim);

}