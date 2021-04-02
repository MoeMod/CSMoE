#pragma once

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
namespace luash
{
	// (angles) => forward, right, up
	inline int Vector_AngleVectors(lua_State* L)
	{
		int NumParams = lua_gettop(L);
		if (NumParams < 1)
		{
			return 0;
		}
		Vector angles;
		lua_pushvalue(L, 1);
		PopVector(L, angles);
		Vector f, r, u;
		AngleVectors(angles, f, r, u);
		
		PushVector(L, f);
		PushVector(L, r);
		PushVector(L, u);
		return 3;
	}
	// (forward) => angles
	inline int Vector_VectorAngles(lua_State* L)
	{
		int NumParams = lua_gettop(L);
		if (NumParams < 1)
		{
			return 0;
		}
		Vector forward;
		lua_pushvalue(L, 1);
		PopVector(L, forward);
		Vector angles;
		VectorAngles(forward, angles);
		
		PushVector(L, angles);
		return 1;
	}
	
	inline void OpenLib_Vector(lua_State *L)
	{
		// TODO
	}
}
}
