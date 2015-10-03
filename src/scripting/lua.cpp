/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file scripting/lua.cpp
 * Provides a Lua interpreter.
 *
 * @warning Lua's error handling is done by setjmp/longjmp, so be careful
 *   never to call a Lua error function that will jump while in the scope
 *   of a C++ object with a destructor. This is why this file uses goto-s
 *   to force object unscoping before erroring out. This is also why
 *   lua_getglobal, lua_setglobal, lua_gettable, lua_settable, lua_getfield,
 *   and lua_setfield, should not be called in tainted context.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 */

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

#include <cassert>
#include <cstring>

#include "scripting/lua.hpp"

#include "actions.hpp"
#include "attack_prediction.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "terrain_translation.hpp"
#include "unit.hpp"
#include "ai/actions.hpp"
#include "ai/composite/engine_lua.hpp"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)


/**
 * Stack storing the queued_event objects needed for calling WML actions.
 */
struct queued_event_context
{
	typedef game_events::queued_event qe;
	static qe default_qe;
	static qe const *current_qe;
	static qe const &get()
	{ return *(current_qe ? current_qe : &default_qe); }
	qe const *previous_qe;

	queued_event_context(qe const *new_qe)
		: previous_qe(current_qe)
	{
		current_qe = new_qe;
	}

	~queued_event_context()
	{ current_qe = previous_qe; }
};

game_events::queued_event const *queued_event_context::current_qe = 0;
game_events::queued_event queued_event_context::default_qe
	("_from_lua", map_location(), map_location(), config());

/* Dummy pointer for getting unique keys for Lua's registry. */
static char const executeKey = 0;
static char const getsideKey = 0;
static char const gettextKey = 0;
static char const gettypeKey = 0;
static char const getunitKey = 0;
static char const tstringKey = 0;
static char const uactionKey = 0;
static char const vconfigKey = 0;
static char const wactionKey = 0;
static char const aisKey     = 0;

/* Global definition so that it does not leak on longjmp. */
static std::string error_buffer;

/**
 * Displays a message in the chat window.
 */
static void chat_message(std::string const &caption, std::string const &msg)
{
	resources::screen->add_chat_message(time(NULL), caption, 0, msg,
		events::chat_handler::MESSAGE_PUBLIC, false);
}

/**
 * Pushes a config as a volatile vconfig on the top of the stack.
 */
static void luaW_pushvconfig(lua_State *L, config const &cfg)
{
	new(lua_newuserdata(L, sizeof(vconfig))) vconfig(cfg, true);
	lua_pushlightuserdata(L, (void *)&vconfigKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
}

/**
 * Pushes a t_string on the top of the stack.
 */
static void luaW_pushtstring(lua_State *L, t_string const &v)
{
	new(lua_newuserdata(L, sizeof(t_string))) t_string(v);
	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
}

/**
 * Converts a string into a Lua object pushed at the top of the stack.
 * Boolean ("yes"/"no") and numbers are detected and typed accordingly.
 */
static void luaW_pushscalar(lua_State *L, t_string const &v)
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
		luaW_pushtstring(L, v);
	}
}

/**
 * Returns true if the metatable of the object is the one found in the registry.
 */
static bool luaW_hasmetatable(lua_State *L, int index, char const &key)
{
	if (!lua_getmetatable(L, index))
		return false;
	lua_pushlightuserdata(L, (void *)&key);
	lua_rawget(L, LUA_REGISTRYINDEX);
	bool ok = lua_rawequal(L, -1, -2);
	lua_pop(L, 2);
	return ok;
}

/**
 * Converts a scalar to a translatable string.
 */
static bool luaW_totstring(lua_State *L, int index, t_string &str)
{
	switch (lua_type(L, index)) {
		case LUA_TBOOLEAN:
			str = lua_toboolean(L, index) ? "yes" : "no";
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			str = lua_tostring(L, index);
			break;
		case LUA_TUSERDATA:
		{
			if (!luaW_hasmetatable(L, index, tstringKey)) return false;
			str = *static_cast<t_string *>(lua_touserdata(L, index));
			break;
		}
		default:
			return false;
	}
	return true;
}

/**
 * Converts a config object to a Lua table.
 * The destination table should be at the top of the stack on entry. It is
 * still at the top on exit.
 */
static void table_of_wml_config(lua_State *L, config const &cfg)
{
	if (!lua_checkstack(L, LUA_MINSTACK))
		return;

	int k = 1;
	BOOST_FOREACH (const config::any_child &ch, cfg.all_children_range())
	{
		lua_createtable(L, 2, 0);
		lua_pushstring(L, ch.key.c_str());
		lua_rawseti(L, -2, 1);
		lua_newtable(L);
		table_of_wml_config(L, ch.cfg);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, k++);
	}
	BOOST_FOREACH (const config::attribute &attr, cfg.attribute_range())
	{
		luaW_pushscalar(L, attr.second);
		lua_setfield(L, -2, attr.first.c_str());
	}
}

#define return_misformed() \
  do { lua_settop(L, initial_top); return false; } while (0)

/**
 * Converts an optional table or vconfig to a config object.
 * @param tstring_meta absolute stack position of t_string's metatable, or 0 if none.
 * @return false if some attributes had not the proper type.
 * @note If the table has holes in the integer keys or floating-point keys,
 *       some keys will be ignored and the error will go undetected.
 */
static bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta = 0)
{
	if (!lua_checkstack(L, LUA_MINSTACK))
		return false;

	// Get the absolute index of the table.
	int initial_top = lua_gettop(L);
	if (-initial_top <= index && index <= -1)
		index = initial_top + index + 1;

	switch (lua_type(L, index))
	{
		case LUA_TTABLE:
			break;
		case LUA_TUSERDATA:
		{
			if (!luaW_hasmetatable(L, index, vconfigKey))
				return false;
			cfg = static_cast<vconfig *>(lua_touserdata(L, index))->get_parsed_config();
			return true;
		}
		case LUA_TNONE:
		case LUA_TNIL:
			return true;
		default:
			return false;
	}

	// Get t_string's metatable, so that it can be used later to detect t_string object.
	if (!tstring_meta) {
		lua_pushlightuserdata(L, (void *)&tstringKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		tstring_meta = initial_top + 1;
	}

	// First convert the children (integer indices).
	for (int i = 1, i_end = lua_objlen(L, index); i <= i_end; ++i)
	{
		lua_rawgeti(L, index, i);
		if (!lua_istable(L, -1)) return_misformed();
		lua_rawgeti(L, -1, 1);
		char const *m = lua_tostring(L, -1);
		if (!m) return_misformed();
		lua_rawgeti(L, -2, 2);
		if (!luaW_toconfig(L, -1, cfg.add_child(m), tstring_meta))
			return_misformed();
		lua_pop(L, 3);
	}

	// Then convert the attributes (string indices).
	for (lua_pushnil(L); lua_next(L, index); lua_pop(L, 1))
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
				bool tstr = lua_rawequal(L, -1, tstring_meta) != 0;
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
 * Gets an optional vconfig from either a table or a userdata.
 * @param def true if an empty config should be created for a missing value.
 * @return false in case of failure.
 */
static bool luaW_tovconfig(lua_State *L, int index, vconfig &vcfg, bool def = true)
{
	switch (lua_type(L, index))
	{
		case LUA_TTABLE:
		{
			config cfg;
			bool ok = luaW_toconfig(L, index, cfg);
			if (!ok) return false;
			vcfg = vconfig(cfg, true);
			break;
		}
		case LUA_TUSERDATA:
			if (!luaW_hasmetatable(L, index, vconfigKey))
				return false;
			vcfg = *static_cast<vconfig *>(lua_touserdata(L, index));
			break;
		case LUA_TNONE:
		case LUA_TNIL:
			if (def)
				vcfg = vconfig(config(), true);
			break;
		default:
			return false;
	}
	return true;
}

/**
 * Calls a Lua function stored below its @a nArgs arguments at the top of the stack.
 * @return true if the call was successful and @a nRets return values are available.
 */
static bool luaW_pcall(lua_State *L
		, int nArgs, int nRets, bool allow_wml_error = false)
{
	// Load the error handler before the function and its arguments.
	lua_pushlightuserdata(L, (void *)&executeKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_insert(L, -2 - nArgs);

	int error_handler_index = lua_gettop(L) - nArgs - 1;

	// Call the function.
	int res = lua_pcall(L, nArgs, nRets, -2 - nArgs);
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		if (allow_wml_error && strncmp(m, "~wml:", 5) == 0) {
			m += 5;
			char const *e = strstr(m, "stack traceback");
			lg::wml_error << std::string(m, e ? e - m : strlen(m));
		} else {
			chat_message("Lua error", m);
			ERR_LUA << m << '\n';
		}
		lua_pop(L, 2);
		return false;
	}

	// Remove the error handler.

	if(nRets == LUA_MULTRET) {
		lua_remove(L, error_handler_index);
	} else {
		lua_remove(L, -1 - nRets);
	}

	return true;
}

/**
 * Storage for a unit, either one on the map, or one owned by the Lua code.
 */
class lua_unit
{
	size_t uid;
	unit *ptr;
	lua_unit(lua_unit const &);

public:
	lua_unit(size_t u): uid(u), ptr(NULL) {}
	lua_unit(unit *u): uid(0), ptr(u) {}
	~lua_unit() { delete ptr; }
	bool on_map() const { return !ptr; }
	void reload();
	unit *get();
};

unit *lua_unit::get()
{
	if (ptr) return ptr;
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return NULL;
	return &ui->second;
}

void lua_unit::reload()
{
	assert(ptr);
	uid = ptr->underlying_id();
	delete ptr;
	ptr= NULL;
}

/**
 * Converts a Lua value to a unit pointer.
 */
static unit *luaW_tounit(lua_State *L, int index, bool only_on_map = false)
{
	if (!luaW_hasmetatable(L, index, getunitKey)) return NULL;
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, index));
	if (only_on_map && !lu->on_map()) return NULL;
	return lu->get();
}

