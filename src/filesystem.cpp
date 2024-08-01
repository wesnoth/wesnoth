/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "filesystem.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/base64.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "utils/general.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/process.hpp>
#include "game_config_view.hpp"

#ifdef _WIN32
#include <boost/locale.hpp>

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

// Work around TDM-GCC not #defining this according to @newfrenchy83.
#ifndef VOLUME_NAME_NONE
#define VOLUME_NAME_NONE 0x4
#endif

#endif /* !_WIN32 */

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <set>

// Copied from boost::predef, as it's there only since 1.55.
#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)

#define WESNOTH_BOOST_OS_IOS (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#include <SDL2/SDL_filesystem.h>

#endif


static lg::log_domain log_filesystem("filesystem");
#define DBG_FS LOG_STREAM(debug, log_filesystem)
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define WRN_FS LOG_STREAM(warn, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace bp = boost::process;
namespace bfs = boost::filesystem;
using boost::system::error_code;

namespace game_config
{
//
// Path info
//
#ifdef WESNOTH_PATH
std::string path = WESNOTH_PATH;
#else
std::string path = "";
#endif

#ifdef DEFAULT_PREFS_PATH
std::string default_preferences_path = DEFAULT_PREFS_PATH;
#else
std::string default_preferences_path = "";
#endif
bool check_migration = false;

const std::string observer_team_name = "observer";

int cache_compression_level = 6;
}

namespace
{
// These are the filenames that get special processing
const std::string maincfg_filename = "_main.cfg";
const std::string finalcfg_filename = "_final.cfg";
const std::string initialcfg_filename = "_initial.cfg";

// only used by windows but put outside the ifdef to let it check by ci build.
class customcodecvt : public std::codecvt<wchar_t /*intern*/, char /*extern*/, std::mbstate_t>
{
private:
	// private static helper things
	template<typename char_t_to>
	struct customcodecvt_do_conversion_writer
	{
		customcodecvt_do_conversion_writer(char_t_to*& _to_next, char_t_to* _to_end)
			: to_next(_to_next)
			, to_end(_to_end)
		{
		}

		char_t_to*& to_next;
		char_t_to* to_end;

		bool can_push(std::size_t count) const
		{
			return static_cast<std::size_t>(to_end - to_next) > count;
		}

		void push(char_t_to val)
		{
			assert(to_next != to_end);
			*to_next++ = val;
		}
	};

	template<typename char_t_from, typename char_t_to>
	static void customcodecvt_do_conversion(std::mbstate_t& /*state*/,
			const char_t_from* from,
			const char_t_from* from_end,
			const char_t_from*& from_next,
			char_t_to* to,
			char_t_to* to_end,
			char_t_to*& to_next)
	{
		typedef typename ucs4_convert_impl::convert_impl<char_t_from>::type impl_type_from;
		typedef typename ucs4_convert_impl::convert_impl<char_t_to>::type impl_type_to;

		from_next = from;
		to_next = to;
		customcodecvt_do_conversion_writer<char_t_to> writer(to_next, to_end);

		while(from_next != from_end) {
			impl_type_to::write(writer, impl_type_from::read(from_next, from_end));
		}
	}

public:
	// Not used by boost filesystem
	int do_encoding() const noexcept
	{
		return 0;
	}

	// Not used by boost filesystem
	bool do_always_noconv() const noexcept
	{
		return false;
	}

	int do_length(std::mbstate_t& /*state*/, const char* /*from*/, const char* /*from_end*/, std::size_t /*max*/) const
	{
		// Not used by boost filesystem
		throw "Not supported";
	}

	std::codecvt_base::result unshift(
			std::mbstate_t& /*state*/, char* /*to*/, char* /*to_end*/, char*& /*to_next*/) const
	{
		// Not used by boost filesystem
		throw "Not supported";
	}

	// there are still some methods which could be implemented but aren't because boost filesystem won't use them.
	std::codecvt_base::result do_in(std::mbstate_t& state,
			const char* from,
			const char* from_end,
			const char*& from_next,
			wchar_t* to,
			wchar_t* to_end,
			wchar_t*& to_next) const
	{
		try {
			customcodecvt_do_conversion<char, wchar_t>(state, from, from_end, from_next, to, to_end, to_next);
		} catch(...) {
			ERR_FS << "Invalid UTF-8 string'" << std::string(from, from_end) << "' with exception: " << utils::get_unknown_exception_type();
			return std::codecvt_base::error;
		}

		return std::codecvt_base::ok;
	}

	std::codecvt_base::result do_out(std::mbstate_t& state,
			const wchar_t* from,
			const wchar_t* from_end,
			const wchar_t*& from_next,
			char* to,
			char* to_end,
			char*& to_next) const
	{
		try {
			customcodecvt_do_conversion<wchar_t, char>(state, from, from_end, from_next, to, to_end, to_next);
		} catch(...) {
			ERR_FS << "Invalid UTF-16 string with exception: " << utils::get_unknown_exception_type();
			return std::codecvt_base::error;
		}

		return std::codecvt_base::ok;
	}
};

#ifdef _WIN32
class static_runner
{
public:
	static_runner()
	{
		// Boost uses the current locale to generate a UTF-8 one
		std::locale utf8_loc = boost::locale::generator().generate("");

		// use a custom locale because we want to use out log.hpp functions in case of an invalid string.
		utf8_loc = std::locale(utf8_loc, new customcodecvt());

		boost::filesystem::path::imbue(utf8_loc);
	}
};

static static_runner static_bfs_path_imbuer;

bool is_filename_case_correct(const std::string& fname, const boost::iostreams::file_descriptor_source& fd)
{
	wchar_t real_path[MAX_PATH];
	GetFinalPathNameByHandleW(fd.handle(), real_path, MAX_PATH - 1, VOLUME_NAME_NONE);

	std::string real_name = filesystem::base_name(unicode_cast<std::string>(std::wstring(real_path)));
	return real_name == filesystem::base_name(fname);
}

#else
bool is_filename_case_correct(const std::string& /*fname*/, const boost::iostreams::file_descriptor_source& /*fd*/)
{
	return true;
}
#endif
} // namespace

namespace filesystem
{

const blacklist_pattern_list default_blacklist{
	{
		/* Blacklist dot-files/dirs, which are hidden files in UNIX platforms */
		".+",
		"#*#",
		"*~",
		"*-bak",
		"*.swp",
		"*.pbl",
		"*.ign",
		"_info.cfg",
		"*.exe",
		"*.bat",
		"*.cmd",
		"*.com",
		"*.scr",
		"*.sh",
		"*.js",
		"*.vbs",
		"*.o",
		"*.ini",
		/* Remove junk created by certain file manager ;) */
		"Thumbs.db",
		/* Eclipse plugin */
		"*.wesnoth",
		"*.project",
	},
	{
		".+",
		/* macOS metadata-like cruft (http://floatingsun.net/2007/02/07/whats-with-__macosx-in-zip-files/) */
		"__MACOSX",
	}
};

static void push_if_exists(std::vector<std::string>* vec, const bfs::path& file, bool full)
{
	if(vec != nullptr) {
		if(full) {
			vec->push_back(file.generic_string());
		} else {
			vec->push_back(file.filename().generic_string());
		}
	}
}

static inline bool error_except_not_found(const error_code& ec)
{
	return ec && ec != boost::system::errc::no_such_file_or_directory;
}

static bool is_directory_internal(const bfs::path& fpath)
{
	error_code ec;
	bool is_dir = bfs::is_directory(fpath, ec);
	if(error_except_not_found(ec)) {
		LOG_FS << "Failed to check if " << fpath.string() << " is a directory: " << ec.message();
	}

	return is_dir;
}

static bool file_exists(const bfs::path& fpath)
{
	error_code ec;
	bool exists = bfs::exists(fpath, ec);
	if(error_except_not_found(ec)) {
		ERR_FS << "Failed to check existence of file " << fpath.string() << ": " << ec.message();
	}

	return exists;
}

static bfs::path get_dir(const bfs::path& dirpath)
{
	bool is_dir = is_directory_internal(dirpath);
	if(!is_dir) {
		error_code ec;
		bfs::create_directory(dirpath, ec);

		if(ec) {
			ERR_FS << "Failed to create directory " << dirpath.string() << ": " << ec.message();
		}

		// This is probably redundant
		is_dir = is_directory_internal(dirpath);
	}

	if(!is_dir) {
		ERR_FS << "Could not open or create directory " << dirpath.string();
		return std::string();
	}

	return dirpath;
}

static bool create_directory_if_missing(const bfs::path& dirpath)
{
	error_code ec;
	bfs::file_status fs = bfs::status(dirpath, ec);

	if(error_except_not_found(ec)) {
		ERR_FS << "Failed to retrieve file status for " << dirpath.string() << ": " << ec.message();
		return false;
	} else if(bfs::is_directory(fs)) {
		DBG_FS << "directory " << dirpath.string() << " exists, not creating";
		return true;
	} else if(bfs::exists(fs)) {
		ERR_FS << "cannot create directory " << dirpath.string() << "; file exists";
		return false;
	}

	bool created = bfs::create_directory(dirpath, ec);
	if(ec) {
		ERR_FS << "Failed to create directory " << dirpath.string() << ": " << ec.message();
	}

	return created;
}

static bool create_directory_if_missing_recursive(const bfs::path& dirpath)
{
	DBG_FS << "creating recursive directory: " << dirpath.string();

	if(dirpath.empty()) {
		return false;
	}

	error_code ec;
	bfs::file_status fs = bfs::status(dirpath);

	if(error_except_not_found(ec)) {
		ERR_FS << "Failed to retrieve file status for " << dirpath.string() << ": " << ec.message();
		return false;
	} else if(bfs::is_directory(fs)) {
		return true;
	} else if(bfs::exists(fs)) {
		return false;
	}

	if(!dirpath.has_parent_path() || create_directory_if_missing_recursive(dirpath.parent_path())) {
		return create_directory_if_missing(dirpath);
	} else {
		ERR_FS << "Could not create parents to " << dirpath.string();
		return false;
	}
}

void get_files_in_dir(const std::string& dir,
		std::vector<std::string>* files,
		std::vector<std::string>* dirs,
		name_mode mode,
		filter_mode filter,
		reorder_mode reorder,
		file_tree_checksum* checksum)
{
	if(bfs::path(dir).is_relative() && !game_config::path.empty()) {
		bfs::path absolute_dir(game_config::path);
		absolute_dir /= dir;

		if(is_directory_internal(absolute_dir)) {
			get_files_in_dir(absolute_dir.string(), files, dirs, mode, filter, reorder, checksum);
			return;
		}
	}

	const bfs::path dirpath(dir);

	if(reorder == reorder_mode::DO_REORDER) {
		LOG_FS << "searching for _main.cfg in directory " << dir;
		const bfs::path maincfg = dirpath / maincfg_filename;

		if(file_exists(maincfg)) {
			LOG_FS << "_main.cfg found : " << maincfg;
			push_if_exists(files, maincfg, mode == name_mode::ENTIRE_FILE_PATH);
			return;
		}
	}

	error_code ec;
	bfs::directory_iterator di(dirpath, ec);
	bfs::directory_iterator end;

	// Probably not a directory, let the caller deal with it.
	if(ec) {
		return;
	}

	for(; di != end; ++di) {
		ec.clear();
		bfs::file_status st = di->status(ec);
		if(ec) {
			LOG_FS << "Failed to get file status of " << di->path().string() << ": " << ec.message();
			continue;
		}

		if(st.type() == bfs::regular_file) {
			{
				std::string basename = di->path().filename().string();
				if(filter == filter_mode::SKIP_PBL_FILES && looks_like_pbl(basename))
					continue;
				if(!basename.empty() && basename[0] == '.')
					continue;
			}

			push_if_exists(files, di->path(), mode == name_mode::ENTIRE_FILE_PATH);

			if(checksum != nullptr) {
				std::time_t mtime = bfs::last_write_time(di->path(), ec);
				if(ec) {
					LOG_FS << "Failed to read modification time of " << di->path().string() << ": " << ec.message();
				} else if(mtime > checksum->modified) {
					checksum->modified = mtime;
				}

				uintmax_t size = bfs::file_size(di->path(), ec);
				if(ec) {
					LOG_FS << "Failed to read filesize of " << di->path().string() << ": " << ec.message();
				} else {
					checksum->sum_size += size;
				}

				checksum->nfiles++;
			}
		} else if(st.type() == bfs::directory_file) {
			std::string basename = di->path().filename().string();

			if(!basename.empty() && basename[0] == '.') {
				continue;
			}

			if(filter == filter_mode::SKIP_MEDIA_DIR && (basename == "images" || basename == "sounds")) {
				continue;
			}

			const bfs::path inner_main(di->path() / maincfg_filename);
			bfs::file_status main_st = bfs::status(inner_main, ec);

			if(error_except_not_found(ec)) {
				LOG_FS << "Failed to get file status of " << inner_main.string() << ": " << ec.message();
			} else if(reorder == reorder_mode::DO_REORDER && main_st.type() == bfs::regular_file) {
				LOG_FS << "_main.cfg found : "
					   << (mode == name_mode::ENTIRE_FILE_PATH ? inner_main.string() : inner_main.filename().string());
				push_if_exists(files, inner_main, mode == name_mode::ENTIRE_FILE_PATH);
			} else {
				push_if_exists(dirs, di->path(), mode == name_mode::ENTIRE_FILE_PATH);
			}
		}
	}

	if(files != nullptr) {
		std::sort(files->begin(), files->end());
	}

	if(dirs != nullptr) {
		std::sort(dirs->begin(), dirs->end());
	}

	if(files != nullptr && reorder == reorder_mode::DO_REORDER) {
		// move finalcfg_filename, if present, to the end of the vector
		for(unsigned int i = 0; i < files->size(); i++) {
			if(boost::algorithm::ends_with((*files)[i], "/" + finalcfg_filename)) {
				files->push_back((*files)[i]);
				files->erase(files->begin() + i);
				break;
			}
		}

		// move initialcfg_filename, if present, to the beginning of the vector
		int foundit = -1;
		for(unsigned int i = 0; i < files->size(); i++)
			if(boost::algorithm::ends_with((*files)[i], "/" + initialcfg_filename)) {
				foundit = i;
				break;
			}
		if(foundit > 0) {
			std::string initialcfg = (*files)[foundit];
			for(unsigned int i = foundit; i > 0; i--)
				(*files)[i] = (*files)[i - 1];
			(*files)[0] = initialcfg;
		}
	}
}

std::string get_dir(const std::string& dir)
{
	return get_dir(bfs::path(dir)).string();
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

static bfs::path user_data_dir, cache_dir;

bool is_userdata_initialized()
{
	return !user_data_dir.string().empty();
}

const std::string get_version_path_suffix(const version_info& version)
{
	std::ostringstream s;
	s << version.major_version() << '.' << version.minor_version();
	return s.str();
}

const std::string& get_version_path_suffix()
{
	static std::string suffix;

	// We only really need to generate this once since
	// the version number cannot change during runtime.

	if(suffix.empty()) {
		suffix = get_version_path_suffix(game_config::wesnoth_version);
	}

	return suffix;
}

#if defined(__APPLE__) && !defined(__IPHONEOS__)
	// Starting from Wesnoth 1.14.6, we have to use sandboxing function on macOS
	// The problem is, that only signed builds can use sandbox. Unsigned builds
	// would use other config directory then signed ones. So if we don't want
	// to have two separate config dirs, we have to create symlink to new config
	// location if exists. This part of code is only required on macOS.
	static void migrate_apple_config_directory_for_unsandboxed_builds()
	{
		const char* home_str = getenv("HOME");
		bfs::path home = home_str ? home_str : ".";

		// We don't know which of the two is in PREFERENCES_DIR now.
		boost::filesystem::path old_saves_dir = home / "Library/Application Support/Wesnoth_";
		old_saves_dir += get_version_path_suffix();
		boost::filesystem::path new_saves_dir = home / "Library/Containers/org.wesnoth.Wesnoth/Data/Library/Application Support/Wesnoth_";
		new_saves_dir += get_version_path_suffix();

		if(bfs::is_directory(new_saves_dir)) {
			if(!bfs::exists(old_saves_dir)) {
				LOG_FS << "Apple developer's userdata migration: symlinking " << old_saves_dir.string() << " to " << new_saves_dir.string();
				bfs::create_symlink(new_saves_dir, old_saves_dir);
			} else if(!bfs::is_symlink(old_saves_dir)) {
				ERR_FS << "Apple developer's userdata migration: Problem! Old (non-containerized) directory " << old_saves_dir.string() << " is not a symlink. Your savegames are scattered around 2 locations.";
			}
			return;
		}
	}
#endif


static void setup_user_data_dir()
{
#if defined(__APPLE__) && !defined(__IPHONEOS__)
	migrate_apple_config_directory_for_unsandboxed_builds();
#endif
	if(!file_exists(user_data_dir / "logs")) {
		game_config::check_migration = true;
	}

	if(!create_directory_if_missing_recursive(user_data_dir)) {
		ERR_FS << "could not open or create user data directory at " << user_data_dir.string();
		return;
	}
	// TODO: this may not print the error message if the directory exists but we don't have the proper permissions

	// Create user data and add-on directories
	create_directory_if_missing(get_sync_dir());
	create_directory_if_missing(get_legacy_editor_dir());
	create_directory_if_missing(get_legacy_editor_dir() + "/maps");
	create_directory_if_missing(get_legacy_editor_dir() + "/scenarios");
	create_directory_if_missing(get_addons_data_dir());
	create_directory_if_missing(get_addons_dir());
	create_directory_if_missing(get_saves_dir());
	create_directory_if_missing(get_wml_persist_dir());
	create_directory_if_missing(get_logs_dir());

	if(file_exists(get_unsynced_prefs_file()) && !file_exists(get_synced_prefs_file())) {
		copy_file(get_unsynced_prefs_file(), get_synced_prefs_file());
	}

	lg::move_log_file();
}

#ifdef _WIN32
/**
 * @return the path to the My Games directory on success or an empty string on failure
 */
static utils::optional<std::string> get_games_path()
{
	PWSTR docs_path = nullptr;
	HRESULT res = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &docs_path);
	utils::optional<std::string> path = utils::nullopt;

	if(res == S_OK) {
		bfs::path games_path = bfs::path(docs_path) / "My Games";
		path = games_path.string();
	} else {
		ERR_FS << "Could not determine path to user's Documents folder! (" << std::hex << "0x" << res << std::dec << ") "
				<< "Please report this as a bug.";
	}

	CoTaskMemFree(docs_path);
	return path;
}
#endif

void set_user_data_dir(std::string newprefdir)
{
#ifdef PREFERENCES_DIR
	if(newprefdir.empty()) {
		newprefdir = PREFERENCES_DIR;
		DBG_FS << "Using PREFERENCES_DIR '" << PREFERENCES_DIR << "'";
	}
#endif

	// if no custom userdata directory was provided, use appropriate default
	// next replace ~ with Documents/My Games on windows and $HOME otherwise
	if(newprefdir.empty()) {
#ifdef _WIN32
		newprefdir = "~/Wesnoth" + get_version_path_suffix();
#elif defined(__APPLE__)
		newprefdir = "~/Library/Application Support/Wesnoth_"+get_version_path_suffix();
#elif defined(WESNOTH_BOOST_OS_IOS)
		char* sdl_pref_path = SDL_GetPrefPath("wesnoth.org", "iWesnoth");
		if(sdl_pref_path) {
			newprefdir = std::string(sdl_pref_path);
			SDL_free(sdl_pref_path);
		} else {
			newprefdir = "~/.wesnoth" + get_version_path_suffix();
		}
#else
		const char* h = std::getenv("HOME");
		std::string home = h ? h : "";
		h = std::getenv("XDG_DATA_HOME");
		std::string xdg_data_home = h ? h : "";
		if (!xdg_data_home.empty()) {
			newprefdir = xdg_data_home + "/wesnoth/" + get_version_path_suffix();
		} else if (!home.empty()) {
			newprefdir = home + "/.local/share/wesnoth/" + get_version_path_suffix();
		} else {
			newprefdir = ".wesnoth" + get_version_path_suffix();
		}
#endif
	}

	bfs::path dir;
	if(newprefdir[0] == '~') {
#ifdef _WIN32
		utils::optional<std::string> games_path = get_games_path();
		if(games_path) {
			create_directory_if_missing(*games_path);
			dir = *games_path;
		} else {
			dir = get_cwd();
			WRN_FS << "Using current directory instead: " << dir.string();
		}
#else
		const char* h = std::getenv("HOME");
		std::string home = h ? h : "";
		if(!home.empty()) {
			dir = home;
		} else {
			dir = get_cwd();
			ERR_FS << "Unable to determine path to user's HOME.";
			WRN_FS << "Using current directory instead: " << dir.string();
		}
#endif
		dir /= newprefdir.substr(1);
	} else {
		dir = newprefdir;
	}
	user_data_dir = dir;
	DBG_FS << "userdata dir set to: " << user_data_dir.string();

	setup_user_data_dir();
	// normalize_path expects the path to exist so calling it after potentially creating it in setup_user_data_dir
	dir = normalize_path(user_data_dir.string(), true, true);
	if(!dir.empty()) {
		user_data_dir = dir;
	}
}

bool rename_dir(const std::string& old_dir, const std::string& new_dir)
{
	error_code ec;
	bfs::rename(old_dir, new_dir, ec);

	if(ec) {
		ERR_FS << "Failed to rename directory '" << old_dir << "' to '" << new_dir << "'";
		return false;
	}
	return true;
}

static void set_cache_path(bfs::path newcache)
{
	cache_dir = newcache;
	if(!create_directory_if_missing_recursive(cache_dir)) {
		ERR_FS << "could not open or create cache directory at " << cache_dir.string() << '\n';
	}
}

void set_cache_dir(const std::string& newcachedir)
{
	set_cache_path(newcachedir);
}

static const bfs::path& get_user_data_path()
{
	assert(!user_data_dir.empty() && "Attempted to access userdata location before userdata initialization!");
	return user_data_dir;
}

std::string get_user_data_dir()
{
	return get_user_data_path().string();
}

std::string get_logs_dir()
{
	return filesystem::get_user_data_dir() + "/logs";
}

std::string get_cache_dir()
{
	if(cache_dir.empty()) {
#if defined(_X11) && !defined(PREFERENCES_DIR)
		char const* xdg_cache = getenv("XDG_CACHE_HOME");

		if(!xdg_cache || xdg_cache[0] == '\0') {
			xdg_cache = getenv("HOME");
			if(!xdg_cache) {
				cache_dir = get_dir(get_user_data_path() / "cache");
				return cache_dir.string();
			}

			cache_dir = xdg_cache;
			cache_dir /= ".cache";
		} else {
			cache_dir = xdg_cache;
		}

		cache_dir /= "wesnoth";
		create_directory_if_missing_recursive(cache_dir);
#else
		cache_dir = get_dir(get_user_data_path() / "cache");
#endif
	}

	return cache_dir.string();
}

std::vector<other_version_dir> find_other_version_saves_dirs()
{
#if !defined(_WIN32) && !defined(_X11) && !defined(__APPLE__)
	// By all means, this situation doesn't make sense
	return {};
#else
	const auto& w_ver = game_config::wesnoth_version;
	const auto& ms_ver = game_config::min_savegame_version;

	if(w_ver.major_version() != 1 || ms_ver.major_version() != 1) {
		// Unimplemented, assuming that version 2 won't use WML-based saves
		return {};
	}

	std::vector<other_version_dir> result;

	// For 1.16, check for saves from all versions up to 1.20.
	for(auto minor = w_ver.minor_version() + 4; minor >= ms_ver.minor_version(); --minor) {
		if(minor == w_ver.minor_version())
			continue;

		auto version = version_info{};
		version.set_major_version(w_ver.major_version());
		version.set_minor_version(minor);
		auto suffix = get_version_path_suffix(version);

		bfs::path path;

		//
		// NOTE:
		// This is a bit of a naive approach. We assume on all platforms that
		// get_user_data_path() will return something resembling the default
		// configuration and that --user-data-dir wasn't used. We will get
		// false negatives when any of these conditions don't hold true.
		//

#if defined(_WIN32)
		path = get_user_data_path().parent_path() / ("Wesnoth" + suffix) / "saves";
#elif defined(_X11)
		path = get_user_data_path().parent_path() / suffix / "saves";
#elif defined(__APPLE__)
		path = get_user_data_path().parent_path() / ("Wesnoth_" + suffix) / "saves";
#endif

		if(bfs::exists(path)) {
			result.emplace_back(suffix, path.string());
		}
	}

	return result;
#endif
}

std::string get_cwd()
{
	error_code ec;
	bfs::path cwd = bfs::current_path(ec);

	if(ec) {
		ERR_FS << "Failed to get current directory: " << ec.message();
		return "";
	}

	return cwd.generic_string();
}

bool set_cwd(const std::string& dir)
{
	error_code ec;
	bfs::current_path(bfs::path{dir}, ec);

	if(ec) {
		ERR_FS << "Failed to set current directory: " << ec.message();
		return false;
	} else {
		LOG_FS << "Process working directory set to " << dir;
	}

	return true;
}

std::string get_exe_path()
{
#ifdef _WIN32
	wchar_t process_path[MAX_PATH];
	SetLastError(ERROR_SUCCESS);

	GetModuleFileNameW(nullptr, process_path, MAX_PATH);

	if(GetLastError() != ERROR_SUCCESS) {
		return get_cwd() + "/wesnoth";
	}

	bfs::path exe(process_path);
	return exe.string();
#elif defined(__APPLE__)
	std::vector<char> buffer(PATH_MAX, 0);
	uint32_t size = PATH_MAX;
	if(_NSGetExecutablePath(&buffer[0], &size) == 0) {
		buffer.resize(size+1);
		return std::string(buffer.begin(), buffer.end());
	} else {
		ERR_FS << "Path to wesnoth executable is too long";
		return get_cwd() + "/The Battle for Wesnoth";
	}
#else
	// first check /proc
	if(bfs::exists("/proc/")) {
		bfs::path self_exe("/proc/self/exe");
		error_code ec;
		bfs::path exe = bfs::read_symlink(self_exe, ec);
		if(!ec) {
			return exe.string();
		}
	}

	// check the PATH for wesnoth's location
	// with version
	std::string version = std::to_string(game_config::wesnoth_version.major_version()) + "." + std::to_string(game_config::wesnoth_version.minor_version());
	std::string exe = filesystem::get_program_invocation("wesnoth-"+version);
	bfs::path search = bp::search_path(exe).string();
	if(!search.string().empty()) {
		return search.string();
	}

	// versionless
	exe = filesystem::get_program_invocation("wesnoth");
	search = bp::search_path(exe).string();
	if(!search.string().empty()) {
		return search.string();
	}

	// return the current working directory
	return get_cwd() + "/wesnoth";
#endif
}

std::string get_exe_dir()
{
	bfs::path path(get_exe_path());
	return path.parent_path().string();
}

std::string get_wesnothd_name()
{
	std::string exe_dir = get_exe_dir();
	std::string exe_name = base_name(get_exe_path());
	// macOS doesn't call the wesnoth client executable "wesnoth"
	// otherwise, add any suffix after the "wesnoth" part of the executable name to wesnothd's name
	std::string wesnothd = exe_dir + "/wesnothd" + exe_name.substr(7);
	if(!file_exists(wesnothd)) {
		return exe_dir + "/" + get_program_invocation("wesnothd");
	}
	return wesnothd;
}

bool make_directory(const std::string& dirname)
{
	error_code ec;
	bool created = bfs::create_directory(bfs::path(dirname), ec);
	if(ec) {
		ERR_FS << "Failed to create directory " << dirname << ": " << ec.message();
	}

	return created;
}

bool delete_directory(const std::string& dirname, const bool keep_pbl)
{
	bool ret = true;
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	error_code ec;

	get_files_in_dir(dirname, &files, &dirs, name_mode::ENTIRE_FILE_PATH, keep_pbl ? filter_mode::SKIP_PBL_FILES : filter_mode::NO_FILTER);

	if(!files.empty()) {
		for(const std::string& f : files) {
			bfs::remove(bfs::path(f), ec);
			if(ec) {
				LOG_FS << "remove(" << f << "): " << ec.message();
				ret = false;
			}
		}
	}

	if(!dirs.empty()) {
		for(const std::string& d : dirs) {
			// TODO: this does not preserve any other PBL files
			// filesystem.cpp does this too, so this might be intentional
			if(!delete_directory(d))
				ret = false;
		}
	}

	if(ret) {
		bfs::remove(bfs::path(dirname), ec);
		if(ec) {
			LOG_FS << "remove(" << dirname << "): " << ec.message();
			ret = false;
		}
	}

	return ret;
}

bool delete_file(const std::string& filename)
{
	error_code ec;
	bool ret = bfs::remove(bfs::path(filename), ec);
	if(ec) {
		ERR_FS << "Could not delete file " << filename << ": " << ec.message();
	}

	return ret;
}

std::vector<uint8_t> read_file_binary(const std::string& fname)
{
	std::ifstream file(fname, std::ios::binary);
	std::vector<uint8_t> file_contents;

	file_contents.reserve(file_size(fname));
	file_contents.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	return file_contents;
}

std::string read_file_as_data_uri(const std::string& fname)
{
	std::vector<uint8_t> file_contents = filesystem::read_file_binary(fname);
	utils::byte_string_view view = {file_contents.data(), file_contents.size()};
	std::string name = filesystem::base_name(fname);
	std::string img = "";

	if(name.find(".") != std::string::npos) {
		// convert to web-safe base64, since the + symbols will get stripped out when reading this back in later
		img = "data:image/"+name.substr(name.find(".")+1)+";base64,"+base64::encode(view);
	}

	return img;
}

std::string read_file(const std::string& fname)
{
	scoped_istream is = istream_file(fname);
	std::stringstream ss;
	ss << is->rdbuf();
	return ss.str();
}

filesystem::scoped_istream istream_file(const std::string& fname, bool treat_failure_as_error)
{
	LOG_FS << "Streaming " << fname << " for reading.";

	if(fname.empty()) {
		ERR_FS << "Trying to open file with empty name.";
		filesystem::scoped_istream s(new bfs::ifstream());
		s->clear(std::ios_base::failbit);
		return s;
	}

	// mingw doesn't  support std::basic_ifstream::basic_ifstream(const wchar_t* fname)
	// that why boost::filesystem::fstream.hpp doesn't work with mingw.
	try {
		boost::iostreams::file_descriptor_source fd(bfs::path(fname), std::ios_base::binary);

		// TODO: has this still use ?
		if(!fd.is_open() && treat_failure_as_error) {
			ERR_FS << "Could not open '" << fname << "' for reading.";
		} else if(!is_filename_case_correct(fname, fd)) {
			ERR_FS << "Not opening '" << fname << "' due to case mismatch.";
			filesystem::scoped_istream s(new bfs::ifstream());
			s->clear(std::ios_base::failbit);
			return s;
		}

		return std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_source>>(fd, 4096, 0);
	} catch(const std::exception&) {
		if(treat_failure_as_error) {
			ERR_FS << "Could not open '" << fname << "' for reading.";
		}

		filesystem::scoped_istream s(new bfs::ifstream());
		s->clear(std::ios_base::failbit);
		return s;
	}
}

filesystem::scoped_ostream ostream_file(const std::string& fname, std::ios_base::openmode mode, bool create_directory)
{
	LOG_FS << "streaming " << fname << " for writing.";
	try {
		boost::iostreams::file_descriptor_sink fd(bfs::path(fname), mode);
		return std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_sink>>(fd, 4096, 0);
	} catch(const BOOST_IOSTREAMS_FAILURE& e) {
		// If this operation failed because the parent directory didn't exist, create the parent directory and
		// retry.
		error_code ec_unused;
		if(create_directory && bfs::create_directories(bfs::path(fname).parent_path(), ec_unused)) {
			return ostream_file(fname, mode, false);
		}

		throw filesystem::io_exception(e.what());
	}
}

// Throws io_exception if an error occurs
void write_file(const std::string& fname, const std::string& data, std::ios_base::openmode mode)
{
	scoped_ostream os = ostream_file(fname, mode);
	os->exceptions(std::ios_base::goodbit);

	const std::size_t block_size = 4096;
	char buf[block_size];

	for(std::size_t i = 0; i < data.size(); i += block_size) {
		const std::size_t bytes = std::min<std::size_t>(block_size, data.size() - i);
		std::copy(data.begin() + i, data.begin() + i + bytes, buf);

		os->write(buf, bytes);
		if(os->bad()) {
			throw io_exception("Error writing to file: '" + fname + "'");
		}
	}
}

void copy_file(const std::string& src, const std::string& dest)
{
	write_file(dest, read_file(src));
}

bool create_directory_if_missing(const std::string& dirname)
{
	return create_directory_if_missing(bfs::path(dirname));
}

bool create_directory_if_missing_recursive(const std::string& dirname)
{
	return create_directory_if_missing_recursive(bfs::path(dirname));
}

bool is_directory(const std::string& fname)
{
	return is_directory_internal(bfs::path(fname));
}

bool file_exists(const std::string& name)
{
	return file_exists(bfs::path(name));
}

std::time_t file_modified_time(const std::string& fname)
{
	error_code ec;
	std::time_t mtime = bfs::last_write_time(bfs::path(fname), ec);
	if(ec) {
		LOG_FS << "Failed to read modification time of " << fname << ": " << ec.message();
	}

	return mtime;
}

bool is_map(const std::string& filename)
{
	return bfs::path(filename).extension() == map_extension;
}

bool is_cfg(const std::string& filename)
{
	return bfs::path(filename).extension() == wml_extension;
}

bool is_mask(const std::string& filename)
{
	return bfs::path(filename).extension() == mask_extension;
}

bool is_gzip_file(const std::string& filename)
{
	return bfs::path(filename).extension() == ".gz";
}

bool is_bzip2_file(const std::string& filename)
{
	return bfs::path(filename).extension() == ".bz2";
}

int file_size(const std::string& fname)
{
	error_code ec;
	uintmax_t size = bfs::file_size(bfs::path(fname), ec);
	if(ec) {
		LOG_FS << "Failed to read filesize of " << fname << ": " << ec.message();
		return -1;
	} else if(size > std::numeric_limits<int>::max()) {
		return std::numeric_limits<int>::max();
	} else {
		return size;
	}
}

int dir_size(const std::string& pname)
{
	bfs::path p(pname);
	uintmax_t size_sum = 0;
	error_code ec;
	for(bfs::recursive_directory_iterator i(p), end; i != end && !ec; ++i) {
		if(bfs::is_regular_file(i->path())) {
			size_sum += bfs::file_size(i->path(), ec);
		}
	}

	if(ec) {
		LOG_FS << "Failed to read directorysize of " << pname << ": " << ec.message();
		return -1;
	} else if(size_sum > std::numeric_limits<int>::max()) {
		return std::numeric_limits<int>::max();
	} else {
		return size_sum;
	}
}

std::string base_name(const std::string& file, const bool remove_extension)
{
	if(!remove_extension) {
		return bfs::path(file).filename().string();
	} else {
		return bfs::path(file).stem().string();
	}
}

std::string directory_name(const std::string& file)
{
	return bfs::path(file).parent_path().string();
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
		if(!ec) {
			p = q;
		}
	} while(ec && !is_root(p.string()));

	return ec ? "" : p.string();
}

bool is_path_sep(char c)
{
	static const bfs::path sep = bfs::path("/").make_preferred();
	const std::string s = std::string(1, c);
	return sep == bfs::path(s).make_preferred();
}

char path_separator()
{
	return bfs::path::preferred_separator;
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
	bfs::path p = resolve_dot_entries ? bfs::canonical(fpath, ec) : bfs::absolute(fpath);

	if(ec) {
		return "";
	}

	if(normalize_separators) {
		return p.make_preferred().string();
	} else {
		return p.string();
	}
}

bool to_asset_path(std::string& path, std::string addon_id, std::string asset_type)
{
	std::string rel_path = "";
	std::string core_asset_dir = get_dir(game_config::path + "/data/core/" + asset_type);
	std::string addon_asset_dir;

	bool found = false;
	bool is_in_core_dir = (path.find(core_asset_dir) != std::string::npos);
	bool is_in_addon_dir = false;

	if (is_in_core_dir) {
		rel_path = path.erase(0, core_asset_dir.size()+1);
		found = true;
	} else if (!addon_id.empty()) {
		addon_asset_dir = get_current_editor_dir(addon_id) + "/" + asset_type;
		is_in_addon_dir = (path.find(addon_asset_dir) != std::string::npos);
		if (is_in_addon_dir) {
			rel_path = path.erase(0, addon_asset_dir.size()+1);
			found = true;
		} else {
			// Not found in either core or addons dirs,
			// return a possible path where the asset could be copied.
			std::string filename = boost::filesystem::path(path).filename().string();
			std::string asset_path = addon_asset_dir + "/" + filename;
			rel_path = filename;
			found = false;
		}
	} else {
		found = false;
	}

	path = rel_path;
	return found;
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
namespace
{
std::set<std::string> binary_paths;

typedef std::map<std::string, std::vector<std::string>> paths_map;
paths_map binary_paths_cache;

} // namespace

static void init_binary_paths()
{
	if(binary_paths.empty()) {
		binary_paths.insert("");
	}
}

binary_paths_manager::binary_paths_manager()
	: paths_()
{
}

binary_paths_manager::binary_paths_manager(const game_config_view& cfg)
	: paths_()
{
	set_paths(cfg);
}

binary_paths_manager::~binary_paths_manager()
{
	cleanup();
}

void binary_paths_manager::set_paths(const game_config_view& cfg)
{
	cleanup();
	init_binary_paths();

	for(const config& bp : cfg.child_range("binary_path")) {
		std::string path = bp["path"].str();
		if(path.find("..") != std::string::npos) {
			ERR_FS << "Invalid binary path '" << path << "'";
			continue;
		}

		if(!path.empty() && path.back() != '/')
			path += "/";
		if(binary_paths.count(path) == 0) {
			binary_paths.insert(path);
			paths_.push_back(path);
		}
	}
}

void binary_paths_manager::cleanup()
{
	binary_paths_cache.clear();

	for(const std::string& p : paths_) {
		binary_paths.erase(p);
	}
}

void clear_binary_paths_cache()
{
	binary_paths_cache.clear();
}

static bool is_legal_file(const std::string& filename_str)
{
	DBG_FS << "Looking for '" << filename_str << "'.";

	if(filename_str.empty()) {
		LOG_FS << "  invalid filename";
		return false;
	}

	if(filename_str.find("..") != std::string::npos) {
		ERR_FS << "Illegal path '" << filename_str << "' (\"..\" not allowed).";
		return false;
	}

	if(filename_str.find('\\') != std::string::npos) {
		ERR_FS << "Illegal path '" << filename_str
			   << R"end(' ("\" not allowed, for compatibility with GNU/Linux and macOS).)end";
		return false;
	}

	bfs::path filepath(filename_str);

	if(default_blacklist.match_file(filepath.filename().string())) {
		ERR_FS << "Illegal path '" << filename_str << "' (blacklisted filename).";
		return false;
	}

	if(std::any_of(filepath.begin(), filepath.end(),
			   [](const bfs::path& dirname) { return default_blacklist.match_dir(dirname.string()); })) {
		ERR_FS << "Illegal path '" << filename_str << "' (blacklisted directory name).";
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

	if(type.find("..") != std::string::npos) {
		// Not an assertion, as language.cpp is passing user data as type.
		ERR_FS << "Invalid WML type '" << type << "' for binary paths";
		static std::vector<std::string> dummy;
		return dummy;
	}

	std::vector<std::string>& res = binary_paths_cache[type];

	init_binary_paths();

	for(const std::string& path : binary_paths) {
		res.push_back(get_user_data_dir() + "/" + path + type + "/");

		if(!game_config::path.empty()) {
			res.push_back(game_config::path + "/" + path + type + "/");
		}
	}

	// not found in "/type" directory, try main directory
	res.push_back(get_user_data_dir() + "/");

	if(!game_config::path.empty()) {
		res.push_back(game_config::path + "/");
	}

	return res;
}

utils::optional<std::string> get_binary_file_location(const std::string& type, const std::string& filename)
{
	// We define ".." as "remove everything before" this is needed because
	// on the one hand allowing ".." would be a security risk but
	// especially for terrains the c++ engine puts a hardcoded "terrain/" before filename
	// and there would be no way to "escape" from "terrain/" otherwise. This is not the
	// best solution but we cannot remove it without another solution (subtypes maybe?).

	{
		std::string::size_type pos = filename.rfind("../");
		if(pos != std::string::npos) {
			return get_binary_file_location(type, filename.substr(pos + 3));
		}
	}

	if(!is_legal_file(filename)) {
		return utils::nullopt;
	}

	std::string result;
	// fix for duplicate mainline paths on macOS for some reason
	// would be good for someone who uses macOS to debug the cause at some point
	const std::vector<std::string> temp = get_binary_paths(type);
	const std::set<std::string> bpaths(temp.begin(), temp.end());
	for(const std::string& bp : bpaths) {
		bfs::path bpath(bp);
		bpath /= filename;

		DBG_FS << "  checking '" << bp << "'";

		if(file_exists(bpath)) {
			DBG_FS << "  found at '" << bpath.string() << "'";
			if(result.empty()) {
				result = bpath.string();
			} else {
				WRN_FS << "Conflicting files in binary_path: '" << result
					   << "' and '" << bpath.string() << "'";
			}
		}
	}

	if(result.empty()) {
		DBG_FS << "  not found";
		return utils::nullopt;
	} else {
		return result;
	}
}

utils::optional<std::string> get_binary_dir_location(const std::string& type, const std::string& filename)
{
	if(!is_legal_file(filename)) {
		return utils::nullopt;
	}

	for(const std::string& bp : get_binary_paths(type)) {
		bfs::path bpath(bp);
		bpath /= filename;
		DBG_FS << "  checking '" << bp << "'";
		if(is_directory_internal(bpath)) {
			DBG_FS << "  found at '" << bpath.string() << "'";
			return bpath.string();
		}
	}

	DBG_FS << "  not found";
	return utils::nullopt;
}

utils::optional<std::string> get_wml_location(const std::string& filename, const std::string& current_dir)
{
	if(!is_legal_file(filename)) {
		return utils::nullopt;
	}

	assert(game_config::path.empty() == false);

	bfs::path fpath(filename);
	bfs::path result;

	if(filename[0] == '~') {
		result /= get_user_data_path() / "data" / filename.substr(1);
		DBG_FS << "  trying '" << result.string() << "'";
	} else if(*fpath.begin() == ".") {
		if(!current_dir.empty()) {
			result /= bfs::path(current_dir);
		} else {
			result /= bfs::path(game_config::path) / "data";
		}

		result /= filename;
	} else if(!game_config::path.empty()) {
		result /= bfs::path(game_config::path) / "data" / filename;
	}

	if(result.empty() || !file_exists(result)) {
		DBG_FS << "  not found";
		return utils::nullopt;
	} else {
		DBG_FS << "  found: '" << result.string() << "'";
		return result.string();
	}
}

static bfs::path subtract_path(const bfs::path& full, const bfs::path& prefix)
{
	bfs::path::iterator fi = full.begin(), fe = full.end(), pi = prefix.begin(), pe = prefix.end();
	while(fi != fe && pi != pe && *fi == *pi) {
		++fi;
		++pi;
	}

	bfs::path rest;
	if(pi == pe) {
		while(fi != fe) {
			rest /= *fi;
			++fi;
		}
	}

	return rest;
}

std::string get_short_wml_path(const std::string& filename)
{
	bfs::path full_path(filename);

	bfs::path partial = subtract_path(full_path, get_user_data_path() / "data");
	if(!partial.empty()) {
		return "~" + partial.generic_string();
	}

	partial = subtract_path(full_path, bfs::path(game_config::path) / "data");
	if(!partial.empty()) {
		return partial.generic_string();
	}

	return filename;
}

utils::optional<std::string> get_independent_binary_file_path(const std::string& type, const std::string& filename)
{
	auto bp = get_binary_file_location(type, filename);
	if(!bp) {
		return utils::nullopt;
	}

	bfs::path full_path{bp.value()};
	bfs::path partial = subtract_path(full_path, get_user_data_path());
	if(!partial.empty()) {
		return partial.generic_string();
	}

	partial = subtract_path(full_path, game_config::path);
	if(!partial.empty()) {
		return partial.generic_string();
	}

	return full_path.generic_string();
}

std::string get_program_invocation(const std::string& program_name)
{
#ifdef _WIN32
	return program_name + ".exe";
#else
	return program_name;
#endif
}

std::string sanitize_path(const std::string& path)
{
#ifdef _WIN32
	const char* user_name = getenv("USERNAME");
#else
	const char* user_name = getenv("USER");
#endif

	std::string canonicalized = filesystem::normalize_path(path, true, false);
	if(user_name != nullptr) {
		boost::replace_all(canonicalized, user_name, "USER");
	}

	return canonicalized;
}

// Return path to localized counterpart of the given file, if any, or empty string.
// Localized counterpart may also be requested to have a suffix to base name.
utils::optional<std::string> get_localized_path(const std::string& file, const std::string& suff)
{
	std::string dir = filesystem::directory_name(file);
	std::string base = filesystem::base_name(file);

	const std::size_t pos_ext = base.rfind(".");

	std::string loc_base;
	if(pos_ext != std::string::npos) {
		loc_base = base.substr(0, pos_ext) + suff + base.substr(pos_ext);
	} else {
		loc_base = base + suff;
	}

	// TRANSLATORS: This is the language code which will be used
	// to store and fetch localized non-textual resources, such as images,
	// when they exist. Normally it is just the code of the PO file itself,
	// e.g. "de" of de.po for German. But it can also be a comma-separated
	// list of language codes by priority, when the localized resource
	// found for first of those languages will be used. This is useful when
	// two languages share sufficient commonality, that they can use each
	// other's resources rather than duplicating them. For example,
	// Swedish (sv) and Danish (da) are such, so Swedish translator could
	// translate this message as "sv,da", while Danish as "da,sv".
	std::vector<std::string> langs = utils::split(_("language code for localized resources^en_US"));

	// In case even the original image is split into base and overlay,
	// add en_US with lowest priority, since the message above will
	// not have it when translated.
	langs.push_back("en_US");
	for(const std::string& lang : langs) {
		std::string loc_file = dir + "/" + "l10n" + "/" + lang + "/" + loc_base;
		if(filesystem::file_exists(loc_file)) {
			return loc_file;
		}
	}

	return utils::nullopt;
}

utils::optional<std::string> get_addon_id_from_path(const std::string& location)
{
	std::string full_path = normalize_path(location, true);
	std::string addons_path = normalize_path(get_addons_dir(), true);

	if(full_path.find(addons_path) == 0) {
		bfs::path path(full_path.substr(addons_path.size()+1));
		if(path.size() > 0) {
			return path.begin()->string();
		}
	}

	return utils::nullopt;
}

} // namespace filesystem
