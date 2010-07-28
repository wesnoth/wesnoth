/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
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
#include "scripting/lua_api.hpp"

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
#include "replay.hpp"
#include "resources.hpp"
#include "terrain_translation.hpp"
#include "sound.hpp"
#include "unit.hpp"
#include "ai/lua/core.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua {

static std::vector<config> preload_scripts;

void extract_preload_scripts(config const &game_config)
{
	preload_scripts.clear();
	foreach (config const &cfg, game_config.child_range("lua")) {
		preload_scripts.push_back(cfg);
	}
}

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
static char const vconfigKey = 0;
static char const dlgclbkKey = 0;

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
 * Pushes a vconfig on the top of the stack.
 */
static void luaW_pushvconfig(lua_State *L, vconfig const &cfg)
{
	new(lua_newuserdata(L, sizeof(vconfig))) vconfig(cfg);
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
static bool luaW_hasmetatable(lua_State *L
		, int index
		, char const &key)
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
void table_of_wml_config(lua_State *L, config const &cfg)
{
	if (!lua_checkstack(L, LUA_MINSTACK))
		return;

	int k = 1;
	foreach (const config::any_child &ch, cfg.all_children_range())
	{
		lua_createtable(L, 2, 0);
		lua_pushstring(L, ch.key.c_str());
		lua_rawseti(L, -2, 1);
		lua_newtable(L);
		table_of_wml_config(L, ch.cfg);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, k++);
	}
	foreach (const config::attribute &attr, cfg.attribute_range())
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
bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta)
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
		config::attribute_value &v = cfg[lua_tostring(L, -2)];
		switch (lua_type(L, -1)) {
			case LUA_TBOOLEAN:
				v = bool(lua_toboolean(L, -1));
				break;
			case LUA_TNUMBER:
			{
				double n = lua_tonumber(L, -1);
				if (n != int(n)) v = n;
				else v = int(n);
				break;
			}
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
				vcfg = vconfig::empty_vconfig();
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
bool luaW_pcall(lua_State *L
		, int nArgs, int nRets, bool allow_wml_error)
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
		} else if (allow_wml_error && strncmp(m, "~lua:", 5) == 0) {
			m += 5;
			char const *e = NULL, *em = m;
			while (em[0] && ((em = strstr(em + 1, "stack traceback"))))
				e = em;
			chat_message("Lua error", std::string(m, e ? e - m : strlen(m)));
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


lua_unit::~lua_unit()
{
	delete ptr;
}


unit *lua_unit::get()
{
	if (ptr) return ptr;
	if (side) {
		foreach (unit &u, (*resources::teams)[side - 1].recall_list()) {
			if (u.underlying_id() == uid) return &u;
		}
		return NULL;
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return NULL;
	return &*ui;
}

/**
 * Converts a Lua value to a unit pointer.
 */
unit *luaW_tounit(lua_State *L, int index, bool only_on_map)
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
 * Special fields __literal, __shallow_literal, __parsed, and
 * __shallow_parsed, return Lua tables.
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
	if (strcmp(m, "__shallow_literal") == 0) {
		lua_newtable(L);
		foreach (const config::attribute &a, v->get_config().attribute_range()) {
			luaW_pushscalar(L, a.second);
			lua_setfield(L, -2, a.first.c_str());
		}
		copy_children:
		int j = 1;
		for (vconfig::all_children_iterator i = v->ordered_begin(),
		     i_end = v->ordered_end(); i != i_end; ++i)
		{
			lua_createtable(L, 2, 0);
			lua_pushstring(L, i.get_key().c_str());
			lua_rawseti(L, -2, 1);
			luaW_pushvconfig(L, i.get_child());
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, j);
			++j;
		}
		return 1;
	}
	if (strcmp(m, "__shallow_parsed") == 0) {
		lua_newtable(L);
		foreach (const config::attribute &a, v->get_config().attribute_range()) {
			luaW_pushscalar(L, v->expand(a.first));
			lua_setfield(L, -2, a.first.c_str());
		}
		goto copy_children;
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
	if (strcmp(m, "loc") == 0) {
		lua_pushinteger(L, u.get_location().x + 1);
		lua_pushinteger(L, u.get_location().y + 1);
		return 2;
	}
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
	modify_int_attrib("hitpoints", u.set_hitpoints(value));
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
 * Gets the unit at the given location or with the given id.
 * - Arg 1: integer.
 * - Args 1, 2: location.
 * - Ret 1: full userdata with __index pointing to impl_unit_get and
 *          __newindex pointing to impl_unit_set.
 */
static int intf_get_unit(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_optint(L, 2, 0) - 1;

	unit_map &units = *resources::units;
	unit_map::const_iterator ui;

	if (lua_isnoneornil(L, 2)) {
		ui = units.find(x + 1);
	} else {
		ui = units.find(map_location(x, y));
	}

	if (!ui.valid()) return 0;

	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Gets all the units matching a given filter.
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

	vconfig filter = vconfig::unconstructed_vconfig();
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
		if (!filter.null() && !ui->matches_filter(filter, ui->get_location()))
			continue;
		new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
		lua_pushvalue(L, 1);
		lua_setmetatable(L, 3);
		lua_rawseti(L, 2, i);
		++i;
	}
	return 1;
}

/**
 * Matches a unit against the given filter.
 * - Arg 1: full userdata.
 * - Arg 2: table containing a filter
 * - Ret 1: boolean.
 */
static int intf_match_unit(lua_State *L)
{
	if (!luaW_hasmetatable(L, 1, getunitKey)) {
		return luaL_typerror(L, 1, "unit");
		error_call_destructors_1:
		return luaL_argerror(L, 1, "unit not found");
		error_call_destructors_2:
		return luaL_typerror(L, 2, "WML table");
	}

	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	unit *u = lu->get();
	if (!u) goto error_call_destructors_1;

	vconfig filter = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, 2, filter, false))
		goto error_call_destructors_2;

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	if (int side = lu->on_recall_list()) {
		team &t = (*resources::teams)[side - 1];
		scoped_recall_unit auto_store("this_unit",
			t.save_id(), u - &t.recall_list()[0]);
		lua_pushboolean(L, u->matches_filter(filter, map_location()));
		return 1;
	}

	lua_pushboolean(L, u->matches_filter(filter, u->get_location()));
	return 1;
}