/**
 * Creates a t_string object (__call metamethod).
 * - Arg 1: userdata containing the domain.
 * - Arg 2: string to translate.
 * - Ret 1: string containing the translatable string.
 */
static int impl_gettext(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	char const *d = static_cast<char *>(lua_touserdata(L, 1));
	// Hidden metamethod, so d has to be a string. Use it to create a t_string.
	luaW_pushtstring(L, t_string(m, d));
	return 1;
}

/**
 * Creates an interface for gettext
 * - Arg 1: string containing the domain.
 * - Ret 1: a full userdata with __call pointing to lua_gettext.
 */
static int intf_textdomain(lua_State *L)
{
	size_t l;
	char const *m = luaL_checklstring(L, 1, &l);
	void *p = lua_newuserdata(L, l + 1);
	memcpy(p, m, l + 1);
	lua_pushlightuserdata(L, (void *)&gettextKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Converts a Lua value at position @a src and appends it to @a dst.
 * @note This function is private to lua_tstring_concat. It expects two things.
 *       First, the t_string metatable is at the top of the stack on entry. (It
 *       is still there on exit.) Second, the caller hasn't any valuable object
 *       with dynamic lifetime, since they would be leaked on error.
 */
static void tstring_concat_aux(lua_State *L, t_string &dst, int src)
{
	switch (lua_type(L, src)) {
		case LUA_TNUMBER:
		case LUA_TSTRING:
			dst += lua_tostring(L, src);
			break;
		case LUA_TUSERDATA:
			// Compare its metatable with t_string's metatable.
			if (!lua_getmetatable(L, src) || !lua_rawequal(L, -1, -2))
				luaL_typerror(L, src, "string");
			dst += *static_cast<t_string *>(lua_touserdata(L, src));
			lua_pop(L, 1);
			break;
		default:
			luaL_typerror(L, src, "string");
	}
}

/**
 * Appends a scalar to a t_string object (__concat metamethod).
 */
static int impl_tstring_concat(lua_State *L)
{
	// Create a new t_string.
	t_string *t = new(lua_newuserdata(L, sizeof(t_string))) t_string;

	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_rawget(L, LUA_REGISTRYINDEX);

	// Append both arguments to t.
	tstring_concat_aux(L, *t, 1);
	tstring_concat_aux(L, *t, 2);

	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Destroys a t_string object before it is collected (__gc metamethod).
 */
static int impl_tstring_collect(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	t->t_string::~t_string();
	return 0;
}

/**
 * Converts a t_string object to a string (__tostring metamethod);
 * that is, performs a translation.
 */
static int impl_tstring_tostring(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	lua_pushstring(L, t->c_str());
	return 1;
}

/**
 * Gets the parsed field of a vconfig object (_index metamethod).
 * Special fields __literal and __parsed return Lua tables.
 */
static int impl_vconfig_get(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));

	if (lua_isnumber(L, 2))
	{
		vconfig::all_children_iterator i = v->ordered_begin();
		unsigned len = std::distance(i, v->ordered_end());
		unsigned pos = lua_tointeger(L, 2) - 1;
		if (pos >= len) return 0;
		std::advance(i, pos);
		lua_createtable(L, 2, 0);
		lua_pushstring(L, i.get_key().c_str());
		lua_rawseti(L, -2, 1);
		new(lua_newuserdata(L, sizeof(vconfig))) vconfig(i.get_child());
		lua_pushlightuserdata(L, (void *)&vconfigKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
		lua_rawseti(L, -2, 2);
		return 1;
	}

	char const *m = luaL_checkstring(L, 2);
	if (strcmp(m, "__literal") == 0) {
		lua_newtable(L);
		table_of_wml_config(L, v->get_config());
		return 1;
	}
	if (strcmp(m, "__parsed") == 0) {
		lua_newtable(L);
		table_of_wml_config(L, v->get_parsed_config());
		return 1;
	}

	if (v->null() || !v->has_attribute(m)) return 0;
	luaW_pushscalar(L, (*v)[m]);
	return 1;
}

/**
 * Returns the number of a child of a vconfig object.
 */
static int impl_vconfig_size(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));
	lua_pushinteger(L, v->null() ? 0 :
		std::distance(v->ordered_begin(), v->ordered_end()));
	return 1;
}

/**
 * Destroys a vconfig object before it is collected (__gc metamethod).
 */
static int impl_vconfig_collect(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));
	v->vconfig::~vconfig();
	return 0;
}

#define return_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		luaW_pushtstring(L, accessor); \
		return 1; \
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

#define return_float_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushnumber(L, accessor); \
		return 1; \
	}

#define return_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushboolean(L, accessor); \
		return 1; \
	}

#define return_cfg_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		config cfg; \
		accessor; \
		lua_newtable(L); \
		table_of_wml_config(L, cfg); \
		return 1; \
	}

#define return_cfgref_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_newtable(L); \
		table_of_wml_config(L, accessor); \
		return 1; \
	}

#define modify_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		if (lua_type(L, -1) == LUA_TUSERDATA) { \
			lua_pushlightuserdata(L, (void *)&tstringKey); \
			lua_rawget(L, LUA_REGISTRYINDEX); \
			if (!lua_getmetatable(L, -2) || !lua_rawequal(L, -1, -2)) { \
				error_buffer = "(translatable) string"; \
				goto error_call_destructors_modify; \
			} \
			const t_string &value = *static_cast<t_string *>(lua_touserdata(L, -3)); \
			accessor; \
		} else { \
			const char *value = lua_tostring(L, -1); \
			if (!value) { \
				error_buffer = "(translatable) string"; \
				goto error_call_destructors_modify; \
			} \
			accessor; \
		} \
		return 0; \
	}

#define modify_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		const char *value = lua_tostring(L, -1); \
		if (!value) { \
			error_buffer = "string"; \
			goto error_call_destructors_modify; \
		} \
		accessor; \
		return 0; \
	}

#define modify_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		if (!lua_isnumber(L, -1)) { \
			error_buffer = "integer"; \
			goto error_call_destructors_modify; \
		} \
		int value = lua_tointeger(L, -1); \
		accessor; \
		return 0; \
	}

#define modify_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		int value = lua_toboolean(L, -1); \
		accessor; \
		return 0; \
	}

/**
 * Gets some data on a unit type (__index metamethod).
 * - Arg 1: table containing an "id" field.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_type_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	lua_pushstring(L, "id");
	lua_rawget(L, 1);
	const unit_type *utp = unit_types.find(lua_tostring(L, -1));
	if (!utp) return luaL_argerror(L, 1, "unknown unit type");
	unit_type const &ut = *utp;

	// Find the corresponding attribute.
	return_tstring_attrib("name", ut.type_name());
	return_int_attrib("max_hitpoints", ut.hitpoints());
	return_int_attrib("max_moves", ut.movement());
	return_int_attrib("max_experience", ut.experience_needed());
	return_int_attrib("cost", ut.cost());
	return_int_attrib("level", ut.level());
	return_cfgref_attrib("__cfg", ut.get_cfg());
	return 0;
}

/**
 * Gets the unit type corresponding to an id.
 * - Arg 1: string containing the unit type id.
 * - Ret 1: table with an "id" field and with __index pointing to lua_unit_type_get.
 */
