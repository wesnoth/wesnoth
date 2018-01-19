/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_end_exceptions.hpp"

#include "config.hpp"
#include "game_config.hpp"

transient_end_level::transient_end_level()
	: carryover_report(true)
	, linger_mode(true)
	, reveal_map(true)
{}

end_level_data::end_level_data()
	: prescenario_save(true)
	, replay_save(true)
	, proceed_to_next_level(false)
	, is_victory(true)
	, transient()
{
}

void end_level_data::write(config& cfg) const
{
	cfg["prescenario_save"] = prescenario_save;
	cfg["replay_save"] = replay_save;
	cfg["proceed_to_next_level"] = proceed_to_next_level;
	cfg["is_victory"] = is_victory;
}

void end_level_data::read(const config& cfg)
{
	prescenario_save = cfg["prescenario_save"].to_bool(true);
	replay_save = cfg["replay_save"].to_bool(true);
	proceed_to_next_level = cfg["proceed_to_next_level"].to_bool(true);
	is_victory = cfg["is_victory"].to_bool(true);
}

config end_level_data::to_config() const
{
	config r;
	write(r);
	return r;
}

config end_level_data::to_config_full() const
{
	config r;
	write(r);
	transient.write(r);
	return r;
}

void transient_end_level::write(config& cfg) const
{
	cfg["carryover_report"] = carryover_report;
	cfg["linger_mode"] = linger_mode;
	cfg["reveal_map"] = reveal_map;
}
