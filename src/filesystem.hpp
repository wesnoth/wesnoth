/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 * Declarations for File-IO.
 */

#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED

#include <ctime>

#include <iosfwd>
#include <string>
#include <vector>

#include "exceptions.hpp"

class config;

struct SDL_RWops;

namespace filesystem {

SDL_RWops* load_RWops(const std::string &path);

/** An exception object used when an IO error occurs */
struct io_exception : public game::error {
	io_exception() : game::error("") {}
	io_exception(const std::string& msg) : game::error(msg) {}
};

struct file_tree_checksum;

enum file_name_option { ENTIRE_FILE_PATH, FILE_NAME_ONLY };
enum file_filter_option { NO_FILTER, SKIP_MEDIA_DIR, SKIP_PBL_FILES };
enum file_reorder_option { DONT_REORDER, DO_REORDER };

/**
 * Populates 'files' with all the files and
 * 'dirs' with all the directories in dir.
 * If files or dirs are nullptr they will not be used.
 *
 * mode: determines whether the entire path or just the filename is retrieved.
 * filter: determines if we skip images and sounds directories
 * reorder: triggers the special handling of _main.cfg and _final.cfg
 * checksum: can be used to store checksum info
 */
void get_files_in_dir(const std::string &dir,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs=nullptr,
                      file_name_option mode = FILE_NAME_ONLY,
                      file_filter_option filter = NO_FILTER,
                      file_reorder_option reorder = DONT_REORDER,
                      file_tree_checksum* checksum = nullptr);

std::string get_dir(const std::string &dir);

// The location of various important files:
std::string get_prefs_file();
std::string get_default_prefs_file();
std::string get_save_index_file();
std::string get_saves_dir();
std::string get_intl_dir();
std::string get_screenshot_dir();
std::string get_addons_dir();

/**
 * Get the next free filename using "name + number (3 digits) + extension"
 * maximum 1000 files then start always giving 999
 */
std::string get_next_filename(const std::string& name, const std::string& extension);
void set_user_config_dir(const std::string& path);
void set_user_data_dir(std::string path);

std::string get_user_config_dir();
std::string get_user_data_dir();
std::string get_cache_dir();

std::string get_cwd();
std::string get_exe_dir();

bool make_directory(const std::string& dirname);
bool delete_directory(const std::string& dirname, const bool keep_pbl = false);
bool delete_file(const std::string &filename);

bool looks_like_pbl(const std::string& file);

// Basic disk I/O:

/** Basic disk I/O - read file. */
std::string read_file(const std::string &fname);
std::istream *istream_file(const std::string& fname, bool treat_failure_as_error = true);
std::ostream *ostream_file(const std::string& fname, bool create_directory = true);
/** Throws io_exception if an error occurs. */
void write_file(const std::string& fname, const std::string& data);

std::string read_map(const std::string& name);

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
time_t file_modified_time(const std::string& fname);

/** Returns true if the file ends with '.gz'. */
bool is_gzip_file(const std::string& filename);

/** Returns true if the file ends with '.bz2'. */
bool is_bzip2_file(const std::string& filename);

inline bool is_compressed_file(const std::string& filename) {
	return is_gzip_file(filename) || is_bzip2_file(filename);
}

struct file_tree_checksum
{
	file_tree_checksum();
	explicit file_tree_checksum(const config& cfg);
	void write(config& cfg) const;
	void reset() {nfiles = 0;modified = 0;sum_size=0;}
	// @todo make variables private!
	size_t nfiles, sum_size;
	time_t modified;
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

bool ends_with(const std::string& str, const std::string& suffix);

/**
 * Returns the base filename of a file, with directory name stripped.
 * Equivalent to a portable basename() function.
 */
std::string base_name(const std::string& file);

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
	binary_paths_manager(const config& cfg);
	~binary_paths_manager();

	void set_paths(const config& cfg);

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
const std::vector<std::string>& get_binary_paths(const std::string& type);

/**
 * Returns a complete path to the actual file of a given @a type
 * or an empty string if the file isn't present.
 */
std::string get_binary_file_location(const std::string& type, const std::string& filename);

/**
 * Returns a complete path to the actual directory of a given @a type
 * or an empty string if the directory isn't present.
 */
std::string get_binary_dir_location(const std::string &type, const std::string &filename);

/**
 * Returns a complete path to the actual WML file or directory
 * or an empty string if the file isn't present.
 */
std::string get_wml_location(const std::string &filename,
	const std::string &current_dir = std::string());

/**
 * Returns a short path to @a filename, skipping the (user) data directory.
 */
std::string get_short_wml_path(const std::string &filename);

/**
 * Returns an image path to @a filename for binary path-independent use in saved games.
 *
 * Example:
 *   units/konrad-fighter.png ->
 *   data/campaigns/Heir_To_The_Throne/images/units/konrad-fighter.png
 */
std::string get_independent_image_path(const std::string &filename);

/**
 * Returns the appropriate invocation for a Wesnoth-related binary, assuming
 * that it is located in the same directory as the running wesnoth binary.
 * This is just a string-transformation based on argv[0], so the returned
 * program is not guaranteed to actually exist.  '-debug' variants are handled
 * correctly.
 */
std::string get_program_invocation(const std::string &program_name);

class scoped_istream {
	std::istream *stream;
public:
	scoped_istream(std::istream *s): stream(s) {}
	scoped_istream& operator=(std::istream *);
	std::istream &operator*() { return *stream; }
	std::istream *operator->() { return stream; }
	~scoped_istream();
};

class scoped_ostream {
	std::ostream *stream;
public:
	scoped_ostream(std::ostream *s): stream(s) {}
	scoped_ostream& operator=(std::ostream *);
	std::ostream &operator*() { return *stream; }
	std::ostream *operator->() { return stream; }
	~scoped_ostream();
};

}

#endif
