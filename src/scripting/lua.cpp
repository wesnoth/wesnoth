/**
 * @file scripting/lua.cpp
 * Provides a Lua interpreter.
 *
 * @warning Lua's error handling is done by setjmp/longjmp, so be careful
 *   never to call a Lua error function that will jump while in the scope
 *   of a C++ object with a destructor. This is why this file uses goto-s
 *   to force object unscoping before erroring out.
 */

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

#include <cassert>
#include <cstring>
#include <iostream>

#include "filesystem.hpp"
#include "foreach.hpp"
#include "gamestatus.hpp"
#include "scripting/lua.hpp"
#include "unit.hpp"

/** Userdata storing the event environment. */
struct event_handler_data
{
	game_events::event_handler *handler;
	unit_map *units;
};

/* Dummy pointer for getting unique keys for Lua's registry. */
static char const gettextKey = 0;
static char const getunitKey = 0;
static char const handlerKey = 0;
static char const tstringKey = 0;

/**
 * Converts a string into a Lua object pushed at the top of the stack.
 * Boolean ("yes"/"no") and numbers are detected and typed accordingly.
 */
static void scalar_of_wml_string(lua_State *L, t_string const &v)
{
	if (!v.translatable())
	{
		char *pe;
		char const *pb = v.c_str();
		double d = strtod(v.c_str(), &pe);
		if (pe != pb && *pe == '\0')
			lua_pushnumber(L, d);
		else if (v == "yes")
			lua_pushboolean(L, true);
		else if (v == "no")
			lua_pushboolean(L, false);
		else
			lua_pushstring(L, pb);
	}
	else
	{
		new(lua_newuserdata(L, sizeof(t_string))) t_string(v);
		lua_pushlightuserdata(L, (void *)&tstringKey);
		lua_gettable(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
	}
}

/**
 * Converts a config object to a Lua table.
 * The destination table should be at the top of the stack on entry. It is
 * still at the top on exit.
 */
static void table_of_wml_config(lua_State *L, config const &cfg)
{
	int k = 1;
	foreach (const config::any_child &ch, cfg.all_children_range())
	{
		lua_createtable(L, 2, 0);
		lua_pushstring(L, ch.key.c_str());
		lua_rawseti(L, -2, 1);
		lua_newtable(L);
		table_of_wml_config(L, ch.cfg);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, k);
	}
	foreach (const config::attribute &attr, cfg.attribute_range())
	{
		scalar_of_wml_string(L, attr.second);
		lua_setfield(L, -2, attr.first.c_str());
	}
}

#define return_misformed() \
  do { lua_settop(L, initial_top); return false; } while (0)

/**
 * Converts a Lua table to a config object.
 * The source table should be at the top of the stack on entry. It is
 * still at the top on exit.
 * @param tstring_meta absolute stack position of t_string's metatable, or 0 if none.
 * @return false if some attributes had not the proper type.
 * @note If the table has holes in the integer keys or floating-point keys,
 *       some keys will be ignored and the error will go undetected.
 */
static bool wml_config_of_table(lua_State *L, config &cfg, int tstring_meta = 0)
{
	// Get t_string's metatable, so that it can be used later to detect t_string object.
	int initial_top = lua_gettop(L);
	if (!tstring_meta) {
		lua_pushlightuserdata(L, (void *)&tstringKey);
		lua_gettable(L, LUA_REGISTRYINDEX);
		tstring_meta = lua_gettop(L);
		lua_pushvalue(L, -2);
	}

	// First convert the children (integer indices).
	for (int i = 1, i_end = lua_objlen(L, -1); i <= i_end; ++i)
	{
		lua_rawgeti(L, -1, i);
		if (!lua_istable(L, -1)) return_misformed();
		lua_rawgeti(L, -1, 1);
		char const *m = lua_tostring(L, -1);
		if (!m) return_misformed();
		lua_rawgeti(L, -2, 2);
		if (!lua_istable(L, -1) ||
		    !wml_config_of_table(L, cfg.add_child(m), tstring_meta))
			return_misformed();
		lua_pop(L, 3);
	}

	// Then convert the attributes (string indices).
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
	{
		if (lua_isnumber(L, -2)) continue;
		if (!lua_isstring(L, -2)) return_misformed();
		t_string v;
		switch (lua_type(L, -1)) {
			case LUA_TBOOLEAN:
				v = lua_toboolean(L, -1) ? "yes" : "no";
				break;
			case LUA_TNUMBER:
			case LUA_TSTRING:
				v = lua_tostring(L, -1);
				break;
			case LUA_TUSERDATA:
			{
				if (!lua_getmetatable(L, -1)) return_misformed();
				bool tstr = lua_rawequal(L, -1, tstring_meta);
				lua_pop(L, 1);
				if (!tstr) return_misformed();
				v = *static_cast<t_string *>(lua_touserdata(L, -1));
				break;
			}
			default:
				return_misformed();
		}
		cfg[lua_tostring(L, -2)] = v;
	}

	lua_settop(L, initial_top);
	return true;
}

