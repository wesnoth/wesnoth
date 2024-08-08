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
 * Declarations for File-IO.
 */

#pragma once

#include <ctime>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "game_version.hpp"
#include "global.hpp"
#include "utils/optional_fwd.hpp"

namespace game_config {
extern std::string path;
extern std::string default_preferences_path;
extern bool check_migration;

/** observer team name used for observer team chat */
extern const std::string observer_team_name;

extern int cache_compression_level;
}

class config;
class game_config_view;
struct SDL_RWops;

namespace filesystem {

using scoped_istream = std::unique_ptr<std::istream>;
using scoped_ostream = std::unique_ptr<std::ostream>;

struct sdl_rwops_deleter
{
	void operator()(SDL_RWops*) const noexcept;
};

using rwops_ptr = std::unique_ptr<SDL_RWops, sdl_rwops_deleter>;

rwops_ptr make_read_RWops(const std::string &path);
rwops_ptr make_write_RWops(const std::string &path);

/** An exception object used when an IO error occurs */
struct io_exception : public game::error {
	io_exception() : game::error("") {}
	io_exception(const std::string& msg) : game::error(msg) {}
};

struct file_tree_checksum;

enum class name_mode { ENTIRE_FILE_PATH, FILE_NAME_ONLY };
enum class filter_mode { NO_FILTER, SKIP_MEDIA_DIR, SKIP_PBL_FILES };
enum class reorder_mode { DONT_REORDER, DO_REORDER };

// default extensions
const std::string map_extension = ".map";
const std::string mask_extension = ".mask";
const std::string wml_extension = ".cfg";

// A list of file and directory blacklist patterns
class blacklist_pattern_list
{
public:
	blacklist_pattern_list()
		: file_patterns_(), directory_patterns_()
	{}
	blacklist_pattern_list(const std::vector<std::string>& file_patterns, const std::vector<std::string>& directory_patterns)
		: file_patterns_(file_patterns), directory_patterns_(directory_patterns)
	{}

	bool match_file(const std::string& name) const;

	bool match_dir(const std::string& name) const;

	void add_file_pattern(const std::string& pattern)
	{
		file_patterns_.push_back(pattern);
	}

	void add_directory_pattern(const std::string& pattern)
	{
		directory_patterns_.push_back(pattern);
	}

	void remove_blacklisted_files_and_dirs(std::vector<std::string>& files, std::vector<std::string>& directories) const;

private:
	std::vector<std::string> file_patterns_;
	std::vector<std::string> directory_patterns_;
};

extern const blacklist_pattern_list default_blacklist;

/**
 * Get a list of all files and/or directories in a given directory.
 *
 * @param dir The directory to examine.
 * @param[out] files The files in @a dir. Won't be used if nullptr.
 * @param[out] dirs The directories in @a dir. Won't be used if nullptr.
 * @param mode Determines whether the entire path or just the filename is retrieved.
 * @param filter Determines if we skip images and sounds directories.
 * @param reorder Triggers the special handling of _main.cfg and _final.cfg.
 * @param[out] checksum Can be used to store checksum info.
 */
void get_files_in_dir(const std::string &dir,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs=nullptr,
                      name_mode mode = name_mode::FILE_NAME_ONLY,
                      filter_mode filter = filter_mode::NO_FILTER,
                      reorder_mode reorder = reorder_mode::DONT_REORDER,
                      file_tree_checksum* checksum = nullptr);

std::string get_dir(const std::string &dir);

/**
 * Try to autodetect the location of the game data dir. Note that
 * the root of the source tree currently doubles as the data dir.
 */
std::string autodetect_game_data_dir(std::string exe_dir);

// The location of various important files/folders:
/**
 * location of preferences file containing preferences that are synced between computers
 * note that wesnoth does not provide the syncing functionality itself
 */
std::string get_synced_prefs_file();
/** location of preferences file containing preferences that aren't synced between computers */
std::string get_unsynced_prefs_file();
std::string get_credentials_file();
std::string get_default_prefs_file();
std::string get_save_index_file();
std::string get_lua_history_file();
/**
 * parent directory for everything that should be synced between systems.
 * implemented due to limitations of Steam's AutoCloud (non-SDK) syncing, but will also simplify things if it's ever added for any other platforms.
 */
std::string get_sync_dir();
std::string get_saves_dir();
std::string get_wml_persist_dir();
std::string get_intl_dir();
std::string get_screenshot_dir();
std::string get_addons_data_dir();
std::string get_addons_dir();
std::string get_current_editor_dir(const std::string& addon_id);
const std::string get_version_path_suffix(const version_info& version);
const std::string& get_version_path_suffix();

/**
 * Get the next free filename using "name + number (3 digits) + extension"
 * maximum 1000 files then start always giving 999
 */
std::string get_next_filename(const std::string& name, const std::string& extension);

bool is_userdata_initialized();
void set_user_data_dir(std::string path);
void set_cache_dir(const std::string& path);

std::string get_user_data_dir();
std::string get_logs_dir();
std::string get_cache_dir();
std::string get_legacy_editor_dir();
std::string get_core_images_dir();

bool rename_dir(const std::string& old_dir, const std::string& new_dir);

struct other_version_dir
{
	/**
	 * Here the version is given as a string instead of a version_info, because the
	 * logic of how many components are significant ("1.16" rather than
	 * "1.16.0") is encapsulated in find_other_version_saves_dirs().
	 */
	std::string version;
	std::string path;

