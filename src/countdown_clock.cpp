/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "preferences/preferences.hpp"
#include "sound.hpp"
#include "utils/rate_counter.hpp"

using namespace std::chrono_literals;

namespace {
	constexpr auto warn_threshold = 20s; // start beeping when 20 seconds are left
	constexpr auto fade_end = warn_threshold / 2;
	utils::rate_counter timer_refresh_rate{50};
}

countdown_clock::countdown_clock(team& team)
	: team_(team)
	, last_timestamp_(clock::now())
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

std::chrono::milliseconds countdown_clock::update_timestamp()
{
	auto now = clock::now();
	auto prev_time = std::exchange(last_timestamp_, now);
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_time);
}

void countdown_clock::update_team()
{
	auto time_passed = update_timestamp();
	team_.set_countdown_time(std::max(0ms, team_.countdown_time() - time_passed));
}

//make sure we think about countdown even while dialogs are open
void countdown_clock::process()
{
	if(timer_refresh_rate.poll()) {
		update();
	}
}

bool countdown_clock::update()
{
	update_team();
	maybe_play_sound();
	return team_.countdown_time() > 0ms;
}

void countdown_clock::maybe_play_sound()
{
	if(!playing_sound_ && team_.countdown_time() < warn_threshold )
	{
		if(prefs::get().turn_bell() || prefs::get().sound() || prefs::get().ui_sound_on())
		{
			const auto loop_ticks = team_.countdown_time();
			const auto fadein_ticks = std::max(loop_ticks - fade_end, 0ms);
			sound::play_timer(game_config::sounds::timer_bell, loop_ticks, fadein_ticks);
			playing_sound_ = true;
		}
	}
}
