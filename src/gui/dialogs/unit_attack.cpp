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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/unit_attack.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "marked-up_text.hpp"
#include "unit.hpp"
#include "utils/foreach.tpp"

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_unit_attack
 *
 * == Unit attack ==
 *
 * This shows the dialog for attacking units.
 *
 * @begin{table}{dialog_widgets}
 * attacker_portrait & & image   & o & Shows the portrait of the attacking unit.
 *                                     $
 * attacker_icon     & & image   & o & Shows the icon of the attacking unit. $
 * attacker_name     & & control & o & Shows the name of the attacking unit. $
 *
 * defender_portrait & & image   & o & Shows the portrait of the defending unit.
 *                                     $
 * defender_icon     & & image   & o & Shows the icon of the defending unit. $
 * defender_name     & & control & o & Shows the name of the defending unit. $
 *
 *
 * weapon_list       & & listbox & m & The list with weapons to choose from. $
 * -attacker_weapon  & & control & o & The weapon for the attacker to use. $
 * -defender_weapon  & & control & o & The weapon for the defender to use. $
 *
 * @end{table}
 */

REGISTER_DIALOG(unit_attack)

tunit_attack::tunit_attack(const unit_map::iterator& attacker_itor,
						   const unit_map::iterator& defender_itor,
						   const std::vector<battle_context>& weapons,
						   const int best_weapon)
	: selected_weapon_(-1)
	, attacker_itor_(attacker_itor)
	, defender_itor_(defender_itor)
	, weapons_(weapons)
	, best_weapon_(best_weapon)
{
}

template <class T>
static void
set_label(twindow& window, const std::string& id, const std::string& label)
{
	T* widget = find_widget<T>(&window, id, false, false);
	if(widget) {
		widget->set_label(label);
	}
}

static void set_attacker_info(twindow& w, unit& u)
{
	set_label<timage>(w, "attacker_portrait", u.absolute_image());
	set_label<timage>(w, "attacker_icon", u.absolute_image());
	set_label<tcontrol>(w, "attacker_name", u.name());
}

static void set_defender_info(twindow& w, unit& u)
{
	set_label<timage>(w, "defender_portrait", u.absolute_image());
	set_label<timage>(w, "defender_icon", u.absolute_image());
	set_label<tcontrol>(w, "defender_name", u.name());
}

static void set_weapon_info(twindow& window,
							const std::vector<battle_context>& weapons,
							const int best_weapon)
{
	tlistbox& weapon_list
			= find_widget<tlistbox>(&window, "weapon_list", false);
	window.keyboard_capture(&weapon_list);

	const config empty;
	const attack_type no_weapon(empty);

	FOREACH(const AUTO & weapon, weapons)
	{
		const battle_context_unit_stats& attacker = weapon.get_attacker_stats();
		const battle_context_unit_stats& defender = weapon.get_defender_stats();

		const attack_type& attacker_weapon = 
			*attacker.weapon;
		const attack_type& defender_weapon = defender.weapon ? 
			*defender.weapon : no_weapon;

		const SDL_Color a_cth_color =
			int_to_color(game_config::red_to_green(attacker.chance_to_hit));
		const SDL_Color d_cth_color =
			int_to_color(game_config::red_to_green(defender.chance_to_hit));

		const std::string& attw_name = !attacker_weapon.name().empty() ? attacker_weapon.name() : " ";
		const std::string& defw_name = !defender_weapon.name().empty() ? defender_weapon.name() : " ";

		std::string range = attacker_weapon.range().empty() ? defender_weapon.range() : attacker_weapon.range();
		if (!range.empty()) {
			range = string_table["range_" + range];
		}

		std::stringstream attacker_stats, defender_stats;

		attacker_stats << "<b>" << attw_name << "</b>" << "\n"
			<< attacker_weapon.damage() << font::weapon_numbers_sep << attacker_weapon.num_attacks()
			<< "  " << attacker_weapon.weapon_specials() << "\n"
			<< font::span_color(a_cth_color) << attacker.chance_to_hit << "%</span>" << "\n";

		defender_stats << "<b>" << defw_name << "</b>" << "\n"
			<< defender_weapon.damage() << font::weapon_numbers_sep << defender_weapon.num_attacks()
			<< "  " << defender_weapon.weapon_specials() << "\n"
			<< font::span_color(d_cth_color) << defender.chance_to_hit << "%</span>" << "\n";

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = attacker_weapon.icon();
		data.insert(std::make_pair("attacker_weapon_icon", item));

		item["label"] = attacker_stats.str();
		item["use_markup"] = "true";
		data.insert(std::make_pair("attacker_weapon", item));

		//item["label"] = << utils::unicode_em_dash + " " << range << " " + utils::unicode_em_dash;
		item["label"] = "<span color='#a69275'>" + utils::unicode_em_dash + " " + range + " " + utils::unicode_em_dash + "</span>";
		item["use_markup"] = "true";
		data.insert(std::make_pair("range", item));

		item["label"] = defender_stats.str();
		item["use_markup"] = "true";
		data.insert(std::make_pair("defender_weapon", item));

		item["label"] = defender_weapon.icon();
		data.insert(std::make_pair("defender_weapon_icon", item));

		weapon_list.add_row(data);
	}

	assert(best_weapon < static_cast<int>(weapon_list.get_item_count()));
	weapon_list.select_row(best_weapon);
}

void tunit_attack::pre_show(CVideo& /*video*/, twindow& window)
{
	set_attacker_info(window, *attacker_itor_);
	set_defender_info(window, *defender_itor_);

	selected_weapon_ = -1;
	set_weapon_info(window, weapons_, best_weapon_);
}

void tunit_attack::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		selected_weapon_ = find_widget<tlistbox>(&window, "weapon_list", false)
								   .get_selected_row();
	}
}

} // namespace gui2
