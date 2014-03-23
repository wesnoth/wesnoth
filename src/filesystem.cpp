/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * File-IO
 */

#include "global.hpp"

// Include files for opendir(3), readdir(3), etc.
// These files may vary from platform to platform,
// since these functions are NOT ANSI-conforming functions.
// They may have to be altered to port to new platforms

//for mkdir
#include <sys/stat.h>

#ifdef _WIN32
#include "filesystem_win32.ii"
#include <cctype>
#else /* !_WIN32 */
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#endif /* !_WIN32 */

// for getenv
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <set>
#include <boost/algorithm/string.hpp>

// for strerror
#include <cstring>

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "loadscreen.hpp"
#include "scoped_resource.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "version.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_filesystem("filesystem");
#define DBG_FS LOG_STREAM(debug, log_filesystem)
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define WRN_FS LOG_STREAM(warn, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace {
	const mode_t AccessMode = 00770;

	// These are the filenames that get special processing
	const std::string maincfg_filename = "_main.cfg";
	const std::string finalcfg_filename = "_final.cfg";
	const std::string initialcfg_filename = "_initial.cfg";

}

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFBase.h>
#endif

bool ends_with(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && std::equal(suffix.begin(),suffix.end(),str.end()-suffix.size());
}

void get_files_in_dir(const std::string &directory,
					  std::vector<std::string>* files,
					  std::vector<std::string>* dirs,
					  file_name_option mode,
					  file_filter_option filter,
					  file_reorder_option reorder,
					  file_tree_checksum* checksum)
{
	// If we have a path to find directories in,
	// then convert relative pathnames to be rooted
	// on the wesnoth path
	if(!directory.empty() && directory[0] != '/' && !game_config::path.empty()){
		std::string dir = game_config::path + "/" + directory;
		if(is_directory(dir)) {
			get_files_in_dir(dir,files,dirs,mode,filter,reorder,checksum);
			return;
		}
	}

	struct stat st;

	if (reorder == DO_REORDER) {
		LOG_FS << "searching for _main.cfg in directory " << directory << '\n';
		std::string maincfg;
		if (directory.empty() || directory[directory.size()-1] == '/')
			maincfg = directory + maincfg_filename;
		else
			maincfg = (directory + "/") + maincfg_filename;

		if (::stat(maincfg.c_str(), &st) != -1) {
			LOG_FS << "_main.cfg found : " << maincfg << '\n';
			if (files != NULL) {
				if (mode == ENTIRE_FILE_PATH)
					files->push_back(maincfg);
				else
					files->push_back(maincfg_filename);
			}
			return;
		}
	}

	DIR* dir = opendir(directory.c_str());

	if(dir == NULL) {
		return;
	}

	struct dirent* entry;
	while((entry = readdir(dir)) != NULL) {
		if(entry->d_name[0] == '.')
			continue;
#ifdef __APPLE__
		// HFS Mac OS X decomposes filenames using combining unicode characters.
		// Try to get the precomposed form.
		char macname[MAXNAMLEN+1];
		CFStringRef cstr = CFStringCreateWithCString(NULL,
							 entry->d_name,
							 kCFStringEncodingUTF8);
		CFMutableStringRef mut_str = CFStringCreateMutableCopy(NULL,
							 0, cstr);
		CFStringNormalize(mut_str, kCFStringNormalizationFormC);
		CFStringGetCString(mut_str,
				macname,sizeof(macname)-1,
				kCFStringEncodingUTF8);
		CFRelease(cstr);
		CFRelease(mut_str);
		const std::string basename = macname;
#else
		// generic Unix
		const std::string basename = entry->d_name;
#endif /* !APPLE */

		std::string fullname;
		if (directory.empty() || directory[directory.size()-1] == '/')
			fullname = directory + basename;
		else
			fullname = directory + "/" + basename;

		if (::stat(fullname.c_str(), &st) != -1) {
			if (S_ISREG(st.st_mode)) {
				if(filter == SKIP_PBL_FILES && looks_like_pbl(basename)) {
					continue;
				}
				if (files != NULL) {
					if (mode == ENTIRE_FILE_PATH)
						files->push_back(fullname);
					else
						files->push_back(basename);
				}
				if (checksum != NULL) {
					if(st.st_mtime > checksum->modified) {
						checksum->modified = st.st_mtime;
					}
					checksum->sum_size += st.st_size;
					checksum->nfiles++;
				}
			} else if (S_ISDIR(st.st_mode)) {
				if (filter == SKIP_MEDIA_DIR
						&& (basename == "images"|| basename == "sounds"))
					continue;

				if (reorder == DO_REORDER &&
						::stat((fullname+"/"+maincfg_filename).c_str(), &st)!=-1 &&
						S_ISREG(st.st_mode)) {
					LOG_FS << "_main.cfg found : ";
					if (files != NULL) {
						if (mode == ENTIRE_FILE_PATH) {
							files->push_back(fullname + "/" + maincfg_filename);
							LOG_FS << fullname << "/" << maincfg_filename << '\n';
						} else {
							files->push_back(basename + "/" + maincfg_filename);
							LOG_FS << basename << "/" << maincfg_filename << '\n';
					}
					} else {
					// Show what I consider strange
						LOG_FS << fullname << "/" << maincfg_filename << " not used now but skip the directory \n";
					}
				} else if (dirs != NULL) {
					if (mode == ENTIRE_FILE_PATH)
						dirs->push_back(fullname);
					else
						dirs->push_back(basename);
				}
			}
		}
	}

	closedir(dir);

	if(files != NULL)
		std::sort(files->begin(),files->end());

	if (dirs != NULL)
		std::sort(dirs->begin(),dirs->end());

	if (files != NULL && reorder == DO_REORDER) {
		// move finalcfg_filename, if present, to the end of the vector
		for (unsigned int i = 0; i < files->size(); i++) {
			if (ends_with((*files)[i], "/" + finalcfg_filename)) {
				files->push_back((*files)[i]);
				files->erase(files->begin()+i);
				break;
			}
		}
		// move initialcfg_filename, if present, to the beginning of the vector
		unsigned int foundit = 0;
		for (unsigned int i = 0; i < files->size(); i++)
			if (ends_with((*files)[i], "/" + initialcfg_filename)) {
				foundit = i;
				break;
			}
		// If _initial.cfg needs to be moved (it was found, but not at index 0).
		if (foundit > 0) {
			std::string initialcfg = (*files)[foundit];
			for (unsigned int i = foundit; i > 0; --i)
				(*files)[i] = (*files)[i-1];
			(*files)[0] = initialcfg;
		}
	}
}

#ifdef __native_client__
// For performance reasons, on NaCl we only keep preferences and saves in persistent storage.
std::string get_prefs_file()
{
	return "/wesnoth-userdata/preferences";
}

std::string get_save_index_file()
{
	return "/wesnoth-userdata/save_index";
}

std::string get_saves_dir()
{
	const std::string dir_path = "/wesnoth-userdata/saves";
	return get_dir(dir_path);
}

#else

std::string get_prefs_file()
{
	return get_user_config_dir() + "/preferences";
}

std::string get_default_prefs_file()
{
#ifdef HAS_RELATIVE_DEFPREF
	return game_config::path + "/" + game_config::default_preferences_path;
#else
	return game_config::default_preferences_path;
#endif
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
#endif

std::string get_addon_campaigns_dir()
{
	const std::string dir_path = get_user_data_dir() + "/data/add-ons";
	return get_dir(dir_path);
}

std::string get_intl_dir()
{
#ifdef _WIN32
	return get_cwd() + "/translations";
#else

#ifdef USE_INTERNAL_DATA
	return get_cwd() + "/" LOCALEDIR;
#endif

#if HAS_RELATIVE_LOCALEDIR
	std::string res = game_config::path + "/" LOCALEDIR;
#else
	std::string res = LOCALEDIR;
#endif

	return res;
#endif
}

std::string get_screenshot_dir()
{
	const std::string dir_path = get_user_data_dir() + "/screenshots";
	return get_dir(dir_path);
}

std::string get_next_filename(const std::string& name, const std::string& extension)
{
	std::string next_filename;
	int counter = 0;

	do {
		std::stringstream filename;

		filename << name;
		filename.width(3);
		filename.fill('0');
		filename.setf(std::ios_base::right);
		filename << counter << extension;
		counter++;
		next_filename = filename.str();
	} while(file_exists(next_filename) && counter < 1000);
	return next_filename;
}


std::string get_dir(const std::string& dir_path)
{
	DIR* dir = opendir(dir_path.c_str());
	if(dir == NULL) {
		const int res = mkdir(dir_path.c_str(),AccessMode);
		if(res == 0) {
			dir = opendir(dir_path.c_str());
		} else {
			ERR_FS << "could not open or create directory: " << dir_path << '\n';
		}
	}

	if(dir == NULL)
		return "";

	closedir(dir);

	return dir_path;
}

bool make_directory(const std::string& path)
{
	return (mkdir(path.c_str(),AccessMode) == 0);
}

bool looks_like_pbl(const std::string& file)
{
	return utils::wildcard_string_match(utf8::lowercase(file), "*.pbl");
}

// This deletes a directory with no hidden files and subdirectories.
// Also deletes a single file.
bool delete_directory(const std::string& path, const bool keep_pbl)
{
	bool ret = true;
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	get_files_in_dir(path, &files, &dirs, ENTIRE_FILE_PATH, keep_pbl ? SKIP_PBL_FILES : NO_FILTER);

	if(!files.empty()) {
		for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
			errno = 0;
			if(remove((*i).c_str()) != 0) {
				LOG_FS << "remove(" << (*i) << "): " << strerror(errno) << "\n";
				ret = false;
			}
		}
	}

	if(!dirs.empty()) {
		for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
			if(!delete_directory(*j))
				ret = false;
		}
	}

	errno = 0;
#ifdef _WIN32
	// remove() doesn't delete directories on windows.
	int (*remove)(const char*);
	if(is_directory(path))
		remove = rmdir;
	else
		remove = ::remove;
#endif
	if(remove(path.c_str()) != 0) {
		LOG_FS << "remove(" << path << "): " << strerror(errno) << "\n";
		ret = false;
	}
	return ret;
}

