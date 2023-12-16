/*
	Copyright (C) 2003 - 2023
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

#include "movetype.hpp"
#include "units/unit_alignments.hpp"
#include "units/id.hpp"
#include "units/ptr.hpp"
#include "units/attack_type.hpp"
#include "units/race.hpp"
#include "utils/variant.hpp"

#include "game_config_view.hpp"

#include <boost/container/flat_set.hpp>
#include <bitset>
#include <optional>

class display;
class team;
class unit_animation_component;
class unit_formula_manager;
class vconfig;
struct color_t;

/** Data typedef for unit_ability_list. */
class unit_status
{
	std::string id_;
	std::string icon_;
	t_string tooltip_;
	t_string title_;
	// persistent statuses are usualyl applied by object
	// nonpersistent statuses get removed whna unit heals,
	bool persistent_;

	static boost::container::flat_set<unit_status> data_;

	bool visible() const { return !icon_.empty(); };
public:
	unit_status(const config& cfg);
	static void read_statuses(const game_config_view& gc);

	static void reset_statuses(std::set<std::string>& ss);
	static void heal_statuses(std::set<std::string>& ss);
	friend bool operator<(const unit_status& a1, const unit_status& a2) {
		return true;//a1.id_ < a2.id_;
	}
	static void report(config& res, const std::set<std::string>& ss);
};
