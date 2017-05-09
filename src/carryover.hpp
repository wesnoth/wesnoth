/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class team;
class config;
#include <vector>
#include <string>
#include <set>
#include <boost/ptr_container/ptr_vector.hpp>

#include "config.hpp"
#include "mt_rng.hpp"
#include "game_events/wmi_container.hpp"

class carryover
{
public:
	carryover()
		: add_ ()
		, current_player_()
		, gold_()
		, previous_recruits_()
		, recall_list_()
		, save_id_()
	{}
	// Turns config from a loaded savegame into carryover_info
	explicit carryover(const config& side);
	carryover(const team& t, const int gold, const bool add);
	~carryover(){}

	const std::string& get_save_id() const{ return save_id_; }
	void transfer_all_gold_to(config& side_cfg);
	void transfer_all_recruits_to(config& side_cfg);
	void transfer_all_recalls_to(config& side_cfg);
	const std::string to_string();
	void to_config(config& cfg);
private:
	bool add_;
	std::string current_player_;
	int gold_;
	std::set<std::string> previous_recruits_;
	// NOTE: we store configs instead of units because units often assume or
	//       assert that various resources:: are available, which is not the
	//       case between scenarios.
	std::vector<config> recall_list_;
	std::string save_id_;
	config variables_;

	std::string get_recruits(bool erase=false);
};

class carryover_info
{
public:
	carryover_info()
		: carryover_sides_()
		, variables_()
		, rng_()
		, wml_menu_items_()
		, next_scenario_()
		, next_underlying_unit_id_()
	{}
	/// Turns config from a loaded savegame into carryover_info
	/// @param from_snapshot true if cfg is a [snapshot], false if cfg is [carryover_sides(_start)]
	explicit carryover_info(const config& cfg, bool from_snapshot = false);

	carryover* get_side(const std::string& save_id);
	std::vector<carryover>& get_all_sides();
	void add_side(const config& cfg);
	void add_side(const team& t, const int gold, const bool add);
	void remove_side(const std::string& id);

	void transfer_all_to(config& side_cfg);

	void transfer_to(config& level);

	void set_variables(const config& vars) { variables_ = vars; }
	const config& get_variables() const { return variables_; }

	const randomness::mt_rng& rng() const { return rng_; }
	randomness::mt_rng& rng() { return rng_; }

	const std::string& next_scenario() const { return next_scenario_; }

	const config to_config();

	void merge_old_carryover(const carryover_info& old_carryover);
private:
	std::vector<carryover> carryover_sides_;
	config variables_;
	randomness::mt_rng rng_;
	boost::ptr_vector<config> wml_menu_items_;
	std::string next_scenario_;    /**< the scenario coming next (for campaigns) */
	int next_underlying_unit_id_;
};
