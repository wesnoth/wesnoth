/*
   Copyright (C) 2009 - 2011 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
 */

#include "foreach.hpp"
#include "tod_manager.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"
#include "formula_string_utils.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_abilities.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

tod_manager::tod_manager(const config& time_cfg, int num_turns, game_state* state):
	savegame_config(),
	currentTime_(0),
	times_(),
	areas_(),
	turn_(1),
	num_turns_(num_turns)
{
	std::string turn_at = time_cfg["turn_at"];
	if (state)
	{
		turn_at = utils::interpolate_variables_into_string(turn_at, *state);

	}

	if(turn_at.empty() == false) {
		turn_ = atoi(turn_at.c_str());
	}

	time_of_day::parse_times(time_cfg,times_);

	set_start_ToD(const_cast<config&>(time_cfg), turn_);
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
	std::stringstream buf;
	buf << turn_;
	cfg["turn_at"] = buf.str();
	buf.str(std::string());
	buf << num_turns_;
	cfg["turns"] = buf.str();
	buf.str(std::string());
	buf << currentTime_;
	cfg["current_tod"] = buf.str();

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
	}
	return cfg;
}

time_of_day tod_manager::get_time_of_day() const
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	return times_[currentTime_];
}

bool tod_manager::set_time_of_day(int newTime)
{
	// newTime can come from network so have to take run time test
	if( newTime >= static_cast<int>(times_.size())
			|| newTime < 0)
	{
		return false;
	}

	currentTime_ = newTime;

	return true;
}

time_of_day tod_manager::get_previous_time_of_day() const
{
	return get_time_of_day_turn(turn() - 1);
}

time_of_day tod_manager::get_time_of_day(int illuminated, const map_location& loc, int n_turn) const
{
	time_of_day res = get_time_of_day_turn(n_turn);

	if (loc.valid()) {
		for (std::vector<area_time_of_day>::const_reverse_iterator
		     i = areas_.rbegin(), i_end = areas_.rend(); i != i_end; ++i)
		{
			if (i->hexes.count(loc) != 1) continue;
			VALIDATE(i->times.size(), _("No time of day has been defined."));
			res = i->times[(n_turn - 1) % i->times.size()];
			break;
		}
	}

	if(illuminated) {
		res.bonus_modified=illuminated;
		res.lawful_bonus += illuminated;
	}
	return res;
}

time_of_day tod_manager::get_time_of_day(int illuminated, const map_location& loc) const
{
	return get_time_of_day(illuminated, loc, turn());
}

bool tod_manager::is_start_ToD(const std::string& random_start_time)
{
	return !random_start_time.empty()
		&& utils::string_bool(random_start_time, true);
}

void tod_manager::add_time_area(const config& cfg)
{
	areas_.push_back(area_time_of_day());
	area_time_of_day &area = areas_.back();
	area.id   = cfg["id"];
	area.xsrc = cfg["x"];
	area.ysrc = cfg["y"];
	std::vector<map_location> const& locs = parse_location_range(area.xsrc, area.ysrc);
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

void tod_manager::set_start_ToD(config &level, int current_turn)
{
	if (!level["current_tod"].empty())
	{
		set_time_of_day(atoi(level["current_tod"].c_str()));
		return;
	}
	std::string random_start_time = level["random_start_time"];
	if (tod_manager::is_start_ToD(random_start_time))
	{
		std::vector<std::string> start_strings =
			utils::split(random_start_time, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);

		if (utils::string_bool(random_start_time,false))
		{
			// We had boolean value
			set_time_of_day(rand()%times_.size());
		}
		else
		{
			set_time_of_day(atoi(start_strings[rand()%start_strings.size()].c_str()) - 1);
		}
	}
	else
	{
		// We have to set right ToD for oldsaves

		set_time_of_day((current_turn - 1) % times_.size());
	}
	// Setting ToD to level data

	std::stringstream buf;
	buf << currentTime_;
	level["current_tod"] = buf.str();

}

time_of_day tod_manager::get_time_of_day_turn(int nturn) const
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	int time = (currentTime_ + nturn  - turn_)% times_.size();

	if (time < 0)
	{
		time += times_.size();
	}

	return times_[time];
}
void tod_manager::next_time_of_day()
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	currentTime_ = (currentTime_ + 1)%times_.size();
}

time_of_day tod_manager::time_of_day_at(const unit_map& units,const map_location& loc, const gamemap& map) const
{
	int lighten = std::max<int>(map.get_terrain_info(map.get_terrain(loc)).light_modification() , 0);
	int darken = std::min<int>(map.get_terrain_info(map.get_terrain(loc)).light_modification() , 0);

	time_of_day tod = get_time_of_day(lighten + darken,loc);

	if(loc.valid()) {
		map_location locs[7];
		locs[0] = loc;
		get_adjacent_tiles(loc,locs+1);

		for(int i = 0; i != 7; ++i) {
			const unit_map::const_iterator itor = units.find(locs[i]);
			if(itor != units.end() &&
			    itor->second.get_ability_bool("illuminates") &&
			    !itor->second.incapacitated())
			{
				unit_ability_list illum = itor->second.get_abilities("illuminates");
				unit_abilities::effect illum_effect(illum,lighten,false);
				int mod = illum_effect.get_composite_value();
				if(mod + tod.lawful_bonus > illum.highest("max_value").first) {
					mod = illum.highest("max_value").first - tod.lawful_bonus;
				}
				lighten = std::max<int>(mod, lighten);
				darken = std::min<int>(mod, darken);
			}
		}
	}
	tod = get_time_of_day(lighten + darken,loc);

	return tod;
}

void tod_manager::modify_turns(const std::string& mod)
{
	num_turns_ = std::max<int>(utils::apply_modifier(num_turns_,mod,0),-1);
}
void tod_manager::add_turns(int num)
{
	num_turns_ = std::max<int>(num_turns_ + num,-1);
}

void tod_manager::set_turn(unsigned int num)
{
	VALIDATE(times_.size(), _("No time of day has been defined."));
	const unsigned int old_num = turn_;
	// Correct ToD
	int current_time = (currentTime_ + num - old_num) % times_.size();
	if (current_time < 0) {
		current_time += times_.size();
	}
	set_time_of_day(current_time);

	if(static_cast<int>(num) > num_turns_ && num_turns_ != -1) {
		add_turns(num - num_turns_);
	}
	turn_ = num;

	LOG_NG << "changed current turn number from " << old_num << " to " << num << '\n';
}

bool tod_manager::next_turn()
{
	next_time_of_day();
	++turn_;
	return is_time_left();
}


bool tod_manager::is_time_left()
{
	return num_turns_ == -1 || turn_ <= size_t(num_turns_);
}
