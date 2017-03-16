/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_fileops.hpp"

#include "exceptions.hpp"
#include "filesystem.hpp"
#include "game_config.hpp" //for game_config::debug_lua
#include "game_errors.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"	// for chat_message, luaW_pcall

#include <exception>
#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/luaconf.h"                // for LUAL_BUFFERSIZE

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua_fileops {
/// resolves @a filename where @a currentdir is the current directory, note that @a currentdir
/// is no absolute directory
/// @returns true iff the filename was sucesful resolved.
static bool resolve_filename(std::string& filename, const std::string& currentdir)
{
	if(filename.size() < 2) {
		return false;
	}
	if(filename[0] == '.' && filename[1] == '/') {
		filename = currentdir + filename.substr(1);
	}
	if(std::find(filename.begin(), filename.end(), '\\') != filename.end()) {
		return false;
	}
	//resolve /./
	while(true) {
		size_t pos = filename.find("/./");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 2, "");
	}
	//resolve //
	while(true) {
		size_t pos = filename.find("//");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 1, "");
	}
	//resolve /../
	while(true) {
		size_t pos = filename.find("/..");
		if(pos == std::string::npos) {
			break;
		}
		size_t pos2 = filename.find_last_of('/', pos - 1);
		if(pos == std::string::npos || pos2 >= pos) {
			return false;
		}
		filename = filename.replace(pos2, pos- pos2 + 3, "");
	}
	if(filename.find("..") != std::string::npos) {
		return false;
	}
	return true;
}

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
/**
 * Checks if a file exists (not necessarily a Lua script).
 * - Arg 1: string containing the file name.
 * - Ret 1: string
 */
int intf_read_file(lua_State *L)
{
	std::string m = luaL_checkstring(L, 1);
	
	std::string current_dir = "";
	lua_Debug ar;
	if(lua_getstack(L, 1, &ar)) {
		lua_getinfo(L, "S", &ar);
		if(ar.source[0] == '@') {
			current_dir = filesystem::directory_name(std::string(ar.source + 1));
		}
	}
	if(!resolve_filename(m, current_dir)) {
		return luaL_argerror(L, -1, "file not found");
	}
	std::string p = filesystem::get_wml_location(m);
	if(p.empty()) {
		return luaL_argerror(L, -1, "file not found");
	}
	const std::unique_ptr<std::istream> fs(filesystem::istream_file(p));
	fs->exceptions(std::ios_base::goodbit);
	size_t size = 0;
	fs->seekg(0, std::ios::end);
	if(!fs->good()) {
		return luaL_error(L, "Error when reading file");
	}
	size = fs->tellg();
	fs->seekg(0, std::ios::beg);
	if(!fs->good()) {
		return luaL_error(L, "Error when reading file");
	}
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	//throws an exception if malloc failed.
	char* out = luaL_prepbuffsize(&b, size); 
	fs->read(out, size);
	if(fs->good()) {
		luaL_addsize(&b, size);
	}
	luaL_pushresult(&b);
	return 1;
}

class lua_filestream
{
public:
	lua_filestream(const std::string& fname)
		: buff_()
		, pistream_(filesystem::istream_file(fname))
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

	static int lua_loadfile(lua_State *L, const std::string& fname, const std::string& relativename)
	{
		lua_filestream lfs(fname);
		//lua uses '@' to know that this is a file (as opposed to something loaded via loadstring )
		std::string chunkname = '@' + relativename;
		LOG_LUA << "starting to read from " << fname << "\n";
		return  lua_load(L, &lua_filestream::lua_read_data, &lfs, chunkname.c_str(), nullptr);
	}
private:
	char buff_[LUAL_BUFFERSIZE];
	const std::unique_ptr<std::istream> pistream_;
};

/**
 * Loads a Lua file and pushes the contents on the stack.
 * - Arg 1: string containing the file name.
 * - Ret 1: the loaded contents of the file
 */
int load_file(lua_State *L)
{
	std::string m = luaL_checkstring(L, -1);
	
	std::string current_dir = "";
	lua_Debug ar;
	if(lua_getstack(L, 1, &ar)) {
		lua_getinfo(L, "S", &ar);
		if(ar.source[0] == '@') {
			current_dir = filesystem::directory_name(std::string(ar.source + 1));
		}
	}
	if(!resolve_filename(m, current_dir)) {
		return luaL_argerror(L, -1, "file not found");
	}
	std::string p = filesystem::get_wml_location(m);
	if (p.empty()) {
		return luaL_argerror(L, -1, "file not found");
	}
	try
	{
		if(lua_filestream::lua_loadfile(L, p, m)) {
			return lua_error(L);
		}
	}
	catch(const std::exception & ex)
	{
		luaL_argerror(L, -1, ex.what());
	}
	lua_remove(L, -2);	//remove the filename from the stack

	return 1;
}

} // end namespace lua_fileops
