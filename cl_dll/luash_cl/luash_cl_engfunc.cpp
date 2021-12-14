
#include "cl_dll.h"
#include "triangleapi.h"
#include "r_efx.h"
#include "event_api.h"
#include "demo_api.h"
#include "net_api.h"
#include "ivoicetweak.h"

#include "luash.hpp"

static_assert(StructMemberCount<tempent_s>::value == 16);

namespace cl
{
	int LuaCL_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engine", &gEngfuncs);
		return 0;
	}
}