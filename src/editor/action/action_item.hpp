/*
	Copyright (C) 2008 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 *
 * \todo remove commented-out code (no separate ticket, but editor cleanup is on the roadmap for 1.15.11)
 * action_item.cpp is full of commented-out code, it seems to be deadweight
 * with the real implementation to be found in mouse_action_item.cpp.
 */
class editor_action_item : public editor_action_location
{
public:
	editor_action_item(map_location loc, const overlay& item)
		: editor_action_location(loc)
		, item_(item)
	{
	}

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;

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

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;
};

class editor_action_item_replace : public editor_action_location
{
public:
	editor_action_item_replace(map_location loc, map_location new_loc)
		: editor_action_location(loc)
		, new_loc_(new_loc)
	{
	}

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;

protected:
	map_location new_loc_;
};

class editor_action_item_facing : public editor_action_location
{
public:
	editor_action_item_facing(
			map_location loc, map_location::direction new_direction, map_location::direction old_direction)
		: editor_action_location(loc)
		, new_direction_(new_direction)
		, old_direction_(old_direction)
	{
	}

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;

protected:
	map_location::direction new_direction_;
	map_location::direction old_direction_;
};

} // end namespace editor
