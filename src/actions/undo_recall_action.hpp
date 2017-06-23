/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "undo_action.hpp"
#include "shroud_clearing_action.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"

namespace actions
{
namespace undo
{


struct recall_action : undo_action, shroud_clearing_action
{
	std::string id;
	map_location recall_from;


	recall_action(const unit_const_ptr recalled, const map_location& loc,
	              const map_location& from, int orig_village_owner, bool time_bonus)
		: undo_action()
		, shroud_clearing_action(recalled, loc, orig_village_owner, time_bonus)
		, id(recalled->id())
		, recall_from(from)
	{
	}
	recall_action(const config & cfg, const map_location & from)
		: undo_action(cfg)
		, shroud_clearing_action(cfg)
		, id(cfg["id"])
		, recall_from(from)
	{}
	virtual const char* get_type() const { return "recall"; }
	virtual ~recall_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side);
};

}
}