static int intf_get_unit_type(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (!unit_types.unit_type_exists(m)) return 0;

	lua_createtable(L, 0, 1);
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "id");
	lua_pushlightuserdata(L, (void *)&gettypeKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Gets the ids of all the unit types.
 * - Ret 1: table containing the ids.
 */
static int intf_get_unit_type_ids(lua_State *L)
{
	lua_newtable(L);
	int i = 1;
	BOOST_FOREACH (const unit_type_data::unit_type_map::value_type &ut, unit_types.types())
	{
		lua_pushstring(L, ut.first.c_str());
		lua_rawseti(L, -2, i);
		++i;
	}
	return 1;
}

/**
 * Destroys a unit object before it is collected (__gc metamethod).
 */
static int impl_unit_collect(lua_State *L)
{
	lua_unit *u = static_cast<lua_unit *>(lua_touserdata(L, 1));
	u->lua_unit::~lua_unit();
	return 0;
}

/**
 * Gets some data on a unit (__index metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_get(lua_State *L)
{
	unit const *pu = static_cast<lua_unit *>(lua_touserdata(L, 1))->get();
	char const *m = luaL_checkstring(L, 2);
	if (!pu) return luaL_argerror(L, 1, "unknown unit");
	unit const &u = *pu;

	// Find the corresponding attribute.
	return_int_attrib("x", u.get_location().x + 1);
	return_int_attrib("y", u.get_location().y + 1);
	return_int_attrib("side", u.side());
	return_string_attrib("id", u.id());
	return_string_attrib("type", u.type_id());
	return_int_attrib("hitpoints", u.hitpoints());
	return_int_attrib("max_hitpoints", u.max_hitpoints());
	return_int_attrib("experience", u.experience());
	return_int_attrib("max_experience", u.max_experience());
	return_int_attrib("moves", u.movement_left());
	return_int_attrib("max_moves", u.total_movement());
	return_tstring_attrib("name", u.name());
	return_bool_attrib("canrecruit", u.can_recruit());
	return_bool_attrib("petrified", u.incapacitated());
	return_bool_attrib("resting", u.resting());
	return_string_attrib("role", u.get_role());
	return_string_attrib("facing", map_location::write_direction(u.facing()));
	return_cfg_attrib("__cfg", u.write(cfg); u.get_location().write(cfg));
	return 0;
}

/**
 * Sets some data on a unit (__newindex metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_unit_set(lua_State *L)
{
	if (false) {
		error_call_destructors_2:
		return luaL_argerror(L, 2, "unknown modifiable property");
		error_call_destructors_modify:
		return luaL_typerror(L, 3, error_buffer.c_str());
	}

	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);
	lua_settop(L, 3);
	unit *pu = lu->get();
	if (!pu) return luaL_argerror(L, 1, "unknown unit");
	unit &u = *pu;

	// Find the corresponding attribute.
	modify_int_attrib("side", u.set_side(value));
	modify_int_attrib("moves", u.set_movement(value));
	modify_bool_attrib("resting", u.set_resting(value != 0));
	modify_tstring_attrib("name", u.set_name(value));
	modify_string_attrib("role", u.set_role(value));
	modify_string_attrib("facing", u.set_facing(map_location::parse_direction(value)));
	if (!lu->on_map()) {
		map_location loc = u.get_location();
		modify_int_attrib("x", loc.x = value - 1; u.set_location(loc));
		modify_int_attrib("y", loc.y = value - 1; u.set_location(loc));
	}
	goto error_call_destructors_2;
}

/**
 * Gets the numeric ids of all the units.
 * - Arg 1: optional table containing a filter
 * - Ret 1: table containing full userdata with __index pointing to
 *          impl_unit_get and __newindex pointing to impl_unit_set.
 */
static int intf_get_units(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	vconfig filter;
	if (!luaW_tovconfig(L, 1, filter, false))
		goto error_call_destructors;

	// Go through all the units while keeping the following stack:
	// 1: metatable, 2: return table, 3: userdata, 4: metatable copy
	lua_settop(L, 0);
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	int i = 1;
	unit_map &units = *resources::units;
	for (unit_map::const_unit_iterator ui = units.begin(), ui_end = units.end();
	     ui != ui_end; ++ui)
	{
		if (!filter.null() && !ui->second.matches_filter(filter, ui->first))
			continue;
		new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->second.underlying_id());
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
 */
static int intf_fire(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 2, "WML table");
	}

	char const *m = luaL_checkstring(L, 1);

	vconfig vcfg;
	if (!luaW_tovconfig(L, 2, vcfg))
		goto error_call_destructors;

	game_events::handle_event_command(m, queued_event_context::get(), vcfg);
	return 0;
}

/**
 * Fires an event.
 * - Arg 1: string containing the event name.
 * - Arg 2,3: optional first location.
 * - Arg 4,5: optional second location.
 * - Arg 6: optional WML table used as the [weapon] tag.
 * - Arg 7: optional WML table used as the [second_weapon] tag.
 * - Ret 1: boolean indicating whether the event was processed or not.
 */
static int intf_fire_event(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);

	int pos = 2;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, pos, "WML table");
	}

	map_location l1, l2;
	config data;

	if (lua_isnumber(L, 2)) {
		l1 = map_location(lua_tointeger(L, 2) - 1, lua_tointeger(L, 3) - 1);
		if (lua_isnumber(L, 4)) {
			l2 = map_location(lua_tointeger(L, 4) - 1, lua_tointeger(L, 5) - 1);
			pos = 6;
		} else pos = 4;
	}

	if (!lua_isnoneornil(L, pos)) {
		if (!luaW_toconfig(L, pos, data.add_child("first")))
			goto error_call_destructors;
	}
	++pos;
	if (!lua_isnoneornil(L, pos)) {
		if (!luaW_toconfig(L, pos, data.add_child("second")))
			goto error_call_destructors;
	}

	bool b = game_events::fire(m, l1, l2, data);
	lua_pushboolean(L, b);
	return 1;
}

/**
 * Gets a WML variable.
 * - Arg 1: string containing the variable name.
 * - Arg 2: optional bool indicating if tables for containers should be left empty.
 * - Ret 1: value of the variable, if any.
 */
static int intf_get_variable(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	variable_info v(m, false, variable_info::TYPE_SCALAR);
	if (v.is_valid) {
		luaW_pushscalar(L, v.as_scalar());
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
static int intf_set_variable(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 2, "WML table or scalar");
	}

	if (lua_isnoneornil(L, 2)) {
		resources::state_of_game->clear_variable(m);
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
			if (luaW_hasmetatable(L, 2, tstringKey)) {
				v.as_scalar() = *static_cast<t_string *>(lua_touserdata(L, 2));
				break;
			}
			// no break
		case LUA_TTABLE:
		{
			config &cfg = v.as_container();
			cfg.clear();
			if (!luaW_toconfig(L, 2, cfg))
				goto error_call_destructors;
			break;
		}
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
static int intf_dofile(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 1, "file not found");
		error_call_destructors_2:
		return lua_error(L);
		continue_call_destructor:
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L);
	}
	std::string p = get_wml_location(m);
	if (p.empty())
		goto error_call_destructors_1;

	lua_settop(L, 0);
	if (luaL_loadfile(L, p.c_str()))
		goto error_call_destructors_2;

	goto continue_call_destructor;
}

/**
 * Loads and executes a Lua file, if there is no corresponding entry in wesnoth.package.
 * Stores the result of the script in wesnoth.package and returns it.
 * - Arg 1: string containing the file name.
 * - Ret 1: value returned by the script.
 */
static int intf_require(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 1, "file not found");
	}

	// Check if there is already an entry.
	lua_pushstring(L, "wesnoth");
	lua_rawget(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "package");
	lua_rawget(L, -2);
	lua_pushvalue(L, 1);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) return 1;
	lua_pop(L, 1);

	std::string p = get_wml_location(m);
	if (p.empty())
		goto error_call_destructors_1;

	// Compile the file.
	int res = luaL_loadfile(L, p.c_str());
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		chat_message("Lua error", m);
		ERR_LUA << m << '\n';
		return 0;
	}

	// Execute it.
	if (!luaW_pcall(L, 0, 1)) return 0;

	// Add the return value to the table.
	lua_pushvalue(L, 1);
	lua_pushvalue(L, -2);
	lua_settable(L, -4);
	return 1;
}

/**
 * Calls a WML action handler (__call metamethod).
 * - Arg 1: userdata containing the handler.
 * - Arg 2: optional WML config.
 */
static int impl_wml_action_call(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 2, "WML table");
	}

	vconfig vcfg;
	if (!luaW_tovconfig(L, 2, vcfg))
		goto error_call_destructors;

	game_events::action_handler **h =
		static_cast<game_events::action_handler **>(lua_touserdata(L, 1));
	// Hidden metamethod, so h has to be an action handler.
	(*h)->handle(queued_event_context::get(), vcfg);
	return 0;
}

/**
 * Destroys a WML action handler before its pointer is collected (__gc metamethod).
 * - Arg 1: userdata containing the handler.
 */
static int impl_wml_action_collect(lua_State *L)
{
	game_events::action_handler **h =
		static_cast<game_events::action_handler **>(lua_touserdata(L, 1));
	// Hidden metamethod, so h has to be an action handler.
	delete *h;
	return 0;
}

