/*
   Copyright (C) 2008 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Editor action classes. Some important points:
 * - This is a polymorphic hierarchy of classes, so actions are usually passed around
 *   as editor_action pointers
 * - The pointers can, in general, be null. Always check for null before doing anything.
 *   The helper functions perform_ that take a pointer do that.
 * - The perform() functions can throw when an error occurs. Use smart pointers if you
 *   need to ensure the pointer is deleted.
 */

#pragma once

#include "editor/action/action.hpp"

#include "units/unit.hpp"

namespace editor {


/**
 * place a new unit on the map
 */
class editor_action_unit : public editor_action_location
{
	public:
		editor_action_unit(map_location loc,
				const unit& u)
		: editor_action_location(loc), u_(u)
		{
		}
		editor_action_unit* clone() const;
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "unit"; }
	protected:
		unit u_;
};

/**
 * Remove a unit from the map.
 */
class editor_action_unit_delete : public editor_action_location
{
	public:
		editor_action_unit_delete(map_location loc)
		: editor_action_location(loc)
		{
		}
		editor_action_unit_delete* clone() const;
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "unit_delete"; }
};

class editor_action_unit_replace : public editor_action_location
{
	public:
		editor_action_unit_replace(map_location loc, map_location new_loc)
		: editor_action_location(loc), new_loc_(new_loc)
		{
		}
		editor_action_unit_replace* clone() const;
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "unit_replace"; }
	protected:
		map_location new_loc_;
};

class editor_action_unit_facing : public editor_action_location
{
	public:
	editor_action_unit_facing(map_location loc, map_location::DIRECTION new_direction, map_location::DIRECTION old_direction)
	: editor_action_location(loc), new_direction_(new_direction), old_direction_(old_direction)
	{
	}
	editor_action_unit_facing* clone() const;
	editor_action* perform(map_context& mc) const;
	void perform_without_undo(map_context& mc) const;
	const char* get_name() const { return "unit_facing"; }
protected:
	map_location::DIRECTION new_direction_;
	map_location::DIRECTION old_direction_;
};


} //end namespace editor
