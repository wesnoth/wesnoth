#include "lua_preferences.hpp"

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <preferences.hpp>

/**
 * The __index metamethod.
 * Parameter 1: the preference table.
 * Parameter 2: preference name, must be a string.
 * Returns: preference value. Returned as a string regardless of the type of the preference.
 * If there isn't such a preference, returns an empty string.
 */
static int impl_preferences_get(lua_State* L)
{
	std::string preference_name = luaL_checkstring(L, 2);
	lua_pushstring(L, preferences::get(preference_name).c_str());
	return 1;
}

/**
 * The __newindex metamethod.
 * Parameter 1: the preference table.
 * Parameter 2: preference name, must be a string.
 * Parameter 3: preference value. Can be a string, boolean or integer.
 * Returns nothing.
 */
static int impl_preferences_set(lua_State* L)
{
	std::string preference_name = luaL_checkstring(L, 2);
	int type = lua_type(L, 3);

	switch (type)
	{
	case LUA_TSTRING:
		preferences::set(preference_name, luaL_checkstring(L, 3));
		break;
	case LUA_TBOOLEAN:
		preferences::set(preference_name, lua_toboolean(L, 3) == 1);
		break;
	case LUA_TNUMBER:
		preferences::set(preference_name, luaL_checkint(L, 3));
		break;
	default:
		return luaL_typerror(L, 3, "string/boolean/number");
		break;
	}

	return 0;
}

namespace lua_preferences
{
	std::string register_table(lua_State* L)
	{
		// Push the wesnoth table to the stack
		lua_getglobal(L, "wesnoth");

		// Create the preferences table
		lua_newtable(L);
		lua_pushcfunction(L, impl_preferences_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_preferences_set);
		lua_setfield(L, -2, "__newindex");

		// Set the table as its own metatable
		lua_pushvalue(L, -1);
		lua_setmetatable(L, -2);

		// Assign the table to wesnoth.preferences
		lua_setfield(L, -2, "preferences");

		// Pop the wesnoth table from the stack
		lua_pop(L, 1);

		return "Adding preferences table...\n";
	}
}