/**
 * Calls the first upvalue and passes the first argument.
 * - Arg 1: optional WML config.
 */
static int cfun_wml_action_proxy(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	lua_pushvalue(L, lua_upvalueindex(1));

	switch (lua_type(L, 1))
	{
		case LUA_TTABLE:
		{
			config cfg;
			if (!luaW_toconfig(L, 1, cfg))
				goto error_call_destructors;
			luaW_pushvconfig(L, cfg);
			break;
		}
		case LUA_TUSERDATA:
		{
			if (!luaW_hasmetatable(L, 1, vconfigKey))
				goto error_call_destructors;
			lua_pushvalue(L, 1);
			break;
		}
		case LUA_TNONE:
		case LUA_TNIL:
			lua_createtable(L, 0, 0);
			break;
		default:
			goto error_call_destructors;
	}

	lua_call(L, 1, 0);
	return 0;
}

/**
 * Proxy class for calling WML action handlers defined in Lua.
 */
struct lua_action_handler : game_events::action_handler
{
	lua_State *L;
	int num;

	lua_action_handler(lua_State *l, int n) : L(l), num(n) {}
	void handle(const game_events::queued_event &, const vconfig &);
	~lua_action_handler();
};

void lua_action_handler::handle(const game_events::queued_event &ev, const vconfig &cfg)
{
	// Load the user function from the registry.
	lua_pushlightuserdata(L, (void *)&uactionKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, num);
	lua_remove(L, -2);

	// Push the WML table argument.
	new(lua_newuserdata(L, sizeof(vconfig))) vconfig(cfg);
	lua_pushlightuserdata(L, (void *)&vconfigKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);

	queued_event_context dummy(&ev);
	luaW_pcall(L, 1, 0, true);
}

lua_action_handler::~lua_action_handler()
{
	// Remove the function from the registry, so that it can be collected.
	lua_pushlightuserdata(L, (void *)&uactionKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num);
	lua_pop(L, 1);
}

/**
 * Registers a function as WML action handler.
 * - Arg 1: string containing the WML tag.
 * - Arg 2: optional function taking a WML table as argument.
 * - Ret 1: previous action handler, if any.
 */
