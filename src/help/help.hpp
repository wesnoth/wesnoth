/*
	Copyright (C) 2003 - 2025
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

#include "help/help_impl.hpp"

#include <boost/logic/tribool.hpp>

#include <memory>
#include <string>

namespace help
{
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
class help_manager
{
public:
	help_manager(const help_manager&) = delete;
	help_manager& operator=(const help_manager&) = delete;

	/** Returns the existing help_manager instance, or a newly allocated object otherwise. */
	static std::shared_ptr<help_manager> get_instance();

	/** Regenerates the cached help topics if necessary. */
	void verify_cache();

	const section& toplevel_section() const
	{
		return default_toplevel_;
	}

private:
	/**
	 * Private default constructor.
	 *
	 * Use @ref get_instance to get a managed instance instead.
	 */
	help_manager() = default;

	int last_num_encountered_units_{-1};
	int last_num_encountered_terrains_{-1};

	boost::tribool last_debug_state_{boost::indeterminate};

	/** The default toplevel. */
	section default_toplevel_;

	/** All sections and topics not referenced from the default toplevel. */
	section hidden_sections_;

	static inline std::shared_ptr<help_manager> singleton_;
};

/**
 * Open the help browser. The help browser will have the topic with id
 * show_topic open if it is not the empty string. The default topic
 * will be shown if show_topic is the empty string.
 */
void show_help(const std::string& show_topic = "");

/**
 * Given a unit type, find the corresponding help topic's id.
 */
std::string get_unit_type_help_id(const unit_type& t);

void show_unit_description(const unit_type &t);
void show_unit_description(const unit &u);
void show_terrain_description(const terrain_type& t);

} // End namespace help.
