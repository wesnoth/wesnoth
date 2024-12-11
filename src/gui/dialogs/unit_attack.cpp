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


#include "gui/dialogs/unit_attack.hpp"

#include "color.hpp"
#include "gui/dialogs/attack_predictions.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "serialization/markup.hpp"
#include "units/unit.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(unit_attack)

unit_attack::unit_attack(const unit_map::iterator& attacker_itor,
						   const unit_map::iterator& defender_itor,
						   std::vector<battle_context>&& weapons,
						   const int best_weapon)
	: modal_dialog(window_id())
	, selected_weapon_(-1)
	, attacker_itor_(attacker_itor)
	, defender_itor_(defender_itor)
	, weapons_(std::move(weapons))
	, best_weapon_(best_weapon)
{
}

void unit_attack::damage_calc_callback()
{
	const std::size_t index = find_widget<listbox>("weapon_list").get_selected_row();
	attack_predictions::display(weapons_[index], attacker_itor_.get_shared_ptr(), defender_itor_.get_shared_ptr());
}

void unit_attack::pre_show()
{
	connect_signal_mouse_left_click(
			find_widget<button>("damage_calculation"),
			std::bind(&unit_attack::damage_calc_callback, this));

	find_widget<unit_preview_pane>("attacker_pane")
		.set_display_data(*attacker_itor_);

	find_widget<unit_preview_pane>("defender_pane")
		.set_display_data(*defender_itor_);

	selected_weapon_ = -1;

	listbox& weapon_list = find_widget<listbox>("weapon_list");
	keyboard_capture(&weapon_list);

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

		const color_t a_cth_color = game_config::red_to_green(attacker.chance_to_hit);
		const color_t d_cth_color = game_config::red_to_green(defender.chance_to_hit);

		const std::string attw_name = !attacker_weapon.name().empty() ? attacker_weapon.name() : " ";
		const std::string defw_name = !defender_weapon.name().empty() ? defender_weapon.name() : " ";

		std::string range = attacker_weapon.range().empty() ? defender_weapon.range() : attacker_weapon.range();
		if (!range.empty()) {
			range = string_table["range_" + range];
		}

		auto a_ctx = attacker_weapon.specials_context(
			attacker_itor_.get_shared_ptr(),
			defender_itor_.get_shared_ptr(),
			attacker_itor_->get_location(),
			defender_itor_->get_location(), true, defender.weapon
		);

		auto d_ctx = defender_weapon.specials_context(
			defender_itor_.get_shared_ptr(),
			attacker_itor_.get_shared_ptr(),
			defender_itor_->get_location(),
			attacker_itor_->get_location(), false, attacker.weapon
		);

		std::pair<std::string, std::string> types = attacker_weapon.damage_type();
		std::string attw_type_second = types.second;
		std::string attw_type = !(types.first).empty() ? types.first : attacker_weapon.type();
		if (!attw_type.empty()) {
			attw_type = string_table["type_" + attw_type];
		}
		if (!attw_type_second.empty()) {
			attw_type_second = ", " + string_table["type_" + attw_type_second];
		}
		std::pair<std::string, std::string> def_types = defender_weapon.damage_type();
		std::string defw_type_second = def_types.second;
		std::string defw_type = !(def_types.first).empty() ? def_types.first : defender_weapon.type();
		if (!defw_type.empty()) {
			defw_type = string_table["type_" + defw_type];
		}
		if (!defw_type_second.empty()) {
			defw_type_second = ", " + string_table["type_" + defw_type_second];
		}

		const std::set<std::string> checking_tags_other = {"damage_type", "disable", "berserk", "drains", "heal_on_hit", "plague", "slow", "petrifies", "firststrike", "poison"};
		std::string attw_specials = attacker_weapon.weapon_specials();
		std::string attw_specials_dmg = attacker_weapon.weapon_specials_value({"leadership", "damage"});
		std::string attw_specials_atk = attacker_weapon.weapon_specials_value({"attacks", "swarm"});
		std::string attw_specials_cth = attacker_weapon.weapon_specials_value({"chance_to_hit"});
		std::string attw_specials_others = attacker_weapon.weapon_specials_value(checking_tags_other);
		bool defender_attack = !(defender_weapon.name().empty() && defender_weapon.damage() == 0 && defender_weapon.num_attacks() == 0 && defender.chance_to_hit == 0);
		std::string defw_specials = defender_attack ? defender_weapon.weapon_specials() : "";
		std::string defw_specials_dmg = defender_attack ? defender_weapon.weapon_specials_value({"leadership", "damage"}) : "";
		std::string defw_specials_atk = defender_attack ? defender_weapon.weapon_specials_value({"attacks", "swarm"}) : "";
		std::string defw_specials_cth = defender_attack ? defender_weapon.weapon_specials_value({"chance_to_hit"}) : "";
		std::string defw_specials_others = defender_attack ? defender_weapon.weapon_specials_value(checking_tags_other) : "";

		if(!attw_specials.empty()) {
			attw_specials = " " + attw_specials;
		}
		if(!attw_specials_dmg.empty()) {
			attw_specials_dmg = " " + attw_specials_dmg;
		}
		if(!attw_specials_atk.empty()) {
			attw_specials_atk = " " + attw_specials_atk;
		}
		if(!attw_specials_cth.empty()) {
			attw_specials_cth = " " + attw_specials_cth;
		}
		if(!attw_specials_others.empty()) {
			attw_specials_others = "\n" + markup::bold(_("Other aspects: ")) + "\n" + markup::italic(attw_specials_others);
		}
		if(!defw_specials.empty()) {
			defw_specials = " " + defw_specials;
		}
		if(!defw_specials_dmg.empty()) {
			defw_specials_dmg = " " + defw_specials_dmg;
		}
		if(!defw_specials_atk.empty()) {
			defw_specials_atk = " " + defw_specials_atk;
		}
		if(!defw_specials_cth.empty()) {
			defw_specials_cth = " " + defw_specials_cth;
		}
		if(!defw_specials_others.empty()) {
			defw_specials_others = "\n" + markup::bold(_("Other aspects: ")) + "\n" + markup::italic(defw_specials_others);
		}

		std::stringstream attacker_stats, defender_stats, attacker_tooltip, defender_tooltip;

		// Use attacker/defender.num_blows instead of attacker/defender_weapon.num_attacks() because the latter does not consider the swarm weapon special
		attacker_stats << markup::bold(attw_name) << "\n"
			<< attw_type << attw_type_second << "\n"
			<< attacker.damage << font::weapon_numbers_sep << attacker.num_blows
			<< attw_specials << "\n"
			<< markup::span_color(a_cth_color, attacker.chance_to_hit, "%");

		attacker_tooltip << _("Weapon: ") << markup::bold(attw_name) << "\n"
			<< _("Type: ") << attw_type << attw_type_second << "\n"
			<< _("Damage: ") << attacker.damage << markup::italic(attw_specials_dmg) << "\n"
			<< _("Attacks: ") << attacker.num_blows << markup::italic(attw_specials_atk) << "\n"
			<< _("Chance to hit: ") << markup::span_color(a_cth_color, attacker.chance_to_hit, "%")
			<< markup::italic(attw_specials_cth) << attw_specials_others;

		defender_stats << markup::bold(defw_name) << "\n"
			<< defw_type << defw_type_second << "\n"
			<< defender.damage << font::weapon_numbers_sep << defender.num_blows
			<< defw_specials << "\n"
			<< markup::span_color(d_cth_color, defender.chance_to_hit, "%");

		defender_tooltip << _("Weapon: ") << markup::bold(defw_name) << "\n"
			<< _("Type: ") << defw_type << defw_type_second << "\n"
			<< _("Damage: ") << defender.damage << markup::italic(defw_specials_dmg) << "\n"
			<< _("Attacks: ") << defender.num_blows <<  markup::italic(defw_specials_atk) << "\n"
			<< _("Chance to hit: ") << markup::span_color(d_cth_color, defender.chance_to_hit, "%")
			<< markup::italic(defw_specials_cth)
			<< defw_specials_others;

		widget_data data;
		widget_item item;

		item["use_markup"] = "true";

		item["label"] = attacker_weapon.icon();
		data.emplace("attacker_weapon_icon", item);

		item["tooltip"] = attacker_tooltip.str();
		item["label"] = attacker_stats.str();
		data.emplace("attacker_weapon", item);
		item["tooltip"] = "";

		item["label"] = markup::span_color("#a69275", font::unicode_em_dash, " ", range, " ", font::unicode_em_dash);
		data.emplace("range", item);

		item["tooltip"] = defender_attack ? defender_tooltip.str() : "";
		item["label"] = defender_stats.str();
		data.emplace("defender_weapon", item);

		item["tooltip"] = "";
		item["label"] = defender_weapon.icon();
		data.emplace("defender_weapon_icon", item);

		weapon_list.add_row(data);
	}

	// If these two aren't the same size, we can't use list selection incides
	// to access to weapons list!
	assert(weapons_.size() == weapon_list.get_item_count());

	weapon_list.select_row(best_weapon_);
}

void unit_attack::post_show()
{
	if(get_retval() == retval::OK) {
		selected_weapon_ = find_widget<listbox>("weapon_list").get_selected_row();
	}
}

} // namespace dialogs
