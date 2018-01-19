/*
   Copyright (C) 2017-2018 the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "units/ptr.hpp"

namespace actions
{
namespace undo
{

struct auto_shroud_action : undo_action_base {
	bool active;

	explicit auto_shroud_action(bool turned_on)
		: undo_action_base()
		, active(turned_on)
	{}
	virtual const char* get_type() const { return "auto_shroud"; }
	virtual ~auto_shroud_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;
};

struct update_shroud_action : undo_action_base {
	// No additional data.

	update_shroud_action()
		: undo_action_base()
	{}
	virtual const char* get_type() const { return "update_shroud"; }
	virtual ~update_shroud_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;
};

}
}