#undef return_misformed

/**
 * Creates a t_string object (__call metamethod).
 * - Arg 1: userdata containing the domain.
 * - Arg 2: string to translate.
 * - Ret 1: string containing the translatable string.
 */
static int lua_gettext(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	char const *d = static_cast<char *>(lua_touserdata(L, 1));
	// Hidden metamethod, so d has to be a string. Use it to create a t_string.
	new(lua_newuserdata(L, sizeof(t_string))) t_string(m, d);
	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Creates an interface for gettext
 * - Arg 1: string containing the domain.
 * - Ret 1: a full userdata with __call pointing to lua_gettext.
 */
static int lua_textdomain(lua_State *L)
{
	size_t l;
	char const *m = luaL_checklstring(L, 1, &l);
	void *p = lua_newuserdata(L, l + 1);
	memcpy(p, m, l + 1);
	lua_pushlightuserdata(L, (void *)&gettextKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Appends a scalar to a t_string object.
 */
static int lua_tstring_concat(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	// Hidden metamethod, so *t has to be a t_string object. Copy it in a new t_string.
	t = new(lua_newuserdata(L, sizeof(t_string))) t_string(*t);

	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_gettable(L, LUA_REGISTRYINDEX);

	switch (lua_type(L, 2)) {
		case LUA_TNUMBER:
		case LUA_TSTRING:
			*t += lua_tostring(L, 2);
			break;
		case LUA_TUSERDATA:
			// Compare its metatable with t_string's metatable.
			if (!lua_getmetatable(L, 2) || !lua_rawequal(L, -1, -2))
				return luaL_typerror(L, 2, "string");
			*t += *static_cast<t_string *>(lua_touserdata(L, 2));
			lua_pop(L, 1);
			break;
		default:
			return luaL_typerror(L, 2, "string");
	}

	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Destroys a t_string object before it is collected.
 */
static int lua_tstring_collect(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	t->t_string::~t_string();
	return 0;
}

#define return_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushstring(L, accessor.c_str()); \
		return 1; \
	}

#define return_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushinteger(L, accessor); \
		return 1; \
	}

/**
 * Gets some data on a unit (__index metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int lua_getunit(lua_State *L)
{
	size_t id = *static_cast<size_t *>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);

	// Retrieve the unit map from the registry.
	lua_pushlightuserdata(L, (void *)&handlerKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	event_handler_data *eh = static_cast<event_handler_data *>(lua_touserdata(L, -1));

	unit_map::const_unit_iterator ui = eh->units->find(id);
	if (!ui.valid()) return 0;
	unit const &u = ui->second;

	// Find the corresponding attribute.
	return_int_attrib("x", ui->first.x);
	return_int_attrib("y", ui->first.y);
	return_int_attrib("side", u.side());
	return_string_attrib("id", u.id());
	return_int_attrib("hitpoints", u.hitpoints());
	return_int_attrib("max_hitpoints", u.max_hitpoints());
	return_string_attrib("name", u.name());
	return_string_attrib("side_id", u.side_id());
	return 0;
}

#undef return_string_attrib
#undef return_int_attrib

/**
 * Gets the numeric ids of all the units.
 * - Arg 1: optional table containing a filter
 * - Ret 1: table containing full userdata with __index pointing to lua_getunit.
 */
static int lua_get_units(lua_State *L)
{
	bool has_filter = lua_gettop(L) >= 1;
	if (has_filter && !lua_istable(L, 1)) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}
	config filter;
	if (has_filter) {
		lua_settop(L, 1);
		if (!wml_config_of_table(L, filter))
			goto error_call_destructors;
	}

	// Retrieve the unit map from the registry.
	lua_pushlightuserdata(L, (void *)&handlerKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	event_handler_data *eh = static_cast<event_handler_data *>(lua_touserdata(L, -1));

	// Go through all the units while keeping the following stack:
	// 1: metatable, 2: return table, 3: userdata, 4: metatable copy
	lua_settop(L, 0);
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	int i = 1;
	for (unit_map::const_unit_iterator ui = eh->units->begin(), ui_end = eh->units->end();
	     ui != ui_end; ++ui)
	{
		if (has_filter && !ui->second.matches_filter(vconfig(&filter), ui->first))
			continue;
		size_t *p = static_cast<size_t *>(lua_newuserdata(L, sizeof(size_t)));
		*p = ui->second.underlying_id();
		lua_pushvalue(L, 1);
		lua_setmetatable(L, 3);
		lua_rawseti(L, 2, i);
		++i;
	}
	return 1;
}

/**
 * Fires a WML event handler.
 * - Arg 1: string containing the handler name.
 * - Arg 2: optional WML config.
 * - Arg 3,4: optional first location.
 * - Arg 5,6: optional second location.
 */
static int lua_fire(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	bool has_config = lua_gettop(L) >= 2;
	if (has_config && !lua_istable(L, 2)) {
		error_call_destructors:
		return luaL_typerror(L, 2, "WML table");
	}
	config cfg, &args = cfg.add_child(m);
	if (has_config) {
		lua_pushvalue(L, 2);
		if (!wml_config_of_table(L, args))
			goto error_call_destructors;
		lua_pop(L, 1);
	}
	map_location l1, l2;
	if (lua_gettop(L) >= 4)
		l1 = map_location(lua_tointeger(L, 3) - 1, lua_tointeger(L, 4) - 1);
	if (lua_gettop(L) >= 6)
		l2 = map_location(lua_tointeger(L, 5) - 1, lua_tointeger(L, 6) - 1);

	// Retrieve the event handler from the registry.
	lua_pushlightuserdata(L, (void *)&handlerKey);
	lua_gettable(L, LUA_REGISTRYINDEX);
	event_handler_data *eh = static_cast<event_handler_data *>(lua_touserdata(L, -1));

	eh->handler->handle_event
		(game_events::queued_event("_from_lua", l1, l2, config()), vconfig(&cfg, true));
	return 0;
}

/**
 * Fires an event the same way the fire_event WML tag does it.
 * - Arg 1: string containing the event name.
 * - Arg 2,3: optional first location.
 * - Arg 4,5: optional second location.
 * - Ret 1: boolean indicating whether the event was processed or not.
 */
static int lua_fire_event(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	map_location l1, l2;
	if (lua_gettop(L) >= 3)
		l1 = map_location(lua_tointeger(L, 2) - 1, lua_tointeger(L, 3) - 1);
	if (lua_gettop(L) >= 5)
		l2 = map_location(lua_tointeger(L, 4) - 1, lua_tointeger(L, 5) - 1);

	bool b = game_events::fire(m, l1, l2);
	lua_pushboolean(L, b);
	return 1;
}

/**
 * Gets a WML variable.
 * - Arg1: string containing the variable name.
 * - Arg2: optional bool indicating if tables for containers should be left empty.
 * - Ret1: value of the variable, if any.
 */
static int lua_get_variable(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	variable_info v(m, false, variable_info::TYPE_SCALAR);
	if (v.is_valid) {
		scalar_of_wml_string(L, v.as_scalar());
		return 1;
	} else {
		variable_info w(m, false, variable_info::TYPE_CONTAINER);
		if (w.is_valid) {
			lua_newtable(L);
			if (lua_toboolean(L, 2))
				table_of_wml_config(L, w.as_container());
			return 1;
		}
	}
	return 0;
}

/**
 * Sets a WML variable.
 * - Arg 1: string containing the variable name.
 * - Arg 2: bool/int/string/table containing the value.
 */
static int lua_set_variable(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 2, "WML table or scalar");
	}

	if (lua_isnoneornil(L, 2)) {
		game_events::get_state_of_game()->clear_variable(m);
		return 0;
	}

	variable_info v(m);
	switch (lua_type(L, 2)) {
		case LUA_TBOOLEAN:
			v.as_scalar() = lua_toboolean(L, 2) ? "yes" : "no";
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			v.as_scalar() = lua_tostring(L, 2);
			break;
		case LUA_TUSERDATA:
			// Compare its metatable with t_string's metatable.
			lua_pushlightuserdata(L, (void *)&tstringKey);
			lua_gettable(L, LUA_REGISTRYINDEX);
			if (!lua_getmetatable(L, 2) || !lua_rawequal(L, -1, -2))
				goto error_call_destructors;
			v.as_scalar() = *static_cast<t_string *>(lua_touserdata(L, 2));
			break;
		case LUA_TTABLE:
			lua_settop(L, 2);
			if (!wml_config_of_table(L, v.as_container()))
				goto error_call_destructors;
			break;
		default:
			goto error_call_destructors;
	}
	return 0;
}

