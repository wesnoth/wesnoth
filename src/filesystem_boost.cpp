/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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

#include "filesystem.hpp"
#include "serialization/unicode.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/windows_error.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <set>

using boost::uintmax_t;

#ifdef _WIN32
#include "log_windows.hpp"

#include <boost/locale.hpp>

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#endif /* !_WIN32 */

// Copied from boost::predef, as it's there only since 1.55.
#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)

#define WESNOTH_BOOST_OS_IOS (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#include <SDL_filesystem.h>

#endif

#include "config.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "version.hpp"

static lg::log_domain log_filesystem("filesystem");
#define DBG_FS LOG_STREAM(debug, log_filesystem)
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define WRN_FS LOG_STREAM(warn, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace bfs = boost::filesystem;
using boost::filesystem::path;
using boost::system::error_code;

namespace {
	// These are the filenames that get special processing
	const std::string maincfg_filename = "_main.cfg";
	const std::string finalcfg_filename = "_final.cfg";
	const std::string initialcfg_filename = "_initial.cfg";
}
namespace {
	//only used by windows but put outside the ifdef to let it check by ci build.
	class customcodecvt : public std::codecvt<wchar_t /*intern*/, char /*extern*/, std::mbstate_t>
	{
	private:
		//private static helper things
		template<typename char_t_to>
		struct customcodecvt_do_conversion_writer
		{
			customcodecvt_do_conversion_writer(char_t_to*& _to_next, char_t_to* _to_end) :
				to_next(_to_next),
				to_end(_to_end)
			{}
			char_t_to*& to_next;
			char_t_to* to_end;

			bool can_push(size_t count)
			{
				return static_cast<size_t>(to_end - to_next) > count;
			}

			void push(char_t_to val)
			{
				assert(to_next != to_end);
				*to_next++  = val;
			}
		};

		template<typename char_t_from , typename char_t_to>
		static void customcodecvt_do_conversion( std::mbstate_t& /*state*/,
			const char_t_from* from,
			const char_t_from* from_end,
			const char_t_from*& from_next,
			char_t_to* to,
			char_t_to* to_end,
			char_t_to*& to_next )
		{
			typedef typename ucs4_convert_impl::convert_impl<char_t_from>::type impl_type_from;
			typedef typename ucs4_convert_impl::convert_impl<char_t_to>::type impl_type_to;

			from_next = from;
			to_next = to;
			customcodecvt_do_conversion_writer<char_t_to> writer(to_next, to_end);
			while(from_next != from_end)
			{
				impl_type_to::write(writer, impl_type_from::read(from_next, from_end));
			}
		}

	public:

		//Not used by boost filesystem
		int do_encoding() const NOEXCEPT { return 0; }
		//Not used by boost filesystem
		bool do_always_noconv() const NOEXCEPT { return false; }
		int do_length( std::mbstate_t& /*state*/,
			const char* /*from*/,
			const char* /*from_end*/,
			std::size_t /*max*/ ) const
		{
			//Not used by boost filesystem
			throw "Not supported";
		}

		std::codecvt_base::result unshift( std::mbstate_t& /*state*/,
			char* /*to*/,
			char* /*to_end*/,
			char*& /*to_next*/) const
		{
			//Not used by boost filesystem
			throw "Not supported";
		}

		//there are still some methods which could be implemented but arent because boost filesystem won't use them.
		std::codecvt_base::result do_in( std::mbstate_t& state,
			const char* from,
			const char* from_end,
			const char*& from_next,
			wchar_t* to,
			wchar_t* to_end,
			wchar_t*& to_next ) const
		{
			try
			{
				customcodecvt_do_conversion<char, wchar_t>(state, from, from_end, from_next, to, to_end, to_next);
			}
			catch(...)
			{
				ERR_FS << "Invalid UTF-8 string'" << std::string(from, from_end) << "' " << std::endl;
				return std::codecvt_base::error;
			}
			return std::codecvt_base::ok;
		}

		std::codecvt_base::result do_out( std::mbstate_t& state,
			const wchar_t* from,
			const wchar_t* from_end,
			const wchar_t*& from_next,
			char* to,
			char* to_end,
			char*& to_next ) const
		{
			try
			{
				customcodecvt_do_conversion<wchar_t, char>(state, from, from_end, from_next, to, to_end, to_next);
			}
			catch(...)
			{
				ERR_FS << "Invalid UTF-16 string" << std::endl;
				return std::codecvt_base::error;
			}
			return std::codecvt_base::ok;
		}
	};

#ifdef _WIN32
	class static_runner {
	public:
		static_runner() {
			// Boost uses the current locale to generate a UTF-8 one
			std::locale utf8_loc = boost::locale::generator().generate("");
			// use a custom locale becasue we want to use out log.hpp functions in case of an invalid string.
			utf8_loc = std::locale(utf8_loc, new customcodecvt());
			boost::filesystem::path::imbue(utf8_loc);
		}
	};

	static static_runner static_bfs_path_imbuer;
#endif
}


namespace filesystem {

static void push_if_exists(std::vector<std::string> *vec, const path &file, bool full) {
	if (vec != nullptr) {
		if (full)
			vec->push_back(file.generic_string());
		else
			vec->push_back(file.filename().generic_string());
	}
}

static inline bool error_except_not_found(const error_code &ec)
{
	return (ec
		&& ec.value() != boost::system::errc::no_such_file_or_directory
#ifdef _WIN32
		&& ec.value() != boost::system::windows_error::path_not_found
#endif /*_WIN32*/
		);
}

static bool is_directory_internal(const path &fpath)
{
	error_code ec;
	bool is_dir = bfs::is_directory(fpath, ec);
	if (error_except_not_found(ec)) {
		LOG_FS << "Failed to check if " << fpath.string() << " is a directory: " << ec.message() << '\n';
	}
	return is_dir;
}

static bool file_exists(const path &fpath)
{
	error_code ec;
	bool exists = bfs::exists(fpath, ec);
	if (error_except_not_found(ec)) {
		ERR_FS << "Failed to check existence of file " << fpath.string() << ": " << ec.message() << '\n';
	}
	return exists;
}
static path get_dir(const path &dirpath)
{
	bool is_dir = is_directory_internal(dirpath);
	if (!is_dir) {
		error_code ec;
		bfs::create_directory(dirpath, ec);
		if (ec) {
			ERR_FS << "Failed to create directory " << dirpath.string() << ": " << ec.message() << '\n';
		}
		// This is probably redundant
		is_dir = is_directory_internal(dirpath);
	}
	if (!is_dir) {
		ERR_FS << "Could not open or create directory " << dirpath.string() << '\n';
		return std::string();
	}

	return dirpath;
}
static bool create_directory_if_missing(const path &dirpath)
{
	error_code ec;
	bfs::file_status fs = bfs::status(dirpath, ec);
	if (error_except_not_found(ec)) {
		ERR_FS << "Failed to retrieve file status for " << dirpath.string() << ": " << ec.message() << '\n';
		return false;
	} else if (bfs::is_directory(fs)) {
		DBG_FS << "directory " << dirpath.string() << " exists, not creating\n";
		return true;
	} else if (bfs::exists(fs)) {
		ERR_FS << "cannot create directory " << dirpath.string() << "; file exists\n";
		return false;
	}

	bool created = bfs::create_directory(dirpath, ec);
	if (ec) {
		ERR_FS << "Failed to create directory " << dirpath.string() << ": " << ec.message() << '\n';
	}
	return created;
}
static bool create_directory_if_missing_recursive(const path& dirpath)
{
	DBG_FS << "creating recursive directory: " << dirpath.string() << '\n';

	if (dirpath.empty())
		return false;
	error_code ec;
	bfs::file_status fs = bfs::status(dirpath);
	if (error_except_not_found(ec)) {
		ERR_FS << "Failed to retrieve file status for " << dirpath.string() << ": " << ec.message() << '\n';
		return false;
	} else if (bfs::is_directory(fs)) {
		return true;
	} else if (bfs::exists(fs)) {
		return false;
	}

	if (!dirpath.has_parent_path() || create_directory_if_missing_recursive(dirpath.parent_path())) {
		return create_directory_if_missing(dirpath);
	} else {
		ERR_FS << "Could not create parents to " << dirpath.string() << '\n';
		return false;
	}
}

void get_files_in_dir(const std::string &dir,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs,
                      file_name_option mode,
                      file_filter_option filter,
                      file_reorder_option reorder,
                      file_tree_checksum* checksum) {
	if(path(dir).is_relative() && !game_config::path.empty()) {
		path absolute_dir(game_config::path);
		absolute_dir /= dir;
		if(is_directory_internal(absolute_dir)) {
			get_files_in_dir(absolute_dir.string(), files, dirs, mode, filter, reorder, checksum);
			return;
		}
	}

	const path dirpath(dir);

	if (reorder == DO_REORDER) {
		LOG_FS << "searching for _main.cfg in directory " << dir << '\n';
		const path maincfg = dirpath / maincfg_filename;

		if (file_exists(maincfg)) {
			LOG_FS << "_main.cfg found : " << maincfg << '\n';
			push_if_exists(files, maincfg, mode == ENTIRE_FILE_PATH);
			return;
		}
	}

	error_code ec;
	bfs::directory_iterator di(dirpath, ec);
	bfs::directory_iterator end;
	if (ec) {
		// Probably not a directory, let the caller deal with it.
		return;
	}
	for(; di != end; ++di) {
		bfs::file_status st = di->status(ec);
		if (ec) {
			LOG_FS << "Failed to get file status of " << di->path().string() << ": " << ec.message() << '\n';
			continue;
		}
		if (st.type() == bfs::regular_file) {
			{
				std::string basename = di->path().filename().string();
				if (filter == SKIP_PBL_FILES && looks_like_pbl(basename))
					continue;
				if(!basename.empty() && basename[0] == '.' )
					continue;
			}
			push_if_exists(files, di->path(), mode == ENTIRE_FILE_PATH);

			if (checksum != nullptr) {
				std::time_t mtime = bfs::last_write_time(di->path(), ec);
				if (ec) {
					LOG_FS << "Failed to read modification time of " << di->path().string() << ": " << ec.message() << '\n';
				} else if (mtime > checksum->modified) {
					checksum->modified = mtime;
				}

				uintmax_t size = bfs::file_size(di->path(), ec);
				if (ec) {
					LOG_FS << "Failed to read filesize of " << di->path().string() << ": " << ec.message() << '\n';
				} else {
					checksum->sum_size += size;
				}
				checksum->nfiles++;
			}
		} else if (st.type() == bfs::directory_file) {
			std::string basename = di->path().filename().string();

			if(!basename.empty() && basename[0] == '.' )
				continue;
			if (filter == SKIP_MEDIA_DIR
				&& (basename == "images" || basename == "sounds"))
				continue;

			const path inner_main(di->path() / maincfg_filename);
			bfs::file_status main_st = bfs::status(inner_main, ec);
			if (error_except_not_found(ec)) {
				LOG_FS << "Failed to get file status of " << inner_main.string() << ": " << ec.message() << '\n';
			} else if (reorder == DO_REORDER && main_st.type() == bfs::regular_file) {
				LOG_FS << "_main.cfg found : " << (mode == ENTIRE_FILE_PATH ? inner_main.string() : inner_main.filename().string()) << '\n';
				push_if_exists(files, inner_main, mode == ENTIRE_FILE_PATH);
			} else {
				push_if_exists(dirs, di->path(), mode == ENTIRE_FILE_PATH);
			}
		}
	}

	if (files != nullptr)
		std::sort(files->begin(),files->end());

	if (dirs != nullptr)
		std::sort(dirs->begin(),dirs->end());

	if (files != nullptr && reorder == DO_REORDER) {
		// move finalcfg_filename, if present, to the end of the vector
		for (unsigned int i = 0; i < files->size(); i++) {
			if (ends_with((*files)[i], "/" + finalcfg_filename)) {
				files->push_back((*files)[i]);
				files->erase(files->begin()+i);
				break;
			}
		}
		// move initialcfg_filename, if present, to the beginning of the vector
		int foundit = -1;
		for (unsigned int i = 0; i < files->size(); i++)
			if (ends_with((*files)[i], "/" + initialcfg_filename)) {
				foundit = i;
				break;
			}
		if (foundit > 0) {
			std::string initialcfg = (*files)[foundit];
			for (unsigned int i = foundit; i > 0; i--)
				(*files)[i] = (*files)[i-1];
			(*files)[0] = initialcfg;
		}
	}
}

std::string get_dir(const std::string &dir)
{
	return get_dir(path(dir)).string();
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

static path user_data_dir, user_config_dir, cache_dir;

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

static void setup_user_data_dir()
{
	if (!create_directory_if_missing_recursive(user_data_dir)) {
		ERR_FS << "could not open or create user data directory at " << user_data_dir.string() << '\n';
		return;
	}
	// TODO: this may not print the error message if the directory exists but we don't have the proper permissions

	// Create user data and add-on directories
	create_directory_if_missing(user_data_dir / "editor");
	create_directory_if_missing(user_data_dir / "editor" / "maps");
	create_directory_if_missing(user_data_dir / "editor" / "scenarios");
	create_directory_if_missing(user_data_dir / "data");
	create_directory_if_missing(user_data_dir / "data" / "add-ons");
	create_directory_if_missing(user_data_dir / "saves");
	create_directory_if_missing(user_data_dir / "persist");

#ifdef _WIN32
	lg::finish_log_file_setup();
#endif
}

#ifdef _WIN32
// As a convenience for portable installs on Windows, relative paths with . or
// .. as the first component are considered relative to the current workdir
// instead of Documents/My Games.
static bool is_path_relative_to_cwd(const std::string& str)
{
	const path p(str);

	if(p.empty()) {
		return false;
	}

	return *p.begin() == "." || *p.begin() == "..";
}
#endif

void set_user_data_dir(std::string newprefdir)
{
#ifdef PREFERENCES_DIR
	if (newprefdir.empty()) newprefdir = PREFERENCES_DIR;
#endif

#ifdef _WIN32
	if(newprefdir.size() > 2 && newprefdir[1] == ':') {
		//allow absolute path override
		user_data_dir = newprefdir;
	} else if(is_path_relative_to_cwd(newprefdir)) {
		// Custom directory relative to workdir (for portable installs, etc.)
		user_data_dir = get_cwd() + "/" + newprefdir;
	} else {
		if(newprefdir.empty()) {
			newprefdir = "Wesnoth" + get_version_path_suffix();
		}

		wchar_t docs_path[MAX_PATH];

		HRESULT res = SHGetFolderPathW(nullptr,
									   CSIDL_PERSONAL | CSIDL_FLAG_CREATE, nullptr,
									   SHGFP_TYPE_CURRENT,
									   docs_path);
		if(res != S_OK) {
			//
			// Crummy fallback path full of pain and suffering.
			//
			ERR_FS << "Could not determine path to user's Documents folder! ("
				   << std::hex << "0x" << res << std::dec << ") "
				   << "User config/data directories may be unavailable for "
				   << "this session. Please report this as a bug.\n";
			user_data_dir = path(get_cwd()) / newprefdir;
		} else {
			path games_path = path(docs_path) / "My Games";
			create_directory_if_missing(games_path);

			user_data_dir = games_path / newprefdir;
		}
	}

#else /*_WIN32*/

	std::string backupprefdir = ".wesnoth" + get_version_path_suffix();

#ifdef WESNOTH_BOOST_OS_IOS
	char *sdl_pref_path = SDL_GetPrefPath("wesnoth.org", "iWesnoth");
	if(sdl_pref_path) {
		backupprefdir = std::string(sdl_pref_path) + backupprefdir;
		SDL_free(sdl_pref_path);
	}
#endif

#ifdef _X11
	const char *home_str = getenv("HOME");

	if (newprefdir.empty()) {
		char const *xdg_data = getenv("XDG_DATA_HOME");
		if (!xdg_data || xdg_data[0] == '\0') {
			if (!home_str) {
				newprefdir = backupprefdir;
				goto other;
			}
			user_data_dir = home_str;
			user_data_dir /= ".local/share";
		} else user_data_dir = xdg_data;
		user_data_dir /= "wesnoth";
		user_data_dir /= get_version_path_suffix();
	} else {
		other:
		path home = home_str ? home_str : ".";

		if (newprefdir[0] == '/')
			user_data_dir = newprefdir;
		else
			user_data_dir = home / newprefdir;
	}
#else
	if (newprefdir.empty()) newprefdir = backupprefdir;

	const char* home_str = getenv("HOME");
	path home = home_str ? home_str : ".";

	if (newprefdir[0] == '/')
		user_data_dir = newprefdir;
	else
		user_data_dir = home / newprefdir;
#endif

#endif /*_WIN32*/
	setup_user_data_dir();
	user_data_dir = normalize_path(user_data_dir.string(), true, true);
}

static void set_user_config_path(path newconfig)
{
	user_config_dir = newconfig;
	if (!create_directory_if_missing_recursive(user_config_dir)) {
		ERR_FS << "could not open or create user config directory at " << user_config_dir.string() << '\n';
	}
}

void set_user_config_dir(const std::string& newconfigdir)
{
	set_user_config_path(newconfigdir);
}

static const path &get_user_data_path()
{
	if (user_data_dir.empty())
	{
		set_user_data_dir(std::string());
	}
	return user_data_dir;
}
std::string get_user_config_dir()
{
	if (user_config_dir.empty())
	{
#if defined(_X11) && !defined(PREFERENCES_DIR)
		char const *xdg_config = getenv("XDG_CONFIG_HOME");
		if (!xdg_config || xdg_config[0] == '\0') {
			xdg_config = getenv("HOME");
			if (!xdg_config) {
				user_config_dir = get_user_data_path();
				return user_config_dir.string();
			}
			user_config_dir = xdg_config;
			user_config_dir /= ".config";
		} else user_config_dir = xdg_config;
		user_config_dir /= "wesnoth";
		set_user_config_path(user_config_dir);
#else
		user_config_dir = get_user_data_path();
#endif
	}
	return user_config_dir.string();
}
std::string get_user_data_dir()
{
	return get_user_data_path().string();
}
std::string get_cache_dir()
{
	if (cache_dir.empty())
	{
#if defined(_X11) && !defined(PREFERENCES_DIR)
		char const *xdg_cache = getenv("XDG_CACHE_HOME");
		if (!xdg_cache || xdg_cache[0] == '\0') {
			xdg_cache = getenv("HOME");
			if (!xdg_cache) {
				cache_dir = get_dir(get_user_data_path() / "cache");
				return cache_dir.string();
			}
			cache_dir = xdg_cache;
			cache_dir /= ".cache";
		} else cache_dir = xdg_cache;
		cache_dir /= "wesnoth";
		create_directory_if_missing_recursive(cache_dir);
#else
		cache_dir = get_dir(get_user_data_path() / "cache");
#endif
	}
	return cache_dir.string();
}

std::string get_cwd()
{
	error_code ec;
	path cwd = bfs::current_path(ec);
	if (ec) {
		ERR_FS << "Failed to get current directory: " << ec.message() << '\n';
		return "";
	}
	return cwd.generic_string();
}
std::string get_exe_dir()
{
#ifdef _WIN32
    wchar_t process_path[MAX_PATH];
    SetLastError(ERROR_SUCCESS);
    GetModuleFileNameW(nullptr, process_path, MAX_PATH);
    if (GetLastError() != ERROR_SUCCESS) {
        return get_cwd();
    }

    path exe(process_path);
    return exe.parent_path().string();
#else
    if (bfs::exists("/proc/")) {
        path self_exe("/proc/self/exe");
        error_code ec;
        path exe = bfs::read_symlink(self_exe, ec);
        if (ec) {
            return std::string();
        }

        return exe.parent_path().string();
    } else {
        return get_cwd();
    }
#endif
}

bool make_directory(const std::string& dirname)
{
	error_code ec;
	bool created = bfs::create_directory(path(dirname), ec);
	if (ec) {
		ERR_FS << "Failed to create directory " << dirname << ": " << ec.message() << '\n';
	}
	return created;
}
bool delete_directory(const std::string& dirname, const bool keep_pbl)
{
	bool ret = true;
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	error_code ec;

	get_files_in_dir(dirname, &files, &dirs, ENTIRE_FILE_PATH, keep_pbl ? SKIP_PBL_FILES : NO_FILTER);

	if(!files.empty()) {
		for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
			bfs::remove(path(*i), ec);
			if (ec) {
				LOG_FS << "remove(" << (*i) << "): " << ec.message() << '\n';
				ret = false;
			}
		}
	}

	if(!dirs.empty()) {
		for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
			//TODO: this does not preserve any other PBL files
			// filesystem.cpp does this too, so this might be intentional
			if(!delete_directory(*j))
				ret = false;
		}
	}

	if (ret) {
		bfs::remove(path(dirname), ec);
		if (ec) {
			LOG_FS << "remove(" << dirname << "): " << ec.message() << '\n';
			ret = false;
		}
	}
	return ret;
}

bool delete_file(const std::string &filename)
{
	error_code ec;
	bool ret = bfs::remove(path(filename), ec);
	if (ec) {
		ERR_FS << "Could not delete file " << filename << ": " << ec.message() << '\n';
	}
	return ret;
}

std::string read_file(const std::string &fname)
{
	scoped_istream is = istream_file(fname);
	std::stringstream ss;
	ss << is->rdbuf();
	return ss.str();
}

filesystem::scoped_istream istream_file(const std::string &fname, bool treat_failure_as_error)
{
	LOG_FS << "Streaming " << fname << " for reading.\n";
	if (fname.empty()) {
		ERR_FS << "Trying to open file with empty name.\n";
		filesystem::scoped_istream s(new bfs::ifstream());
		s->clear(std::ios_base::failbit);
		return s;
	}

	//mingw doesn't  support std::basic_ifstream::basic_ifstream(const wchar_t* fname)
	//that why boost::filesystem::fstream.hpp doesnt work with mingw.
	try
	{
		boost::iostreams::file_descriptor_source fd(bfs::path(fname), std::ios_base::binary);
		//TODO: has this still use ?
		if (!fd.is_open() && treat_failure_as_error) {
			ERR_FS << "Could not open '" << fname << "' for reading.\n";
		}
		return filesystem::scoped_istream(new boost::iostreams::stream<boost::iostreams::file_descriptor_source>(fd, 4096, 0));
	}
	catch(const std::exception ex)
	{
		if(treat_failure_as_error)
		{
			ERR_FS << "Could not open '" << fname << "' for reading.\n";
		}
		filesystem::scoped_istream s(new bfs::ifstream());
		s->clear(std::ios_base::failbit);
		return s;
	}
}

filesystem::scoped_ostream ostream_file(const std::string& fname, bool create_directory)
{
	LOG_FS << "streaming " << fname << " for writing.\n";
#if 1
	try
	{
		boost::iostreams::file_descriptor_sink fd(bfs::path(fname), std::ios_base::binary);
		return filesystem::scoped_ostream(new boost::iostreams::stream<boost::iostreams::file_descriptor_sink>(fd, 4096, 0));
	}
	catch(BOOST_IOSTREAMS_FAILURE& e)
	{
		// If this operation failed because the parent directoy didn't exist, create the parent directoy and retry.
		error_code ec_unused;
		if(create_directory && bfs::create_directories(bfs::path(fname).parent_path(), ec_unused))
		{
			return ostream_file(fname, false);
		}
		throw filesystem::io_exception(e.what());
	}
#else
	return new bfs::ofstream(path(fname), std::ios_base::binary);
#endif
}
// Throws io_exception if an error occurs
void write_file(const std::string& fname, const std::string& data)
{
	scoped_ostream os = ostream_file(fname);
	os->exceptions(std::ios_base::goodbit);

	const size_t block_size = 4096;
	char buf[block_size];

	for(size_t i = 0; i < data.size(); i += block_size) {
		const size_t bytes = std::min<size_t>(block_size,data.size() - i);
		std::copy(data.begin() + i, data.begin() + i + bytes,buf);

		os->write(buf, bytes);
		if (os->bad())
			throw io_exception("Error writing to file: '" + fname + "'");
	}
}

bool create_directory_if_missing(const std::string& dirname)
{
	return create_directory_if_missing(path(dirname));
}
bool create_directory_if_missing_recursive(const std::string& dirname)
{
	return create_directory_if_missing_recursive(path(dirname));
}

bool is_directory(const std::string& fname)
{
	return is_directory_internal(path(fname));
}

bool file_exists(const std::string& name)
{
	return file_exists(path(name));
}

time_t file_modified_time(const std::string& fname)
{
	error_code ec;
	std::time_t mtime = bfs::last_write_time(path(fname), ec);
	if (ec) {
		LOG_FS << "Failed to read modification time of " << fname << ": " << ec.message() << '\n';
	}
	return mtime;
}

bool is_gzip_file(const std::string& filename)
{
	return path(filename).extension() == ".gz";
}

bool is_bzip2_file(const std::string& filename)
{
	return path(filename).extension() == ".bz2";
}

int file_size(const std::string& fname)
{
	error_code ec;
	uintmax_t size = bfs::file_size(path(fname), ec);
	if (ec) {
		LOG_FS << "Failed to read filesize of " << fname << ": " << ec.message() << '\n';
		return -1;
	} else if (size > INT_MAX)
		return INT_MAX;
	else
		return size;
}

int dir_size(const std::string& pname)
{
	bfs::path p(pname);
	uintmax_t size_sum = 0;
	error_code ec;
	for ( bfs::recursive_directory_iterator i(p), end; i != end && !ec; ++i ) {
		if(bfs::is_regular_file(i->path())) {
			size_sum += bfs::file_size(i->path(), ec);
		}
	}
	if (ec) {
		LOG_FS << "Failed to read directorysize of " << pname << ": " << ec.message() << '\n';
		return -1;
	}
	else if (size_sum > INT_MAX)
		return INT_MAX;
	else
		return size_sum;
}

std::string base_name(const std::string& file)
{
	return path(file).filename().string();
}

std::string directory_name(const std::string& file)
{
	return path(file).parent_path().string();
}

std::string nearest_extant_parent(const std::string& file)
{
	if(file.empty()) {
		return "";
	}

	bfs::path p{file};
	error_code ec;

	do {
		p = p.parent_path();
		bfs::path q = canonical(p, ec);
		if (!ec) {
			p = q;
		}
	} while(ec && !is_root(p.string()));

	return ec ? "" : p.string();
}

bool is_path_sep(char c)
{
	static const path sep = path("/").make_preferred();
	const std::string s = std::string(1, c);
	return sep == path(s).make_preferred();
}

char path_separator()
{
	return path::preferred_separator;
}

bool is_root(const std::string& path)
{
#ifndef _WIN32
	error_code ec;
	const bfs::path& p = bfs::canonical(path, ec);
	return ec ? false : !p.has_parent_path();
#else
	//
	// Boost.Filesystem is completely unreliable when it comes to detecting
	// whether a path refers to a drive's root directory on Windows, so we are
	// forced to take an alternative approach here. Instead of hand-parsing
	// strings we'll just call a graphical shell service.
	//
	// There are several poorly-documented ways to refer to a drive in Windows by
	// escaping the filesystem namespace using \\.\, \\?\, and \??\. We're just
	// going to ignore those here, which may yield unexpected results in places
	// such as the file dialog. This function really shouldn't be used for
	// security validation anyway, and there are virtually infinite ways to name
	// a drive's root using the NT object namespace so it's pretty pointless to
	// try to catch those there.
	//
	// (And no, shlwapi.dll's PathIsRoot() doesn't recognize \\.\C:\, \\?\C:\, or
	// \??\C:\ as roots either.)
	//
	// More generally, do NOT use this code in security-sensitive applications.
	//
	// See also: <https://googleprojectzero.blogspot.com/2016/02/the-definitive-guide-on-win32-to-nt.html>
	//
	const std::wstring& wpath = bfs::path{path}.make_preferred().wstring();
	return PathIsRootW(wpath.c_str()) == TRUE;
#endif
}

std::string root_name(const std::string& path)
{
	return bfs::path{path}.root_name().string();
}

bool is_relative(const std::string& path)
{
	return bfs::path{path}.is_relative();
}

std::string normalize_path(const std::string& fpath, bool normalize_separators, bool resolve_dot_entries)
{
	if(fpath.empty()) {
		return fpath;
	}

	error_code ec;
	bfs::path p = resolve_dot_entries
			? bfs::canonical(fpath, ec)
			: bfs::absolute(fpath);

	if(ec) {
		return "";
	}

	if(normalize_separators) {
		return p.make_preferred().string();
	} else {
		return p.string();
	}
}

/**
 *  The paths manager is responsible for recording the various paths
 *  that binary files may be located at.
 *  It should be passed a config object which holds binary path information.
 *  This is in the format
 *@verbatim
 *    [binary_path]
 *      path=<path>
 *    [/binary_path]
 *  Binaries will be searched for in [wesnoth-path]/data/<path>/images/
 *@endverbatim
 */
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

	for (const config &bp : cfg.child_range("binary_path"))
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

static bool is_legal_file(const std::string &filename)
{
	DBG_FS << "Looking for '" << filename << "'.\n";

	if (filename.empty()) {
		LOG_FS << "  invalid filename\n";
		return false;
	}

	if (filename.find("..") != std::string::npos) {
		ERR_FS << "Illegal path '" << filename << "' (\"..\" not allowed).\n";
		return false;
	}

	if (looks_like_pbl(filename)) {
		ERR_FS << "Illegal path '" << filename << "' (.pbl files are not allowed)." << std::endl;
		return false;
	}

	return true;
}

/**
 * Returns a vector with all possible paths to a given type of binary,
 * e.g. 'images', 'sounds', etc,
 */
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

	for(const std::string &path : binary_paths)
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
	// We define ".." as "remove everything before" this is needed becasue
	// on the one hand allowing ".." would be a security risk but
	// especialy for terrains the c++ engine puts a hardcoded "terrain/" before filename
	// and there would be no way to "escape" from "terrain/" otherwise. This is not the
	// best solution but we cannot remove it without another solution (subtypes maybe?).

	{
		std::string::size_type pos = filename.rfind("../");
		if(pos != std::string::npos) {
			return get_binary_file_location(type, filename.substr(pos + 3));
		}
	}

	if (!is_legal_file(filename))
		return std::string();

	for(const std::string &bp : get_binary_paths(type))
	{
		path bpath(bp);
		bpath /= filename;
		DBG_FS << "  checking '" << bp << "'\n";
		if (file_exists(bpath)) {
			DBG_FS << "  found at '" << bpath.string() << "'\n";
			return bpath.string();
		}
	}

	DBG_FS << "  not found\n";
	return std::string();
}

