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

#pragma once

class terrain_type;
class unit;
class unit_type;
class game_config_view;

#include <memory>
#include <string>

namespace help {

/**
 * The help implementation caches data parsed from the game_config. This class
 * is used to control the lifecycle of that cache, so that the cache will be
 * cleared before the game_config itself changes.
 *
 * Note: it's okay to call any of the help::show_* functions without creating
 * an instance of help_manager - that will simply mean that the cache is
 * cleared before the show function returns.
 *
 * Creating two instances of this will cause an assert.
 */
struct help_manager {
	help_manager(const game_config_view *game_config);
	help_manager(const help_manager&) = delete;
	help_manager& operator=(const help_manager&) = delete;
	~help_manager();
};

/**
 * Helper function for any of the show_help functions to control the cache's
 * lifecycle; can also be used by any other caller that wants to ensure the
 * cache is reused over multiple show_help calls.
 *
 * Treat the return type as opaque, it can return nullptr on success. Also
 * don't extend the cache lifecycle beyond the lifecycle of the
 * game_config_manager or over a reload of the game config.
 *
 *@pre game_config_manager has been initialised
 */
std::unique_ptr<help_manager> ensure_cache_lifecycle();

void init_help();

/**
 * Open the help browser. The help browser will have the topic with id
 * show_topic open if it is not the empty string. The default topic
 * will be shown if show_topic is the empty string.
 *
 *@pre game_config_manager has been initialised, or the instance of help_manager
 * has been created with an alternative config.
 */
void show_help(const std::string& show_topic="");

/** wrapper to add unit prefix and hiding symbol */
void show_unit_help(const std::string& unit_id, bool has_variations=false,
				bool hidden = false);

/** wrapper to add variation prefix and hiding symbol */
void show_variation_help(const std::string &unit_id, const std::string &variation,
				bool hidden = false);

/** wrapper to add terrain prefix and hiding symbol */
void show_terrain_help(const std::string& unit_id, bool hidden = false);

void show_unit_description(const unit_type &t);
void show_unit_description(const unit &u);
void show_terrain_description(const terrain_type& t);

} // End namespace help.