/**
 * Gets the numeric ids of all the units matching a given filter on the recall lists.
 * - Arg 1: optional table containing a filter
 * - Ret 1: table containing full userdata with __index pointing to
 *          impl_unit_get and __newindex pointing to impl_unit_set.
 */
static int intf_get_recall_units(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	vconfig filter = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, 1, filter, false))
		goto error_call_destructors;

	// Go through all the units while keeping the following stack:
	// 1: metatable, 2: return table, 3: userdata, 4: metatable copy
	lua_settop(L, 0);
	lua_pushlightuserdata(L, (void *)&getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	int i = 1, s = 1;
	foreach (team &t, *resources::teams)
	{
		foreach (unit &u, t.recall_list())
		{
			if (!filter.null()) {
				scoped_recall_unit auto_store("this_unit",
					t.save_id(), &u - &t.recall_list()[0]);
				if (!u.matches_filter(filter, map_location()))
					continue;
			}
			new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(s, u.underlying_id());
			lua_pushvalue(L, 1);
			lua_setmetatable(L, 3);
			lua_rawseti(L, 2, i);
			++i;
		}
		++s;
	}
	return 1;
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
			v.as_scalar() = bool(lua_toboolean(L, 2));
			break;
		case LUA_TNUMBER:
		{
			double n = lua_tonumber(L, -1);
			if (n != int(n)) v.as_scalar() = n;
			else v.as_scalar() = int(n);
			break;
		}
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
 * Returns whether the first side is an enemy of the second one.
 * - Args 1,2: side numbers.
 * - Ret 1: bool.
 */
static int intf_is_enemy(lua_State *L)
{
	unsigned side_1 = luaL_checkint(L, 1) - 1;
	unsigned side_2 = luaL_checkint(L, 2) - 1;
	std::vector<team> &teams = *resources::teams;
	if (side_1 >= teams.size() || side_2 >= teams.size()) return 0;
	lua_pushboolean(L, teams[side_1].is_enemy(side_2));
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
	return_int_attrib("recall_cost", t.recall_cost());
	return_int_attrib("base_income", t.base_income());
	return_int_attrib("total_income", t.total_income());
	return_bool_attrib("objectives_changed", t.objectives_changed());
	return_tstring_attrib("user_team_name", t.user_team_name());
	return_string_attrib("team_name", t.team_name());

	if (strcmp(m, "recruit") == 0) {
		std::set<std::string> const &recruits = t.recruits();
		lua_createtable(L, recruits.size(), 0);
		int i = 1;
		foreach (std::string const &r, t.recruits()) {
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
	modify_int_attrib("recall_cost", t.set_recall_cost(value));
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
	return_int_attrib("last_turn", resources::tod_manager->number_of_turns());
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
	modify_int_attrib("last_turn", resources::tod_manager->set_number_of_turns(value));
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
			cfg["x1"] = ev.loc1.x + 1;
			cfg["y1"] = ev.loc1.y + 1;
		}
		if (ev.loc2.valid()) {
			cfg["x2"] = ev.loc2.x + 1;
			cfg["y2"] = ev.loc2.y + 1;
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

	vconfig cond = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, 1, cond, false))
		goto error_call_destructors;

	bool b = game_events::conditional_passed(cond);
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
		if (ui.valid()) u = &*ui;
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
				*u, viewing_team, see_all, ignore_units);
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
		u = &*ui;
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
			if (unit_arg == 1) return 0;
			resources::units->move(u->get_location(), loc);
			return 0;
		} else if (int side = lu->on_recall_list()) {
			team &t = (*resources::teams)[side - 1];
			unit *v = new unit(*u);
			std::vector<unit> &rl = t.recall_list();
			rl.erase(rl.begin() + (u - &rl[0]));
			u = v;
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
			u = new unit(cfg, true, resources::state_of_game);
		} catch (const game::error &e) {
			error_buffer = "broken unit WML [" + e.message + "]";
			goto error_call_destructors_2;
		}
	}

	resources::units->erase(loc);
	if (u) {
		resources::units->add(loc, *u);
		if (lu) {
			size_t uid = u->underlying_id();
			lu->lua_unit::~lua_unit();
			new(lu) lua_unit(uid);
		} else
			delete u;
	}

	return 0;
}