static int intf_register_wml_action(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	bool enable = !lua_isnoneornil(L, 2);

	// Retrieve the user action table from the registry.
	lua_pushlightuserdata(L, (void *)&uactionKey);
	lua_rawget(L, LUA_REGISTRYINDEX);

	lua_action_handler *h = NULL;
	if (enable)
	{
		// Push the function in the table so that it is not collected.
		size_t length = lua_objlen(L, -1);
		lua_pushvalue(L, 2);
		lua_rawseti(L, -2, length + 1);

		// Create the proxy C++ action handler.
		h = new lua_action_handler(L, length + 1);
	}

	// Register the new handler and retrieve the previous one.
	game_events::action_handler *previous;
	game_events::register_action_handler(m, h, &previous);
	if (!previous) return 0;

	// Detect if the previous handler was already from Lua.
	lua_action_handler *lua_prev = dynamic_cast<lua_action_handler *>(previous);
	if (lua_prev)
	{
		// Extract the function from the table,
		// and put a lightweight wrapper around it.
		lua_rawgeti(L, -1, lua_prev->num);
		lua_pushcclosure(L, cfun_wml_action_proxy, 1);

		// Delete the old heavyweight wraper.
		delete lua_prev;
		return 1;
	}

	// Wrap and return the previous handler.
	void *p = lua_newuserdata(L, sizeof(game_events::action_handler *));
	*static_cast<game_events::action_handler **>(p) = previous;
	lua_pushlightuserdata(L, (void *)&wactionKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Gets some data on a side (__index metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_side_get(lua_State *L)
{
	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t = **static_cast<team **>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("gold", t.gold());
	return_tstring_attrib("objectives", t.objectives());
	return_int_attrib("village_gold", t.village_gold());
	return_int_attrib("base_income", t.base_income());
	return_int_attrib("total_income", t.total_income());
	return_bool_attrib("objectives_changed", t.objectives_changed());
	return_tstring_attrib("user_team_name", t.user_team_name());
	return_string_attrib("team_name", t.team_name());

	if (strcmp(m, "recruit") == 0) {
		std::set<std::string> const &recruits = t.recruits();
		lua_createtable(L, recruits.size(), 0);
		int i = 1;
		BOOST_FOREACH (std::string const &r, t.recruits()) {
			lua_pushstring(L, r.c_str());
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}

	return_cfg_attrib("__cfg", t.write(cfg));
	return 0;
}

/**
 * Sets some data on a side (__newindex metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_side_set(lua_State *L)
{
	if (false) {
		error_call_destructors_modify:
		return luaL_typerror(L, 3, error_buffer.c_str());
	}

	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t = **static_cast<team **>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);
	lua_settop(L, 3);

	// Find the corresponding attribute.
	modify_int_attrib("gold", t.set_gold(value));
	modify_tstring_attrib("objectives", t.set_objectives(value, true));
	modify_int_attrib("village_gold", t.set_village_gold(value));
	modify_int_attrib("base_income", t.set_base_income(value));
	modify_bool_attrib("objectives_changed", t.set_objectives_changed(value != 0));
	modify_tstring_attrib("user_team_name", t.change_team(t.team_name(), value));
	modify_string_attrib("team_name", t.change_team(value, t.user_team_name()));

	if (strcmp(m, "recruit") == 0) {
		t.set_recruits(std::set<std::string>());
		if (!lua_istable(L, 3)) return 0;
		for (int i = 1;; ++i) {
			lua_rawgeti(L, 3, i);
			if (lua_isnil(L, -1)) break;
			t.add_recruit(lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		return 0;
	}

	return luaL_argerror(L, 2, "unknown modifiable property");
}

/**
 * Gets a proxy userdata for a side.
 * - Arg 1: integer for the side.
 * - Ret 1: full userdata with __index pointing to lua_side_get
 *          and __newindex pointing to lua_side_set.
 */
static int intf_get_side(lua_State *L)
{
	int s = luaL_checkint(L, 1);

	size_t t = s - 1;
	std::vector<team> &teams = *resources::teams;
	if (t >= teams.size()) return 0;

	// Create a full userdata containing a pointer to the team.
	team **p = static_cast<team **>(lua_newuserdata(L, sizeof(team *)));
	*p = &teams[t];

	// Get the metatable from the registry and set it.
	lua_pushlightuserdata(L, (void *)&getsideKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);

	return 1;
}

/**
 * Gets a terrain code.
 * - Args 1,2: map location.
 * - Ret 1: string.
 */
static int intf_get_terrain(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);

	t_translation::t_terrain const &t = resources::game_map->
		get_terrain(map_location(x - 1, y - 1));
	lua_pushstring(L, t_translation::write_terrain_code(t).c_str());
	return 1;
}

/**
 * Sets a terrain code.
 * - Args 1,2: map location.
 * - Arg 3: terrain code string.
 */
static int intf_set_terrain(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	char const *m = luaL_checkstring(L, 3);

	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	if (t != t_translation::NONE_TERRAIN)
		change_terrain(map_location(x - 1, y - 1), t, gamemap::BOTH, false);
	return 0;
}

/**
 * Gets details about a terrain.
 * - Arg 1: terrain code string.
 * - Ret 1: table.
 */
static int intf_get_terrain_info(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	if (t == t_translation::NONE_TERRAIN) return 0;
	terrain_type const &info = resources::game_map->get_terrain_info(t);

	lua_newtable(L);
	lua_pushstring(L, info.id().c_str());
	lua_setfield(L, -2, "id");
	luaW_pushtstring(L, info.name());
	lua_setfield(L, -2, "name");
	luaW_pushtstring(L, info.description());
	lua_setfield(L, -2, "description");
	lua_pushboolean(L, info.is_village());
	lua_setfield(L, -2, "village");
	lua_pushboolean(L, info.is_castle());
	lua_setfield(L, -2, "castle");
	lua_pushboolean(L, info.is_keep());
	lua_setfield(L, -2, "keep");
	lua_pushinteger(L, info.gives_healing());
	lua_setfield(L, -2, "healing");

	return 1;
}

/**
 * Gets the side of a village owner.
 * - Args 1,2: map location.
 * - Ret 1: integer.
 */
static int intf_get_village_owner(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);

	map_location loc(x - 1, y - 1);
	if (!resources::game_map->is_village(loc))
		return 0;

	int side = village_owner(loc, *resources::teams) + 1;
	if (!side) return 0;
	lua_pushinteger(L, side);
	return 1;
}

/**
 * Sets the owner of a village.
 * - Args 1,2: map location.
 * - Arg 3: integer for the side or empty to remove ownership.
 */
static int intf_set_village_owner(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int new_side = lua_isnoneornil(L, 3) ? 0 : luaL_checkint(L, 3);

	std::vector<team> &teams = *resources::teams;
	map_location loc(x - 1, y - 1);
	if (!resources::game_map->is_village(loc))
		return 0;

	int old_side = village_owner(loc, teams) + 1;
	if (new_side == old_side || new_side < 0 || new_side > (int)teams.size())
		return 0;

	if (old_side) teams[old_side - 1].lose_village(loc);
	if (new_side) teams[new_side - 1].get_village(loc);
	return 0;
}

/**
 * Returns the map size.
 * - Ret 1: width.
 * - Ret 2: height.
 * - Ret 3: border size.
 */
static int intf_get_map_size(lua_State *L)
{
	const gamemap &map = *resources::game_map;
	lua_pushinteger(L, map.w());
	lua_pushinteger(L, map.h());
	lua_pushinteger(L, map.border_size());
	return 3;
}

/**
 * Returns the currently selected tile.
 * - Ret 1: x.
 * - Ret 2: y.
 */
static int intf_get_selected_tile(lua_State *L)
{
	const map_location &loc = resources::screen->selected_hex();
	if (!resources::game_map->on_board(loc)) return 0;
	lua_pushinteger(L, loc.x + 1);
	lua_pushinteger(L, loc.y + 1);
	return 2;
}

/**
 * Gets some game_config data (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_game_config_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("base_income", game_config::base_income);
	return_int_attrib("village_income", game_config::village_income);
	return_int_attrib("poison_amount", game_config::poison_amount);
	return_int_attrib("rest_heal_amount", game_config::rest_heal_amount);
	return_int_attrib("recall_cost", game_config::recall_cost);
	return_int_attrib("kill_experience", game_config::kill_experience);
	return_string_attrib("version", game_config::version);
	return_bool_attrib("debug", game_config::debug);
	return 0;
}

/**
 * Sets some game_config data (__newindex metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_game_config_set(lua_State *L)
{
	if (false) {
		error_call_destructors_modify:
		return luaL_typerror(L, 3, error_buffer.c_str());
	}
	char const *m = luaL_checkstring(L, 2);
	lua_settop(L, 3);

	// Find the corresponding attribute.
	modify_int_attrib("base_income", game_config::base_income = value);
	modify_int_attrib("village_income", game_config::village_income = value);
	modify_int_attrib("poison_amount", game_config::poison_amount = value);
	modify_int_attrib("rest_heal_amount", game_config::rest_heal_amount = value);
	modify_int_attrib("recall_cost", game_config::recall_cost = value);
	modify_int_attrib("kill_experience", game_config::kill_experience = value);
	return luaL_argerror(L, 2, "unknown modifiable property");
}

/**
 * Gets some data about current point of game (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_current_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("side", resources::controller->current_side());
	return_int_attrib("turn", resources::controller->turn());

	if (strcmp(m, "event_context") == 0)
	{
		const game_events::queued_event &ev = queued_event_context::get();
		config cfg;
		cfg["name"] = ev.name;
		if (const config &weapon = ev.data.child("first")) {
			cfg.add_child("weapon", weapon);
		}
		if (const config &weapon = ev.data.child("second")) {
			cfg.add_child("second_weapon", weapon);
		}
		if (ev.loc1.valid()) {
			cfg["x1"] = str_cast(ev.loc1.x + 1);
			cfg["y1"] = str_cast(ev.loc1.y + 1);
		}
		if (ev.loc2.valid()) {
			cfg["x2"] = str_cast(ev.loc2.x + 1);
			cfg["y2"] = str_cast(ev.loc2.y + 1);
		}
		lua_newtable(L);
		table_of_wml_config(L, cfg);
		return 1;
	}

	return 0;
}

/**
 * Displays a message in the chat window and in the logs.
 * - Arg 1: optional message header.
 * - Arg 2 (or 1): message.
 */
static int intf_message(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	char const *h = m;
	if (lua_isnone(L, 2)) {
		h = "Lua";
	} else {
		m = luaL_checkstring(L, 2);
	}
	chat_message(h, m);
	LOG_LUA << "Script says: \"" << m << "\"\n";
	return 0;
}

/**
 * Evaluates a boolean WML conditional.
 * - Arg 1: WML table.
 * - Ret 1: boolean.
 */
static int intf_eval_conditional(lua_State *L)
{
	if (lua_isnoneornil(L, 1)) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	vconfig cond;
	if (!luaW_tovconfig(L, 1, cond, false))
		goto error_call_destructors;

	bool b = game_events::conditional_passed(resources::units, cond);
	lua_pushboolean(L, b);
	return 1;
}

/**
 * Cost function object relying on a Lua function.
 * @note The stack index of the Lua function must be valid each time the cost is computed.
 */
struct lua_calculator : pathfind::cost_calculator
{
	lua_State *L;
	int index;

	lua_calculator(lua_State *L_, int i): L(L_), index(i) {}
	double cost(const map_location &loc, double so_far) const;
};

double lua_calculator::cost(const map_location &loc, double so_far) const
{
	// Copy the user function and push the location and current cost.
	lua_pushvalue(L, index);
	lua_pushinteger(L, loc.x + 1);
	lua_pushinteger(L, loc.y + 1);
	lua_pushnumber(L, so_far);

	// Execute the user function.
	if (!luaW_pcall(L, 3, 1)) return 1.;

	// Return a cost of at least 1 mp to avoid issues in pathfinder.
	// (Condition is inverted to detect NaNs.)
	double cost = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return !(cost >= 1.) ? 1. : cost;
}

/**
 * Finds a path between two locations.
 * - Args 1,2: source location. (Or Arg 1: unit.)
 * - Args 3,4: destination.
 * - Arg 5: optional cost function or
 *          table (optional fields: ignore_units, ignore_teleport, max_cost, viewing_side).
 * - Ret 1: array of pairs containing path steps.
 * - Ret 2: path cost.
 */
static int intf_find_path(lua_State *L)
{
	int arg = 1;
	if (false) {
		error_call_destructors_1:
		return luaL_typerror(L, 1, "unit");
		error_call_destructors_2:
		return luaL_typerror(L, arg, "number");
		error_call_destructors_3:
		return luaL_argerror(L, 1, "no unit found");
	}

	map_location src, dst;
	unit_map &units = *resources::units;
	const unit *u = NULL;

	if (lua_isuserdata(L, arg))
	{
		u = luaW_tounit(L, 1);
		if (!u) goto error_call_destructors_1;
		src = u->get_location();
		++arg;
	}
	else
	{
		if (!lua_isnumber(L, arg))
			goto error_call_destructors_2;
		src.x = lua_tointeger(L, arg) - 1;
		++arg;
		if (!lua_isnumber(L, arg))
			goto error_call_destructors_2;
		src.y = lua_tointeger(L, arg) - 1;
		unit_map::const_unit_iterator ui = units.find(src);
		if (ui.valid()) u = &ui->second;
		++arg;
	}

	if (!lua_isnumber(L, arg))
		goto error_call_destructors_2;
	dst.x = lua_tointeger(L, arg) - 1;
	++arg;
	if (!lua_isnumber(L, arg))
		goto error_call_destructors_2;
	dst.y = lua_tointeger(L, arg) - 1;
	++arg;

	std::vector<team> &teams = *resources::teams;
	gamemap &map = *resources::game_map;
	int viewing_side = 0;
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	double stop_at = 10000;
	pathfind::cost_calculator *calc = NULL;

	if (lua_istable(L, arg))
	{
		lua_pushstring(L, "ignore_units");
		lua_rawget(L, arg);
		ignore_units = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "ignore_teleport");
		lua_rawget(L, arg);
		ignore_teleport = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "max_cost");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
			stop_at = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = lua_tointeger(L, -1);
			if (i >= 1 && i <= int(teams.size())) viewing_side = i;
			else see_all = true;
		}
		lua_pop(L, 1);
	}
	else if (lua_isfunction(L, arg))
	{
		calc = new lua_calculator(L, arg);
	}

	pathfind::teleport_map teleport_locations;

	if (!calc) {
		if (!u) goto error_call_destructors_3;

		team &viewing_team = teams[(viewing_side ? viewing_side : u->side()) - 1];
		if (!ignore_teleport) {
			teleport_locations = pathfind::get_teleport_locations(
				*u, units, viewing_team, see_all, ignore_units);
		}
		calc = new pathfind::shortest_path_calculator(*u, viewing_team,
			units, teams, map, ignore_units, false, see_all);
	}

	pathfind::plain_route res = pathfind::a_star_search(src, dst, stop_at, calc, map.w(), map.h(),
		&teleport_locations);
	delete calc;

	int nb = res.steps.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, res.steps[i].x + 1);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, res.steps[i].y + 1);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushinteger(L, res.move_cost);

	return 2;
}

