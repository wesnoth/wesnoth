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
#include "overlay.hpp"

//#include "item_types.hpp"
//#include "item.hpp"

namespace editor
{
/**
 * place a new item on the map
 */
class editor_action_item : public editor_action_location
{
public:
	editor_action_item(map_location loc, const overlay& item)
		: editor_action_location(loc)
		, item_(item)
	{
	}

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	editor_action_item* clone() const;

	editor_action* perform(map_context& mc) const;

	void perform_without_undo(map_context& mc) const;

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	const std::string& get_name() const;

protected:
	overlay item_;
};

/**
 * Remove a item from the map.
 */
class editor_action_item_delete : public editor_action_location
{
public:
	editor_action_item_delete(map_location loc)
		: editor_action_location(loc)
	{
	}

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	editor_action_item_delete* clone() const;

	editor_action* perform(map_context& mc) const;

	void perform_without_undo(map_context& mc) const;

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	const std::string& get_name() const;
};

class editor_action_item_replace : public editor_action_location
{
public:
	editor_action_item_replace(map_location loc, map_location new_loc)
		: editor_action_location(loc)
		, new_loc_(new_loc)
	{
	}

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	editor_action_item_replace* clone() const;

	editor_action* perform(map_context& mc) const;

	void perform_without_undo(map_context& mc) const;

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	const std::string& get_name() const;

protected:
	map_location new_loc_;
};

class editor_action_item_facing : public editor_action_location
{
public:
	editor_action_item_facing(
			map_location loc, map_location::DIRECTION new_direction, map_location::DIRECTION old_direction)
		: editor_action_location(loc)
		, new_direction_(new_direction)
		, old_direction_(old_direction)
	{
	}

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	editor_action_item_facing* clone() const;

	editor_action* perform(map_context& mc) const;

	void perform_without_undo(map_context& mc) const;

	/** Inherited from editor_action, implemented by IMPLEMENT_ACTION. */
	const std::string& get_name() const;

protected:
	map_location::DIRECTION new_direction_;
	map_location::DIRECTION old_direction_;
};

} // end namespace editor