/**
 * Puts a unit on a recall list.
 * - Arg 1: WML table or unit.
 * - Arg 2: (optional) side.
 */
static int intf_put_recall_unit(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_typerror(L, 1, "WML table or unit");
		error_call_destructors_2:
		return luaL_argerror(L, 1, error_buffer.c_str());
		error_call_destructors_3:
		return luaL_argerror(L, 1, "unit not found");
		error_call_destructors_4:
		return luaL_argerror(L, 2, "nonpersistent side");
	}

	lua_unit *lu = NULL;
	unit *u = NULL;
	int side = lua_tointeger(L, 2);
	if (unsigned(side) > resources::teams->size()) side = 0;

	if (luaW_hasmetatable(L, 1, getunitKey))
	{
		lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
		u = lu->get();
		if (!u) goto error_call_destructors_3;
		if (lu->on_recall_list())
			goto error_call_destructors_3;
	}
	else
	{
		config cfg;
		if (!luaW_toconfig(L, 1, cfg))
			goto error_call_destructors_1;
		try {
			u = new unit(cfg, true, resources::state_of_game);
		} catch (const game::error &e) {
			error_buffer = "broken unit WML [" + e.message + "]";
			goto error_call_destructors_2;
		}
	}

	if (!side) side = u->side();
	team &t = (*resources::teams)[side - 1];
	if (!t.persistent())
		goto error_call_destructors_4;
	std::vector<unit> &rl = t.recall_list();

	// Avoid duplicates in the recall list.
	size_t uid = u->underlying_id();
	std::vector<unit>::iterator i = rl.begin();
	while (i != rl.end()) {
		if (i->underlying_id() == u->underlying_id()) {
			i = rl.erase(i);
		} else ++i;
	}

	rl.push_back(*u);
	if (lu) {
		if (lu->on_map())
			resources::units->erase(u->get_location());
		lu->lua_unit::~lua_unit();
		new(lu) lua_unit(side, uid);
	}

	return 0;
}

/**
 * Extracts a unit from the map or a recall list and gives it to Lua.
 * - Arg 1: unit userdata.
 */