std::string get_binary_dir_location(const std::string &type, const std::string &filename)
{
	if (!is_legal_file(filename))
		return std::string();

	for (const std::string &bp : get_binary_paths(type))
	{
		path bpath(bp);
		bpath /= filename;
		DBG_FS << "  checking '" << bp << "'\n";
		if (is_directory_internal(bpath)) {
			DBG_FS << "  found at '" << bpath.string() << "'\n";
			return bpath.string();
		}
	}

	DBG_FS << "  not found\n";
	return std::string();
}

std::string get_wml_location(const std::string &filename, const std::string &current_dir)
{
	if (!is_legal_file(filename))
		return std::string();

	assert(game_config::path.empty() == false);

	path fpath(filename);
	path result;

	if (filename[0] == '~')
	{
		result /= get_user_data_path() / "data" / filename.substr(1);
		DBG_FS << "  trying '" << result.string() << "'\n";
	} else if (*fpath.begin() == ".") {
		if (!current_dir.empty()) {
			result /= path(current_dir);
		} else {
			result /= path(game_config::path) / "data";
		}

		result /= filename;
	} else if (!game_config::path.empty()) {
		result /= path(game_config::path) / "data" / filename;
	}
	if (result.empty() || !file_exists(result)) {
		DBG_FS << "  not found\n";
		result.clear();
	} else
		DBG_FS << "  found: '" << result.string() << "'\n";

	return result.string();
}

