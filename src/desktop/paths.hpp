/*
	Copyright (C) 2016 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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
 * Desktop paths, storage media and bookmark functions.
 */

#pragma once

#include "tstring.hpp"

#include <iosfwd>
#include <set>
#include <vector>

namespace desktop
{

/**
 * Returns the path to the user profile dir (e.g. /home/username).
 *
 * An empty string is returned if the path cannot be determined somehow.
 */
std::string user_profile_dir();

struct path_info
{
	/** Path name or drive letter/mount point path; may be a translatable string if it's a game resources path. */
	t_string    name;
	/** System-defined label, if the path is a drive or mount point. */
	std::string label;
	/** Real path. */
	std::string path;

	/**
	 * Formats this path for UI display.
	 */
	std::string display_name() const;
};

std::ostream& operator<<(std::ostream& os, const path_info& pinf);

enum GAME_PATH_TYPES
{
	GAME_BIN_DIR = 0,				/**< Game executable dir. */
	GAME_CORE_DATA_DIR = 1,			/**< Game data dir. */
	GAME_USER_DATA_DIR = 3,			/**< User data dir. */
	GAME_EDITOR_MAP_DIR = 4,		/**< Editor map dir */
};

enum SYSTEM_PATH_TYPES
{
	SYSTEM_ALL_DRIVES = 0,			/**< Paths for each storage media found (Windows), /media and/or /mnt (X11, if non-empty). */
	SYSTEM_USER_PROFILE = 1,		/**< Path to the user's profile dir (e.g. /home/username or X:\\Users\\Username). */
	SYSTEM_ROOTFS = 2				/**< Path to the root of the filesystem hierarchy (ignored on Windows). */
};

/**
 * Returns a list of game-related paths.
 *
 * These paths are guaranteed to be their canonical forms (with links and dot
 * entries resolved) and using the platform's preferred path delimiter.
 */
std::vector<path_info> game_paths(const std::set<GAME_PATH_TYPES>& paths);

/**
 * Returns a list of system-defined paths.
 *
 * This includes removable media on platforms where we can reasonably
 * accurately enumerate those (FIXME: only Windows right now), the path to the
 * user's profile directory (/home/username, X:\\Users\\Username, etc.), and
 * the system drive root.
 *
 * These paths are guaranteed to be their canonical forms (with links and dot
 * entries resolved) and using the platform's preferred path delimiter.
 */
std::vector<path_info> system_paths(const std::set<SYSTEM_PATH_TYPES>& paths);

struct bookmark_info
{
	/** User defined label. */
	std::string label;
	/** Real path. */
	std::string path;
};

unsigned add_user_bookmark(const std::string& label, const std::string& path);

void remove_user_bookmark(unsigned index);

std::vector<bookmark_info> user_bookmarks();

} // namespace desktop
