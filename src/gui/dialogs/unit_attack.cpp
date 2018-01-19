/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "font/text_formatting.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/attack_predictions.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "language.hpp"
#include "color.hpp"
#include "team.hpp"
#include "units/unit.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
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
 *                                     $
 * attacker_icon     & & image   & o & Shows the icon of the attacking unit. $
 * attacker_name     & & styled_widget & o & Shows the name of the attacking unit. $
 *
 * defender_portrait & & image   & o & Shows the portrait of the defending unit.
 *                                     $
 * defender_icon     & & image   & o & Shows the icon of the defending unit. $
 * defender_name     & & styled_widget & o & Shows the name of the defending unit. $
 *
 *
 * weapon_list       & & listbox & m & The list with weapons to choose from. $
 * -attacker_weapon  & & styled_widget & o & The weapon for the attacker to use. $
 * -defender_weapon  & & styled_widget & o & The weapon for the defender to use. $
 *
 * @end{table}
 */

REGISTER_DIALOG(unit_attack)

unit_attack::unit_attack(const unit_map::iterator& attacker_itor,
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

void unit_attack::damage_calc_callback(window& window)
{
	const size_t index = find_widget<listbox>(&window, "weapon_list", false).get_selected_row();
	attack_predictions::display(weapons_[index], *attacker_itor_, *defender_itor_);
}

void unit_attack::pre_show(window& window)
{
	connect_signal_mouse_left_click(
			find_widget<button>(&window, "damage_calculation", false),
			std::bind(&unit_attack::damage_calc_callback, this, std::ref(window)));

	find_widget<unit_preview_pane>(&window, "attacker_pane", false)
		.set_displayed_unit(*attacker_itor_);

	find_widget<unit_preview_pane>(&window, "defender_pane", false)
		.set_displayed_unit(*defender_itor_);

	selected_weapon_ = -1;

	listbox& weapon_list = find_widget<listbox>(&window, "weapon_list", false);
	window.keyboard_capture(&weapon_list);

	// Possible TODO: If a "blank weapon" is generally useful, add it as a static member in attack_type.
	static const config empty;
	static const_attack_ptr no_weapon(new attack_type(empty));

	for(const auto & weapon : weapons_) {
		const battle_context_unit_stats& attacker = weapon.get_attacker_stats();
		const battle_context_unit_stats& defender = weapon.get_defender_stats();

		const attack_type& attacker_weapon =
			*attacker.weapon;
		const attack_type& defender_weapon = defender.weapon ?
			*defender.weapon : *no_weapon;

		// Don't show if the atacker's weapon has at least one active "disable" special.
		if(attacker_weapon.get_special_bool("disable")) {
			continue;
		}

		const color_t a_cth_color = game_config::red_to_green(attacker.chance_to_hit);
		const color_t d_cth_color = game_config::red_to_green(defender.chance_to_hit);

		const std::string attw_name = !attacker_weapon.name().empty() ? attacker_weapon.name() : " ";
		const std::string defw_name = !defender_weapon.name().empty() ? defender_weapon.name() : " ";

		std::string range = attacker_weapon.range().empty() ? defender_weapon.range() : attacker_weapon.range();
		if (!range.empty()) {
			range = string_table["range_" + range];
		}

		const std::string& attw_apecials =
			!attacker_weapon.weapon_specials().empty() ? " " + attacker_weapon.weapon_specials() : "";
		const std::string& defw_specials =
			!defender_weapon.weapon_specials().empty() ? " " + defender_weapon.weapon_specials() : "";

		std::stringstream attacker_stats, defender_stats;

		// Use attacker/defender.num_blows instead of attacker/defender_weapon.num_attacks() because the latter does not consider the swarm weapon special
		attacker_stats << "<b>" << attw_name << "</b>" << "\n"
			<< attacker.damage << font::weapon_numbers_sep << attacker.num_blows
			<< attw_apecials << "\n"
			<< font::span_color(a_cth_color) << attacker.chance_to_hit << "%</span>";

		defender_stats << "<b>" << defw_name << "</b>" << "\n"
			<< defender.damage << font::weapon_numbers_sep << defender.num_blows
			<< defw_specials << "\n"
			<< font::span_color(d_cth_color) << defender.chance_to_hit << "%</span>";

		std::map<std::string, string_map> data;
		string_map item;

		item["use_markup"] = "true";

		item["label"] = attacker_weapon.icon();
		data.emplace("attacker_weapon_icon", item);

		item["label"] = attacker_stats.str();
		data.emplace("attacker_weapon", item);

		item["label"] = "<span color='#a69275'>" + font::unicode_em_dash + " " + range + " " + font::unicode_em_dash + "</span>";
		data.emplace("range", item);

		item["label"] = defender_stats.str();
		data.emplace("defender_weapon", item);

		item["label"] = defender_weapon.icon();
		data.emplace("defender_weapon_icon", item);

		weapon_list.add_row(data);
	}

	const int last_item = weapon_list.get_item_count() - 1;
	weapon_list.select_row(std::min(best_weapon_, last_item));
}

void unit_attack::post_show(window& window)
{
	if(get_retval() == window::OK) {
		selected_weapon_ = find_widget<listbox>(&window, "weapon_list", false).get_selected_row();
	}
}

} // namespace dialogs
} // namespace gui2
