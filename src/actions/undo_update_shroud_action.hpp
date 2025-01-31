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

#include "undo_action.hpp"

namespace actions::undo
{
struct auto_shroud_action : undo_action {
	bool active;

	explicit auto_shroud_action(bool turned_on)
		: undo_action()
		, active(turned_on)
	{
	}
	explicit auto_shroud_action(const config& cfg)
		: undo_action()
		, active(cfg["active"].to_bool())
	{
	}

	static const char* get_type_impl() { return "auto_shroud"; }
	virtual const char* get_type() const { return get_type_impl(); }

	virtual bool undo(int);

	virtual ~auto_shroud_action()
	{
	}

	/** Writes this into the provided config. */
	virtual void write(config & cfg) const;
};

}
