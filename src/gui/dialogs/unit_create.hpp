/*
	Copyright (C) 2009 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include "units/race.hpp"

#include <string>
#include <vector>

class display;
class unit_type;

namespace gui2
{

class menu_button;
class text_box_base;

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the debug-mode dialog to create new units on the map.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * male_toggle       | toggle_button |yes      |Option button to select the "male" gender for created units.
 * female_toggle     | toggle_button |yes      |Option button to select the "female" gender for created units.
 * unit_type_list    | @ref listbox  |yes      |Listbox displaying existing unit types sorted by name and race.
 * unit_type         | control       |yes      |Widget which shows the unit type name label.
 * race              | control       |yes      |Widget which shows the unit race name label.
 */
class unit_create : public modal_dialog
{
public:
	unit_create();

	/** Unit type choice from the user. */
	const std::string& choice() const
	{
		return choice_;
	}

	/** Whether the user actually chose a unit type or not. */
	bool no_choice() const
	{
		return choice_.empty();
	}

	/** Gender choice from the user. */
	unit_race::GENDER gender()
	{
		return gender_;
	}

	/** Variation choice from the user. */
	std::string variation() const
	{
		return variation_;
	}

private:
	std::vector<const unit_type*> units_;

	unit_race::GENDER gender_;

	std::string choice_;

	std::string variation_;

	std::vector<std::string> last_words_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed(const std::string& text);
	void gender_toggle_callback(const unit_race::GENDER val);
	void variation_menu_callback();

	void update_displayed_type();

	group<unit_race::GENDER> gender_toggle;
};
} // namespace dialogs
} // namespace gui2
