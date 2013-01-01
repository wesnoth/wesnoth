/*
   Copyright (C) 2009 - 2013 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
 */

#include "tod_manager.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"
#include "formula_string_utils.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_abilities.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

tod_manager::tod_manager(const config& scenario_cfg, const int num_turns):
	savegame_config(),
	currentTime_(0),
	times_(),
	areas_(),
	turn_(1),
	num_turns_(num_turns)
{
	const config::attribute_value& turn_at = scenario_cfg["turn_at"];
	if(!turn_at.blank()) {
		turn_ = turn_at.to_int(1);
	}

	time_of_day::parse_times(scenario_cfg,times_);

	currentTime_ = get_start_ToD(scenario_cfg);
	//TODO:
	//Very bad, since we're pretending to not modify the cfg. Needed to transfer the result
	//to the network clients in a mp game, otherwise we have OOS.
	config& non_const_config = const_cast<config&>(scenario_cfg);
	non_const_config["current_tod"] = currentTime_;
}

tod_manager& tod_manager::operator=(const tod_manager& manager)
{
	if(this == &manager) {
		return *this;
	}

	currentTime_ = manager.currentTime_;
	times_ = manager.times_;
	areas_ = manager.areas_;

	turn_ = manager.turn_;
	num_turns_ = manager.num_turns_;

	return *this;
}

config tod_manager::to_config() const
{
	config cfg;
	cfg["turn_at"] = turn_;
	cfg["turns"] = num_turns_;
	cfg["current_tod"] = currentTime_;

	std::vector<time_of_day>::const_iterator t;
	for(t = times_.begin(); t != times_.end(); ++t) {
		t->write(cfg.add_child("time"));
	}
	for(std::vector<area_time_of_day>::const_iterator i = areas_.begin(); i != areas_.end(); ++i) {
		config& area = cfg.add_child("time_area");
		// if no ranges, then use hexes to generate ranges
		if(i->xsrc.empty() && i->ysrc.empty()) {
			write_location_range(i->hexes, area);
		} else {
			area["x"] = i->xsrc;
			area["y"] = i->ysrc;
		}
		for(t = i->times.begin(); t != i->times.end(); ++t) {
			t->write(area.add_child("time"));
		}
		area["current_time"] = i->currentTime;
	}
	return cfg;
}

const time_of_day& tod_manager::get_previous_time_of_day() const
{
	return get_time_of_day_turn(times_, turn_ - 1, currentTime_);
}

const time_of_day& tod_manager::get_time_of_day(const map_location& loc, int n_turn) const
{
	if(n_turn == 0)
		n_turn = turn_;

	for (std::vector<area_time_of_day>::const_reverse_iterator
		 i = areas_.rbegin(), i_end = areas_.rend(); i != i_end; ++i)
	{
		if (i->hexes.find(loc) != i->hexes.end())
			return get_time_of_day_turn(i->times, n_turn, i->currentTime);
	}

	return get_time_of_day_turn(times_, n_turn, currentTime_);
}

const time_of_day tod_manager::get_illuminated_time_of_day(const map_location& loc, int for_turn) const
{
	// get ToD ignoring illumination
	time_of_day tod = get_time_of_day(loc, for_turn);

	// now add illumination
	const gamemap& map = *resources::game_map;
	const unit_map& units = *resources::units;
	int light_modif =  map.get_terrain_info(map.get_terrain(loc)).light_modification();

	int light = tod.lawful_bonus + light_modif;
	int illum_light = light;

	if(loc.valid()) {
		map_location locs[7];
		locs[0] = loc;
		get_adjacent_tiles(loc,locs+1);

		for(int i = 0; i != 7; ++i) {
			const unit_map::const_iterator itor = units.find(locs[i]);
			if(itor != units.end() &&
			    itor->get_ability_bool("illuminates") &&
			    !itor->incapacitated())
			{
				unit_ability_list illum = itor->get_abilities("illuminates");
				unit_abilities::effect illum_effect(illum, light, false);

				illum_light = light + illum_effect.get_composite_value();
				//max_value and min_value control the final result
				//unless ToD + terrain effect is stronger
				int max = std::max(light, illum.highest("max_value").first);
				int min = std::min(light, illum.lowest("min_value").first);
				if(illum_light > max) {
					illum_light = max;
				} else if (illum_light < min) {
					illum_light = min;
				}

			}
		}
	}

	tod.bonus_modified = illum_light - tod.lawful_bonus;
	tod.lawful_bonus = illum_light;

	return tod;
}


bool tod_manager::is_start_ToD(const std::string& random_start_time)
{
	return !random_start_time.empty()
		&& utils::string_bool(random_start_time, true);
}

void tod_manager::replace_schedule(const config& time_cfg)
{
	times_.clear();
	time_of_day::parse_times(time_cfg,times_);
	currentTime_ = 0;
}