/**
 * Finds all the locations reachable by a unit.
 * - Args 1,2: source location. (Or Arg 1: unit.)
 * - Arg 3: optional table (optional fields: ignore_units, ignore_teleport, additional_turns, viewing_side).
 * - Ret 1: array of triples (coordinates + remaining movement).
 */
static int intf_find_reach(lua_State *L)
{
	int arg = 1;
	if (false) {
		error_call_destructors_1:
		return luaL_typerror(L, 1, "unit");
		error_call_destructors_2:
		return luaL_typerror(L, arg, "number");
		error_call_destructors_3:
		return luaL_argerror(L, 1, "no unit found");
	}

	map_location src;
	unit_map &units = *resources::units;
	const unit *u = NULL;

	if (lua_isuserdata(L, arg))
	{
		u = luaW_tounit(L, 1);
		if (!u) goto error_call_destructors_1;
		src = u->get_location();
		++arg;
	}
	else
	{
		if (!lua_isnumber(L, arg))
			goto error_call_destructors_2;
		src.x = lua_tointeger(L, arg) - 1;
		++arg;
		if (!lua_isnumber(L, arg))
			goto error_call_destructors_2;
		src.y = lua_tointeger(L, arg) - 1;
		unit_map::const_unit_iterator ui = units.find(src);
		if (!ui.valid())
			goto error_call_destructors_3;
		u = &ui->second;
		++arg;
	}

	std::vector<team> &teams = *resources::teams;
	gamemap &map = *resources::game_map;
	int viewing_side = 0;
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	int additional_turns = 0;

	if (lua_istable(L, arg))
	{
		lua_pushstring(L, "ignore_units");
		lua_rawget(L, arg);
		ignore_units = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "ignore_teleport");
		lua_rawget(L, arg);
		ignore_teleport = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "additional_turns");
		lua_rawget(L, arg);
		additional_turns = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = lua_tointeger(L, -1);
			if (i >= 1 && i <= int(teams.size())) viewing_side = i;
			else see_all = true;
		}
		lua_pop(L, 1);
	}

	team &viewing_team = teams[(viewing_side ? viewing_side : u->side()) - 1];
	pathfind::paths res(map, units, src, teams, ignore_units, !ignore_teleport,
		viewing_team, additional_turns, see_all, ignore_units);

	int nb = res.destinations.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		pathfind::paths::step &s = res.destinations[i];
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, s.curr.x + 1);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, s.curr.y + 1);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, s.move_left);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

/**
 * Places a unit on the map.
 * - Args 1,2: (optional) location.
 * - Arg 3: WML table describing a unit, or nothing/nil to delete.
 */
static int intf_put_unit(lua_State *L)
{
	int unit_arg = 1;
	if (false) {
		error_call_destructors_1:
		return luaL_typerror(L, unit_arg, "WML table or unit");
		error_call_destructors_2:
		return luaL_argerror(L, unit_arg, error_buffer.c_str());
		error_call_destructors_3:
		return luaL_argerror(L, 1, "invalid location");
		error_call_destructors_4:
		return luaL_argerror(L, unit_arg, "unit not found");
	}

	lua_unit *lu = NULL;
	unit *u = NULL;
	map_location loc;
	if (lua_isnumber(L, 1)) {
		unit_arg = 3;
		loc.x = lua_tointeger(L, 1) - 1;
		loc.y = lua_tointeger(L, 2) - 1;
		if (!resources::game_map->on_board(loc))
			goto error_call_destructors_3;
	}

	if (luaW_hasmetatable(L, unit_arg, getunitKey))
	{
		lu = static_cast<lua_unit *>(lua_touserdata(L, unit_arg));
		u = lu->get();
		if (!u) goto error_call_destructors_4;
		if (lu->on_map()) {
			if (unit_arg == 1 || u->get_location() == loc) return 0;
			resources::units->erase(loc);
			resources::units->move(u->get_location(), loc);
			return 0;
		}
		if (unit_arg == 1) {
			loc = u->get_location();
			if (!resources::game_map->on_board(loc))
				goto error_call_destructors_3;
		}
	}
	else if (!lua_isnoneornil(L, unit_arg))
	{
		config cfg;
		if (!luaW_toconfig(L, unit_arg, cfg))
			goto error_call_destructors_1;
		if (unit_arg == 1) {
			loc.x = lexical_cast_default(cfg["x"], 0) - 1;
			loc.y = lexical_cast_default(cfg["y"], 0) - 1;
			if (!resources::game_map->on_board(loc))
				goto error_call_destructors_3;
		}
		try {
			u = new unit(resources::units, cfg, true, resources::state_of_game);
		} catch (const game::error &e) {
			error_buffer = "broken unit WML [" + e.message + "]";
			goto error_call_destructors_2;
		}
	}

	resources::units->erase(loc);
	if (u) {
		resources::units->add(loc, *u);
		if (!lu) delete u;
		else lu->reload();
	}

	return 0;
}

/**
 * Finds a vacant tile.
 * - Args 1,2: location.
 * - Arg 3: optional unit for checking movement type.
 */
static int intf_find_vacant_tile(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 3, "unit");
	}

	int x = luaL_checkint(L, 1) - 1, y = luaL_checkint(L, 2) - 1;

	const unit *u = NULL;
	bool fake_unit = false;
	if (!lua_isnoneornil(L, 3)) {
		if (luaW_hasmetatable(L, 3, getunitKey)) {
			u = static_cast<lua_unit *>(lua_touserdata(L, 3))->get();
		} else {
			config cfg;
			if (!luaW_toconfig(L, 3, cfg))
				goto error_call_destructors;
			try {
				u = new unit(resources::units, cfg, false, resources::state_of_game);
			} catch (const game::error &) {
				goto error_call_destructors;
			}
			fake_unit = true;
		}
	}

	map_location res = find_vacant_tile(*resources::game_map,
		*resources::units, map_location(x, y), pathfind::VACANT_ANY, u);

	if (fake_unit) delete u;

	if (!res.valid()) return 0;
	lua_pushinteger(L, res.x + 1);
	lua_pushinteger(L, res.y + 1);
	return 2;
}

/**
 * Floats some text on the map.
 * - Args 1,2: location.
 * - Arg 3: string.
 */
static int intf_float_label(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 3, "invalid string");
	}

	map_location loc;
	loc.x = lua_tointeger(L, 1) - 1;
	loc.y = lua_tointeger(L, 2) - 1;

	t_string text;
	if (!luaW_totstring(L, 3, text)) goto error_call_destructors_1;
	resources::screen->float_label(loc, text, font::LABEL_COLOUR.r,
		font::LABEL_COLOUR.g, font::LABEL_COLOUR.b);
	return 0;
}

/**
 * Creates a unit from its WML description.
 * - Arg 1: WML table.
 */
static int intf_create_unit(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 1, "WML table");
		error_call_destructors_2:
		return luaL_argerror(L, 1, error_buffer.c_str());
	}

	config cfg;
	if (!luaW_toconfig(L, 1, cfg))
		goto error_call_destructors_1;
	try {
		unit *u = new unit(resources::units, cfg, true, resources::state_of_game);
		new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(u);
		lua_pushlightuserdata(L, (void *)&getunitKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
		return 1;
	} catch (const game::error &e) {
		error_buffer = "broken unit WML [" + e.message + "]";
		goto error_call_destructors_2;
	}
}

/**
 * Copies a unit.
 * - Arg 1: unit userdata.
 */
static int intf_copy_unit(lua_State *L)
{
	unit const *u = luaW_tounit(L, 1);
	if (!u) return luaL_typerror(L, 1, "unit");

	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(new unit(*u));
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Returns unit resistance against a given attack type.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the attack type.
 * - Arg 3: boolean indicating if attacker.
 * - Args 4/5: optional location.
 */
static int intf_unit_resistance(lua_State *L)
{
	unit const *u = luaW_tounit(L, 1);
	if (!u) return luaL_typerror(L, 1, "unit");
	char const *m = luaL_checkstring(L, 2);
	bool a = lua_toboolean(L, 3);

	map_location loc = u->get_location();
	if (!lua_isnoneornil(L, 4)) {
		loc.x = lua_tointeger(L, 4) - 1;
		loc.y = lua_tointeger(L, 5) - 1;
	}

	lua_pushinteger(L, u->resistance_against(m, a, loc));
	return 1;
}

/**
 * Returns unit movement cost on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 */
static int intf_unit_movement_cost(lua_State *L)
{
	unit const *u = luaW_tounit(L, 1);
	if (!u) return luaL_typerror(L, 1, "unit");
	char const *m = luaL_checkstring(L, 2);

	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u->movement_cost(t));
	return 1;
}

/**
 * Returns unit defense on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 */