static int intf_extract_unit(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_typerror(L, 1, "unit");
		error_call_destructors_2:
		return luaL_argerror(L, 1, "unit not found");
	}

	if (!luaW_hasmetatable(L, 1, getunitKey))
		goto error_call_destructors_1;
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	unit *u = lu->get();
	if (!u) goto error_call_destructors_2;

	if (lu->on_map()) {
		u = resources::units->extract(u->get_location());
		assert(u);
	} else if (int side = lu->on_recall_list()) {
		team &t = (*resources::teams)[side - 1];
		unit *v = new unit(*u);
		std::vector<unit> &rl = t.recall_list();
		rl.erase(rl.begin() + (u - &rl[0]));
		u = v;
	} else {
		return 0;
	}

	lu->lua_unit::~lua_unit();
	new(lu) lua_unit(u);
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
				u = new unit(cfg, false, resources::state_of_game);
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
	resources::screen->float_label(loc, text, font::LABEL_COLOR.r,
		font::LABEL_COLOR.g, font::LABEL_COLOR.b);
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
		unit *u = new unit(cfg, true, resources::state_of_game);
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

/**
 * Creates a vconfig containing the WML table.
 * - Arg 1: WML table.
 * - Ret 1: vconfig userdata.
 */
static int intf_tovconfig(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	vconfig vcfg = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, 1, vcfg))
		goto error_call_destructors;

	luaW_pushvconfig(L, vcfg);
	return 1;
}

/**
 * Modifies the music playlist.
 * - Arg 1: WML table, or nil to force changes.
 */
static int intf_set_music(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	if (lua_isnoneornil(L, 1)) {
		sound::commit_music_changes();
		return 0;
	}

	config cfg;
	if (!luaW_toconfig(L, 1, cfg))
		goto error_call_destructors;
	sound::play_music_config(cfg);
	return 0;
}

/**
 * Plays a sound, possibly repeated.
 * - Arg 1: string.
 * - Arg 2: optional integer.
 */
static int intf_play_sound(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (resources::controller->is_skipping_replay()) return 0;
	int repeats = lua_tointeger(L, 2);
	sound::play_sound(m, sound::SOUND_FX, repeats);
	return 0;
}

/**
 * Scrolls to given tile.
 * - Args 1,2: location.
 * - Arg 3: boolean preventing scroll to fog.
 */
static int intf_scroll_to_tile(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	bool check_fogged = lua_toboolean(L, 3);
	resources::screen->scroll_to_tile(map_location(x, y),
		game_display::SCROLL, check_fogged);
	return 0;
}

struct lua_synchronize : mp_sync::user_choice
{
	lua_State *L;
	lua_synchronize(lua_State *l): L(l) {}

	virtual config query_user() const
	{
		config cfg;
		if (luaW_pcall(L, 0, 1, false))
			luaW_toconfig(L, -1, cfg);
		return cfg;
	}

	virtual config random_choice(rand_rng::simple_rng &) const
	{
		return config();
	}
};

/**
 * Ensures a value is synchronized among all the clients.
 * - Arg 1: function to compute the value, called if the client is the master.
 * - Ret 1: WML table returned by the function.
 */
static int intf_synchronize_choice(lua_State *L)
{
	lua_settop(L, 1);
	config cfg = mp_sync::get_user_choice("input", lua_synchronize(L));
	lua_newtable(L);
	table_of_wml_config(L, cfg);
	return 1;
}

struct scoped_dialog
{
	lua_State *L;
	scoped_dialog *prev;
	static scoped_dialog *current;
	gui2::twindow *window;
	typedef std::map<gui2::twidget *, int> callback_map;
	callback_map callbacks;

	scoped_dialog(lua_State *l, gui2::twindow *w);
	~scoped_dialog();
private:
	scoped_dialog(const scoped_dialog &);
};

scoped_dialog *scoped_dialog::current = NULL;

scoped_dialog::scoped_dialog(lua_State *l, gui2::twindow *w)
	: L(l), prev(current), window(w)
{
	lua_pushlightuserdata(L, (void *)&dlgclbkKey);
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, -2);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawseti(L, -2, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);
	current = this;
}

