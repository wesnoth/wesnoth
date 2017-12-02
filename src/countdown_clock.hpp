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
#include "events.hpp"
#include <SDL_timer.h>

class team;
class countdown_clock : public events::pump_monitor
{
public:
	countdown_clock(team& team);
	~countdown_clock();
	/// @returns ticks passed since last update
	/// @param new_timestamp latest result of SDL_GetTicks()
	int update_timestamp(int new_timestamp);
	/// @param new_timestamp latest result of SDL_GetTicks()
	void update_team(int new_timestamp);
	void process(events::pump_info &info);
	///	@return whether there is time left
	/// @param new_timestamp latest result of SDL_GetTicks()
	bool update(int new_timestamp = SDL_GetTicks());
	void maybe_play_sound();
private:
	team& team_;
	int last_timestamp_;
	bool playing_sound_;
};

