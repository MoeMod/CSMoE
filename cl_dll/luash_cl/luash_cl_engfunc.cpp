
#include "cl_dll.h"
#include "triangleapi.h"
#include "r_efx.h"
#include "event_api.h"
#include "demo_api.h"
#include "net_api.h"
#include "ivoicetweak.h"

#include "luash.hpp"
#include "luash_cl_extern.h"

namespace cl
{
	int LuaCL_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engine", &gEngfuncs);
		return 0;
	}
}