scoped_dialog::~scoped_dialog()
{
	delete window;
	current = prev;
	lua_pushlightuserdata(L, (void *)&dlgclbkKey);
	lua_pushvalue(L, -1);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

static gui2::twidget *find_widget(lua_State *L, int i, bool readonly)
{
	if (!scoped_dialog::current) {
		luaL_error(L, "no visible dialog");
		error_call_destructors_1:
		luaL_argerror(L, i, "out of bounds");
		error_call_destructors_2:
		luaL_typerror(L, i, "string");
		error_call_destructors_3:
		luaL_argerror(L, i, "widget not found");
		return NULL;
	}

	gui2::twidget *w = scoped_dialog::current->window;
	for (; !lua_isnoneornil(L, i); ++i)
	{
		if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = l->get_item_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				string_map dummy;
				for (; n < v; ++n)
					l->add_row(dummy);
			}
			w = l->get_row_grid(v - 1);
		}
		else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w))
		{
			int v = lua_tointeger(L, i);
			if (v < 1)
				goto error_call_destructors_1;
			int n = l->get_page_count();
			if (v > n) {
				if (readonly)
					goto error_call_destructors_1;
				string_map dummy;
				for (; n < v; ++n)
					l->add_page(dummy);
			}
			w = &l->page_grid(v - 1);
		}
		else
		{
			char const *m = lua_tostring(L, i);
			if (!m) goto error_call_destructors_2;
			w = w->find(m, false);
		}
		if (!w) goto error_call_destructors_3;
	}

	return w;
}

/**
 * Displays a window.
 * - Arg 1: WML table describing the window.
 * - Arg 2: function called at pre-show.
 * - Arg 3: function called at post-show.
 * - Ret 1: integer.
 */
static int intf_show_dialog(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_argerror(L, 1, error_buffer.c_str());
	}

	int v;
	try {
		config def_cfg;
		luaW_toconfig(L, 1, def_cfg);
		gui2::twindow_builder::tresolution def(def_cfg);
		scoped_dialog w(L, gui2::build(resources::screen->video(), &def));
		if (!lua_isnoneornil(L, 2)) {
			lua_pushvalue(L, 2);
			if (!luaW_pcall(L, 0, 0)) return 0;
		}
		v = scoped_dialog::current->window->show(true, 0);
		if (!lua_isnoneornil(L, 3)) {
			lua_pushvalue(L, 3);
			if (!luaW_pcall(L, 0, 0)) return 0;
		}
	} catch(twml_exception &e) {
		error_buffer = e.user_message;
		ERR_LUA << "failed to generate dialog: " << e.dev_message << '\n';
		goto error_call_destructors;
	}

	lua_pushinteger(L, v);
	return 1;
}

/**
 * Sets the value of a widget on the current dialog.
 * - Arg 1: scalar.
 * - Args 2..n: path of strings and integers.
 */
static int intf_set_dialog_value(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, 1, "out of bounds");
		error_call_destructors_2:
		return luaL_typerror(L, 1, "translatable string");
		error_call_destructors_3:
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
		error_call_destructors_4:
		return luaL_argerror(L, 1, error_buffer.c_str());
	}

	gui2::twidget *w = find_widget(L, 2, false);

	try {
		if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w))
		{
			int v = lua_tointeger(L, 1);
			int n = l->get_item_count();
			if (1 <= v && v <= n)
				l->select_row(v - 1);
			else
				goto error_call_destructors_1;
		}
		else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w))
		{
			int v = lua_tointeger(L, 1);
			int n = l->get_page_count();
			if (1 <= v && v <= n)
				l->select_page(v - 1);
			else
				goto error_call_destructors_1;
		}
		else if (gui2::ttoggle_button *b = dynamic_cast<gui2::ttoggle_button *>(w))
		{
			b->set_value(lua_toboolean(L, 1));
		}
		else
		{
			t_string v;
			if (!luaW_totstring(L, 1, v))
				goto error_call_destructors_2;
			gui2::tcontrol *c = dynamic_cast<gui2::tcontrol *>(w);
			if (!c) goto error_call_destructors_3;
			c->set_label(v);
		}
	} catch(twml_exception &e) {
		error_buffer = e.user_message;
		ERR_LUA << "failed to set dialog value: " << e.dev_message << '\n';
		goto error_call_destructors_4;
	}

	return 0;
}

/**
 * Gets the value of a widget on the current dialog.
 * - Args 1..n: path of strings and integers.
 * - Ret 1: scalar.
 */