std::string get_cwd()
{
	char buf[1024];
	const char* const res = getcwd(buf,sizeof(buf));
	if(res != NULL) {
		std::string str(res);

#ifdef _WIN32
		std::replace(str.begin(),str.end(),'\\','/');
#endif

		return str;
	} else {
		return "";
	}
}

std::string get_exe_dir()
{
#ifndef _WIN32
	char buf[1024];
	size_t path_size = readlink("/proc/self/exe", buf, sizeof(buf)-1);
	if(path_size == static_cast<size_t>(-1))
		return std::string();
	buf[path_size] = 0;
	return std::string(dirname(buf));
#else
	return get_cwd();
#endif
}

bool create_directory_if_missing(const std::string& dirname)
{
	if(is_directory(dirname)) {
		DBG_FS << "directory " << dirname << " exists, not creating\n";
		return true;
	} else if(file_exists(dirname)) {
		ERR_FS << "cannot create directory " << dirname << "; file exists\n";
		return false;
	}
	DBG_FS << "creating missing directory " << dirname << '\n';
	return make_directory(dirname);
}
bool create_directory_if_missing_recursive(const std::string& dirname)
{
	DBG_FS<<"creating recursive directory: "<<dirname<<'\n';
	if (is_directory(dirname) == false && dirname.empty() == false)
	{
		std::string tmp_dirname = dirname;
		// remove trailing slashes or backslashes
		while ((tmp_dirname[tmp_dirname.size()-1] == '/' ||
			  tmp_dirname[tmp_dirname.size()-1] == '\\') &&
			  !tmp_dirname.empty())
		{
			tmp_dirname.erase(tmp_dirname.size()-1);
		}

		// create the first non-existing directory
		size_t pos = tmp_dirname.rfind("/");

		// we get the most right directory and *skip* it
		// we are creating it when we get back here
		if (tmp_dirname.rfind('\\') != std::string::npos &&
			tmp_dirname.rfind('\\') > pos )
			pos = tmp_dirname.rfind('\\');

		if (pos != std::string::npos)
			create_directory_if_missing_recursive(tmp_dirname.substr(0,pos));

		return create_directory_if_missing(tmp_dirname);
	}
	return create_directory_if_missing(dirname);
}