static int intf_unit_defense(lua_State *L)
{
	unit const *u = luaW_tounit(L, 1);
	if (!u) return luaL_typerror(L, 1, "unit");
	char const *m = luaL_checkstring(L, 2);

	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u->defense_modifier(t));
	return 1;
}

/**
 * Puts a table at the top of the stack with some combat result.
 */
static void luaW_pushsimdata(lua_State *L, const combatant &cmb)
{
	int n = cmb.hp_dist.size();
	lua_createtable(L, 0, 4);
	lua_pushnumber(L, cmb.poisoned);
	lua_setfield(L, -2, "poisoned");
	lua_pushnumber(L, cmb.slowed);
	lua_setfield(L, -2, "slowed");
	lua_pushnumber(L, cmb.average_hp());
	lua_setfield(L, -2, "average_hp");
	lua_createtable(L, n, 0);
	for (int i = 0; i < n; ++i) {
		lua_pushnumber(L, cmb.hp_dist[i]);
		lua_rawseti(L, -2, i);
	}
	lua_setfield(L, -2, "hp_chance");
}

/**
 * Simulates a combat between two units.
 * - Arg 1: attacker userdata.
 * - Arg 2: optional weapon index.
 * - Arg 3: defender userdata.
 * - Arg 4: optional weapon index.
 * - Ret 1: attacker results.
 * - Ret 2: defender results.
 */
static int intf_simulate_combat(lua_State *L)
{
	int arg_num = 1, att_w = -1, def_w = -1;

	unit const *att = luaW_tounit(L, arg_num);
	if (!att) return luaL_typerror(L, arg_num, "unit");
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		att_w = lua_tointeger(L, arg_num) - 1;
		if (att_w < 0 || att_w >= int(att->attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	unit const *def = luaW_tounit(L, arg_num, true);
	if (!def) return luaL_typerror(L, arg_num, "unit");
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		def_w = lua_tointeger(L, arg_num) - 1;
		if (def_w < 0 || def_w >= int(def->attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	battle_context context(*resources::units, att->get_location(),
		def->get_location(), att_w, def_w, 0.0, NULL, att);

	luaW_pushsimdata(L, context.get_attacker_combatant());
	luaW_pushsimdata(L, context.get_defender_combatant());
	return 2;
}

LuaKernel::LuaKernel()
	: mState(luaL_newstate())
{
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
		{ "copy_unit",                &intf_copy_unit                },
		{ "create_unit",              &intf_create_unit              },
		{ "dofile",                   &intf_dofile                   },
		{ "eval_conditional",         &intf_eval_conditional         },
		{ "find_path",                &intf_find_path                },
		{ "find_reach",               &intf_find_reach               },
		{ "find_vacant_tile",         &intf_find_vacant_tile         },
		{ "fire",                     &intf_fire                     },
		{ "fire_event",               &intf_fire_event               },
		{ "float_label",              &intf_float_label              },
		{ "get_map_size",             &intf_get_map_size             },
		{ "get_selected_tile",        &intf_get_selected_tile        },
		{ "get_side",                 &intf_get_side                 },
		{ "get_terrain",              &intf_get_terrain              },
		{ "get_terrain_info",         &intf_get_terrain_info         },
		{ "get_unit_type",            &intf_get_unit_type            },
		{ "get_unit_type_ids",        &intf_get_unit_type_ids        },
		{ "get_units",                &intf_get_units                },
		{ "get_variable",             &intf_get_variable             },
		{ "get_village_owner",        &intf_get_village_owner        },
		{ "message",                  &intf_message                  },
		{ "put_unit",                 &intf_put_unit                 },
		{ "register_wml_action",      &intf_register_wml_action      },
		{ "require",                  &intf_require                  },
		{ "set_terrain",              &intf_set_terrain              },
		{ "set_variable",             &intf_set_variable             },
		{ "set_village_owner",        &intf_set_village_owner        },
		{ "simulate_combat",          &intf_simulate_combat          },
		{ "textdomain",               &intf_textdomain               },
		{ "unit_defense",             &intf_unit_defense             },
		{ "unit_movement_cost",       &intf_unit_movement_cost       },
		{ "unit_resistance",          &intf_unit_resistance          },
		{ NULL, NULL }
	};
	luaL_register(L, "wesnoth", callbacks);

	// Create the getside metatable.
	lua_pushlightuserdata(L, (void *)&getsideKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_side_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_side_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "side");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the gettext metatable.
	lua_pushlightuserdata(L, (void *)&gettextKey);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, impl_gettext);
	lua_setfield(L, -2, "__call");
	lua_pushstring(L, "message domain");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the gettype metatable.
	lua_pushlightuserdata(L, (void *)&gettypeKey);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, impl_unit_type_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "unit type");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the getunit metatable.
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_createtable(L, 0, 4);
	lua_pushcfunction(L, impl_unit_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, impl_unit_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_unit_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "unit");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the tstring metatable.
	lua_pushlightuserdata(L, (void *)&tstringKey);
	lua_createtable(L, 0, 4);
	lua_pushcfunction(L, impl_tstring_concat);
	lua_setfield(L, -2, "__concat");
	lua_pushcfunction(L, impl_tstring_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, impl_tstring_tostring);
	lua_setfield(L, -2, "__tostring");
	lua_pushstring(L, "translatable string");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the vconfig metatable.
	lua_pushlightuserdata(L, (void *)&vconfigKey);
	lua_createtable(L, 0, 4);
	lua_pushcfunction(L, impl_vconfig_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, impl_vconfig_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_vconfig_size);
	lua_setfield(L, -2, "__len");
	lua_pushstring(L, "wml object");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the wml action metatable.
	lua_pushlightuserdata(L, (void *)&wactionKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_wml_action_call);
	lua_setfield(L, -2, "__call");
	lua_pushcfunction(L, impl_wml_action_collect);
	lua_setfield(L, -2, "__gc");
	lua_pushstring(L, "wml action handler");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);


	// Create the ai elements table.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);


	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

	// Create the user action table.
	lua_pushlightuserdata(L, (void *)&uactionKey);
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the game_config variable with its metatable.
	lua_getglobal(L, "wesnoth");
	lua_newuserdata(L, 0);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_game_config_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_game_config_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "game config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "game_config");
	lua_pop(L, 1);

	// Create the current variable with its metatable.
	lua_getglobal(L, "wesnoth");
	lua_newuserdata(L, 0);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, impl_current_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "current config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "current");
	lua_pop(L, 1);

	// Create the package table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "package");
	lua_pop(L, 1);

	// Store the error handler, then close debug.
	lua_pushlightuserdata(L, (void *)&executeKey);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_setglobal(L, "debug");

	lua_settop(L, 0);
}

LuaKernel::~LuaKernel()
{
	lua_close(mState);
}

/**
 * Runs a script from an event handler.
 */
void LuaKernel::run_event(vconfig const &cfg, game_events::queued_event const &ev)
{
	lua_State *L = mState;

	// Get user-defined arguments; append locations and weapons to it.
	config args;
	vconfig vargs = cfg.child("args");
	if (!vargs.null()) {
		args = vargs.get_config();
	}
	if (const config &weapon = ev.data.child("first")) {
		args.add_child("weapon", weapon);
	}
	if (const config &weapon = ev.data.child("second")) {
		args.add_child("second_weapon", weapon);
	}
	if (ev.loc1.valid()) {
		args["x1"] = str_cast(ev.loc1.x + 1);
		args["y1"] = str_cast(ev.loc1.y + 1);
	}
	if (ev.loc2.valid()) {
		args["x2"] = str_cast(ev.loc2.x + 1);
		args["y2"] = str_cast(ev.loc2.y + 1);
	}

	// Get the code from the uninterpolated config object, so that $ symbols
	// are not messed with.
	const std::string &prog = cfg.get_config()["code"];

	queued_event_context dummy(&ev);
	luaW_pushvconfig(L, args);
	execute(prog.c_str(), 1, 0);
}

/**
 * Runs a script from a unit filter.
 * The script is an already compiled function given by its name.
 */
bool LuaKernel::run_filter(char const *name, unit const &u)
{
	lua_State *L = mState;

	unit_map::const_unit_iterator ui = resources::units->find(u.get_location());
	if (!ui.valid()) return false;

	// Get the user filter by name.
	lua_pushstring(L, name);
	lua_rawget(L, LUA_GLOBALSINDEX);

	// Pass the unit as argument.
	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->second.underlying_id());
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);

	if (!luaW_pcall(L, 1, 1)) return false;

	bool b = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return b;
}

/**
 * Runs a script on a stack containing @a nArgs arguments.
 * @return true if the script was successful and @a nRets return values are available.
 */
bool LuaKernel::execute(char const *prog, int nArgs, int nRets)
{
	lua_State *L = mState;

	// Compile script into a variadic function.
	int res = luaL_loadstring(L, prog);
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		chat_message("Lua error", m);
		ERR_LUA << m << '\n';
		lua_pop(L, 2);
		return false;
	}

	// Place the function before its arguments.
	if (nArgs)
		lua_insert(L, -1 - nArgs);

	return luaW_pcall(L, nArgs, nRets);
}