void tod_manager::add_time_area(const config& cfg)
{
	areas_.push_back(area_time_of_day());
	area_time_of_day &area = areas_.back();
	area.id = cfg["id"].str();
	area.xsrc = cfg["x"].str();
	area.ysrc = cfg["y"].str();
	area.currentTime = cfg["current_time"].to_int(0);
	std::vector<map_location> const& locs = parse_location_range(area.xsrc, area.ysrc, true);
	std::copy(locs.begin(), locs.end(), std::inserter(area.hexes, area.hexes.end()));
	time_of_day::parse_times(cfg, area.times);
}

void tod_manager::add_time_area(const std::string& id, const std::set<map_location>& locs,
		const config& time_cfg)
{
	areas_.push_back(area_time_of_day());
	area_time_of_day& area = areas_.back();
	area.id = id;
	area.hexes = locs;
	time_of_day::parse_times(time_cfg, area.times);
}

void tod_manager::remove_time_area(const std::string& area_id)
{
	if(area_id.empty()) {
		areas_.clear();
	} else {
		// search for all time areas that match the id.
		std::vector<area_time_of_day>::iterator i = areas_.begin();
		while(i != areas_.end()) {
			if((*i).id == area_id) {
				i = areas_.erase(i);
			} else {
				++i;
			}
		}
	}
}

int tod_manager::get_start_ToD(const config &level) const
{
	const config::attribute_value& current_tod = level["current_tod"];
	if (!current_tod.blank())
	{
		return calculate_current_time(times_.size(), turn_, current_tod.to_int(0), true);
	}

	const int default_result = calculate_current_time(times_.size(), turn_, currentTime_);

	const config::attribute_value& cfg_random_start_time = level["random_start_time"];
	if(!cfg_random_start_time.blank()) {
		const std::string& random_start_time = cfg_random_start_time.str();
		//TODO:
		//Here there is danger of OOS (bug #15948)
		//But this randomization is needed on the other hand to make the "random start time" option
		//in the mp game selection screen work.

		//process the random_start_time string, which can be boolean yes/no true/false or a
		//comma-separated string of integers >= 1 referring to the times_ array indices
		const std::vector<std::string>& random_start_time_strings = utils::split(random_start_time);
		const int random_index = calculate_current_time(random_start_time_strings.size(), turn_, rand(), true);
		const int given_current_time = lexical_cast_default<int, std::string>(random_start_time_strings[random_index], 0) - 1;
		if(given_current_time >= 0) return calculate_current_time(times_.size(), turn_, given_current_time, true);
		if(cfg_random_start_time.to_bool(false)) return calculate_current_time(times_.size(), turn_, rand(), true);
	}

	return default_result;
}

const time_of_day& tod_manager::get_time_of_day_turn(const std::vector<time_of_day>& times, int nturn, const int current_time) const
{
	const int time = calculate_current_time(times.size(), nturn, current_time);
	return times[time];
}

void tod_manager::modify_turns(const std::string& mod)
{
	num_turns_ = std::max<int>(utils::apply_modifier(num_turns_,mod,0),-1);
}
void tod_manager::set_number_of_turns(int num)
{
	num_turns_ = std::max<int>(num, -1);
}

void tod_manager::set_turn(const int num, const bool increase_limit_if_needed)
{
	const int new_turn = std::max<int>(num, 1);
	LOG_NG << "changing current turn number from " << turn_ << " to " << new_turn << '\n';
	// Correct ToD
	set_new_current_times(new_turn);

	if(increase_limit_if_needed && (new_turn > num_turns_) && num_turns_ != -1) {
		set_number_of_turns(new_turn);
	}
	turn_ = new_turn;
	resources::state_of_game->get_variable("turn_number") = new_turn;
}

void tod_manager::set_new_current_times(const int new_current_turn_number)
{
	currentTime_ = calculate_current_time(times_.size(), new_current_turn_number, currentTime_);
	BOOST_FOREACH(area_time_of_day& area, areas_) {
		area.currentTime = calculate_current_time(
			area.times.size(),
			new_current_turn_number,
			area.currentTime);
	}
}

int tod_manager::calculate_current_time(
	const int number_of_times,
	const int for_turn_number,
	const int current_time,
	const bool only_to_allowed_range) const
{
	int new_current_time = 0;
	if(only_to_allowed_range) new_current_time = current_time % number_of_times;
	else new_current_time = (current_time + for_turn_number - turn_) % number_of_times;
	while(new_current_time < 0) { new_current_time += number_of_times; }
	return new_current_time;
}

bool tod_manager::next_turn()
{
	set_turn(turn_ + 1, false);
	return is_time_left();
}


bool tod_manager::is_time_left()
{
	return num_turns_ == -1 || turn_ <= num_turns_;
}