static std::string user_data_dir, user_config_dir, cache_dir;

static void setup_user_data_dir();

#ifndef _WIN32
static const std::string& get_version_path_suffix()
{
	static std::string suffix;

	// We only really need to generate this once since
	// the version number cannot change during runtime.

	if(suffix.empty()) {
		std::ostringstream s;
		s << game_config::wesnoth_version.major_version() << '.'
		  << game_config::wesnoth_version.minor_version();
		suffix = s.str();
	}

	return suffix;
}
#endif

void set_user_data_dir(std::string path)
{
#ifdef _WIN32
	if(path.empty()) {
		user_data_dir = get_cwd() + "/userdata";
	} else if (path.size() > 2 && path[1] == ':') {
		//allow absolute path override
		user_data_dir = path;
	} else {
		typedef BOOL (WINAPI *SHGSFPAddress)(HWND, LPTSTR, int, BOOL);
		SHGSFPAddress SHGetSpecialFolderPath;
		HMODULE module = LoadLibrary("shell32");
		SHGetSpecialFolderPath = reinterpret_cast<SHGSFPAddress>(GetProcAddress(module, "SHGetSpecialFolderPathA"));
		if(SHGetSpecialFolderPath) {
			LOG_FS << "Using SHGetSpecialFolderPath to find My Documents\n";
			char my_documents_path[MAX_PATH];
			if(SHGetSpecialFolderPath(NULL, my_documents_path, 5, 1)) {
				std::string mygames_path = std::string(my_documents_path) + "/" + "My Games";
				boost::algorithm::replace_all(mygames_path, std::string("\\"), std::string("/"));
				create_directory_if_missing(mygames_path);
				user_data_dir = mygames_path + "/" + path;
			} else {
				WRN_FS << "SHGetSpecialFolderPath failed\n";
				user_data_dir = get_cwd() + "/" + path;
			}
		} else {
			LOG_FS << "Failed to load SHGetSpecialFolderPath function\n";
			user_data_dir = get_cwd() + "/" + path;
		}
	}

#else /*_WIN32*/

#ifdef PREFERENCES_DIR
	if (path.empty()) path = PREFERENCES_DIR;
#endif

	std::string path2 = ".wesnoth" + get_version_path_suffix();

#ifdef _X11
	const char *home_str = getenv("HOME");

	if (path.empty()) {
		char const *xdg_data = getenv("XDG_DATA_HOME");
		if (!xdg_data || xdg_data[0] == '\0') {
			if (!home_str) {
				path = path2;
				goto other;
			}
			user_data_dir = home_str;
			user_data_dir += "/.local/share";
		} else user_data_dir = xdg_data;
		user_data_dir += "/wesnoth/";
		user_data_dir += get_version_path_suffix();
		create_directory_if_missing_recursive(user_data_dir);
	} else {
		other:
		std::string home = home_str ? home_str : ".";

		if (path[0] == '/')
			user_data_dir = path;
		else
			user_data_dir = home + "/" + path;
	}
#else
	if (path.empty()) path = path2;

	const char* home_str = getenv("HOME");
	std::string home = home_str ? home_str : ".";

	if (path[0] == '/')
		user_data_dir = path;
	else
		user_data_dir = home + std::string("/") + path;
#endif

#endif /*_WIN32*/
	setup_user_data_dir();
}

