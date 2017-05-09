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
 * Select the given locations
 */
class editor_action_select : public editor_action_area
{
	public:
		editor_action_select(const std::set<map_location>& area)
		: editor_action_area(area)
		{
		}
		editor_action_select* clone() const;
		void extend(const editor_map& map, const std::set<map_location>& locs);
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "select"; }
};

/**
 * Deselect the given locations
 */
class editor_action_deselect : public editor_action_area
{
public:
	editor_action_deselect(const std::set<map_location>& area) :
		editor_action_area(area) {}

	editor_action_deselect* clone() const;
	void extend(const editor_map& map, const std::set<map_location>& locs);
	editor_action* perform(map_context& mc) const;
	void perform_without_undo(map_context& mc) const;
	const char* get_name() const { return "deselect"; }
};

/**
 * Select the entire map
 */
class editor_action_select_all : public editor_action
{
	public:
		editor_action_select_all()
		{
		}
		editor_action_select_all* clone() const;
		editor_action_select* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "select_all"; }
};

/**
 * Clear selection
 */
class editor_action_select_none : public editor_action
{
	public:
		editor_action_select_none()
		{
		}
		editor_action_select_none* clone() const;
		editor_action_select* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "select_none"; }
};

/**
 * Invert the selection
 */
class editor_action_select_inverse : public editor_action
{
	public:
		editor_action_select_inverse()
		{
		}
		editor_action_select_inverse* clone() const;
		editor_action_select_inverse* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		const char* get_name() const { return "select_inverse"; }
};



} //end namespace editor