static int intf_get_dialog_value(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
		error_call_destructors_2:
		return luaL_argerror(L, 1, error_buffer.c_str());
	}

	gui2::twidget *w = find_widget(L, 1, true);

	try {
		if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w)) {
			lua_pushinteger(L, l->get_selected_row() + 1);
		} else if (gui2::tmulti_page *l = dynamic_cast<gui2::tmulti_page *>(w)) {
			lua_pushinteger(L, l->get_selected_page() + 1);
		} else if (gui2::ttoggle_button *b = dynamic_cast<gui2::ttoggle_button *>(w)) {
			lua_pushboolean(L, b->get_value());
		} else if (gui2::ttext_box *t = dynamic_cast<gui2::ttext_box *>(w)) {
			lua_pushstring(L, t->get_value().c_str());
		} else
			goto error_call_destructors_1;
	} catch(twml_exception &e) {
		error_buffer = e.user_message;
		ERR_LUA << "failed to get dialog value: " << e.dev_message << '\n';
		goto error_call_destructors_2;
	}

	return 1;
}

static void dialog_callback(gui2::twidget *w)
{
	int cb;
	{
		scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
		scoped_dialog::callback_map::const_iterator i = m.find(w);
		if (i == m.end()) return;
		cb = i->second;
	}
	lua_State *L = scoped_dialog::current->L;
	lua_pushlightuserdata(L, (void *)&dlgclbkKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, cb);
	lua_remove(L, -2);
	lua_call(L, 0, 0);
}

/**
 * Sets a callback on a widget of the current dialog.
 * - Arg 1: function.
 * - Args 2..n: path of strings and integers.
 */
static int intf_set_dialog_callback(lua_State *L)
{
	if (false) {
		error_call_destructors_1:
		return luaL_argerror(L, lua_gettop(L), "unsupported widget");
		error_call_destructors_2:
		return luaL_argerror(L, 1, error_buffer.c_str());
	}

	gui2::twidget *w = find_widget(L, 2, true);

	scoped_dialog::callback_map &m = scoped_dialog::current->callbacks;
	scoped_dialog::callback_map::iterator i = m.find(w);
	if (i != m.end())
	{
		lua_pushlightuserdata(L, (void *)&dlgclbkKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);
		lua_rawseti(L, -2, i->second);
		lua_pop(L, 1);
		m.erase(i);
	}

	if (lua_isnil(L, 1)) return 0;

	try {
		if (gui2::tlistbox *l = dynamic_cast<gui2::tlistbox *>(w)) {
			l->set_callback_value_change(&dialog_callback);
		} else if (gui2::ttoggle_button *b = dynamic_cast<gui2::ttoggle_button *>(w)) {
			b->set_callback_state_change(&dialog_callback);
		} else
			goto error_call_destructors_1;
	} catch(twml_exception &e) {
		error_buffer = e.user_message;
		ERR_LUA << "failed to get dialog value: " << e.dev_message << '\n';
		goto error_call_destructors_2;
	}

	lua_pushlightuserdata(L, (void *)&dlgclbkKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	int n = lua_objlen(L, -1) + 1;
	m[w] = n;
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, n);
	lua_pop(L, 1);

	return 0;
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
		{ "extract_unit",             &intf_extract_unit             },
		{ "find_path",                &intf_find_path                },
		{ "find_reach",               &intf_find_reach               },
		{ "find_vacant_tile",         &intf_find_vacant_tile         },
		{ "fire_event",               &intf_fire_event               },
		{ "float_label",              &intf_float_label              },
		{ "get_dialog_value",         &intf_get_dialog_value         },
		{ "get_map_size",             &intf_get_map_size             },
		{ "get_recall_units",         &intf_get_recall_units         },
		{ "get_selected_tile",        &intf_get_selected_tile        },
		{ "get_terrain",              &intf_get_terrain              },
		{ "get_terrain_info",         &intf_get_terrain_info         },
		{ "get_unit",                 &intf_get_unit                 },
		{ "get_units",                &intf_get_units                },
		{ "get_variable",             &intf_get_variable             },
		{ "get_village_owner",        &intf_get_village_owner        },
		{ "is_enemy",                 &intf_is_enemy                 },
		{ "match_unit",               &intf_match_unit               },
		{ "message",                  &intf_message                  },
		{ "play_sound",               &intf_play_sound               },
		{ "put_recall_unit",          &intf_put_recall_unit          },
		{ "put_unit",                 &intf_put_unit                 },
		{ "require",                  &intf_require                  },
		{ "scroll_to_tile",           &intf_scroll_to_tile           },
		{ "set_dialog_callback",      &intf_set_dialog_callback      },
		{ "set_dialog_value",         &intf_set_dialog_value         },
		{ "set_music",                &intf_set_music                },
		{ "set_terrain",              &intf_set_terrain              },
		{ "set_variable",             &intf_set_variable             },
		{ "set_village_owner",        &intf_set_village_owner        },
		{ "show_dialog",              &intf_show_dialog              },
		{ "simulate_combat",          &intf_simulate_combat          },
		{ "synchronize_choice",       &intf_synchronize_choice       },
		{ "textdomain",               &intf_textdomain               },
		{ "tovconfig",                &intf_tovconfig                },
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

	// Create the ai elements table.
	ai::lua_ai_context::init(L);

	// Delete dofile and loadfile.
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

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

	// Create the wml_actions table.
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "wml_actions");
	lua_pop(L, 1);

	// Remove the math.random function, since it is not OoS-proof.
	lua_getglobal(L, "math");
	lua_pushnil(L);
	lua_setfield(L, -2, "random");
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

