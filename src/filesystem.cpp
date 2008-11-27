/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file filesystem.cpp 
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

#ifdef __BEOS__
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
BPath be_path;
#endif

// for getenv
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <set>

// for strerror
#include <cstring>

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "loadscreen.hpp"

#define DBG_FS LOG_STREAM(debug, filesystem)
#define LOG_FS LOG_STREAM(info, filesystem)
#define WRN_FS LOG_STREAM(warn, filesystem)
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

// Don't pass directory as reference, it seems to break on 
// arklinux with GCC-4.3.
void get_files_in_dir(const std::string directory,
					  std::vector<std::string>* files,
					  std::vector<std::string>* dirs,
					  FILE_NAME_MODE mode,
					  FILE_FILTER filter,
					  FILE_REORDER_OPTION reorder,
					  file_tree_checksum* checksum)
{
	// If we have a path to find directories in, 
	// then convert relative pathnames to be rooted 
	// on the wesnoth path
#ifndef __AMIGAOS4__
	if(!directory.empty() && directory[0] != '/' && !game_config::path.empty()){
		const std::string& dir = game_config::path + "/" + directory;
		if(is_directory(dir)) {
			get_files_in_dir(dir,files,dirs,mode,filter,reorder,checksum);
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
		if (directory.empty() || directory[directory.size()-1] == '/'
#ifdef __AMIGAOS4__
			|| (directory[directory.size()-1]==':')
#endif /* __AMIGAOS4__ */
		)
			fullname = directory + basename;
		else
			fullname = directory + "/" + basename;

		if (::stat(fullname.c_str(), &st) != -1) {
			if (S_ISREG(st.st_mode)) {
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
						::stat((fullname+"/"+MAINCFG).c_str(), &st)!=-1 &&
						S_ISREG(st.st_mode)) {
					LOG_FS << "_main.cfg found : ";
					if (files != NULL) {
						if (mode == ENTIRE_FILE_PATH) {
							files->push_back(fullname + "/" + MAINCFG);
							LOG_FS << fullname << "/" << MAINCFG << '\n';
						} else {
							files->push_back(basename + "/" + MAINCFG);
							LOG_FS << basename << "/" << MAINCFG << '\n';
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

std::string get_addon_campaigns_dir()
{
	const std::string dir_path = get_user_data_dir() + "/data/campaigns";
	return get_dir(dir_path);
}

std::vector< std::string > get_addon_dirs()
{
	std::vector< std::string > ret;

	ret.push_back(get_addon_campaigns_dir());

	return ret;
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
	size_t path_size = readlink("/proc/self/exe", buf, 1024);
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


namespace {
	std::string user_data_dir;
}

static std::string setup_user_data_dir();

void set_preferences_dir(std::string path)
{
#ifndef PREFERENCES_DIR
const std::string PREFERENCES_DIR = ".wesnoth" + std::string(game_config::version).substr(0,3);
#endif
#ifdef _WIN32
#ifdef APPDATA_USERDATA
	const char* const appdata = getenv("APPDATA");
#else
	const char* appdata = "";
#endif /*APPDATA_USERDATA*/
	if (path.size() > 2 && path[1] == ':') {
		//allow absolute path override 
		game_config::preferences_dir = path;
		return;
	}	
	if (strlen(appdata) > 0)
	{
		if (path.empty())
		{
			path = PREFERENCES_DIR;
		}
		game_config::preferences_dir = appdata +std::string("/")+ path;
		return;
	} else {
		if (path.empty())
		{
			path = "userdata";
		}	
#ifndef APPDATA_USERDATA
		else
		{
			// if the path is given fallback to %APPDATA%.
			appdata = getenv("APPDATA");
			game_config::preferences_dir = appdata + std::string("/") + path;
			return;
		}
#endif /*APPDATA_USERDATA*/
		char buf[512];
		const char* const res = getcwd(buf,sizeof(buf));
		if (res == NULL)
		{
			game_config::preferences_dir = path;
			return;
		} else {
			std::string cur_path(res);
			std::replace(cur_path.begin(),cur_path.end(),'\\','/');
			game_config::preferences_dir = cur_path + "/" + path;
			return;
		}
	}

#else /*_WIN32*/
	if (path.empty()) {
		path = PREFERENCES_DIR;
	}
#ifndef __AMIGAOS4__
	const char* const current_dir = ".";
	const char* home_str = getenv("HOME");
#else
	const char* const current_dir = " ";
	const char* home_str = "PROGDIR:";
#endif
	if(home_str == NULL)
		home_str = current_dir;

	const std::string home(home_str);

#ifndef __AMIGAOS4__
	game_config::preferences_dir = home + std::string("/") + path;
#else
	game_config::preferences_dir = home + path;
#endif

#endif /*_WIN32*/
	user_data_dir = setup_user_data_dir();
}


static std::string setup_user_data_dir()
{
	if (game_config::preferences_dir.empty())
		set_preferences_dir(std::string());
#ifdef _WIN32
	_mkdir((game_config::preferences_dir).c_str());
	_mkdir((game_config::preferences_dir + "/editor").c_str());
	_mkdir((game_config::preferences_dir + "/editor/maps").c_str());
	_mkdir((game_config::preferences_dir + "/data").c_str());
	_mkdir((game_config::preferences_dir + "/data/ais").c_str());
	_mkdir((game_config::preferences_dir + "/data/campaigns").c_str());
	_mkdir((game_config::preferences_dir + "/data/multiplayer").c_str());
	_mkdir((game_config::preferences_dir + "/data/maps").c_str());
	_mkdir((game_config::preferences_dir + "/data/maps/multiplayer").c_str());
	_mkdir((game_config::preferences_dir + "/data/units").c_str());
	_mkdir((game_config::preferences_dir + "/saves").c_str());

	return game_config::preferences_dir;
#elif defined(__BEOS__)
	if (be_path.InitCheck() != B_OK) {
		BPath tpath;
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &be_path, true) == B_OK) {
			be_path.Append("wesnoth");
		} else {
			be_path.SetTo("/boot/home/config/settings/wesnoth");
		}
	#define BEOS_CREATE_PREFERENCES_SUBDIR(subdir) \
			tpath = be_path;                       \
			tpath.Append(subdir);                  \
			create_directory(tpath.Path(), 0775);

		BEOS_CREATE_PREFERENCES_SUBDIR("editor");
		BEOS_CREATE_PREFERENCES_SUBDIR("editor/maps");
		BEOS_CREATE_PREFERENCES_SUBDIR("data");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/ais");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/campaigns");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/multiplayer");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/maps");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/maps/multiplayer");
		BEOS_CREATE_PREFERENCES_SUBDIR("data/units");
		BEOS_CREATE_PREFERENCES_SUBDIR("saves");
	#undef BEOS_CREATE_PREFERENCES_SUBDIR
	}
	return be_path.Path();
#else
	const std::string& dir_path = game_config::preferences_dir;

	const bool res = create_directory_if_missing(dir_path);
	// probe read permissions (if we could make the directory)
	DIR* const dir = res ? opendir(dir_path.c_str()) : NULL;
	if(dir == NULL) {
		ERR_FS << "could not open or create preferences directory at " << dir_path << '\n';
		return "";
	}
	closedir(dir);

	// Create user data and add-on directories
	create_directory_if_missing(dir_path + "/editor");
	create_directory_if_missing(dir_path + "/editor/maps");
	create_directory_if_missing(dir_path + "/data");
	create_directory_if_missing(dir_path + "/data/ais");
	create_directory_if_missing(dir_path + "/data/campaigns");
	create_directory_if_missing(dir_path + "/data/multiplayer");
	create_directory_if_missing(dir_path + "/data/maps");
	create_directory_if_missing(dir_path + "/data/maps/multiplayer");
	create_directory_if_missing(dir_path + "/data/units");
	create_directory_if_missing(dir_path + "/saves");

	return dir_path;
#endif
}

const std::string& get_user_data_dir()
{
	// ensure setup gets called only once per session
	// FIXME: this is okay and optimized, but how should we react
	// if the user deletes a dir while we are running?
	if (user_data_dir.empty())
	{
		user_data_dir = setup_user_data_dir();
	}
	return user_data_dir;
}

static std::string read_stream(std::istream& s)
{
	std::stringstream ss;
	ss << s.rdbuf();
	return ss.str();
}

std::istream *istream_file(const std::string& fname, bool relative_from_game_path /*=true*/)
{
	std::string fname2(fname);
	return istream_file(fname2, relative_from_game_path);
}

std::istream *istream_file(std::string& fname, bool relative_from_game_path /*=true*/)
{
	LOG_FS << "streaming " << fname << " for reading.\n";
	if (fname.empty())
	{
		ERR_FS << "Trying to open file with empty name\n";
		return new std::ifstream();
	}
#ifndef _WIN32
	if (relative_from_game_path && fname[0] != '/') {
#else
	// Check if not start with driver letter
	if (relative_from_game_path && !std::isalpha(fname[0])) {
#endif
		if (!game_config::path.empty())
			fname = game_config::path + "/" + fname;
		else
			WRN_FS << "Using relative path for opening file without game_config::path set\n";
	}
	std::ifstream *s = new std::ifstream(fname.c_str(),std::ios_base::binary);
	if (s->is_open())
		return s;
	LOG_FS << "could not open '" << fname << "' for reading.\n";
	return s;

}

std::string read_file(const std::string &fname, bool relative_from_game_path /*=true*/)
{
	scoped_istream s = istream_file(fname, relative_from_game_path);
	return read_stream(*s);
}

std::string read_file(std::string &fname, bool relative_from_game_path /*=true*/)
{
	scoped_istream s = istream_file(fname, relative_from_game_path);
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

std::string next_filename(const std::string &dirname, unsigned int max)
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

	std::vector<std::string> dirs;
	get_files_in_dir(path,NULL,&dirs, ENTIRE_FILE_PATH, SKIP_MEDIA_DIR, DONT_REORDER, &res);
	increment_filesystem_progress();

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

	// not found in "/type" directory, try main directory
	res.push_back(get_user_data_dir());

	if(!game_config::path.empty())
		res.push_back(game_config::path+"/");

	res.push_back("");

	return res;
}

std::string get_binary_file_location(const std::string& type, const std::string& filename)
{
	DBG_FS << "Looking for " << filename << "\n";

	if (filename.empty()) {
		DBG_FS << "  invalid filename ( type: " << type <<")\n";
		return "";
	}

	const std::vector<std::string>& paths = get_binary_paths(type);
	for(std::vector<std::string>::const_iterator i = paths.begin(); i != paths.end(); ++i) {
		const std::string file = *i + filename;
		DBG_FS << "  Checking " << *i << "\n";
		if(file_exists(file)) {
		  DBG_FS << "  Found at " << file << "\n";
			return file;
		}
	}

	DBG_FS << "  " << filename << " not found.\n";
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
	DBG_FS <<"deleting stream";
	delete stream;
	DBG_FS << " ok\n";
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
