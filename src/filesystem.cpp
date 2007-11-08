/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file filesystem.cpp 
//! File-IO

#include "global.hpp"

// Include files for opendir(3), readdir(3), etc. 
// These files may vary from platform to platform, 
// since these functions are NOT ANSI-conforming functions. 
// They may have to be altered to port to new platforms
#include <sys/types.h>

//for mkdir
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32

/* /////////////////////////////////////////////////////////////////////////
 * This code swiped from dirent.c in the unixem library, version 1.7.3.
 * See http://synesis.com.au/software/unixem.html for full sources.
 * It's under BSD license.
 */

#include <direct.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>
#include <windows.h>


/* /////////////////////////////////////////////////////////////////////////
 * Compiler differences
 */

#if defined(__BORLANDC__)
# define DIRENT_PROVIDED_BY_COMPILER
#elif defined(__DMC__)
# define DIRENT_PROVIDED_BY_COMPILER
#elif defined(__GNUC__)
# define DIRENT_PROVIDED_BY_COMPILER
#elif defined(__INTEL_COMPILER)
#elif defined(_MSC_VER)
#elif defined(__MWERKS__)
#elif defined(__WATCOMC__)
#else
# error Compiler not discriminated
#endif /* compiler */

#if defined(DIRENT_PROVIDED_BY_COMPILER)
#include <dirent.h>
#else

/* ////////////////////////////////////////////////////////////////////// */

#include <stddef.h>

#ifndef NAME_MAX
# define NAME_MAX   (260)
#endif /* !NAME_MAX */

struct dirent
{
	char	d_name[NAME_MAX + 1];	   /*!< file name (null-terminated) */
	int 	d_mode;
};

struct DIR
{
	char			  directory[_MAX_DIR+1];   /* . */
	WIN32_FIND_DATAA  find_data;			   /* The Win32 FindFile data. */
	HANDLE			  hFind;				   /* The Win32 FindFile handle. */
	struct dirent	  dirent;				   /* The handle's entry. */
};

#ifndef FILE_ATTRIBUTE_ERROR
# define FILE_ATTRIBUTE_ERROR			(0xFFFFFFFF)
#endif /* FILE_ATTRIBUTE_ERROR */

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

static HANDLE dirent__findfile_directory(char const *name, LPWIN32_FIND_DATAA data)
{
	char	search_spec[_MAX_PATH +1];

	// Simply add the *.*, ensuring the path separator is included.
	(void)lstrcpyA(search_spec, name);
	if( '\\' != search_spec[lstrlenA(search_spec) - 1] &&
		'/' != search_spec[lstrlenA(search_spec) - 1])
	{
		(void)lstrcatA(search_spec, "\\*.*");
	}
	else
	{
		(void)lstrcatA(search_spec, "*.*");
	}

	return FindFirstFileA(search_spec, data);
}

/* /////////////////////////////////////////////////////////////////////////
 * API functions
 */

DIR *opendir(char const *name)
{
	DIR 	*result =	NULL;
	DWORD	dwAttr;

	// Must be a valid name 
	if( !name ||
		!*name ||
		(dwAttr = GetFileAttributes(name)) == 0xFFFFFFFF)
	{
		errno = ENOENT;
	}
	// Must be a directory 
	else if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		errno = ENOTDIR;
	}
	else
	{
		result = (DIR*)malloc(sizeof(DIR));

		if(result == NULL)
		{
			errno = ENOMEM;
		}
		else
		{
			result->hFind=dirent__findfile_directory(name, &result->find_data);

			if(result->hFind == INVALID_HANDLE_VALUE)
			{
				free(result);

				result = NULL;
			}
			else
			{
				// Save the directory, in case of rewind. 
				(void)lstrcpyA(result->directory, name);
				(void)lstrcpyA(result->dirent.d_name, result->find_data.cFileName);
				result->dirent.d_mode	=	(int)result->find_data.dwFileAttributes;
			}
		}
	}

	return result;
}

int closedir(DIR *dir)
{
	int ret;

	if(dir == NULL)
	{
		errno = EBADF;

		ret = -1;
	}
	else
	{
		// Close the search handle, if not already done.
		if(dir->hFind != INVALID_HANDLE_VALUE)
		{
			(void)FindClose(dir->hFind);
		}

		free(dir);

		ret = 0;
	}

	return ret;
}

