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
struct dismiss_action : undo_action
{
	unit_ptr dismissed_unit;

	explicit dismiss_action(const unit_const_ptr& dismissed);
	explicit dismiss_action(const config& cfg);

	static const char* get_type_impl() { return "dismiss"; }
	virtual const char* get_type() const { return get_type_impl(); }

	virtual ~dismiss_action() {}

	/** Writes this into the provided config. */
	virtual void write(config & cfg) const;

	/** Undoes this action. */
	virtual bool undo(int side);
};

}