/**
 * Loads and executes a Lua file.
 * - Arg 1: string containing the file name.
 * - Ret *: values returned by executing the file body.
 */
static int lua_dofile(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 1, "file not found");
		error_call_destructors_2:
		return lua_error(L);
		continue_call_destructor:
		lua_call(L, 0, 1);
		return 1;
	}
	std::string p = get_wml_location(m);
	if (p.empty())
		goto error_call_destructors_1;

	if (luaL_loadfile(L, p.c_str()))
		goto error_call_destructors_2;

	goto continue_call_destructor;
}

static int lua_message(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	std::cerr << "Lua script says: \"" << m << "\"\n";
	return 0;
}

LuaKernel::LuaKernel()
{
	mState = luaL_newstate();
	lua_State *L = mState;

	// Open safe libraries. (Debug is not, but it will be closed below.)
	static const luaL_Reg safe_libs[] = {
		{ "",       luaopen_base   },
		{ "table",  luaopen_table  },
		{ "string", luaopen_string },
		{ "math",   luaopen_math   },
		{ "debug",  luaopen_debug  },
		{ NULL, NULL }
	};
	for (luaL_Reg const *lib = safe_libs; lib->func; ++lib)
	{
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}

	// Put some callback functions in the scripting environment.
	static luaL_reg const callbacks[] = {
		{ "fire",         &lua_fire         },
		{ "fire_event",   &lua_fire_event   },
		{ "get_units",    &lua_get_units    },
		{ "get_variable", &lua_get_variable },
		{ "message",      &lua_message      },
		{ "dofile",       &lua_dofile       },
		{ "set_variable", &lua_set_variable },
		{ "textdomain",   &lua_textdomain   },
		{ NULL, NULL }
	};
	luaL_register(L, "wesnoth", callbacks);

	// Create the gettext metatable.
	lua_pushlightuserdata(L, (void *)&gettextKey);
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, lua_gettext);
	lua_setfield(L, -2, "__call");
	lua_pushstring(L, "Hands off! (gettext metatable)");
	lua_setfield(L, -2, "__metatable");
	lua_settable(L, LUA_REGISTRYINDEX);

	// Create the getunit metatable.
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, lua_getunit);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "Hands off! (getunit metatable)");
	lua_setfield(L, -2, "__metatable");
	lua_settable(L, LUA_REGISTRYINDEX);

	// Create the tstring metatable.
	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, lua_tstring_concat);
	lua_setfield(L, -2, "__concat");
	lua_pushcfunction(L, lua_tstring_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushstring(L, "Hands off! (tstring metatable)");
	lua_setfield(L, -2, "__metatable");
	lua_settable(L, LUA_REGISTRYINDEX);

	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

	// Push the error handler, then close debug.
	lua_settop(L, 0);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_pushnil(L);
	lua_setglobal(L, "debug");
}

