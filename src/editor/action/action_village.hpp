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


namespace editor {

/**
 * Sets the ownership of a village to the current side.
 */
class editor_action_village : public editor_action_location
{
	public:
		editor_action_village(map_location loc, int side_number)
		: editor_action_location(loc), side_number_(side_number)
		{
		}
		editor_action_village* clone() const;
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "village"; }
	private:
		int side_number_;
};

/**
 * Clears the ownership of a village.
 */
class editor_action_village_delete : public editor_action_location
{
	public:
		editor_action_village_delete(map_location loc)
		: editor_action_location(loc)
		{
		}
		editor_action_village_delete* clone() const;
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "village_delete"; }
};


} //end namespace editor