void set_user_config_dir(std::string path)
{
	user_config_dir = path;
	create_directory_if_missing(user_config_dir);
}

static void setup_user_data_dir()
{
#ifdef _WIN32
	_mkdir(user_data_dir.c_str());
	_mkdir((user_data_dir + "/editor").c_str());
	_mkdir((user_data_dir + "/editor/maps").c_str());
	_mkdir((user_data_dir + "/editor/scenarios").c_str());
	_mkdir((user_data_dir + "/data").c_str());
	_mkdir((user_data_dir + "/data/add-ons").c_str());
	_mkdir((user_data_dir + "/saves").c_str());
	_mkdir((user_data_dir + "/persist").c_str());
#else
	const std::string& dir_path = user_data_dir;

	const bool res = create_directory_if_missing(dir_path);
	// probe read permissions (if we could make the directory)
	DIR* const dir = res ? opendir(dir_path.c_str()) : NULL;
	if(dir == NULL) {
		ERR_FS << "could not open or create preferences directory at " << dir_path << '\n';
		return;
	}
	closedir(dir);

	// Create user data and add-on directories
	create_directory_if_missing(dir_path + "/editor");
	create_directory_if_missing(dir_path + "/editor/maps");
	create_directory_if_missing(dir_path + "/editor/scenarios");
	create_directory_if_missing(dir_path + "/data");
	create_directory_if_missing(dir_path + "/data/add-ons");
	create_directory_if_missing(dir_path + "/saves");
	create_directory_if_missing(dir_path + "/persist");
#endif
}