struct dirent *readdir(DIR *dir)
{
	// The last find exhausted the matches, so return NULL.
	if(dir->hFind == INVALID_HANDLE_VALUE)
	{
		if(FILE_ATTRIBUTE_ERROR == dir->find_data.dwFileAttributes)
		{
			errno = EBADF;
		}
		else
		{
			dir->find_data.dwFileAttributes = FILE_ATTRIBUTE_ERROR;
		}

		return NULL;
	}
	else
	{
		// Copy the result of the last successful match to dirent.
		(void)lstrcpyA(dir->dirent.d_name, dir->find_data.cFileName);

		// Attempt the next match.
		if(!FindNextFileA(dir->hFind, &dir->find_data))
		{
			// Exhausted all matches, so close and null the handle.
			(void)FindClose(dir->hFind);
			dir->hFind = INVALID_HANDLE_VALUE;
		}

		return &dir->dirent;
	}
}

/*
 * Microsoft C uses _stat instead of stat,
 * for both the function name and the structure name.
 * See <http://svn.ghostscript.com:8080/ghostscript/trunk/gs/src/stat_.h>
 */
#ifdef _MSC_VER
#  define stat _stat
namespace {
	typedef int mode_t;
}
#endif

#ifndef S_IFMT
#define S_IFMT	(S_IFDIR|S_IFREG)
#endif
#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#endif /* !DIRENT_PROVIDED_BY_COMPILER */

#define mkdir(a,b) (_mkdir(a))

#else /* !_WIN32 */

#include <unistd.h>

#include <dirent.h>

#endif /* !_WIN32 */

#ifdef __BEOS__
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
BPath be_path;
#endif

// for getenv
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>

#include "wesconfig.h"
#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "scoped_resource.hpp"
#include "util.hpp"
#include "loadscreen.hpp"

#define LOG_FS LOG_STREAM(info, filesystem)
#define ERR_FS LOG_STREAM(err, filesystem)

