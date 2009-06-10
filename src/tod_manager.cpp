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
#include "log.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

tod_manager::tod_manager(const config& time_cfg, int num_turns, game_state* state):
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

	set_start_ToD(const_cast<config&>(time_cfg), turn_, NULL);

	foreach (const config &t, time_cfg.child_range("time_area")) {
		this->add_time_area(t);
	}
}

config tod_manager::to_config()
{
	config cfg;
	std::stringstream buf;
	buf << currentTime_;
	cfg["current_tod"] = buf.str();

	std::vector<time_of_day>::const_iterator t;
	for(t = times_.begin(); t != times_.end(); ++t) {
		t->write(cfg.add_child("time"));
	}
	for(std::vector<area_time_of_day>::const_iterator i = areas_.begin(); i != areas_.end(); ++i) {
		config& area = cfg.add_child("time_area");
		area["x"] = i->xsrc;
		area["y"] = i->ysrc;
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
	return get_time_of_day_turn(0, 1);
}

time_of_day tod_manager::get_time_of_day(int illuminated, const map_location& loc, int n_turn, int current_turn) const
{
	time_of_day res = get_time_of_day_turn(n_turn, current_turn);

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

time_of_day tod_manager::get_time_of_day(int illuminated, const map_location& loc, int current_turn) const
{
	return get_time_of_day(illuminated,loc,current_turn);
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

void tod_manager::set_start_ToD(config &level, int current_turn, game_state* s_o_g)
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

		set_time_of_day((current_turn - 1) % times_.size());
	}
	// Setting ToD to level data

	std::stringstream buf;
	buf << currentTime_;
	level["current_tod"] = buf.str();

}

time_of_day tod_manager::get_time_of_day_turn(int nturn, int current_turn) const
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	int time = (currentTime_ + nturn  - current_turn)% times_.size();

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
	int current_time = (num  - 1) % times_.size();
	if (current_time < 0) {
		current_time += times_.size();
	}
	set_time_of_day(current_time);

	if(static_cast<int>(num) > num_turns_ && num_turns_ != -1) {
		this->add_turns(num_turns_ - num);
	}
	turn_ = num;

	LOG_NG << "changed current turn number from " << old_num << " to " << num << '\n';
}

bool tod_manager::next_turn()
{
	next_time_of_day();
	++turn_;
	return num_turns_ == -1 || turn_ <= size_t(num_turns_);
}