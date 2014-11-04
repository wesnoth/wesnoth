/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lua_map_generator.hpp"

#include "config.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_api.hpp"

#include <string>

#include <boost/foreach.hpp>

lua_map_generator::lua_map_generator(const config & cfg)
	: id_(cfg["id"])
	, config_name_(cfg["config_name"])
	, create_map_(cfg["create_map"])
	, mState_(luaL_newstate())
{
	const char* required[] = {"id", "config_name", "create_map"};
	BOOST_FOREACH(std::string req, required) {
		if (!cfg.has_attribute(req)) {
			std::string msg = "Error when constructing a lua map generator -- missing a required attribute '";
			msg += req + "'\n";
			msg += "Config was '" + cfg.debug() + "'";
			throw mapgen_exception(msg);
		}
	}
}

lua_map_generator::~lua_map_generator()
{
	lua_close(mState_);
}

std::string lua_map_generator::create_map()
{
	{
		int errcode = luaL_loadstring(mState_, create_map_.c_str());
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_map.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when parsing create_map function. ";
			if (errcode == LUA_ERRSYNTAX) {
				msg += "There was a syntax error:\n";
			} else {
				msg += "There was a memory error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	{
		int errcode = lua_pcall(mState_, 0, 1, 0);
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_map.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when running create_map function. ";
			if (errcode == LUA_ERRRUN) {
				msg += "There was a runtime error:\n";
			} else if (errcode == LUA_ERRERR) {
				msg += "There was an error with the attached debugger:\n";
			} else {
				msg += "There was a memory or garbage collection error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	if (!lua_isstring(mState_,-1)) {
		std::string msg = "Error when running lua_map_generator create_map.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += "create_map did not return a string, instead it returned '";
		msg += lua_typename(mState_, lua_type(mState_, -1));
		msg += "'";
		throw mapgen_exception(msg);
	}
	return lua_tostring(mState_, -1);
}