static int transform_ai_action(lua_State *L, ai::action_result_ptr action_result)
{
	lua_newtable(L);
	lua_pushboolean(L,action_result->is_ok());
	lua_setfield(L, -2, "ok");
	lua_pushboolean(L,action_result->is_gamestate_changed());
	lua_setfield(L, -2, "gamestate_changed");
	lua_pushinteger(L,action_result->get_status());
	lua_setfield(L, -2, "status");
	return 1;
}

static bool to_map_location(lua_State *L, int &index, map_location &res)
{
	if (lua_isuserdata(L, index))
	{
		if (!luaW_hasmetatable(L, index, getunitKey)) return false;
		unit const *u = static_cast<lua_unit *>(lua_touserdata(L, index))->get();
		if (!u) return false;
		res = u->get_location();
		++index;
	}
	else
	{
		if (!lua_isnumber(L, index)) return false;
		res.x = lua_tointeger(L, index) - 1;
		++index;
		if (!lua_isnumber(L, index)) return false;
		res.y = lua_tointeger(L, index) - 1;
		++index;
	}

	return true;
}

static int ai_execute_move(lua_State *L, bool remove_movement)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location from, to;
	if (!to_map_location(L, index, from)) goto error_call_destructors;
	if (!to_map_location(L, index, to)) goto error_call_destructors;
	ai::move_result_ptr move_result = ai::actions::execute_move_action(side,true,from,to,remove_movement);
	return transform_ai_action(L,move_result);
}

static int cfun_ai_execute_move_full(lua_State *L)
{
	return ai_execute_move(L, true);
}

/**
 * Loads the "package" package into the Lua environment.
 * This action is inherently unsafe, as Lua scripts will now be able to
 * load C libraries on their own, hence granting them the same privileges
 * as the Wesnoth binary itsef.
 */
void LuaKernel::load_package()
{
	lua_State *L = mState;
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, "package");
	lua_call(L, 1, 0);
}

static int cfun_ai_execute_move_partial(lua_State *L)
{
	return ai_execute_move(L, false);
}

static int cfun_ai_execute_attack(lua_State *L)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	ai::readonly_context &context = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context();

	int side = context.get_side();
	map_location attacker, defender;
	if (!to_map_location(L, index, attacker)) goto error_call_destructors;
	if (!to_map_location(L, index, defender)) goto error_call_destructors;

	int attacker_weapon = -1;//-1 means 'select what is best'
	double aggression = context.get_aggression();//use the aggression from the context

	if (!lua_isnoneornil(L, index+1) && lua_isnumber(L,index+1)) {
		aggression = lua_tonumber(L, index+1);
	}

	if (!lua_isnoneornil(L, index)) {
		attacker_weapon = lua_tointeger(L, index);
	}

	ai::attack_result_ptr attack_result = ai::actions::execute_attack_action(side,true,attacker,defender,attacker_weapon,aggression);
	return transform_ai_action(L,attack_result);
}

static int ai_execute_stopunit_select(lua_State *L, bool remove_movement, bool remove_attacks)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location loc;
	if (!to_map_location(L, index, loc)) goto error_call_destructors;

	ai::stopunit_result_ptr stopunit_result = ai::actions::execute_stopunit_action(side,true,loc,remove_movement,remove_attacks);
	return transform_ai_action(L,stopunit_result);
}

static int cfun_ai_execute_stopunit_moves(lua_State *L)
{
	return ai_execute_stopunit_select(L, true, false);
}

static int cfun_ai_execute_stopunit_attacks(lua_State *L)
{
	return ai_execute_stopunit_select(L, false, true);
}

static int cfun_ai_execute_stopunit_all(lua_State *L)
{
	return ai_execute_stopunit_select(L, true, true);
}

static int cfun_ai_execute_recruit(lua_State *L)
{
	const char *unit_name = luaL_checkstring(L, 1);
	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}

	ai::recruit_result_ptr recruit_result = ai::actions::execute_recruit_action(side,true,std::string(unit_name),where);
	return transform_ai_action(L,recruit_result);
}

static int cfun_ai_execute_recall(lua_State *L)
{
	const char *unit_id = luaL_checkstring(L, 1);
	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}

	ai::recall_result_ptr recall_result = ai::actions::execute_recall_action(side,true,std::string(unit_id),where);
	return transform_ai_action(L,recall_result);
}

lua_ai_context* LuaKernel::create_ai_context(char const *code, ai::engine_lua *engine)
{
	lua_State *L = mState;
	int res_ai = luaL_loadstring(L, code);//stack size is now 1 [ -1: ai_context]
	if (res_ai)
	{

		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while initializing ai:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return NULL;
	}
	//push data table here
	lua_newtable(L);// stack size is 2 [ -1: new table, -2: ai as string ]
	lua_pushinteger(L, engine->get_readonly_context().get_side());

	lua_setfield(L, -2, "side");//stack size is 2 [- 1: new table; -2 ai as string]

	static luaL_reg const callbacks[] = {
		{ "attack",           &cfun_ai_execute_attack           },
		{ "move",             &cfun_ai_execute_move_partial     },
		{ "move_full",        &cfun_ai_execute_move_full        },
		{ "recall",           &cfun_ai_execute_recall           },
		{ "recruit",          &cfun_ai_execute_recruit          },
		{ "stopunit_all",     &cfun_ai_execute_stopunit_all     },
		{ "stopunit_attacks", &cfun_ai_execute_stopunit_attacks },
		{ "stopunit_moves",   &cfun_ai_execute_stopunit_moves   },
		{ NULL, NULL }
	};

	for (const luaL_reg *p = callbacks; p->name; ++p) {
		lua_pushlightuserdata(L, engine);
		lua_pushcclosure(L, p->func, 1);
		lua_setfield(L, -2, p->name);
	}

	//compile the ai as a closure
	if (!luaW_pcall(L, 1, 1, true)) {
		return NULL;//return with stack size 0 []
	}

	// Retrieve the ai elements table from the registry.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length_ai = lua_objlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: ai_context  -2: ais_table  -3: ai_context]
	lua_rawseti(L, -2, length_ai + 1);// ais_table[length+1]=ai_context.  stack size is now 2 [-1: ais_table  -2: ai_context]
	lua_pop(L, 2);
	return new lua_ai_context(L, length_ai + 1, engine->get_readonly_context().get_side());
}

lua_ai_action_handler* LuaKernel::create_ai_action_handler(char const *code, lua_ai_context &context)
{
	lua_State *L = mState;

	int res = luaL_loadstring(L, code);//stack size is now 1 [ -1: f]
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while creating ai function:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return NULL;
	}


	// Retrieve the ai elements table from the registry.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length = lua_objlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: f  -2: ais_table  -3: f]
	lua_rawseti(L, -2, length + 1);// ais_table[length+1]=f.  stack size is now 2 [-1: ais_table  -2: f]
	lua_remove(L, -1);//stack size is now 1 [-1: f]
	lua_remove(L, -1);//stack size is now 0 []
	// Create the proxy C++ action handler.
	return new lua_ai_action_handler(L, context, length + 1);
}


void lua_ai_context::load()
{
	lua_pushlightuserdata(L, (void *)&aisKey);//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_context -2: ais_table]
	lua_remove(L,-2);
}

lua_ai_context::~lua_ai_context()
{
	// Remove the ai context from the registry, so that it can be collected.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

void lua_ai_action_handler::handle(config &cfg, bool configOut)
{
	int initial_top = lua_gettop(L);//get the old stack size

	// Load the user function from the registry.
	lua_pushlightuserdata(L, (void *)&aisKey);//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_action  -2: ais_table]
	lua_remove(L, -2);//stack size is 1 [-1: ai_action]
	//load the lua ai context as a parameter
	context_.load();//stack size is 2 [-1: ai_context -2: ai_action]

	if (!configOut)
	{
		lua_newtable(L);//stack size is 3 [-1: table -2: ai_context -3: ai_action]
		table_of_wml_config(L, cfg);//the new table now contains the config
		luaW_pcall(L, 2, LUA_MULTRET, true);
	}
	else if (lua_gettop(L) > initial_top)
	{
		if (luaW_pcall(L, 1, LUA_MULTRET, true)) {
			int score = lua_tonumber(L, initial_top + 1);//get score

			if (lua_gettop(L) >= initial_top + 2) {//check if we also have config
				luaW_toconfig(L, initial_top + 2, cfg);//get config
			}

			cfg["score"] = str_cast(score);//write score to the config
		}
	}

	lua_settop(L, initial_top);//empty stack
}

lua_ai_action_handler::~lua_ai_action_handler()
{
	// Remove the function from the registry, so that it can be collected.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}