	// constructor because emplace_back() doesn't use aggregate initialization
	other_version_dir(const std::string& v, const std::string& p)
		: version(v)
		, path(p)
	{
	}
};

/**
 * Searches for directories containing saves created by other versions of Wesnoth.
 *
 * The directories returned will exist, but might not contain any saves. Changes to
 * the filesystem (by running other versions or by deleting old directories) may
 * change the results returned by the function.
 */
std::vector<other_version_dir> find_other_version_saves_dirs();

std::string get_cwd();
bool set_cwd(const std::string& dir);

std::string get_exe_path();
std::string get_exe_dir();
std::string get_wesnothd_name();

bool make_directory(const std::string& dirname);
bool delete_directory(const std::string& dirname, const bool keep_pbl = false);
bool delete_file(const std::string& filename);

bool looks_like_pbl(const std::string& file);

// Basic disk I/O:

/** Basic disk I/O - read file. */
std::string read_file(const std::string& fname);
std::vector<uint8_t> read_file_binary(const std::string& fname);
std::string read_file_as_data_uri(const std::string& fname);

filesystem::scoped_istream istream_file(const std::string& fname, bool treat_failure_as_error = true);
filesystem::scoped_ostream ostream_file(const std::string& fname, std::ios_base::openmode mode = std::ios_base::binary, bool create_directory = true);
/** Throws io_exception if an error occurs. */
void write_file(const std::string& fname, const std::string& data, std::ios_base::openmode mode = std::ios_base::binary);
/**
 * Read a file and then writes it back out.
 *
 * @param src The source file.
 * @param dest The destination of the copied file.
 */
void copy_file(const std::string& src, const std::string& dest);

std::string read_map(const std::string& name);
std::string read_scenario(const std::string& name);

/**
 * Creates a directory if it does not exist already.
 *
 * @param dirname                 Path to directory. All parents should exist.
 * @returns                       True if the directory exists or could be
 *                                successfully created; false otherwise.
 */
bool create_directory_if_missing(const std::string& dirname);
/**
 * Creates a recursive directory tree if it does not exist already
 * @param dirname                 Full path of target directory. Non existing parents
 *                                will be created
 * @return                        True if the directory exists or could be
 *                                successfully created; false otherwise.
 */
bool create_directory_if_missing_recursive(const std::string& dirname);

/** Returns true if the given file is a directory. */
bool is_directory(const std::string& fname);

/** Returns true if a file or directory with such name already exists. */
bool file_exists(const std::string& name);

/** Get the modification time of a file. */
std::time_t file_modified_time(const std::string& fname);

/** Returns true if the file ends with the mapfile extension. */
bool is_map(const std::string& filename);

/** Returns true if the file ends with the wmlfile extension. */
bool is_cfg(const std::string& filename);

/** Returns true if the file ends with the maskfile extension. */
bool is_mask(const std::string& filename);

/** Returns true if the file ends with '.gz'. */
bool is_gzip_file(const std::string& filename);

/** Returns true if the file ends with '.bz2'. */
bool is_bzip2_file(const std::string& filename);

inline bool is_compressed_file(const std::string& filename) {
	return is_gzip_file(filename) || is_bzip2_file(filename);
}

/**
 * Returns whether the given filename is a legal name for a user-created file.
 *
 * This is meant to be used for any files created by Wesnoth where user input
 * is required, including save files and add-on files for uploading to the
 * add-ons server.
 *
 * @param name                 File name to verify.
 * @param allow_whitespace     Whether whitespace should be allowed.
 */
bool is_legal_user_file_name(const std::string& name, bool allow_whitespace = true);

struct file_tree_checksum
{
	file_tree_checksum();
	explicit file_tree_checksum(const config& cfg);
	void write(config& cfg) const;
	void reset() {nfiles = 0;modified = 0;sum_size=0;}
	// @todo make variables private!
	std::size_t nfiles, sum_size;
	std::time_t modified;
	bool operator==(const file_tree_checksum &rhs) const;
	bool operator!=(const file_tree_checksum &rhs) const
	{ return !operator==(rhs); }
};

/** Get the time at which the data/ tree was last modified at. */
const file_tree_checksum& data_tree_checksum(bool reset = false);

/** Returns the size of a file, or -1 if the file doesn't exist. */
int file_size(const std::string& fname);

/** Returns the sum of the sizes of the files contained in a directory. */
int dir_size(const std::string& path);

/**
 * Returns the base filename of a file, with directory name stripped.
 * Equivalent to a portable basename() function.
 *
 * If @a remove_extension is true, the filename extension will be stripped
 * from the returned value.
 */
std::string base_name(const std::string& file, const bool remove_extension = false);

/**
 * Returns the directory name of a file, with filename stripped.
 * Equivalent to a portable dirname()
 */
std::string directory_name(const std::string& file);

/**
 * Finds the nearest parent in existence for a file or directory.
 *
 * @note    The file's own existence is not checked.
 *
 * @returns An absolute path to the closest parent of the given path, or an
 *          empty string if none could be found. While on POSIX platforms this
 *          cannot happen (unless the original path was already empty), on
 *          Windows it might be the case that the original path refers to a
 *          drive letter or network share that does not exist.
 */
std::string nearest_extant_parent(const std::string& file);

/**
 * Returns the absolute path of a file.
 *
 * @param path                 Original path.
 * @param normalize_separators Whether to substitute path separators with the
 *                             platform's preferred format.
 * @param resolve_dot_entries  Whether to resolve . and .. directory entries.
 *                             This requires @a path to refer to a valid
 *                             existing object.
 *
 * @returns An absolute path -- that is, a path that is independent of the
 *          current working directory for the process. If resolve_dot_entries
 *          is set to true, the returned path has . and .. components resolved;
 *          however, if resolution fails because a component does not exist, an
 *          empty string is returned instead.
 */
std::string normalize_path(const std::string& path,
						   bool normalize_separators = false,
						   bool resolve_dot_entries = false);

/** Helper function to convert absolute path to wesnoth relative path */
bool to_asset_path(std::string& abs_path,
                   std::string addon_id,
                   std::string asset_type);

/**
 * Sanitizes a path to remove references to the user's name.
 */
std::string sanitize_path(const std::string& path);

/**
 * Returns whether the path is the root of the file hierarchy.
 *
 * @note This function is unreliable for paths that do not exist -- it will
 *       always return @a false for those.
 */
bool is_root(const std::string& path);

/**
 * Returns the name of the root device if included in the given path.
 *
 * This only properly makes sense on Windows with paths containing a drive
 * letter or UNC at the start -- otherwise, it will return the empty string. To
 * ensure that a suitable root name can be found you might want to use
 * normalize_path() first with @a resolve_dot_entries set to true.
 */
std::string root_name(const std::string& path);

/**
 * Returns whether the path seems to be relative.
 */
bool is_relative(const std::string& path);

/**
 * Returns whether @a c is a path separator.
 *
 * @note / is always a path separator. Additionally, on Windows \\ is a
 *       path separator as well.
 */
bool is_path_sep(char c);

/**
 * Returns the standard path separator for the current platform.
 */
char path_separator();

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
struct binary_paths_manager
{
	binary_paths_manager();
	binary_paths_manager(const game_config_view& cfg);
	~binary_paths_manager();

