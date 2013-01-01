/*
   Copyright (C) 2009 - 2013 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef DEBUG_LUA

#include <cassert>

#include "debug_lua.hpp"

#include "log.hpp"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)


static void value_to_stringstream(
	std::stringstream& output,
	int i, lua_State* L,
	std::string indent,
	const bool verbose_table = true)
{
	const int t = lua_type(L, i);
	switch (t) {
		case LUA_TSTRING:
			output << "STRING; VALUE: " << lua_tostring(L, i);
		break;
		case LUA_TBOOLEAN:
			output << "BOOLEAN; VALUE: " << (lua_toboolean(L, i) ? "true" : "false");
		break;
		case LUA_TNUMBER:
			output << "NUMBER; VALUE: " << lua_tonumber(L, i);
		break;
		case LUA_TNIL:
			output << "NIL; VALUE: nil";
		break;
		case LUA_TTABLE:
		{
			output << "TABLE; VALUE: " << lua_topointer(L, i);
			if(verbose_table)
			{
				indent += "\t";
				unsigned keyindex = lua_gettop(L) + 1;
				lua_pushnil(L);
				while(lua_next(L, i) != 0)
				{
					output << "\n" << indent << "KEY: ";
					const int keytype = lua_type(L, keyindex);
					switch(keytype) {
						case LUA_TSTRING:
							output << lua_tostring(L, keyindex);
						break;
						case LUA_TBOOLEAN:
							output << (lua_toboolean(L, keyindex) ? "true" : "false");
						break;
						case LUA_TNUMBER:
							output << lua_tonumber(L, keyindex);
						break;
						default:
							output << lua_topointer(L, keyindex);
						break;
					}
					output << "; TYPE: ";
					value_to_stringstream(output, keyindex + 1, L, indent);
					lua_pop(L, 1);
				}
			}
		}
		break;
		case LUA_TUSERDATA:
			output << "USERDATA; VALUE: " << lua_topointer(L, i);
		break;
		case LUA_TFUNCTION:
			output << "FUNCTION; VALUE: " << lua_topointer(L, i);
		break;
		case LUA_TTHREAD:
			output << "THREAD; VALUE: " << lua_topointer(L, i);
		break;
		case LUA_TLIGHTUSERDATA:
			output << "LIGHTUSERDATA; VALUE: " << lua_topointer(L, i);
		break;
		default:
			//There are no other types!
			assert(false);
		break;
	}
}

void ds(lua_State *L, const bool verbose_table) {
	std::stringstream output;
	output << "\n";
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		output << "INDEX: " << i << "; TYPE: ";
		value_to_stringstream(output, i, L, "", verbose_table);
		output << "\n";
	}
	output << "\n";
	LOG_LUA << output.str();
}

#endif

