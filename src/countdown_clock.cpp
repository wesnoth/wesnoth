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

#include "countdown_clock.hpp"

#include "team.hpp"
#include "saved_game.hpp"
#include "preferences/general.hpp"
#include "sound.hpp"

namespace {
	const int WARNTIME = 20000; //start beeping when 20 seconds are left (20,000ms)
	unsigned timer_refresh = 0;
	const unsigned timer_refresh_rate = 50; // prevents calling SDL_GetTicks() too frequently
}


countdown_clock::countdown_clock(team& team)
	: team_(team)
	, last_timestamp_(SDL_GetTicks())
	, playing_sound_(false)
{
}


countdown_clock::~countdown_clock()
{
	if(playing_sound_)
	{
		sound::stop_bell();
	}
}

int countdown_clock::update_timestamp(int new_timestamp)
{
	int res = std::max<int>(0, new_timestamp - last_timestamp_);
	last_timestamp_ = new_timestamp;
	return res;
}

void countdown_clock::update_team(int new_timestamp)
{
	int time_passed = update_timestamp(new_timestamp);
	team_.set_countdown_time(std::max<int>(0, team_.countdown_time() - time_passed));
}

//make sure we think about countdown even while dialogs are open
void countdown_clock::process(events::pump_info &info)
{
	if(info.ticks(&timer_refresh, timer_refresh_rate)) {
		update(info.ticks());
	}
}

bool countdown_clock::update(int new_timestamp)
{
	update_team(new_timestamp);
	maybe_play_sound();
	return team_.countdown_time() > 0;
}

void countdown_clock::maybe_play_sound()
{
	if(!playing_sound_ && team_.countdown_time() < WARNTIME )
	{
		if(preferences::turn_bell() || preferences::sound_on() || preferences::UI_sound_on())
		{
			const int loop_ticks = team_.countdown_time();
			const int fadein_ticks = (loop_ticks > WARNTIME / 2) ? loop_ticks - WARNTIME / 2 : 0;
			sound::play_timer(game_config::sounds::timer_bell, loop_ticks, fadein_ticks);
			playing_sound_ = true;
		}
	}
}
