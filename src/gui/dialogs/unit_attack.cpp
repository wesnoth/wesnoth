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

#include "attack_prediction_display.hpp"

#include "gui/dialogs/unit_attack.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "display.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "language.hpp"
#include "marked-up_text.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include "utils/foreach.tpp"

#include <boost/bind.hpp>

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
						   const int best_weapon,
						   display* disp)
	: selected_weapon_(-1)
	, attacker_itor_(attacker_itor)
	, defender_itor_(defender_itor)
	, weapons_(weapons)
	, best_weapon_(best_weapon)
	, disp_(disp)
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

static std::string format_stats(const unit& u)
{
	const std::string name = "<span size='large'>" + (!u.name().empty() ? u.name() : " ") + "</span>";
	std::string traits;

	BOOST_FOREACH(const std::string& trait, u.get_traits_list()) {
		traits += (traits.empty() ? "" : ", ") + trait;
	}

	if (traits.empty()) {
		traits = " ";
	}

	std::stringstream str;

	str << name << "\n";

	str << "<small>";

	str << "<span color='#f5e6c1'>" << u.type_name() << "</span>" << "\n";

	str << "Lvl " << u.level() << "\n";

	str << u.alignment() << "\n";

	str << traits << "\n";

	str << font::span_color(u.hp_color()) 
		<< _("HP: ") << u.hitpoints() << "/" << u.max_hitpoints() << "</span>" << "\n";
	str << font::span_color(u.xp_color()) 
		<< _("XP: ") << u.experience() << "/" << u.max_experience() << "</span>" << "\n";

	str << "</small>" << "\n";

	return str.str();
}

static std::string get_image_mods(const unit& u)
{
	std::string res
		= "~RC(" + u.team_color() + ">" + team::get_side_color_index(u.side()) + ")";

	if (u.can_recruit()) {
		res += "~BLIT(" + unit::leader_crown() + ")";
	}

	BOOST_FOREACH(const std::string& overlay, u.overlays()) {
		res += "~BLIT(" + overlay + ")";
	}

	return res;
}

static void set_attacker_info(twindow& window, const unit& u)
{
	set_label<timage>(window, "attacker_image", u.absolute_image() + get_image_mods(u));

	tcontrol& attacker_name =
		find_widget<tcontrol>(&window, "attacker_stats", false);

	attacker_name.set_use_markup(true);
	attacker_name.set_label(format_stats(u));
}

static void set_defender_info(twindow& window, const unit& u)
{
	// Ensure the defender image is always facing left
	set_label<timage>(window, "defender_image", u.absolute_image() + "~FL(horiz)" + get_image_mods(u));

	tcontrol& defender_name =
		find_widget<tcontrol>(&window, "defender_stats", false);

	defender_name.set_use_markup(true);
	defender_name.set_label(format_stats(u));
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

		const std::string& attw_apecials = 
			!attacker_weapon.weapon_specials().empty() ? "  " + attacker_weapon.weapon_specials() : "";
		const std::string& defw_specials = 
			!defender_weapon.weapon_specials().empty() ? "  " + defender_weapon.weapon_specials() : "";

		std::stringstream attacker_stats, defender_stats;

		attacker_stats << "<b>" << attw_name << "</b>" << "\n"
			<< attacker.damage << font::weapon_numbers_sep << attacker_weapon.num_attacks()
			<< attw_apecials << "\n"
			<< font::span_color(a_cth_color) << attacker.chance_to_hit << "%</span>" << "\n";

		defender_stats << "<b>" << defw_name << "</b>" << "\n"
			<< defender.damage << font::weapon_numbers_sep << defender_weapon.num_attacks()
			<< defw_specials << "\n"
			<< font::span_color(d_cth_color) << defender.chance_to_hit << "%</span>" << "\n";

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = attacker_weapon.icon();
		data.insert(std::make_pair("attacker_weapon_icon", item));

		item["label"] = attacker_stats.str();
		item["use_markup"] = "true";
		data.insert(std::make_pair("attacker_weapon", item));

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

void tunit_attack::profile_button_callback(const std::string& type)
{
	if (!disp_) {
		return;
	}

	help::show_unit_help(disp_->video(), type);
}

void tunit_attack::damage_calc_callback(twindow& window)
{
	const int selection
		= find_widget<tlistbox>(&window, "weapon_list", false).get_selected_row();

	attack_prediction_displayer predition_dialog(weapons_, (*attacker_itor_).get_location(), (*defender_itor_).get_location());
	predition_dialog.button_pressed(selection);
}

void tunit_attack::pre_show(CVideo& /*video*/, twindow& window)
{
	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "attacker_profile", false),
			boost::bind(&tunit_attack::profile_button_callback, this, (*attacker_itor_).type_id()));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "defender_profile", false),
			boost::bind(&tunit_attack::profile_button_callback, this, (*defender_itor_).type_id()));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "damage_calculation", false),
			boost::bind(&tunit_attack::damage_calc_callback, this, boost::ref(window)));

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
