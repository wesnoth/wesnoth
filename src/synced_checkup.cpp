/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "synced_checkup.hpp"
#include "log.hpp"
#include "map_location.hpp"
#include "unit_map.hpp"
#include "unit.hpp"
#include "resources.hpp"
#include "game_display.hpp"
static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

ignored_checkup default_instnce;

checkup* checkup_instance = &default_instnce;

checkup::checkup()
{
}

checkup::~checkup()
{

}

void checkup::unit_checksum(const map_location& loc, bool local)
{
	unit_map::iterator u =  resources::units->find(loc);
	bool equals;
	config real;
	config expected;
	
	if (!u.valid()) {
		std::stringstream message;
		message << "non existent unit to checksum at " << loc.x+1 << "," << loc.y+1 << "!";
		resources::screen->add_chat_message(time(NULL), "verification", 1, message.str(),
				events::chat_handler::MESSAGE_PRIVATE, false);
	}
	else
	{
		expected["checksum"] = get_checksum(*u);
	}

	if(local)
	{
		equals = local_checkup(expected, real);
	}
	else
	{
		equals = this->networked_checkup(expected, real);
	}

	if(!equals && ((game_config::mp_debug && !local) || local))
	{
		std::stringstream message;
		message << "checksum mismatch at " << loc.x+1 << "," << loc.y+1 << "!";
		resources::screen->add_chat_message(time(NULL), "verification", 1, message.str(), 
			events::chat_handler::MESSAGE_PRIVATE, false);
	}
}


ignored_checkup::ignored_checkup()
{
}

ignored_checkup::~ignored_checkup()
{
}

bool ignored_checkup::local_checkup(const config& /*expected_data*/, config& real_data)
{
	assert(real_data.empty());
	LOG_REPLAY << "ignored_checkup::local_checkup called\n";
	return true;
}

bool ignored_checkup::networked_checkup(const config& /*expected_data*/, config& real_data)
{
	assert(real_data.empty());

	LOG_REPLAY << "ignored_checkup::networked_checkup called\n";
	return true;
}


synced_checkup::synced_checkup(config& buffer)
	: buffer_(buffer), pos_(0)
{
}

synced_checkup::~synced_checkup()
{
}

bool synced_checkup::local_checkup(const config& expected_data, config& real_data)
{
	assert(real_data.empty());
	if(buffer_.child_count("result") > pos_)
	{
		//copying objects :o
		real_data = buffer_.child("result",pos_);
		pos_ ++;
		return real_data == expected_data;
	}
	else
	{
		assert(buffer_.child_count("result") == pos_);
		buffer_.add_child("result", expected_data);
		pos_++;
		return true;
	}
}

bool synced_checkup::networked_checkup(const config& /*expected_data*/, config& real_data)
{
	assert(real_data.empty());
	throw "not implemented";
	//TODO: something with get_user_choice :).
}
