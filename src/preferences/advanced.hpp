/*
	Copyright (C) 2020 - 2024
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

#include "config.hpp"
#include "tstring.hpp"

#include <functional>
#include <string>
#include <vector>

class game_config_view;

namespace preferences
{
class advanced_manager
{
public:
	explicit advanced_manager(const game_config_view& gc);

	~advanced_manager();

	struct option
	{
		option(const config& pref);

		enum class avd_type { TOGGLE, SLIDER, COMBO, SPECIAL };

		/** The preference type. */
		avd_type type;

		/** Displayed name. */
		t_string name;

		/** Displayed description. */
		t_string description;

		/** The actual field saved in the prefs file and by which this preference may be accessed. */
		std::string field;

		/** The full config, including type-specific options. */
		config cfg;
	};

	const std::vector<option>& get_preferences() const
	{
		return prefs;
	}

private:
	std::vector<option> prefs;
};

/** Initializes the manager singleton. */
void init_advanced_manager(const game_config_view& gc);

using advanced_pref_list = std::vector<advanced_manager::option>;

/** Gets a list of the available advanced preferences. */
const advanced_pref_list& get_advanced_preferences();

} // namespace preferences