	void set_paths(const game_config_view& cfg);

private:
	binary_paths_manager(const binary_paths_manager& o);
	binary_paths_manager& operator=(const binary_paths_manager& o);

	void cleanup();

	std::vector<std::string> paths_;
};

void clear_binary_paths_cache();

/**
 * Returns a vector with all possible paths to a given type of binary,
 * e.g. 'images', 'sounds', etc,
 */
NOT_DANGLING const std::vector<std::string>& get_binary_paths(const std::string& type);

/**
 * Returns a complete path to the actual file of a given @a type, if it exists.
 */
utils::optional<std::string> get_binary_file_location(const std::string& type, const std::string& filename);

/**
 * Returns a complete path to the actual directory of a given @a type, if it exists.
 */
utils::optional<std::string> get_binary_dir_location(const std::string &type, const std::string &filename);

/**
 * Returns a complete path to the actual WML file or directory, if either exists.
 */
utils::optional<std::string> get_wml_location(const std::string &filename,
	const std::string &current_dir = std::string());

/**
 * Returns a short path to @a filename, skipping the (user) data directory.
 */
std::string get_short_wml_path(const std::string &filename);

/**
 * Returns an asset path to @a filename for binary path-independent use in saved games.
 *
 * Example:
 *   images, units/konrad-fighter.png ->
 *   data/campaigns/Heir_To_The_Throne/images/units/konrad-fighter.png
 */
utils::optional<std::string> get_independent_binary_file_path(const std::string& type, const std::string &filename);

/**
 * Returns the appropriate invocation for a Wesnoth-related binary, assuming
 * that it is located in the same directory as the running wesnoth binary.
 * This is just a string-transformation based on argv[0], so the returned
 * program is not guaranteed to actually exist.  '-debug' variants are handled
 * correctly.
 */
std::string get_program_invocation(const std::string &program_name);

/**
 * Returns the localized version of the given filename, if it exists.
 */
utils::optional<std::string> get_localized_path(const std::string& file, const std::string& suff = "");

/**
 * Returns the add-on ID from a path.
 * aka the directory directly following the "add-ons" folder, or an empty string if none is found.
 */
utils::optional<std::string> get_addon_id_from_path(const std::string& location);

}