static path subtract_path(const path &full, const path &prefix)
{
	path::iterator fi = full.begin()
	             , fe = full.end()
	             , pi = prefix.begin()
	             , pe = prefix.end();
	while (fi != fe && pi != pe && *fi == *pi) {
		++fi;
		++pi;
	}
	path rest;
	if (pi == pe)
		while (fi != fe) {
			rest /= *fi;
			++fi;
		}
	return rest;
}

std::string get_short_wml_path(const std::string &filename)
{
	path full_path(filename);

	path partial = subtract_path(full_path, get_user_data_path() / "data");
	if (!partial.empty())
		return "~" + partial.generic_string();

	partial = subtract_path(full_path, path(game_config::path) / "data");
	if (!partial.empty())
		return partial.generic_string();

	return filename;
}

std::string get_independent_image_path(const std::string &filename)
{
	path full_path(get_binary_file_location("images", filename));

	if (full_path.empty())
		return full_path.string();

	path partial = subtract_path(full_path, get_user_data_path());
	if (!partial.empty())
		return partial.string();

	partial = subtract_path(full_path, game_config::path);
	if (!partial.empty())
		return partial.string();

	return full_path.string();
}

std::string get_program_invocation(const std::string &program_name)
{
	const std::string real_program_name(program_name
#ifdef DEBUG
		+ "-debug"
#endif
#ifdef _WIN32
		+ ".exe"
#endif
	);
	return (path(game_config::wesnoth_program_dir) / real_program_name).string();
}

}
