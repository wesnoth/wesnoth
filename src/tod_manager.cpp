/*
   Copyright (C) 2009 by Eugen Jiresch
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

tod_manager::tod_manager(const config& time_cfg):
	turn_(1),
	currentTime_(0)
{
	std::string turn_at = time_cfg["turn_at"];
	std::string current_tod = time_cfg["current_tod"];
	std::string random_start_time = time_cfg["random_start_time"];

	if(turn_at.empty() == false) {
		turn_ = atoi(turn_at.c_str());
	}

	time_of_day::parse_times(time_cfg,times_);

	set_start_ToD(const_cast<config&>(time_cfg),NULL);

	foreach (const config &t, time_cfg.child_range("time_area")) {
		this->add_time_area(t);
	}
}

config tod_manager::toConfig()
{
	//FIXME: dummy function
	return config();
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
	return get_time_of_day_turn(turn_-1);
}

time_of_day tod_manager::get_time_of_day(int illuminated, const map_location& loc, int n_turn) const
{
	time_of_day res = get_time_of_day_turn(n_turn);

	if(loc.valid()) {
		for(std::vector<area_time_of_day>::const_iterator i = areas_.begin(); i != areas_.end(); ++i) {
			if(i->hexes.count(loc) == 1) {

				VALIDATE(i->times.size(), _("No time of day has been defined."));

				res = i->times[(n_turn-1)%i->times.size()];
				break;
			}
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
	return get_time_of_day(illuminated,loc,turn_);
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

void tod_manager::set_start_ToD(config &level, game_state* s_o_g)
{
	if (!level["current_tod"].empty())
	{
		set_time_of_day(atoi(level["current_tod"].c_str()));
		return;
	}
	std::string random_start_time = level["random_start_time"];
	if (s_o_g)
	{
		random_start_time = utils::interpolate_variables_into_string(random_start_time, *s_o_g);
	}
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

		set_time_of_day((turn_ - 1) % times_.size());
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
