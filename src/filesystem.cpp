/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//disable the very annoying VC++ warning 4786
#ifdef WIN32
#pragma warning(disable:4786)
#endif

//include files for opendir(3), readdir(3), etc. These files may vary
//from platform to platform, since these functions are NOT ANSI-conforming
//functions. They may have to be altered to port to new platforms
#include <sys/types.h>

//for mkdir
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32

#include <direct.h>

#include <io.h>

//#define mkdir(a,b) (_mkdir(a))

#define mode_t int

#else

#include <unistd.h>

#include <dirent.h>

#endif

//for getenv
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>

#include "wesconfig.h"
#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "util.hpp"

namespace {
	const mode_t AccessMode = 00770;
}

#ifdef _WIN32
#define DIR_INVALID(d) (d == -1)
#else
#define DIR_INVALID(d) (d == NULL)
#endif

void get_files_in_dir(const std::string& directory,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs,
                      FILE_NAME_MODE mode)
{
	//if we have a path to find directories in, then convert relative
	//pathnames to be rooted on the wesnoth path
	if(!directory.empty() && directory[0] != '/' && !game_config::path.empty()){
		const std::string& dir = game_config::path + "/" + directory;
		if(is_directory(dir)) {
			get_files_in_dir(dir,files,dirs,mode);
			return;
		}
	}

#ifdef _WIN32
	_finddata_t fileinfo;
	long dir = _findfirst((directory + "/*.*").c_str(),&fileinfo);
#else

	DIR* dir = opendir(directory.c_str());
#endif

	if(DIR_INVALID(dir)) {
		return;
	}

#ifdef _WIN32

	int res = dir;
	do {
		if(fileinfo.name[0] != '.') {
			const std::string path = (mode == ENTIRE_FILE_PATH ?
				directory + "/" : std::string("")) + fileinfo.name;

			if(fileinfo.attrib&_A_SUBDIR) {
				if(dirs != NULL)
					dirs->push_back(path);
			} else {
				if(files != NULL)
					files->push_back(path);
			}
		}

		res = _findnext(dir,&fileinfo);
	} while(!DIR_INVALID(res));

	_findclose(dir);

#else
	struct dirent* entry;
	while((entry = readdir(dir)) != NULL) {
		if(entry->d_name[0] == '.')
			continue;

		const std::string name((directory + "/") + entry->d_name);

		//try to open it as a directory to test if it is a directory
		DIR* const temp_dir = opendir(name.c_str());
		if(temp_dir != NULL) {
			closedir(temp_dir);
			if(dirs != NULL) {
				if(mode == ENTIRE_FILE_PATH)
					dirs->push_back(name);
				else
					dirs->push_back(entry->d_name);
			}
		} else if(files != NULL) {
			if(mode == ENTIRE_FILE_PATH)
				files->push_back(name);
			else
				files->push_back(entry->d_name);
		}
	}

	closedir(dir);
#endif

	if(files != NULL)
		std::sort(files->begin(),files->end());

	if(dirs != NULL)
		std::sort(dirs->begin(),dirs->end());
}

std::string get_prefs_file()
{
	return get_user_data_dir() + "/preferences";
}

std::string get_save_index_file()
{
	return get_user_data_dir() + "/save_index";
}

std::string get_saves_dir()
{
	const std::string dir_path = get_user_data_dir() + "/saves";
	return get_dir(dir_path);
}

std::string get_cache_dir()
{
	const std::string dir_path = get_user_data_dir() + "/cache";
	return get_dir(dir_path);
}

std::string get_intl_dir()
{
#ifdef _WIN32
	std::string res = get_cwd() + "/po";
	return res;
#else
	return LOCALEDIR;
#endif
}

std::string get_dir(const std::string& dir_path)
{
#ifdef _WIN32
	_mkdir(dir_path.c_str());
#else

	DIR* dir = opendir(dir_path.c_str());
	if(dir == NULL) {
		const int res = mkdir(dir_path.c_str(),AccessMode);
		if(res == 0) {
			dir = opendir(dir_path.c_str());
		} else {
			std::cerr << "Could not open or create directory: '" << dir_path
			          << "'\n";
		}
	}

	if(dir == NULL)
		return "";

	closedir(dir);
#endif

	return dir_path;
}

