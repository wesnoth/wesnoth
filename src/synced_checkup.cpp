/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
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
#include "replay.hpp"
#include "synced_user_choice.hpp"

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

static ignored_checkup default_instnce;

checkup* checkup_instance = &default_instnce;

checkup::checkup()
{
}

checkup::~checkup()
{

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


namespace
{
	struct checkup_choice : public mp_sync::user_choice
	{
		checkup_choice(const config& cfg) : cfg_(cfg)
		{
		}
		virtual ~checkup_choice()
		{
		}
		virtual config random_choice(int /*side*/) const override
		{
			throw "not implemented";
		}
		virtual bool is_visible() const override
		{
			return false;
		}
		virtual config query_user(int /*side*/) const override
		{
			return cfg_;
		}
		const config& cfg_;
	};
}

mp_debug_checkup::mp_debug_checkup()
{
}

mp_debug_checkup::~mp_debug_checkup()
{
}

bool mp_debug_checkup::local_checkup(const config& expected_data, config& real_data)
{
	assert(real_data.empty());
	real_data = get_user_choice("mp_checkup", checkup_choice(expected_data));
	return real_data == expected_data;
}