const std::string& get_user_data_dir()
{
	// ensure setup gets called only once per session
	// FIXME: this is okay and optimized, but how should we react
	// if the user deletes a dir while we are running?
	if (user_data_dir.empty())
	{
		set_user_data_dir(std::string());
	}
	return user_data_dir;
}

const std::string &get_user_config_dir()
{
	if (user_config_dir.empty())
	{
#if defined(_X11) && !defined(PREFERENCES_DIR)
		char const *xdg_config = getenv("XDG_CONFIG_HOME");
		std::string path;
		if (!xdg_config || xdg_config[0] == '\0') {
			xdg_config = getenv("HOME");
			if (!xdg_config) {
				user_config_dir = get_user_data_dir();
				return user_config_dir;
			}
			path = xdg_config;
			path += "/.config";
		} else path = xdg_config;
		path += "/wesnoth";
		set_user_config_dir(path);
#else
		user_config_dir = get_user_data_dir();
#endif
	}
	return user_config_dir;
}

const std::string &get_cache_dir()
{
	if (cache_dir.empty())
	{
#if defined(_X11) && !defined(PREFERENCES_DIR)
		char const *xdg_cache = getenv("XDG_CACHE_HOME");
		if (!xdg_cache || xdg_cache[0] == '\0') {
			xdg_cache = getenv("HOME");
			if (!xdg_cache) {
				cache_dir = get_dir(get_user_data_dir() + "/cache");
				return cache_dir;
			}
			cache_dir = xdg_cache;
			cache_dir += "/.cache";
		} else cache_dir = xdg_cache;
		cache_dir += "/wesnoth";
		create_directory_if_missing_recursive(cache_dir);
#else
		cache_dir = get_dir(get_user_data_dir() + "/cache");
#endif
	}
	return cache_dir;
}

static std::string read_stream(std::istream& s)
{
	std::stringstream ss;
	ss << s.rdbuf();
	return ss.str();
}

std::istream *istream_file(const std::string &fname)
{
	LOG_FS << "Streaming " << fname << " for reading.\n";
	if (fname.empty())
	{
		ERR_FS << "Trying to open file with empty name.\n";
		std::ifstream *s = new std::ifstream();
		s->clear(std::ios_base::failbit);
		return s;
	}

	std::ifstream *s = new std::ifstream(fname.c_str(),std::ios_base::binary);
	if (s->is_open())
		return s;
	ERR_FS << "Could not open '" << fname << "' for reading.\n";
	return s;

}

std::string read_file(const std::string &fname)
{
	scoped_istream s = istream_file(fname);
	return read_stream(*s);
}

std::ostream *ostream_file(std::string const &fname)
{
	LOG_FS << "streaming " << fname << " for writing.\n";
	return new std::ofstream(fname.c_str(), std::ios_base::binary);
}

// Throws io_exception if an error occurs
void write_file(const std::string& fname, const std::string& data)
{
	//const util::scoped_resource<FILE*,close_FILE> file(fopen(fname.c_str(),"wb"));
	const util::scoped_FILE file(fopen(fname.c_str(),"wb"));
	if(file.get() == NULL) {
		throw io_exception("Could not open file for writing: '" + fname + "'");
	}

	const size_t block_size = 4096;
	char buf[block_size];

	for(size_t i = 0; i < data.size(); i += block_size) {
		const size_t bytes = std::min<size_t>(block_size,data.size() - i);
		std::copy(data.begin() + i, data.begin() + i + bytes,buf);
		const size_t res = fwrite(buf,1,bytes,file.get());
		if(res != bytes) {
			throw io_exception("Error writing to file: '" + fname + "'");
		}
	}
}