std::string get_cwd()
{
	char buf[1024];
	const char* const res = getcwd(buf,sizeof(buf));
	if(res != NULL) {
		std::string str(res);

#ifdef WIN32
		std::replace(str.begin(),str.end(),'\\','/');
#endif

		return str;
	} else {
		return "";
	}
}

std::string get_user_data_dir()
{
#ifdef _WIN32

	static bool inited_dirs = false;

	if(!inited_dirs) {
		_mkdir("userdata");
		_mkdir("userdata/editor");
		_mkdir("userdata/editor/maps");
		inited_dirs = true;
	}

	char buf[256];
	const char* const res = getcwd(buf,sizeof(buf));

	if(res != NULL) {
		std::string cur_path(res);
		std::replace(cur_path.begin(),cur_path.end(),'\\','/');
		return cur_path + "/userdata";
	} else {
		return "userdata";
	}
#else

	static const char* const current_dir = ".";

	const char* home_str = getenv("HOME");
	if(home_str == NULL)
		home_str = current_dir;

	const std::string home(home_str);

#ifndef PREFERENCES_DIR
#define PREFERENCES_DIR ".wesnoth"
#endif

	const std::string dir_path = home + std::string("/") + PREFERENCES_DIR;
	DIR* dir = opendir(dir_path.c_str());
	if(dir == NULL) {
		const int res = mkdir(dir_path.c_str(),AccessMode);

		//also create the maps directory
		mkdir((dir_path + "/editor").c_str(),AccessMode);
		mkdir((dir_path + "/editor/maps").c_str(),AccessMode);
		if(res == 0) {
			dir = opendir(dir_path.c_str());
		} else {
			std::cerr << "Could not open or create directory: '" << dir_path
			          << "'\n";
		}
	}

	if(dir == NULL)
		return "";

	closedir(dir);

	return dir_path;
#endif
}

std::string read_map(const std::string& name)
{
	std::string res = read_file("data/maps/" + name);
	if(res == "") {
		res = read_file(get_user_data_dir() + "/data/maps/" + name);
	}

	if(res == "") {
		res = read_file(get_user_data_dir() + "/editor/maps/" + name);
	}

	return res;
}

namespace {

bool is_directory_internal(const std::string& fname)
{
#ifdef _WIN32
	_finddata_t info;
	const long handle = _findfirst((fname + "/*").c_str(),&info);
	if(handle >= 0) {
		_findclose(handle);
		return true;
	} else {
		return false;
	}

#else
	struct stat dir_stat;
	if(::stat(fname.c_str(), &dir_stat) == -1) {
		return false;
	}

	return S_ISDIR(dir_stat.st_mode);
#endif
}

}

bool is_directory(const std::string& fname)
{
	if(!fname.empty() && fname[0] != '/' && !game_config::path.empty()) {
		if(is_directory_internal(game_config::path + "/" + fname))
			return true;
	}

	return is_directory_internal(fname);
}

bool file_exists(const std::string& name)
{
	std::ifstream file(name.c_str());
	if (file.rdstate() != 0)
	        return false;
	file.close();
        return true;
}

time_t file_create_time(const std::string& fname)
{
	struct stat buf;
	if(::stat(fname.c_str(),&buf) == -1)
		return 0;

	return buf.st_mtime;
}

file_tree_checksum::file_tree_checksum()
    : nfiles(0), sum_size(0), modified(0)
{}

file_tree_checksum::file_tree_checksum(const config& cfg)
{
	nfiles = lexical_cast_default<size_t>(cfg["nfiles"]);
	sum_size = lexical_cast_default<size_t>(cfg["size"]);
	modified = lexical_cast_default<time_t>(cfg["modified"]);
}

void file_tree_checksum::write(config& cfg) const
{
	cfg["nfiles"] = lexical_cast<std::string>(nfiles);
	cfg["size"] = lexical_cast<std::string>(sum_size);
	cfg["modified"] = lexical_cast<std::string>(modified);
}

bool operator==(const file_tree_checksum& lhs, const file_tree_checksum& rhs)
{
	return lhs.nfiles == rhs.nfiles && lhs.sum_size == rhs.sum_size &&
	       lhs.modified == rhs.modified;
}

