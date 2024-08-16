/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_fileops.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"	// for chat_message, luaW_pcall
#include "scripting/push_check.hpp"
#include "picture.hpp"
#include "sdl/point.hpp"

#include <algorithm>
#include <exception>
#include <string>

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

/**
* Gets the dimension of an image.
* - Arg 1: string.
* - Ret 1: width.
* - Ret 2: height.
*/
static int intf_get_image_size(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	image::locator img(m);
	if(!image::exists(img)) return 0;
	const point s = get_size(img);
	lua_pushinteger(L, s.x);
	lua_pushinteger(L, s.y);
	return 2;
}

/**
 * Returns true if an asset with the given path can be found in the binary paths.
 * - Arg 1: asset type (generally one of images, sounds, music, maps)
 * - Arg 2: relative path
 */
static int intf_have_asset(lua_State* L)
{
	std::string type = luaL_checkstring(L, 1), name = luaL_checkstring(L, 2);
	lua_pushboolean(L, filesystem::get_binary_file_location(type, name).has_value());
	return 1;
}

/**
 * Given an asset path relative to binary paths, resolves to an absolute
 * asset path starting from data/
 * - Arg 1: asset type
 * - Arg 2: relative path
 */
static int intf_resolve_asset(lua_State* L)
{
	std::string type = luaL_checkstring(L, 1), name = luaL_checkstring(L, 2);
	lua_push(L, filesystem::get_independent_binary_file_path(type, name).value_or(""));
	return 1;
}

namespace lua_fileops {
static std::string get_calling_file(lua_State* L)
{
	std::string currentdir;
	lua_Debug ar;
	if(lua_getstack(L, 1, &ar)) {
		lua_getinfo(L, "S", &ar);
		if(ar.source[0] == '@') {
			std::string calling_file(ar.source + 1);
			for(int stack_pos = 2; calling_file == "lua/package.lua"; stack_pos++) {
				if(!lua_getstack(L, stack_pos, &ar)) {
					return currentdir;
				}
				lua_getinfo(L, "S", &ar);
				if(ar.source[0] == '@') {
					calling_file.assign(ar.source + 1);
				}
			}
			currentdir = filesystem::directory_name(calling_file);
		}
	}
	return currentdir;
}
/**
 * resolves @a filename to an absolute path
 * @returns true if the filename was successfully resolved.
 */
static bool canonical_path(std::string& filename, const std::string& currentdir)
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
		std::size_t pos = filename.find("/./");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 2, "");
	}
	//resolve //
	while(true) {
		std::size_t pos = filename.find("//");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 1, "");
	}
	//resolve /../
	while(true) {
		std::size_t pos = filename.find("/..");
		if(pos == std::string::npos) {
			break;
		}
		std::size_t pos2 = filename.find_last_of('/', pos - 1);
		if(pos2 == std::string::npos || pos2 >= pos) {
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
 * resolves @a filename to an absolute path
 * @returns true if the filename was successfully resolved.
 */
static bool resolve_filename(std::string& filename, const std::string& currentdir, std::string* rel = nullptr)
{
	if(!canonical_path(filename, currentdir)) {
		return false;
	}
	auto p = filesystem::get_wml_location(filename);
	if(!p) {
		return false;
	}
	if(rel) {
		*rel = filename;
	}
	filename = p.value();
	return true;
}

int intf_canonical_path(lua_State *L)
{
	std::string m = luaL_checkstring(L, 1);
	if(canonical_path(m, get_calling_file(L))) {
		lua_push(L, m);
		return 1;
	} else {
		return luaL_argerror(L, 1, "invalid path");
	}
}
/**
 * Checks if a file exists (not necessarily a Lua script).
 * - Arg 1: string containing the file name.
 * - Arg 2: if true, the file must be a real file and not a directory
 * - Ret 1: boolean
 */
int intf_have_file(lua_State *L)
{
	std::string m = luaL_checkstring(L, 1);
	if(!resolve_filename(m, get_calling_file(L))) {
		lua_pushboolean(L, false);
	} else if(luaW_toboolean(L, 2)) {
		lua_pushboolean(L, !filesystem::is_directory(m));
	} else {
		lua_pushboolean(L, true);
	}
	return 1;
}

/**
 * Reads a file into a string, or a directory into a list of files therein.
 * - Arg 1: string containing the file name.
 * - Ret 1: string
 */
int intf_read_file(lua_State *L)
{
	std::string p = luaL_checkstring(L, 1);

	if(!resolve_filename(p, get_calling_file(L))) {
		return luaL_argerror(L, -1, "file not found");
	}

	if(filesystem::is_directory(p)) {
		std::vector<std::string> files, dirs;
		filesystem::get_files_in_dir(p, &files, &dirs);
		filesystem::default_blacklist.remove_blacklisted_files_and_dirs(files, dirs);
		std::size_t ndirs = dirs.size();
		std::copy(files.begin(), files.end(), std::back_inserter(dirs));
		lua_push(L, dirs);
		lua_pushnumber(L, ndirs);
		lua_setfield(L, -2, "ndirs");
		return 1;
	}
	const std::unique_ptr<std::istream> fs(filesystem::istream_file(p));
	fs->exceptions(std::ios_base::goodbit);
	std::size_t size = 0;
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

	static const char * lua_read_data(lua_State * /*L*/, void *data, std::size_t *size)
	{
		lua_filestream* lfs = static_cast<lua_filestream*>(data);

		//int startpos = lfs->pistream_->tellg();
		lfs->pistream_->read(lfs->buff_, luaL_buffersize);
		//int newpos = lfs->pistream_->tellg();
		*size = lfs->pistream_->gcount();
#if 0
		ERR_LUA << "read bytes from " << startpos << " to " << newpos << " in total " *size << " from steam";
		ERR_LUA << "streamstate being "
			<< " goodbit:" << lfs->pistream_->good()
			<< " endoffile:" << lfs->pistream_->eof()
			<< " badbit:" <<  lfs->pistream_->bad()
			<< " failbit:" << lfs->pistream_->fail();
#endif
		return lfs->buff_;
	}

	static int lua_loadfile(lua_State *L, const std::string& fname, const std::string& relativename)
	{
		lua_filestream lfs(fname);
		//lua uses '@' to know that this is a file (as opposed to something loaded via loadstring )
		std::string chunkname = '@' + relativename;
		LOG_LUA << "starting to read from " << fname;
		return  lua_load(L, &lua_filestream::lua_read_data, &lfs, chunkname.c_str(), "t");
	}
private:
	char buff_[luaL_buffersize];
	const std::unique_ptr<std::istream> pistream_;
};

/**
 * Loads a Lua file and pushes the contents on the stack.
 * - Arg 1: string containing the file name.
 * - Ret 1: the loaded contents of the file
 */
int load_file(lua_State *L)
{
	std::string p = luaL_checkstring(L, -1);
	std::string rel;

	if(!resolve_filename(p, get_calling_file(L), &rel)) {
		return luaL_argerror(L, -1, "file not found");
	}

	try
	{
		if(lua_filestream::lua_loadfile(L, p, rel)) {
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

int luaW_open(lua_State* L)
{
	static luaL_Reg const callbacks[] {
		{ "have_file", &lua_fileops::intf_have_file },
		{ "read_file", &lua_fileops::intf_read_file },
		{ "canonical_path", &lua_fileops::intf_canonical_path },
		{ "image_size", &intf_get_image_size },
		{ "have_asset", &intf_have_asset },
		{ "resolve_asset", &intf_resolve_asset },
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_setfuncs(L, callbacks, 0);
	return 1;
}

} // end namespace lua_fileops
