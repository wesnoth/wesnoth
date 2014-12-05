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

#include "lua_fileops.hpp"

#include "exceptions.hpp"
#include "filesystem.hpp"
#include "game_config.hpp" //for game_config::debug_lua
#include "game_errors.hpp"
#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/luaconf.h"                // for LUAL_BUFFERSIZE
#include "scripting/lua_api.hpp"	// for chat_message, luaW_pcall
#include "scripting/lua_common.hpp"	// for chat_message, luaW_pcall

#include <exception>
#include <string>

#include <boost/scoped_ptr.hpp>

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua_fileops {

/**
 * Checks if a file exists (not necessarily a Lua script).
 * - Arg 1: string containing the file name.
 * - Ret 1: boolean
 */
int intf_have_file(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	std::string p = filesystem::get_wml_location(m);
	if (p.empty()) { lua_pushboolean(L, false); }
	else { lua_pushboolean(L, true); }
	return 1;
}

class lua_filestream
{
public:
	lua_filestream(const std::string& fname)
		: pistream_(filesystem::istream_file(fname))
	{

	}

	static const char * lua_read_data(lua_State * /*L*/, void *data, size_t *size)
	{
		lua_filestream* lfs = static_cast<lua_filestream*>(data);

		//int startpos = lfs->pistream_->tellg();
		lfs->pistream_->read(lfs->buff_, LUAL_BUFFERSIZE);
		//int newpos = lfs->pistream_->tellg();
		*size = lfs->pistream_->gcount();
#if 0
		ERR_LUA << "read bytes from " << startpos << " to " << newpos << " in total " *size << " from steam\n";
		ERR_LUA << "streamstate beeing "
			<< " goodbit:" << lfs->pistream_->good()
			<< " endoffile:" << lfs->pistream_->eof()
			<< " badbit:" <<  lfs->pistream_->bad()
			<< " failbit:" << lfs->pistream_->fail() << "\n";
#endif
		return lfs->buff_;
	}

	static int lua_loadfile(lua_State *L, const std::string& fname)
	{
		lua_filestream lfs(fname);
		//lua uses '@' to know that this is a file (as opposed to a something as opposed to something loaded via loadstring )
		std::string chunkname = '@' + fname;
		LOG_LUA << "starting to read from " << fname << "\n";
		return  lua_load(L, &lua_filestream::lua_read_data, &lfs, chunkname.c_str(), NULL);
	}
private:
	char buff_[LUAL_BUFFERSIZE];
	boost::scoped_ptr<std::istream> pistream_;
};

/**
 * Loads a Lua file and pushes the contents on the stack.
 * - Arg 1: string containing the file name.
 * - Ret 1: the loaded contents of the file
 */
int load_file(lua_State *L)
{
	char const *m = luaL_checkstring(L, -1);
	std::string p = filesystem::get_wml_location(m);
	if (p.empty())
		return luaL_argerror(L, -1, "file not found");

#if 1
	try
	{
		if(lua_filestream::lua_loadfile(L, p))
			return lua_error(L);
	}
	catch(const std::exception & ex)
	{
		luaL_argerror(L, -1, ex.what());
	}
#else
	//oldcode to be deleted if newcode works
	if (luaL_loadfile(L, p.c_str()))
		return lua_error(L);
#endif
	lua_remove(L, -2);	//remove the filename from the stack

	return 1;
}

} // end namespace lua_fileops
