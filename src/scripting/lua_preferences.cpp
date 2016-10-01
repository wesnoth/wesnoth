#include "lua_preferences.hpp"

#include <config.hpp>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <preferences.hpp>
#include <scripting/lua_common.hpp>

/**
 * The __index metamethod.
 * Parameter 1: the preference table.
 * Parameter 2: preference name, must be a string.
 * Returns: preference value. If there isn't such a preference, returns nil.
 */
static int impl_preferences_get(lua_State* L)
{
	std::string preference_name = luaL_checkstring(L, 2);
	luaW_pushscalar(L, preferences::get_as_attribute(preference_name));
	return 1;
}

/**
 * The __newindex metamethod.
 * Parameter 1: the preference table.
 * Parameter 2: preference name, must be a string.
 * Parameter 3: preference value.
 * Returns nothing.
 */
static int impl_preferences_set(lua_State* L)
{
	std::string preference_name = luaL_checkstring(L, 2);
	config::attribute_value value;
	luaW_toscalar(L, 3, value);
	preferences::set(preference_name, value);
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
		lua_pushstring(L, "src/scripting/lua_preferences.cpp");
		lua_setfield(L, -2, "__metatable");

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
