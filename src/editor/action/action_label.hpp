/*
	Copyright (C) 2010 - 2024
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

#include "color.hpp"
#include "editor/action/action.hpp"

namespace editor
{
/**
 * Set label action
 */
class editor_action_label : public editor_action_location
{
public:
	editor_action_label(map_location loc,
			const std::string& text,
			const std::string& team_name,
			color_t color,
			bool visible_fog,
			bool visible_shroud,
			bool immutable,
			std::string category)
		: editor_action_location(loc)
		, text_(text)
		, team_name_(team_name)
		, category_(category)
		, color_(color)
		, visible_fog_(visible_fog)
		, visible_shroud_(visible_shroud)
		, immutable_(immutable)
	{
	}

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;

protected:
	const std::string text_;
	const std::string team_name_;
	const std::string category_;

	color_t color_;

	bool visible_fog_;
	bool visible_shroud_;
	bool immutable_;
};

class editor_action_label_delete : public editor_action_location
{
public:
	editor_action_label_delete(map_location loc)
		: editor_action_location(loc)
	{
	}

	std::unique_ptr<editor_action> clone() const override;
	std::unique_ptr<editor_action> perform(map_context& mc) const override;
	void perform_without_undo(map_context& mc) const override;
	const std::string& get_name() const override;
};

} // end namespace editor
