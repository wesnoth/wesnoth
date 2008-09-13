/* $Id$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log.hpp"
#include "unit_id.hpp"

#define DBG_UT LOG_STREAM(debug, engine)

namespace n_unit {
	id_manager id_manager::manager_;

	id_manager::id_manager() : next_id_(0)
	{}

	id_manager& id_manager::instance()
	{
		return manager_;
	}

	size_t id_manager::next_id()
	{
		DBG_UT << "id: " << next_id_ << "\n";
		return ++next_id_;
	}

	size_t id_manager::get_save_id()
	{
		return next_id_;
	}

	void id_manager::set_save_id(size_t id)
	{
		DBG_UT << "set save id: " << id << "\n";
		next_id_ = id;
	}

	void id_manager::clear()
	{
		next_id_ = 0;
	}
}
