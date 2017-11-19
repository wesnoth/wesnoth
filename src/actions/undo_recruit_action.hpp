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
#include "units/types.hpp"

namespace actions
{
namespace undo
{

struct recruit_action : undo_action, shroud_clearing_action
{
	const unit_type & u_type;
	map_location recruit_from;


	recruit_action(const unit_const_ptr recruited, const map_location& loc,
	               const map_location& from, int orig_village_owner, bool time_bonus);
	recruit_action(const config & cfg, const unit_type & type, const map_location& from);
	virtual const char* get_type() const { return "recruit"; }
	virtual ~recruit_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side);
};

}
}