std::string read_map(const std::string& name)
{
	std::string res;
	std::string map_location = get_wml_location("maps/" + name);
	if(!map_location.empty()) {
		res = read_file(map_location);
	}

	if (res.empty()) {
		res = read_file(get_user_data_dir() + "/editor/maps/" + name);
	}

	return res;
}

static bool is_directory_internal(const std::string& fname)
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

bool is_directory(const std::string& fname)
{
	if(fname.empty()) {
		return false;
	}
	if(fname[0] != '/' && !game_config::path.empty()) {
		if(is_directory_internal(game_config::path + "/" + fname))
			return true;
	}

	return is_directory_internal(fname);
}

bool file_exists(const std::string& name)
{
#ifdef _WIN32
       struct stat st;
       return (::stat(name.c_str(), &st) == 0);
#else
	struct stat st;
	return (::stat(name.c_str(), &st) != -1);
#endif
}

time_t file_create_time(const std::string& fname)
{
	struct stat buf;
	if(::stat(fname.c_str(),&buf) == -1)
		return 0;

	return buf.st_mtime;
}

/**
 * Returns true if the file ends with '.gz'.
 *
 * @param filename                The name to test.
 */
bool is_gzip_file(const std::string& filename)
{
	return (filename.length() > 3
		&& filename.substr(filename.length() - 3) == ".gz");
}

/**
 * Returns true if the file ends with '.bz2'.
 *
 * @param filename                The name to test.
 */
bool is_bzip2_file(const std::string& filename)
{
	return (filename.length() > 4
		&& filename.substr(filename.length() - 4) == ".bz2");
}

file_tree_checksum::file_tree_checksum()
	: nfiles(0), sum_size(0), modified(0)
{}

file_tree_checksum::file_tree_checksum(const config& cfg) :
	nfiles	(cfg["nfiles"].to_size_t()),
	sum_size(cfg["size"].to_size_t()),
	modified(cfg["modified"].to_time_t())
{
}

void file_tree_checksum::write(config& cfg) const
{
	cfg["nfiles"] = nfiles;
	cfg["size"] = sum_size;
	cfg["modified"] = modified;
}

bool file_tree_checksum::operator==(const file_tree_checksum &rhs) const
{
	return nfiles == rhs.nfiles && sum_size == rhs.sum_size &&
		modified == rhs.modified;
}

static void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{

	std::vector<std::string> dirs;
	get_files_in_dir(path,NULL,&dirs, ENTIRE_FILE_PATH, SKIP_MEDIA_DIR, DONT_REORDER, &res);
	loadscreen::increment_progress();

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		get_file_tree_checksum_internal(*j,res);
	}
}

