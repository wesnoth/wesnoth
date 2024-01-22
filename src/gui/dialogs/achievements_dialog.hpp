/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "achievements.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"

#pragma once

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows a dialog displaying achievements.
 *
 * Key                       |Type           |Mandatory|Description
 * --------------------------|-------------|---------|-----------
 * selected_achievements_list|menu_button  |yes      |Allows selecting achievements by what content they're for.
 * name                      |label        |yes      |The user displayed name of the achievement.
 * description               |label        |yes      |The achievement's longer description.
 * icon                      |image        |yes      |An icon to display to the left of the achievement.
 */
class achievements_dialog : public modal_dialog
{
public:
	DEFINE_SIMPLE_EXECUTE_WRAPPER(achievements_dialog)

	achievements_dialog();

private:
	achievements achieve_;
	std::string last_selected_;
	listbox* achievements_box_;
	menu_button* content_names_;

	void set_sub_achievements(grid& newrow, const achievement& ach);

	void set_achievements_row();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace gui2::dialogs
