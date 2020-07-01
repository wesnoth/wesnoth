/*
   Copyright (C) 2020 by the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <algorithm>
#include <ctime>
#include <iosfwd>

#include <memory>
#include <string>
#include <vector>

#include "wml_path.hpp"

#include "exceptions.hpp"
#include "serialization/string_utils.hpp"

class config;
class game_config_view;

namespace filesystem {

using scoped_istream = std::unique_ptr<std::istream>;
using scoped_ostream = std::unique_ptr<std::ostream>;


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
};

void clear_binary_paths_cache();

/**
 * Returns a vector with all possible paths to a given type of binary,
 * e.g. 'images', 'sounds', etc,
 */
const std::vector<wml_path>& get_binary_paths(const std::string& type);

/**
 * Returns a complete path to the actual file of a given @a type
 * or an empty string if the file isn't present.
 */
wml_path get_binary_file_location(const std::string& type, const std::string& filename);

/**
 * Returns a complete path to the actual directory of a given @a type
 * or an empty string if the directory isn't present.
 */
std::string get_binary_dir_location(const std::string &type, const std::string &filename);

/**
 * Returns an image path to @a filename for binary path-independent use in saved games.
 *
 * Example:
 *   units/konrad-fighter.png ->
 *   data/campaigns/Heir_To_The_Throne/images/units/konrad-fighter.png
 */
std::string get_independent_image_path(const std::string &filename);


}