const file_tree_checksum& data_tree_checksum(bool reset)
{
	static file_tree_checksum checksum;
	if (reset)
		checksum.reset();
	if(checksum.nfiles == 0) {
		get_file_tree_checksum_internal("data/",checksum);
		get_file_tree_checksum_internal(get_user_data_dir() + "/data/",checksum);
		LOG_FS << "calculated data tree checksum: "
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
// Analogous to POSIX basename(3), but for C++ string-object pathnames
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
// Analogous to POSIX dirname(3), but for C++ string-object pathnames
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

}

static void init_binary_paths()
{
	if(binary_paths.empty()) {
		binary_paths.insert("");
	}
}

binary_paths_manager::binary_paths_manager() : paths_()
{}

binary_paths_manager::binary_paths_manager(const config& cfg) : paths_()
{
	set_paths(cfg);
}

binary_paths_manager::~binary_paths_manager()
{
	cleanup();
}

void binary_paths_manager::set_paths(const config& cfg)
{
	cleanup();
	init_binary_paths();

	BOOST_FOREACH(const config &bp, cfg.child_range("binary_path"))
	{
		std::string path = bp["path"].str();
		if (path.find("..") != std::string::npos) {
			ERR_FS << "Invalid binary path '" << path << "'\n";
			continue;
		}
		if (!path.empty() && path[path.size()-1] != '/') path += "/";
		if(binary_paths.count(path) == 0) {
			binary_paths.insert(path);
			paths_.push_back(path);
		}
	}
}

void binary_paths_manager::cleanup()
{
	binary_paths_cache.clear();

	for(std::vector<std::string>::const_iterator i = paths_.begin(); i != paths_.end(); ++i) {
		binary_paths.erase(*i);
	}
}

void clear_binary_paths_cache()
{
	binary_paths_cache.clear();
}

const std::vector<std::string>& get_binary_paths(const std::string& type)
{
	const paths_map::const_iterator itor = binary_paths_cache.find(type);
	if(itor != binary_paths_cache.end()) {
		return itor->second;
	}

	if (type.find("..") != std::string::npos) {
		// Not an assertion, as language.cpp is passing user data as type.
		ERR_FS << "Invalid WML type '" << type << "' for binary paths\n";
		static std::vector<std::string> dummy;
		return dummy;
	}

	std::vector<std::string>& res = binary_paths_cache[type];

	init_binary_paths();

	BOOST_FOREACH(const std::string &path, binary_paths)
	{
		res.push_back(get_user_data_dir() + "/" + path + type + "/");

		if(!game_config::path.empty()) {
			res.push_back(game_config::path + "/" + path + type + "/");
		}
	}

	// not found in "/type" directory, try main directory
	res.push_back(get_user_data_dir() + "/");

	if(!game_config::path.empty())
		res.push_back(game_config::path+"/");

	return res;
}

std::string get_binary_file_location(const std::string& type, const std::string& filename)
{
	DBG_FS << "Looking for '" << filename << "'.\n";

	if (filename.empty()) {
		LOG_FS << "  invalid filename (type: " << type <<")\n";
		return std::string();
	}

	// Some parts of Wesnoth enjoy putting ".." inside filenames. This is
	// bad and should be fixed. But in the meantime, deal with them in a dumb way.
	std::string::size_type pos = filename.rfind("../");
	if (pos != std::string::npos) {
		std::string nf = filename.substr(pos + 3);
		LOG_FS << "Illegal path '" << filename << "' replaced by '" << nf << "'\n";
		return get_binary_file_location(type, nf);
	}

	if (filename.find("..") != std::string::npos) {
		ERR_FS << "Illegal path '" << filename << "' (\"..\" not allowed).\n";
		return std::string();
	}

	BOOST_FOREACH(const std::string &path, get_binary_paths(type))
	{
		const std::string file = path + filename;
		DBG_FS << "  checking '" << path << "'\n";
		if(file_exists(file)) {
			DBG_FS << "  found at '" << file << "'\n";
			return file;
		}
	}

	DBG_FS << "  not found\n";
	return std::string();
}

std::string get_binary_dir_location(const std::string &type, const std::string &filename)
{
	DBG_FS << "Looking for '" << filename << "'.\n";

	if (filename.empty()) {
		LOG_FS << "  invalid filename (type: " << type <<")\n";
		return std::string();
	}

	if (filename.find("..") != std::string::npos) {
		ERR_FS << "Illegal path '" << filename << "' (\"..\" not allowed).\n";
		return std::string();
	}

	BOOST_FOREACH(const std::string &path, get_binary_paths(type))
	{
		const std::string file = path + filename;
		DBG_FS << "  checking '" << path << "'\n";
		if (is_directory(file)) {
			DBG_FS << "  found at '" << file << "'\n";
			return file;
		}
	}

	DBG_FS << "  not found\n";
	return std::string();
}

std::string get_wml_location(const std::string &filename, const std::string &current_dir)
{
	DBG_FS << "Looking for '" << filename << "'.\n";

	std::string result;

	if (filename.empty()) {
		LOG_FS << "  invalid filename\n";
		return result;
	}

	if (filename.find("..") != std::string::npos) {
		ERR_FS << "Illegal path '" << filename << "' (\"..\" not allowed).\n";
		return result;
	}

	bool already_found = false;

	if (filename[0] == '~')
	{
		// If the filename starts with '~', look in the user data directory.
		result = get_user_data_dir() + "/data/" + filename.substr(1);
		DBG_FS << "  trying '" << result << "'\n";

		already_found = file_exists(result) || is_directory(result);
	}
	else if (filename.size() >= 2 && filename[0] == '.' && filename[1] == '/')
	{
		// If the filename begins with a "./", look in the same directory
		// as the file currently being preprocessed.
		result = current_dir + filename.substr(2);
	}
	else if (!game_config::path.empty())
		result = game_config::path + "/data/" + filename;

	DBG_FS << "  trying '" << result << "'\n";

	if (result.empty() ||
	    (!already_found && !file_exists(result) && !is_directory(result)))
	{
		DBG_FS << "  not found\n";
		result.clear();
	}
	else
		DBG_FS << "  found: '" << result << "'\n";

	return result;
}

std::string get_short_wml_path(const std::string &filename)
{
	std::string match = get_user_data_dir() + "/data/";
	if (filename.find(match) == 0) {
		return "~" + filename.substr(match.size());
	}
	match = game_config::path + "/data/";
	if (filename.find(match) == 0) {
		return filename.substr(match.size());
	}
	return filename;
}

std::string get_independent_image_path(const std::string &filename)
{
	std::string full_path = get_binary_file_location("images", filename);

	if(!full_path.empty()) {
		std::string match = get_user_data_dir() + "/";
		if(full_path.find(match) == 0) {
			return full_path.substr(match.size());
		}
		match = game_config::path + "/";
		if(full_path.find(match) == 0) {
			return full_path.substr(match.size());
		}
	}

	return full_path;
}

std::string get_program_invocation(const std::string& program_name) {
#ifdef DEBUG
  #ifdef _WIN32
	const char *program_suffix = "-debug.exe";
  #else
	const char *program_suffix = "-debug";
  #endif
#else
  #ifdef _WIN32
	const char *program_suffix = ".exe";
  #else
	const char *program_suffix = "";
  #endif
#endif

	const std::string real_program_name(program_name + program_suffix);
	if(game_config::wesnoth_program_dir.empty()) return real_program_name;
#ifdef _WIN32
	return game_config::wesnoth_program_dir + "\\" + real_program_name;
#else
	return game_config::wesnoth_program_dir + "/" + real_program_name;
#endif
}

bool is_path_sep(char c)
{
#ifdef _WIN32
	if (c == '/' || c == '\\') return true;
#else
	if (c == '/') return true;
#endif
	return false;
}

std::string normalize_path(const std::string &p1)
{
	if (p1.empty()) return p1;

	std::string p2;
#ifdef _WIN32
	if (p1.size() >= 2 && p1[1] == ':')
		// Windows relative paths with explicit drive name are not handled.
		p2 = p1;
	else
#endif
	if (!is_path_sep(p1[0]))
		p2 = get_cwd() + "/" + p1;
	else
		p2 = p1;

#ifdef _WIN32
	std::string drive;
	if (p2.size() >= 2 && p2[1] == ':') {
		drive = p2.substr(0, 2);
		p2.erase(0, 2);
	}
#endif

	std::vector<std::string> components(1);
	for (int i = 0, i_end = p2.size(); i <= i_end; ++i)
	{
		std::string &last = components[components.size() - 1];
		char c = p2.c_str()[i];
		if (is_path_sep(c) || c == 0)
		{
			if (last == ".")
				last.clear();
			else if (last == "..")
			{
				if (components.size() >= 2) {
					components.pop_back();
					components[components.size() - 1].clear();
				} else
					last.clear();
			}
			else if (!last.empty())
				components.push_back(std::string());
		}
		else
			last += c;
	}

	std::ostringstream p4;
	components.pop_back();

#ifdef _WIN32
	p4 << drive;
#endif

	BOOST_FOREACH(const std::string &s, components)
	{
		p4 << '/' << s;
	}

	DBG_FS << "Normalizing '" << p2 << "' to '" << p4.str() << "'\n";

	return p4.str();
}

scoped_istream& scoped_istream::operator=(std::istream *s)
{
	delete stream;
	stream = s;
	return *this;
}

scoped_istream::~scoped_istream()
{
	delete stream;
}

scoped_ostream& scoped_ostream::operator=(std::ostream *s)
{
	delete stream;
	stream = s;
	return *this;
}

scoped_ostream::~scoped_ostream()
{
	delete stream;
}
