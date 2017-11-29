#ifndef tichainlib_h
#define tichainlib_h

#include <glua/lprefix.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include <memory>

#include <glua/lua.h>
#include <glua/lauxlib.h>
#include <glua/lualib.h>
#include <glua/lapi.h>

#include <glua/tichain_lua_api.h>
#include <glua/tichain_lua_lib.h>

namespace glua
{
	namespace lib
	{
		int tichainlib_get_storage(lua_State *L);
		// TiValue.get_storage具体的实现
		int tichainlib_get_storage_impl(lua_State *L,
			const char *contract_id, const char *name);

		int tichainlib_set_storage(lua_State *L);

        int tichainlib_set_storage_impl(lua_State *L,
          const char *contract_id, const char *name, int value_index);
	}
}

#endif