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

#pragma once

#include "events.hpp"
#include <chrono>

class team;
class countdown_clock : public events::pump_monitor
{
	using clock = std::chrono::steady_clock;

public:
	countdown_clock(team& team);
	~countdown_clock();

	/** @returns ticks passed since last update */
	std::chrono::milliseconds update_timestamp();

	void update_team();

	/** Inherited from pump_monitor */
	virtual void process() override;

	/** @returns whether there is time left */
	bool update();

	void maybe_play_sound();

private:
	team& team_;
	clock::time_point last_timestamp_;
	bool playing_sound_;
};
