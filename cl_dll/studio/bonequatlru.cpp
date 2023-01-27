
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio_util.h"
#include "bonequatlru.h"

#include "util/lru_cache.h"

namespace cl {

	using bonequatlru_key = std::pair<const mstudioanim_t*, int>; // frame, pbone, panim

	struct bonequatlru_bone_data
	{
		vec4_t q1, q2;
		vec3_t pos_start;
		vec3_t pos1, pos2;
		vec3_t pos_scale;
	};
	struct bonequatlru_data
	{
		int numbones;
		bonequatlru_bone_data bone[MAXSTUDIOBONES];
	};

	using bonequatlru_value = std::shared_ptr<bonequatlru_data>;

	static lru_cache<bonequatlru_key, bonequatlru_value> bonequatlru(65536);

	static void BoneQuatLru_StudioCalcBoneQuaterion2(int frame, const mstudiobone_t* pbone, const mstudioanim_t* panim, vec4_t_ref q1, vec4_t_ref q2)
	{
		int j, k;
		vec3_t angle1, angle2;
		mstudioanimvalue_t* panimvalue;

		for (j = 0; j < 3; j++)
		{
			if (panim->offset[j + 3] == 0)
			{
				angle2[j] = angle1[j] = pbone->value[j + 3];
			}
			else
			{
				panimvalue = (mstudioanimvalue_t*)((byte*)panim + panim->offset[j + 3]);
				k = frame;

				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;

				while (panimvalue->num.total <= k)
				{
					k -= panimvalue->num.total;
					panimvalue += panimvalue->num.valid + 1;

					if (panimvalue->num.total < panimvalue->num.valid)
						k = 0;
				}

				if (panimvalue->num.valid > k)
				{
					angle1[j] = panimvalue[k + 1].value;

					if (panimvalue->num.valid > k + 1)
					{
						angle2[j] = panimvalue[k + 2].value;
					}
					else
					{
						if (panimvalue->num.total > k + 1)
							angle2[j] = angle1[j];
						else
							angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
					}
				}
				else
				{
					angle1[j] = panimvalue[panimvalue->num.valid].value;

					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}

				angle1[j] = pbone->value[j + 3] + angle1[j] * pbone->scale[j + 3];
				angle2[j] = pbone->value[j + 3] + angle2[j] * pbone->scale[j + 3];
			}
		}

		if (!VectorCompare(angle1, angle2))
		{
			AngleQuaternion(angle1, q1);
			AngleQuaternion(angle2, q2);
		}
		else
		{
			AngleQuaternion(angle1, q1);
			AngleQuaternion(angle1, q2);
		}
	}
	
	static void BoneQuatLru_StudioCalcBonePosition2(int frame, const mstudiobone_t* pbone, const mstudioanim_t* panim, vec3_t_ref pos0, vec3_t_ref pos1, vec3_t_ref pos2, vec3_t_ref pos_scale)
	{
		int j, k;
		const mstudioanimvalue_t* panimvalue;

		for (j = 0; j < 3; j++)
		{
			pos0[j] = pbone->value[j];

			if (panim->offset[j] != 0)
			{
				panimvalue = (const mstudioanimvalue_t*)((const byte*)panim + panim->offset[j]);
				k = frame;

				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;

				while (panimvalue->num.total <= k)
				{
					k -= panimvalue->num.total;
					panimvalue += panimvalue->num.valid + 1;

					if (panimvalue->num.total < panimvalue->num.valid)
						k = 0;
				}

				if (panimvalue->num.valid > k)
				{
					if (panimvalue->num.valid > k + 1)
					{
						pos1[j] = panimvalue[k + 1].value;
						pos2[j] = panimvalue[k + 2].value;
						pos_scale[j] = pbone->scale[j];
					}
					else
					{
						pos1[j] = pos2[j] = panimvalue[k + 1].value;
						pos_scale[j] = pbone->scale[j];
					}
				}
				else
				{
					if (panimvalue->num.total <= k + 1)
					{
						pos1[j] = panimvalue[panimvalue->num.valid].value;
						pos2[j] = panimvalue[panimvalue->num.valid + 2].value;
						pos_scale[j] = pbone->scale[j];
					}
					else
					{
						pos1[j] = pos2[j] = panimvalue[panimvalue->num.valid].value;
						pos_scale[j] = pbone->scale[j];
					}
				}
			}
		}
	}

	static void BoneQuatLru_StudioCalcBoneBatch_MakeCacheData(int frame, int numbones, const mstudiobone_t* pbone, const mstudioanim_t* panim, bonequatlru_data &framecache)
	{
		framecache.numbones = numbones;
		for (int i = 0; i < numbones; i++, pbone++, panim++)
		{
			auto& bonedata = framecache.bone[i];
			BoneQuatLru_StudioCalcBoneQuaterion2(frame, pbone, panim, bonedata.q1, bonedata.q2);
			BoneQuatLru_StudioCalcBonePosition2(frame, pbone, panim, bonedata.pos_start, bonedata.pos1, bonedata.pos2, bonedata.pos_scale);
		}
	}

	void BoneQuatLru_StudioCalcBoneBatch(vec3_t pos[], vec4_t q[], int frame, float s, int numbones, const mstudiobone_t* pbone, const mstudioanim_t* panim)
	{
		auto key = std::make_pair(panim, frame);
		auto pcache = bonequatlru.get(key).value_or(nullptr);

		// no data, construct now
		if (!pcache)
		{
			pcache = std::make_shared<bonequatlru_value::element_type>();
			BoneQuatLru_StudioCalcBoneBatch_MakeCacheData(frame, numbones, pbone, panim, *pcache);
			bonequatlru.insert(key, pcache);
		}

		// write result
		{
			bonequatlru_data& framecache = *pcache;
			for (int i = 0; i < framecache.numbones; i++)
			{
				auto& bonedata = framecache.bone[i];
				if (bonedata.q1 != bonedata.q2)
				{
					QuaternionSlerp(bonedata.q1, bonedata.q2, s, q[i]);
				}
				else
				{
					q[i] = bonedata.q1;
				}

#if defined(XASH_SIMD) && U_VECTOR_NEON
				pos[i] = vfmaq_f32(bonedata.pos_start, vfmaq_n_f32(bonedata.pos1, vsubq_f32(bonedata.pos2, bonedata.pos1), s), bonedata.pos_scale);
#else
				for (int j = 0; j < 3; ++j)
				{
					pos[i][j] = bonedata.pos_start[j] + std::lerp(bonedata.pos1[j], bonedata.pos2[j], s) * bonedata.pos_scale[j];
				}
#endif
			}
		}
	}
}