namespace {
	const mode_t AccessMode = 00770;
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

// These are the filenames that get special processing
#define MAINCFG "_main.cfg"
#define FINALCFG	"_final.cfg"

void get_files_in_dir(const std::string& directory,
					  std::vector<std::string>* files,
					  std::vector<std::string>* dirs,
					  FILE_NAME_MODE mode,
					  FILE_REORDER_OPTION reorder)
{
	// If we have a path to find directories in, 
	// then convert relative pathnames to be rooted 
	// on the wesnoth path
#ifndef __AMIGAOS4__
	if(!directory.empty() && directory[0] != '/' && !game_config::path.empty()){
		const std::string& dir = game_config::path + "/" + directory;
		if(is_directory(dir)) {
			get_files_in_dir(dir,files,dirs,mode,reorder);
			return;
		}
	}
#endif /* __AMIGAOS4__ */

	struct stat st;

	if (reorder == DO_REORDER) {
		LOG_FS << "searching _main.cfg in directory " << directory << '\n';
		std::string maincfg;
		if (directory.empty() || directory[directory.size()-1] == '/'
#ifdef __AMIGAOS4__
			|| (directory[directory.size()-1]==':')
#endif /* __AMIGAOS4__ */
		)
			maincfg = directory + MAINCFG;
		else
			maincfg = (directory + "/") + MAINCFG;

		if (::stat(maincfg.c_str(), &st) != -1) {
			LOG_FS << "_main.cfg found : " << maincfg << '\n';
			if (files != NULL) {
				if (mode == ENTIRE_FILE_PATH)
					files->push_back(maincfg);
				else
					files->push_back(MAINCFG);
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
		char basename[MAXNAMLEN+1];
		CFStringRef cstr = CFStringCreateWithCString(NULL,
							 entry->d_name,
							 kCFStringEncodingUTF8);
		CFMutableStringRef mut_str = CFStringCreateMutableCopy(NULL,
							 0, cstr);
		CFStringNormalize(mut_str, kCFStringNormalizationFormC);
		CFStringGetCString(mut_str,
				basename,sizeof(basename)-1,
				kCFStringEncodingUTF8);
		CFRelease(cstr);
		CFRelease(mut_str);
#else
		// generic Unix 
		char *basename = entry->d_name;
#endif /* !APPLE */

		std::string fullname;
		if (directory.empty() || directory[directory.size()-1] == '/'
#ifdef __AMIGAOS4__
			|| (directory[directory.size()-1]==':')
#endif /* __AMIGAOS4__ */
		)
			fullname = directory + basename;
		else
			fullname = (directory + "/") + basename;

		if (::stat(fullname.c_str(), &st) != -1) {
			if (S_ISREG(st.st_mode)) {
				if (files != NULL) {
					if (mode == ENTIRE_FILE_PATH)
						files->push_back(fullname);
					else
						files->push_back(basename);
				}
			} else if (S_ISDIR(st.st_mode)) {
				if (reorder == DO_REORDER &&
						::stat((fullname+"/"+MAINCFG).c_str(), &st)!=-1 &&
						S_ISREG(st.st_mode)) {
					LOG_FS << "_main.cfg found : ";
					if (files != NULL) {
						if (mode == ENTIRE_FILE_PATH) {
							files->push_back(fullname + "/" + MAINCFG);
							LOG_FS << fullname << "/" << MAINCFG << '\n';
						} else {
							files->push_back(std::string(basename) + "/" + MAINCFG);
							LOG_FS << std::string(basename) << "/" << MAINCFG << '\n';
					}
					} else {
					// Show what I consider strange
						LOG_FS << fullname << "/" << MAINCFG << " not used now but skip the directory \n";
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
		for (unsigned int i = 0; i < files->size(); i++)
			if (ends_with((*files)[i], FINALCFG)) {
				files->push_back((*files)[i]);
				files->erase(files->begin()+i);
				break;
			}
	}
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
	return get_cwd() + "/po";
#endif

#ifdef USE_INTERNAL_DATA
	return get_cwd() + "/" LOCALEDIR;
#endif

#if HAS_RELATIVE_LOCALEDIR
	std::string res = game_config::path + "/" LOCALEDIR;
#else
	std::string res = LOCALEDIR;
#endif

	return res;
}

std::string get_screenshot_dir()
{
	const std::string dir_path = get_user_data_dir() + "/screenshots";
	return get_dir(dir_path);
}

std::string get_upload_dir()
{
	const std::string dir_path = get_user_data_dir() + "/upload";
	return get_dir(dir_path);
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

// This deletes a directory with no hidden files and subdirectories.
// Also deletes a single file.
bool delete_directory(const std::string& path)
{
	bool ret = true;
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	get_files_in_dir(path, &files, &dirs, ENTIRE_FILE_PATH);

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
		_mkdir("userdata/data");
		_mkdir("userdata/data/ais");
		_mkdir("userdata/data/scenarios");
		_mkdir("userdata/data/scenarios/multiplayer");
		_mkdir("userdata/data/maps");
		_mkdir("userdata/data/maps/multiplayer");
		_mkdir("userdata/saves");
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
#elif defined(__BEOS__)
	if (be_path.InitCheck() != B_OK) {
		BPath tpath;
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &be_path, true) == B_OK) {
			be_path.Append("wesnoth");
		} else {
			be_path.SetTo("/boot/home/config/settings/wesnoth");
		}
		tpath = be_path;
		tpath.Append("editor/maps");
		create_directory(tpath.Path(), 0775);
	}
	return be_path.Path();
#else

#ifndef __AMIGAOS4__
	static const char* const current_dir = ".";
	const char* home_str = getenv("HOME");
#else
	static const char* const current_dir = " ";
	const char* home_str = "PROGDIR:";
#endif
	if(home_str == NULL)
		home_str = current_dir;

	const std::string home(home_str);

#ifndef PREFERENCES_DIR
#define PREFERENCES_DIR ".wesnoth"
#endif

#ifndef __AMIGAOS4__
	const std::string dir_path = home + std::string("/") + PREFERENCES_DIR;
#else
	const std::string dir_path = home + PREFERENCES_DIR;
#endif
	DIR* dir = opendir(dir_path.c_str());
	if(dir == NULL) {
		const int res = mkdir(dir_path.c_str(),AccessMode);

		// Also create the maps directory
		mkdir((dir_path + "/editor").c_str(),AccessMode);
		mkdir((dir_path + "/editor/maps").c_str(),AccessMode);
		mkdir((dir_path + "/data").c_str(),AccessMode);
		mkdir((dir_path + "/data/ais").c_str(),AccessMode);
		mkdir((dir_path + "/data/scenarios").c_str(),AccessMode);
		mkdir((dir_path + "/data/scenarios/multiplayer").c_str(),AccessMode);
		mkdir((dir_path + "/data/maps").c_str(),AccessMode);
		mkdir((dir_path + "/data/maps/multiplayer").c_str(),AccessMode);
		mkdir((dir_path + "/saves").c_str(),AccessMode);
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
#endif
}

static std::string read_stream(std::istream& s)
{
	std::stringstream ss;
	ss << s.rdbuf();
	return ss.str();
}

std::istream *istream_file(std::string const &fname)
{
	LOG_FS << "streaming " << fname << " for reading.\n";
	if (!fname.empty() && fname[0] != '/' && !game_config::path.empty()) {
		std::ifstream *s = new std::ifstream((game_config::path + "/" + fname).c_str(),std::ios_base::binary);
		if (s->is_open())
			return s;
		LOG_FS << "could not open " << fname << " for reading.\n";
		delete s;
	}

	//! @todo FIXME: why do we rely on this even with relative paths ?
	// Still useful with zipios, for things like cache and prefs.
	// NOTE zipios has been removed - not sure what to do with this code.
	std::istream *s = new std::ifstream(fname.c_str(), std::ios_base::binary);
	if (s->fail())
		LOG_FS << "streaming " << fname << " failed.\n";
	return s;
}

std::string read_file(std::string const &fname)
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
		const size_t bytes = minimum<size_t>(block_size,data.size() - i);
		std::copy(data.begin() + i, data.begin() + i + bytes,buf);
		const size_t res = fwrite(buf,1,bytes,file.get());
		if(res != bytes) {
			throw io_exception("Error writing to file: '" + fname + "'");
		}
	}
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
	std::ifstream file(name.c_str(),std::ios_base::binary);
	if (file.rdstate() != 0) {
		return false;
	}
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

std::string next_filename(const std::string &dirname, unsigned int max)
// Return the next ordered full filename within this directory
{
	std::vector<std::string> files;
	std::stringstream fname;
	unsigned int num = 1;

	// These are sorted, so we can simply add one to last one.
	get_files_in_dir(dirname, &files);

	// Make sure we skip over any files we didn't create ourselves.
	std::vector<std::string>::reverse_iterator i;
	for (i = files.rbegin(); i != files.rend(); ++i) {
		if (i->length() == 8) {
			try {
				num = lexical_cast<int>(*i)+1;
				break;
			} catch (bad_lexical_cast &) {
			}
		}
	}

	// Erase oldest files if we have too many
	if (max) {
		for (unsigned int j = 0; j + max < files.size(); j++) {
			delete_directory(dirname + "/" + files[j]);
		}
	}

	fname << std::setw(8) << std::setfill('0') << num;
	return dirname + "/" + fname.str();
}

file_tree_checksum::file_tree_checksum()
	: nfiles(0), sum_size(0), modified(0)
{}

file_tree_checksum::file_tree_checksum(const config& cfg) :
	nfiles	(lexical_cast_default<size_t>(cfg["nfiles"])),
	sum_size(lexical_cast_default<size_t>(cfg["size"])),
	modified(lexical_cast_default<time_t>(cfg["modified"]))
{
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

static void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{
	std::vector<std::string> files, dirs;
	get_files_in_dir(path,&files,&dirs,ENTIRE_FILE_PATH);
	increment_filesystem_progress();
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

const file_tree_checksum& data_tree_checksum()
{
	static file_tree_checksum checksum;
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

	const config::child_list& items = cfg.get_children("binary_path");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		std::string path = (**i)["path"].str();
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
	if(!filename.empty()) {
		LOG_STREAM(info, paths) << "Looking for " << filename << " in  '.'\n";
		if(file_exists(filename) || is_directory(filename)) {
		  LOG_STREAM(info, paths) << "  Found at " << filename << "\n";
			return filename;
		}
	}

	for(std::vector<std::string>::const_iterator i = paths.begin(); i != paths.end(); ++i) {
		const std::string file = *i + filename;
		LOG_STREAM(info, paths) << "  Checking " << *i << "\n";
		if(file_exists(file) || is_directory(file)) {
		  LOG_STREAM(info, paths) << "  Found at " << file << "\n";
			return file;
		}
	}

	LOG_STREAM(info, paths) << "  " << filename << " not found.\n";
	return "";
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
