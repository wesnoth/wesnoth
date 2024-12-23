/*
	Copyright (C) 2009 - 2024
	by Eugen Jiresch
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "tod_manager.hpp"

#include "actions/attack.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "random.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "units/abilities.hpp"
#include "units/unit.hpp"
#include "units/unit_alignments.hpp"
#include "utils/general.hpp"

#include <algorithm>
#include <iterator>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

tod_manager::tod_manager(const config& scenario_cfg)
	: currentTime_(0)
	, times_()
	, areas_()
	, liminal_bonus_(25)
	, turn_(scenario_cfg["turn_at"].to_int(1))
	, num_turns_(scenario_cfg["turns"].to_int(-1))
	, has_turn_event_fired_(!scenario_cfg["it_is_a_new_turn"].to_bool(true))
	, has_tod_bonus_changed_(false)
	, has_cfg_liminal_bonus_(false)
{
	// ? : operator doesn't work in this case.
	if(scenario_cfg["current_time"].to_int(-17403) == -17403) {
		random_tod_ = scenario_cfg["random_start_time"];
	} else {
		random_tod_ = false;
	}

	time_of_day::parse_times(scenario_cfg, times_);
	liminal_bonus_ = std::max(25, calculate_best_liminal_bonus(times_));

	if(scenario_cfg.has_attribute("liminal_bonus")) {
		liminal_bonus_ = scenario_cfg["liminal_bonus"].to_int(liminal_bonus_);
		has_cfg_liminal_bonus_ = true;
	}

	// We need to call parse_times before fix_time_index because otherwise the first parameter will always be 0.
	currentTime_ = fix_time_index(times_.size(), scenario_cfg["current_time"].to_int(0));
}

void tod_manager::resolve_random(randomness::rng& r)
{
	// process the random_start_time string, which can be boolean yes/no true/false or a
	// comma-separated string of integers >= 1 referring to the times_ array indices
	std::vector<std::string> output_strings = utils::split(random_tod_.str());
	std::vector<int> output;

	try {
		std::transform(output_strings.begin(), output_strings.end(), std::back_inserter(output),
			[](const std::string& str) { return std::stoi(str); });
	} catch(const std::invalid_argument&) {
		// This happens if the random_start_time string is a boolean.
		// Simply ignore the exception.
	}

	// Remove non-positive times
	utils::erase_if(output, [](int time) { return time <= 0; });

	if(!output.empty()) {
		int chosen = output[r.next_random() % output.size()];
		currentTime_ = fix_time_index(times_.size(), chosen);
		r.next_random();
	} else if(random_tod_.to_bool(false)) {
		currentTime_ = fix_time_index(times_.size(), r.next_random());
	}

	random_tod_ = false;
}

config tod_manager::to_config(const std::string& textdomain) const
{
	config cfg;
	cfg["turn_at"] = turn_;
	cfg["turns"] = num_turns_;

	// this 'if' is for the editor.
	if(times_.size() != 0) {
		cfg["current_time"] = currentTime_;
	}

	cfg["random_start_time"] = random_tod_;
	cfg["it_is_a_new_turn"] = !has_turn_event_fired_;

	if(has_cfg_liminal_bonus_) {
		cfg["liminal_bonus"] = liminal_bonus_;
	}

	for(const time_of_day& tod : times_) {
		// Don't write stub ToD
		if(tod.id != "nulltod") {
			tod.write(cfg.add_child("time"), textdomain);
		}
	}

	for(const area_time_of_day& a_tod : areas_) {
		config& area = cfg.add_child("time_area");

		// If no ranges, then use hexes to generate ranges
		if(a_tod.xsrc.empty() && a_tod.ysrc.empty()) {
			write_location_range(a_tod.hexes, area);
		} else {
			area["x"] = a_tod.xsrc;
			area["y"] = a_tod.ysrc;
		}

		for(const time_of_day& tod : a_tod.times) {
			// Don't write the stub default ToD if it happens to be present.
			if(tod.id != "nulltod") {
				tod.write(area.add_child("time"), textdomain);
			}
		}

		area["current_time"] = a_tod.currentTime;

		if(!a_tod.id.empty()) {
			area["id"] = a_tod.id;
		}
	}

	return cfg;
}

static const time_of_day& dummytime()
{
	static time_of_day* pdummy = new time_of_day();
	return *pdummy;
}

const time_of_day& tod_manager::get_previous_time_of_day() const
{
	return get_time_of_day_turn(times_, turn_ - 1, currentTime_);
}

int tod_manager::get_current_area_time(int index) const
{
	assert(index < static_cast<int>(areas_.size()));
	return areas_[index].currentTime;
}

int tod_manager::get_current_time(const map_location& loc) const
{
	if(loc != map_location::null_location()) {
		for(auto i = areas_.rbegin(), i_end = areas_.rend();
			i != i_end; ++i) {
			if(i->hexes.find(loc) != i->hexes.end()) {
				return i->currentTime;
			}
		}
	}

	return currentTime_;
}

const std::vector<time_of_day>& tod_manager::times(const map_location& loc) const
{
	if(loc != map_location::null_location()) {
		for(auto i = areas_.rbegin(), i_end = areas_.rend();
			i != i_end; ++i) {
			if(i->hexes.find(loc) != i->hexes.end() && !i->times.empty())
				return i->times;
		}
	}

	return times_;
}

const time_of_day& tod_manager::get_time_of_day(const map_location& loc, int n_turn) const
{
	if(n_turn == 0) {
		n_turn = turn_;
	}

	if(loc != map_location::null_location()) {
		for(auto i = areas_.rbegin(), i_end = areas_.rend();
			i != i_end; ++i) {
			if(i->hexes.find(loc) != i->hexes.end() && !i->times.empty())
				return get_time_of_day_turn(i->times, n_turn, i->currentTime);
		}
	}

	return get_time_of_day_turn(times_, n_turn, currentTime_);
}

const time_of_day& tod_manager::get_area_time_of_day(int area_i, int n_turn) const
{
	assert(area_i < static_cast<int>(areas_.size()));
	if(n_turn == 0) {
		n_turn = turn_;
	}
	return get_time_of_day_turn(areas_[area_i].times, n_turn, areas_[area_i].currentTime);
}

const time_of_day tod_manager::get_illuminated_time_of_day(
	const unit_map& units, const gamemap& map, const map_location& loc, int for_turn) const
{
	// get ToD ignoring illumination
	time_of_day tod = get_time_of_day(loc, for_turn);

	if(map.on_board_with_border(loc)) {
		// Now add terrain illumination.
		const int terrain_light = map.get_terrain_info(loc).light_bonus(tod.lawful_bonus);

		std::vector<int> mod_list;
		std::vector<int> max_list;
		std::vector<int> min_list;
		int most_add = 0;
		int most_sub = 0;

		// Find the "illuminates" effects from units that can affect loc.
		std::array<map_location, 7> locs;
		locs[0] = loc;
		get_adjacent_tiles(loc, locs.data() + 1); // start at [1]

		for(std::size_t i = 0; i < locs.size(); ++i) {
			const auto itor = units.find(locs[i]);
			if(itor != units.end() && !itor->incapacitated()) {
				unit_ability_list illum = itor->get_abilities("illuminates");
				if(!illum.empty()) {
					unit_abilities::effect illum_effect(illum, terrain_light, nullptr, unit_abilities::EFFECT_WITHOUT_CLAMP_MIN_MAX);
					const int unit_mod = illum_effect.get_composite_value();

					// Record this value.
					mod_list.push_back(unit_mod);
					max_list.push_back(illum.highest("max_value").first);
					min_list.push_back(illum.lowest("min_value").first);

					if(unit_mod > most_add) {
						most_add = unit_mod;
					} else if(unit_mod < most_sub) {
						most_sub = unit_mod;
					}
				}
			}
		}
		const bool net_darker = most_add < -most_sub;

		// Apply each unit's effect, tracking the best result.
		int best_result = terrain_light;
		const int base_light = terrain_light + (net_darker ? most_add : most_sub);

		for(std::size_t i = 0; i != mod_list.size(); ++i) {
			int result = bounded_add(base_light, mod_list[i], max_list[i], min_list[i]);

			if(net_darker && result < best_result) {
				best_result = result;
			} else if(!net_darker && result > best_result) {
				best_result = result;
			}
		}

		// Update the object we will return.
		tod.bonus_modified = best_result - tod.lawful_bonus;
		tod.lawful_bonus = best_result;
	}

	return tod;
}

bool tod_manager::is_start_ToD(const std::string& random_start_time)
{
	return !random_start_time.empty() && utils::string_bool(random_start_time, true);
}

void tod_manager::replace_schedule(const config& time_cfg)
{
	std::vector<time_of_day> new_scedule;
	time_of_day::parse_times(time_cfg, new_scedule);
	replace_schedule(new_scedule, time_cfg["current_time"].to_int(0));
}

void tod_manager::replace_schedule(const std::vector<time_of_day>& schedule, int initial_time)
{
	if(times_.empty() || schedule.empty() || times_[currentTime_].lawful_bonus != schedule.front().lawful_bonus) {
		has_tod_bonus_changed_ = true;
	}

	times_ = schedule;
	currentTime_ = initial_time;
}

void tod_manager::replace_area_locations(int area_index, const std::set<map_location>& locs)
{
	assert(area_index < static_cast<int>(areas_.size()));
	areas_[area_index].hexes = locs;
	has_tod_bonus_changed_ = true;
}

void tod_manager::replace_local_schedule(const std::vector<time_of_day>& schedule, int area_index, int initial_time)
{
	assert(area_index < static_cast<int>(areas_.size()));
	area_time_of_day& area = areas_[area_index];

	if(area.times.empty() || schedule.empty()) {
		// If one of those is empty then their 'previous' time of day might depend on other areas_,
		// its better to just assume the illumination has changes than to do the explicit computation.
		has_tod_bonus_changed_ = true;
	} else if(area.times[area.currentTime].lawful_bonus != schedule.front().lawful_bonus) {
		// the current illumination on these tiles has changes.
		has_tod_bonus_changed_ = true;
	}

	area.times = schedule;
	area.currentTime = initial_time;
}

void tod_manager::set_area_id(int area_index, const std::string& id)
{
	assert(area_index < static_cast<int>(areas_.size()));
	areas_[area_index].id = id;
}

const std::string& tod_manager::get_area_id(int area_index) const
{
	assert(area_index < static_cast<int>(areas_.size()));
	return areas_[area_index].id;
}

std::vector<std::string> tod_manager::get_area_ids() const
{
	std::vector<std::string> areas;
	for(const area_time_of_day& area : areas_) {
		areas.push_back(area.id);
	}

	return areas;
}

const std::set<map_location>& tod_manager::get_area_by_id(const std::string& id) const
{
	for(const area_time_of_day& area : areas_) {
		if(area.id == id) {
			return area.hexes;
		}
	}

	static const std::set<map_location> res;
	return res;
}

const std::set<map_location>& tod_manager::get_area_by_index(int index) const
{
	return areas_[index].hexes;
}

std::pair<int, std::string> tod_manager::get_area_on_hex(const map_location& loc) const
{
	if(loc != map_location::null_location()) {
		for(auto i = areas_.rbegin(), i_end = areas_.rend();
			i != i_end; ++i) {
			if(i->hexes.find(loc) != i->hexes.end() && !i->times.empty())
				return {std::distance(areas_.rbegin(), i), i->id};
		}
	}

	return {-1, ""};
}

void tod_manager::add_time_area(const gamemap& map, const config& cfg)
{
	areas_.emplace_back();
	area_time_of_day& area = areas_.back();
	area.id = cfg["id"].str();
	area.xsrc = cfg["x"].str();
	area.ysrc = cfg["y"].str();
	area.currentTime = cfg["current_time"].to_int(0);
	const std::vector<map_location>& locs(map.parse_location_range(area.xsrc, area.ysrc, true));
	area.hexes.insert(locs.begin(), locs.end());
	time_of_day::parse_times(cfg, area.times);
	has_tod_bonus_changed_ = true;
}

void tod_manager::add_time_area(const std::string& id, const std::set<map_location>& locs, const config& time_cfg)
{
	areas_.emplace_back();
	area_time_of_day& area = areas_.back();
	area.id = id;
	area.hexes = locs;
	area.currentTime = time_cfg["current_time"].to_int(0);
	time_of_day::parse_times(time_cfg, area.times);
	has_tod_bonus_changed_ = true;
}

void tod_manager::remove_time_area(const std::string& area_id)
{
	if(area_id.empty()) {
		areas_.clear();
	} else {
		// search for all time areas that match the id.
		auto i = areas_.begin();
		while(i != areas_.end()) {
			if((*i).id == area_id) {
				i = areas_.erase(i);
			} else {
				++i;
			}
		}
	}

	has_tod_bonus_changed_ = true;
}

void tod_manager::remove_time_area(int area_index)
{
	assert(area_index < static_cast<int>(areas_.size()));
	areas_.erase(areas_.begin() + area_index);
	has_tod_bonus_changed_ = true;
}

const time_of_day& tod_manager::get_time_of_day_turn(
	const std::vector<time_of_day>& times, int nturn, const int current_time) const
{
	if(times.empty()) {
		return dummytime();
	}

	const int time = calculate_time_index_at_turn(times.size(), nturn, current_time);
	return times[time];
}

void tod_manager::modify_turns(const std::string& mod)
{
	num_turns_ = std::max<int>(utils::apply_modifier(num_turns_, mod, 0), -1);
}

void tod_manager::set_number_of_turns(int num)
{
	num_turns_ = std::max<int>(num, -1);
}

void tod_manager::update_server_information() const
{
	if(resources::controller->current_team().is_local()) {
		// The currently active side informs the mp server about the turn change.
		// NOTE: The current implementation does not guarantee that the server gets informed
		// about those changes in 100% of cases. But that is ok because the information is only
		// used to display the turn limit in the lobby (as opposed to things that cause OOS).
		resources::controller->send_to_wesnothd(config {
			"change_turns_wml", config {
				"current", turn_,
				"max", num_turns_,
			},
		});
	}
}

void tod_manager::modify_turns_by_wml(const std::string& mod)
{
	modify_turns(mod);
	update_server_information();
}

void tod_manager::set_number_of_turns_by_wml(int num)
{
	set_number_of_turns(num);
	update_server_information();
}

void tod_manager::set_turn(const int num, game_data* vars, const bool increase_limit_if_needed)
{
	has_tod_bonus_changed_ = false;
	const int new_turn = std::max<int>(num, 1);
	LOG_NG << "changing current turn number from " << turn_ << " to " << new_turn;

	// Correct ToD
	set_new_current_times(new_turn);

	if(increase_limit_if_needed && (new_turn > num_turns_) && num_turns_ != -1) {
		set_number_of_turns(new_turn);
	}

	turn_ = new_turn;

	if(vars) {
		vars->get_variable("turn_number") = new_turn;
	}
}

void tod_manager::set_turn_by_wml(const int num, game_data* vars, const bool increase_limit_if_needed)
{
	set_turn(num, vars, increase_limit_if_needed);
	update_server_information();
}

void tod_manager::set_new_current_times(const int new_current_turn_number)
{
	set_current_time(calculate_time_index_at_turn(times_.size(), new_current_turn_number, currentTime_));

	for(area_time_of_day& area : areas_) {
		set_current_time(calculate_time_index_at_turn(area.times.size(), new_current_turn_number, area.currentTime), area);
	}
}

int tod_manager::fix_time_index(int number_of_times, int time)
{
	if(number_of_times == 0) {
		return 0;
	}

	return modulo(time, number_of_times);
}

int tod_manager::calculate_time_index_at_turn(int number_of_times, int for_turn_number, int current_time) const
{
	if(number_of_times == 0) {
		return 0;
	}

	return modulo(current_time + for_turn_number - turn_, number_of_times);
}

void tod_manager::set_current_time(int time)
{
	time = fix_time_index(times_.size(), time);
	if(!times_.empty() && times_[time].lawful_bonus != times_[currentTime_].lawful_bonus) {
		has_tod_bonus_changed_ = true;
	}

	currentTime_ = time;
}

void tod_manager::set_current_time(int time, int area_index)
{
	assert(area_index < static_cast<int>(areas_.size()));
	set_current_time(time, areas_[area_index]);
}

void tod_manager::set_current_time(int time, const std::string& area_id)
{
	for(area_time_of_day& area : areas_) {
		if(area.id == area_id) {
			set_current_time(time, area);
		}
	}
}

void tod_manager::set_current_time(int time, area_time_of_day& area)
{
	time = fix_time_index(area.times.size(), time);
	if(area.times[time].lawful_bonus != area.times[area.currentTime].lawful_bonus) {
		has_tod_bonus_changed_ = true;
	}

	area.currentTime = time;
}

bool tod_manager::next_turn(game_data* vars)
{
	set_turn(turn_ + 1, vars, false);
	has_turn_event_fired_ = false;
	return is_time_left();
}

bool tod_manager::is_time_left() const
{
	return num_turns_ == -1 || turn_ <= num_turns_;
}

int tod_manager::calculate_best_liminal_bonus(const std::vector<time_of_day>& schedule) const
{
	int fearless_chaotic = 0;
	int fearless_lawful = 0;

	std::set<int> bonuses;
	for(const auto& tod : schedule) {
		fearless_chaotic += generic_combat_modifier(tod.lawful_bonus, unit_alignments::type::chaotic, true, 0);
		fearless_lawful += generic_combat_modifier(tod.lawful_bonus, unit_alignments::type::lawful, true, 0);
		bonuses.insert(std::abs(tod.lawful_bonus));
	}

	int target = std::max(fearless_chaotic, fearless_lawful);
	int delta = target;
	int result = 0;

	for(int bonus : bonuses) {
		int liminal_effect = 0;
		for(const auto& tod : schedule) {
			liminal_effect += generic_combat_modifier(tod.lawful_bonus, unit_alignments::type::liminal, false, bonus);
		}

		if(std::abs(target - liminal_effect) < delta) {
			result = bonus;
			delta = std::abs(target - liminal_effect);
		}
	}

	return result;
}
