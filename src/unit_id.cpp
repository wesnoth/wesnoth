/*
   Copyright (C) 2008 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log.hpp"
#include "unit_id.hpp"

#include <cassert>

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)

// The following line sets the value to less than maximum of size_t,
// but is required since config can't hold size_t and so whiteboard
// chops it anyway during serialization to config, leading to later
// errors (and some slowdown).
// Setting the initial value to 2^32-1 is a safe and easy way to fix this.
static const size_t INITIAL_FAKE_ID = 4294967295u;

namespace n_unit {
	id_manager id_manager::manager_;

	id_manager::id_manager() : next_id_(0), fake_id_(INITIAL_FAKE_ID)
	{}

	id_manager& id_manager::instance()
	{
		return manager_;
	}

	size_t id_manager::next_id()
	{
		assert(next_id_ != fake_id_);
		DBG_UT << "id: " << next_id_ << "\n";
		return ++next_id_;
	}

	size_t id_manager::next_fake_id()
	{
		assert(next_id_ != fake_id_);
		DBG_UT << "fake id: " << fake_id_ << "\n";
		return --fake_id_;
	}

	size_t id_manager::get_save_id()
	{
		return next_id_;
	}

	void id_manager::set_save_id(size_t id)
	{
		clear();
		DBG_UT << "set save id: " << id << "\n";
		next_id_ = id;
	}

	void id_manager::reset_fake()
	{
		fake_id_ = INITIAL_FAKE_ID;
	}

	void id_manager::clear()
	{
		next_id_ = 0;
		reset_fake();
	}
}
