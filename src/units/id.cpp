/*
   Copyright (C) 2008 - 2017 by David White <dave@whitevine.net>
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
#include "units/id.hpp"

#include <cassert>

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)

namespace n_unit
{
	id_manager id_manager::manager_(0);

	unit_id id_manager::next_id()
	{
		assert(next_id_ < unit_id::highest_bit);
		DBG_UT << "id: " << next_id_ << "\n";
		return unit_id::create_real(++next_id_);
	}

	unit_id id_manager::next_fake_id()
	{
		assert(fake_id_ < unit_id::highest_bit);
		DBG_UT << "fake id: " << fake_id_ << "\n";
		return unit_id::create_fake(++fake_id_);
	}

	size_t id_manager::get_save_id() const
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
		fake_id_ = 0;
	}

	void id_manager::clear()
	{
		next_id_ = 0;
		reset_fake();
	}
}
