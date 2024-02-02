/*
	Copyright (C) 2010 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "actions/attack.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "units/map.hpp"

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog for attacking units.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * attacker_portrait | @ref image   |no       |Shows the portrait of the attacking unit.
 * attacker_icon     | @ref image   |no       |Shows the icon of the attacking unit.
 * attacker_name     | control      |no       |Shows the name of the attacking unit.
 * defender_portrait | @ref image   |no       |Shows the portrait of the defending unit.
 * defender_icon     | @ref image   |no       |Shows the icon of the defending unit.
 * defender_name     | control      |no       |Shows the name of the defending unit.
 * weapon_list       | @ref listbox |yes      |The list with weapons to choose from.
 * attacker_weapon   | control      |no       |The weapon for the attacker to use.
 * defender_weapon   | control      |no       |The weapon for the defender to use.
 */
class unit_attack : public modal_dialog
{
public:
	unit_attack(const unit_map::iterator& attacker_itor,
				 const unit_map::iterator& defender_itor,
				 std::vector<battle_context>&& weapons,
				 const int best_weapon);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_selected_weapon() const
	{
		return selected_weapon_;
	}

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	void damage_calc_callback();

	/** The index of the selected weapon. */
	int selected_weapon_;

	/** Iterator pointing to the attacker. */
	unit_map::iterator attacker_itor_;

	/** Iterator pointing to the defender. */
	unit_map::iterator defender_itor_;

	/** List of all battle contexts used for getting the weapons. */
	std::vector<battle_context> weapons_;

	/** The best weapon, aka the one high-lighted. */
	int best_weapon_;
};

} // namespace dialogs
