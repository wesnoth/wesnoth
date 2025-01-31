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

	virtual void pre_show() override;

	virtual void post_show() override;

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
