/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_ATTACK_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_ATTACK_HPP_INCLUDED

#include "actions/attack.hpp"
#include "gui/dialogs/dialog.hpp"
#include "display.hpp"
#include "unit_map.hpp"

namespace gui2
{

class tunit_attack : public tdialog
{
public:
	tunit_attack(const unit_map::iterator& attacker_itor,
				 const unit_map::iterator& defender_itor,
				 const std::vector<battle_context>& weapons,
				 const int best_weapon);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_selected_weapon() const
	{
		return selected_weapon_;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void profile_button_callback(twindow& window, const std::string& type);

	void damage_calc_callback(twindow& window);

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

} // namespace gui2

#endif