LuaKernel::~LuaKernel()
{
	lua_close(mState);
}

/**
 * Runs a script from an event handler.
 */
void LuaKernel::run_event(char const *prog, game_events::queued_event const &ev,
                          game_events::event_handler *handler, unit_map *units)
{
	lua_State *L = mState;

	// Store the event data in the registry.
	lua_pushlightuserdata(L, (void *)&handlerKey);
	event_handler_data *eh = static_cast<event_handler_data *>
		(lua_newuserdata(L, sizeof(event_handler_data)));
	eh->handler = handler;
	eh->units = units;
	lua_settable(L, LUA_REGISTRYINDEX);

	// Push location arguments.
	int args = 0;
	if (ev.loc1.valid())
	{
		lua_pushinteger(L, ev.loc1.x + 1);
		lua_pushinteger(L, ev.loc1.y + 1);
		args += 2;
		if (ev.loc2.valid())
		{
			lua_pushinteger(L, ev.loc2.x + 1);
			lua_pushinteger(L, ev.loc2.y + 1);
			args += 2;
		}
	}

	execute(prog, args, 0);

	// Clear registry.
	lua_pushlightuserdata(L, (void *)&handlerKey);
	lua_pushnil(L);
	lua_settable(L, LUA_REGISTRYINDEX);
}

/**
 * Runs a plain script.
 */
void LuaKernel::run(char const *prog)
{
	execute(prog, 0, 0);
}

/**
 * Runs a script on a preset stack.
 */
void LuaKernel::execute(char const *prog, int nArgs, int nRets)
{
	lua_State *L = mState;

	// Compile script into a variadic function.
	int res = luaL_loadstring(L, prog);
	if (res)
	{
		std::cerr << "Failure while loading Lua script: "
		          << lua_tostring(L, -1) << '\n';
		lua_pop(L, 1);
		return;
	}

	// Place the function before its arguments.
	if (nArgs)
		lua_insert(L, -1 - nArgs);

	res = lua_pcall(L, nArgs, nRets, 1);
	if (res)
	{
		std::cerr << "Failure while running Lua script: "
		          << lua_tostring(L, -1) << '\n';
		lua_pop(L, 1);
		return;
	}
}