void LuaKernel::initialize()
{
	lua_State *L = mState;
	lua_getglobal(L, "wesnoth");

	// Create the sides table.
	std::vector<team> &teams = *resources::teams;
	lua_pushlightuserdata(L, (void *)&getsideKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_createtable(L, teams.size(), 0);
	for (unsigned i = 0; i != teams.size(); ++i)
	{
		// Create a full userdata containing a pointer to the team.
		team **p = static_cast<team **>(lua_newuserdata(L, sizeof(team *)));
		*p = &teams[i];
		lua_pushvalue(L, -3);
		lua_setmetatable(L, -2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_setfield(L, -3, "sides");
	lua_pop(L, 1);

	// Create the unit_types table.
	lua_pushlightuserdata(L, (void *)&gettypeKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	foreach (const unit_type_data::unit_type_map::value_type &ut, unit_types.types())
	{
		lua_createtable(L, 0, 1);
		lua_pushstring(L, ut.first.c_str());
		lua_setfield(L, -2, "id");
		lua_pushvalue(L, -3);
		lua_setmetatable(L, -2);
		lua_setfield(L, -2, ut.first.c_str());
	}
	lua_setfield(L, -3, "unit_types");
	lua_pop(L, 2);

	// Execute the preload scripts.
	foreach (const config &cfg, preload_scripts) {
		execute(cfg["code"].str().c_str(), 0, 0);
	}
}

LuaKernel::~LuaKernel()
{
	lua_close(mState);
}

/**
 * Executes its upvalue as a wml action.
 */
static int cfun_wml_action(lua_State *L)
{
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, 1, "WML table");
	}

	game_events::action_handler h = reinterpret_cast<game_events::action_handler>
		(lua_touserdata(L, lua_upvalueindex(1)));
	vconfig vcfg = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, 1, vcfg))
		goto error_call_destructors;

	h(queued_event_context::get(), vcfg);
	return 0;
}

/**
 * Registers a function for use as an action handler.
 */
void LuaKernel::set_wml_action(std::string const &cmd, game_events::action_handler h)
{
	lua_State *L = mState;

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "wml_actions");
	lua_rawget(L, -2);
	lua_pushstring(L, cmd.c_str());
	lua_pushlightuserdata(L, (void *)h);
	lua_pushcclosure(L, cfun_wml_action, 1);
	lua_rawset(L, -3);
	lua_pop(L, 2);
}

/**
 * Runs a command from an event handler.
 * @return true if there is a handler for the command.
 * @note @a cfg should be either volatile or long-lived since the Lua
 *       code may grab it for an arbitrary long time.
 */
bool LuaKernel::run_wml_action(std::string const &cmd, vconfig const &cfg,
	game_events::queued_event const &ev)
{
	lua_State *L = mState;

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "wml_actions");
	lua_rawget(L, -2);
	lua_remove(L, -2);
	lua_pushstring(L, cmd.c_str());
	lua_rawget(L, -2);
	lua_remove(L, -2);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	queued_event_context dummy(&ev);
	luaW_pushvconfig(L, cfg);
	luaW_pcall(L, 1, 0, true);
	return true;
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
	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
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

ai::lua_ai_context* LuaKernel::create_lua_ai_context(char const *code, ai::engine_lua *engine)
{
	return ai::lua_ai_context::create(mState,code,engine);
}

ai::lua_ai_action_handler* LuaKernel::create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context)
{
	return ai::lua_ai_action_handler::create(mState,code,context);
}

} // of namespace lua