bool operator!=(const file_tree_checksum& lhs, const file_tree_checksum& rhs)
{
	return !operator==(lhs,rhs);
}

namespace {

void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{
	std::vector<std::string> files, dirs;
	get_files_in_dir(path,&files,&dirs,ENTIRE_FILE_PATH);

	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		++res.nfiles;

		struct stat buf;
		if(::stat(i->c_str(),&buf) != -1) {
			if(buf.st_mtime > res.modified) {
				res.modified = buf.st_mtime;
			}

			res.sum_size += buf.st_size;
		}
	}

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		get_file_tree_checksum_internal(*j,res);
	}
}

} //end anonymous namespace

file_tree_checksum get_file_tree_checksum(const std::string& path)
{
	file_tree_checksum res;
	get_file_tree_checksum_internal(path,res);
	return res;
}

const file_tree_checksum& data_tree_checksum()
{
	static file_tree_checksum checksum;
	if(checksum.nfiles == 0) {
		get_file_tree_checksum_internal("data/",checksum);
		get_file_tree_checksum_internal(get_user_data_dir() + "data/",checksum);
		std::cerr << "calculated data tree checksum: "
		          << checksum.nfiles << " files; "
		          << checksum.sum_size << " bytes\n";
	}

	return checksum;
}

int file_size(const std::string& fname)
{
	struct stat buf;
	if(::stat(fname.c_str(),&buf) == -1)
		return -1;

	return buf.st_size;
}

std::string file_name(const std::string& file)
{
#ifdef _WIN32
	static const std::string dir_separators = "\\/:";
#else
	static const std::string dir_separators = "/";
#endif

	std::string::size_type pos = file.find_last_of(dir_separators);

	if(pos == std::string::npos)
		return file;
	if(pos >= file.size()-1)
		return "";

	return file.substr(pos+1);
}

std::string directory_name(const std::string& file)
{
#ifdef _WIN32
	static const std::string dir_separators = "\\/:";
#else
	static const std::string dir_separators = "/";
#endif

	std::string::size_type pos = file.find_last_of(dir_separators);

	if(pos == std::string::npos)
		return "";

	return file.substr(0,pos+1);
}

namespace {

std::set<std::string> binary_paths;

typedef std::map<std::string,std::vector<std::string> > paths_map;
paths_map binary_paths_cache;

void init_binary_paths()
{
	if(binary_paths.empty()) {
		binary_paths.insert("");
	}
}

}

binary_paths_manager::binary_paths_manager(const config& cfg)
{
	binary_paths_cache.clear();
	init_binary_paths();

	const config::child_list& items = cfg.get_children("binary_path");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		const std::string path = (**i)["path"] + "/";
		if(binary_paths.count(path) == 0) {
			binary_paths.insert(path);
			paths_.push_back(path);
		}
	}
}

binary_paths_manager::~binary_paths_manager()
{
	binary_paths_cache.clear();

	for(std::vector<std::string>::const_iterator i = paths_.begin(); i != paths_.end(); ++i) {
		binary_paths.erase(*i);
	}
}

const std::vector<std::string>& get_binary_paths(const std::string& type)
{
	const paths_map::const_iterator itor = binary_paths_cache.find(type);
	if(itor != binary_paths_cache.end()) {
		return itor->second;
	}

	std::vector<std::string>& res = binary_paths_cache[type];
	
	init_binary_paths();

	for(std::set<std::string>::const_iterator i = binary_paths.begin(); i != binary_paths.end(); ++i) {
		res.push_back(get_user_data_dir() + "/" + *i + type + "/");
		
		if(!game_config::path.empty()) {
			res.push_back(game_config::path + "/" + *i + type + "/");
		}

		res.push_back(*i + type + "/");
	}

	return res;
}

std::string get_binary_file_location(const std::string& type, const std::string& filename)
{
	const std::vector<std::string>& paths = get_binary_paths(type);
	for(std::vector<std::string>::const_iterator i = paths.begin(); i != paths.end(); ++i) {
		const std::string file = *i + filename;
		if(file_exists(file)) {
			return file;
		}
	}

	return "";
}

