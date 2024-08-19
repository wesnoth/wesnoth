/*
	Copyright (C) 2023 - 2024
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/*  unit type editor dialog */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/editor/edit_unit.hpp"

#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/scroll_text.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/spinner.hpp"
#include "gui/widgets/tab_container.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "picture.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "units/types.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_unit);

editor_edit_unit::editor_edit_unit(const game_config_view& game_config, const std::string& addon_id)
	: modal_dialog(window_id())
	, game_config_(game_config)
	, addon_id_(addon_id)
{
	config specials;

	read(specials, *(preprocess_file(game_config::path+"/data/core/macros/weapon_specials.cfg", &specials_map_)));
	for (const auto& x : specials_map_) {
		specials_list_.emplace_back("label", x.first, "checkbox", false);
	}

	read(specials, *(preprocess_file(game_config::path+"/data/core/macros/abilities.cfg", &abilities_map_)));
	for (const auto& x : abilities_map_) {
		// Don't add any macros that have INTERNAL
		if (x.first.find("INTERNAL") == std::string::npos) {
			abilities_list_.emplace_back("label", x.first, "checkbox", false);
		}
	}

	connect_signal<event::SDL_KEY_DOWN>(std::bind(
		&editor_edit_unit::signal_handler_sdl_key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));
}

void editor_edit_unit::pre_show(window& win) {
	tab_container& tabs = find_widget<tab_container>(&win, "tabs", false);
	connect_signal_notify_modified(tabs, std::bind(&editor_edit_unit::on_page_select, this));

	//
	// Main Stats tab
	//

	tabs.select_tab(0);

	menu_button& alignments = find_widget<menu_button>(&win, "alignment_list", false);
	for (auto& align : unit_alignments::values) {
		// Show the user the translated strings,
		// but use the untranslated align strings for generated WML
		align_list_.emplace_back("label", t_string(static_cast<std::string>(align), "wesnoth"));
	}
	alignments.set_values(align_list_);

	menu_button& races = find_widget<menu_button>(&win, "race_list", false);
	for(const race_map::value_type &i : unit_types.races()) {
		const std::string& race_name = i.second.id();
		race_list_.emplace_back("label", race_name);
	}

	if (race_list_.size() > 0) {
		races.set_values(race_list_);
	}

	button& load = find_widget<button>(&win, "load_unit_type", false);
	std::stringstream tooltip;
	tooltip << t_string("Hotkey(s): ", "wesnoth");
	#ifdef __APPLE__
		tooltip << "cmd+o";
	#else
		tooltip << "ctrl+o";
	#endif
	load.set_tooltip(tooltip.str());
	connect_signal_mouse_left_click(load, std::bind(&editor_edit_unit::load_unit_type, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_unit_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/units", "unit_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_unit_image", false),
		std::bind(&editor_edit_unit::update_image, this, "unit_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_portrait_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/portraits", "portrait_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_portrait_image", false),
		std::bind(&editor_edit_unit::update_image, this, "portrait_image"));

	connect_signal_notify_modified(
		find_widget<text_box>(&win, "name_box", false),
		std::bind(&editor_edit_unit::button_state_change, this));
	connect_signal_notify_modified(
		find_widget<text_box>(&win, "id_box", false),
		std::bind(&editor_edit_unit::button_state_change, this));

	//
	// Advanced Tab
	//
	tabs.select_tab(1);

	menu_button& movetypes = find_widget<menu_button>(&win, "movetype_list", false);
	for(const auto& mt : unit_types.movement_types()) {
		movetype_list_.emplace_back("label", mt.first);
	}

	if (movetype_list_.size() > 0) {
		movetypes.set_values(movetype_list_);
	}

	menu_button& defenses = find_widget<menu_button>(&win, "defense_list", false);
	const config& defense_attr = game_config_
				.mandatory_child("units")
				.mandatory_child("movetype")
				.mandatory_child("defense");
	for (const auto& attribute : defense_attr.attribute_range()) {
		defense_list_.emplace_back("label", attribute.first);
	}

	if (defense_list_.size() > 0) {
		defenses.set_values(defense_list_);
		def_toggles_.resize(defense_list_.size());
	}

	menu_button& movement_costs = find_widget<menu_button>(&win, "movement_costs_list", false);
	if (defense_list_.size() > 0) {
		movement_costs.set_values(defense_list_);
		move_toggles_.resize(defense_list_.size());
	}

	menu_button& resistances = find_widget<menu_button>(&win, "resistances_list", false);

	const config& resistances_attr = game_config_
				.mandatory_child("units")
				.mandatory_child("movetype")
				.mandatory_child("resistance");
	for (const auto& attribute : resistances_attr.attribute_range()) {
		resistances_list_.emplace_back("label", attribute.first);
	}

	if (resistances_list_.size() > 0) {
		resistances.set_values(resistances_list_);
		res_toggles_.resize(resistances_list_.size());
	}

	menu_button& usage_types = find_widget<menu_button>(&win, "usage_list", false);
	usage_type_list_.emplace_back("label", _("scout"));
	usage_type_list_.emplace_back("label", _("fighter"));
	usage_type_list_.emplace_back("label", _("archer"));
	usage_type_list_.emplace_back("label", _("mixed fighter"));
	usage_type_list_.emplace_back("label", _("healer"));
	usage_types.set_values(usage_type_list_);

	multimenu_button& abilities = find_widget<multimenu_button>(&win, "abilities_list", false);
	abilities.set_values(abilities_list_);

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_small_profile_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/portraits", "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_small_profile_image", false),
		std::bind(&editor_edit_unit::update_image, this, "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "load_movetype", false),
		std::bind(&editor_edit_unit::load_movetype, this));
	connect_signal_notify_modified(
		find_widget<slider>(&win, "resistances_slider", false),
		std::bind(&editor_edit_unit::store_resistances, this));
	connect_signal_notify_modified(
		find_widget<menu_button>(&win, "resistances_list", false),
		std::bind(&editor_edit_unit::update_resistances, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>(&win, "resistances_checkbox", false),
		std::bind(&editor_edit_unit::enable_resistances_slider, this));

	connect_signal_notify_modified(
		find_widget<slider>(&win, "defense_slider", false),
		std::bind(&editor_edit_unit::store_defenses, this));
	connect_signal_notify_modified(
		find_widget<menu_button>(&win, "defense_list", false),
		std::bind(&editor_edit_unit::update_defenses, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>(&win, "defense_checkbox", false),
		std::bind(&editor_edit_unit::enable_defense_slider, this));

	connect_signal_notify_modified(
		find_widget<slider>(&win, "movement_costs_slider", false),
		std::bind(&editor_edit_unit::store_movement_costs, this));
	connect_signal_notify_modified(
		find_widget<menu_button>(&win, "movement_costs_list", false),
		std::bind(&editor_edit_unit::update_movement_costs, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>(&win, "movement_costs_checkbox", false),
		std::bind(&editor_edit_unit::enable_movement_slider, this));

	if (!res_toggles_.empty()) {
		enable_resistances_slider();
	}

	if (!def_toggles_.empty()) {
		enable_defense_slider();
	}

	if (!move_toggles_.empty()) {
		enable_movement_slider();
	}

	//
	// Attack Tab
	//
	tabs.select_tab(2);
	multimenu_button& specials = find_widget<multimenu_button>(&win, "weapon_specials_list", false);
	specials.set_values(specials_list_);
	group<std::string> range_group;
	range_group.add_member(find_widget<toggle_button>(&win, "range_melee", false, true), "melee");
	range_group.add_member(find_widget<toggle_button>(&win, "range_ranged", false, true), "ranged");
	range_group.set_member_states("melee");

	menu_button& attack_types = find_widget<menu_button>(&win, "attack_type_list", false);
	if (resistances_list_.size() > 0) {
		attack_types.set_values(resistances_list_);
	}

	// Connect signals
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_attack_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/attacks", "attack_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_attack_image", false),
		std::bind(&editor_edit_unit::update_image, this, "attack_image"));
	connect_signal_notify_modified(
		find_widget<menu_button>(&win, "atk_list", false),
		std::bind(&editor_edit_unit::select_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "atk_new", false),
		std::bind(&editor_edit_unit::add_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "atk_delete", false),
		std::bind(&editor_edit_unit::delete_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "atk_next", false),
		std::bind(&editor_edit_unit::next_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "atk_prev", false),
		std::bind(&editor_edit_unit::prev_attack, this));

	update_index();

	tabs.select_tab(0);

	// Disable OK button at start, since ID and Name boxes are empty
	button_state_change();
}

void editor_edit_unit::on_page_select()
{
	save_unit_type();

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	if (tabs.get_active_tab_index() == 3) {
		update_wml_view();
	}
}

void editor_edit_unit::select_file(const std::string& default_dir, const std::string& id_stem)
{
	gui2::dialogs::file_dialog dlg;
	dlg.set_title(_("Choose File"))
		.set_ok_label(_("Select"))
		.set_path(default_dir)
		.set_read_only(true);

	if (dlg.show()) {

		std::string dn = dlg.path();
		const std::string& message
						= _("This file is outside Wesnoth's data dirs. Do you wish to copy it into your add-on?");

		if(id_stem == "unit_image") {

			if (!filesystem::to_asset_path(dn, addon_id_, "images")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}

		} else if((id_stem == "portrait_image")||(id_stem == "small_profile_image")) {

			if (!filesystem::to_asset_path(dn, addon_id_, "images")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}

		} else if(id_stem == "attack_image") {

			if (!filesystem::to_asset_path(dn, addon_id_, "images")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}

		}

		find_widget<text_box>(get_window(), "path_"+id_stem, false).set_value(dn);
		update_image(id_stem);
	}
}

void editor_edit_unit::load_unit_type() {
	gui2::dialogs::unit_create dlg_uc;
	if (dlg_uc.show()) {
		const unit_type *type = unit_types.find(dlg_uc.choice());

		tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
		tabs.select_tab(0);

		find_widget<text_box>(get_window(), "id_box", false).set_value(type->id());
		find_widget<text_box>(get_window(), "name_box", false).set_value(type->type_name().base_str());
		find_widget<spinner>(get_window(), "level_box", false).set_value(type->level());
		find_widget<slider>(get_window(), "cost_slider", false).set_value(type->cost());
		find_widget<text_box>(get_window(), "adv_box", false).set_value(utils::join(type->advances_to()));
		find_widget<slider>(get_window(), "hp_slider", false).set_value(type->hitpoints());
		find_widget<slider>(get_window(), "xp_slider", false).set_value(type->experience_needed());
		find_widget<slider>(get_window(), "move_slider", false).set_value(type->movement());
		find_widget<scroll_text>(get_window(), "desc_box", false).set_value(type->unit_description().base_str());
		find_widget<text_box>(get_window(), "adv_box", false).set_value(utils::join(type->advances_to(), ", "));
		find_widget<text_box>(get_window(), "path_unit_image", false).set_value(type->image());
		find_widget<text_box>(get_window(), "path_portrait_image", false).set_value(type->big_profile());

		for (const auto& gender : type->genders())
		{
			if (gender == unit_race::GENDER::MALE) {
				find_widget<toggle_button>(get_window(), "gender_male", false).set_value(true);
			}

			if (gender == unit_race::GENDER::FEMALE) {
				find_widget<toggle_button>(get_window(), "gender_female", false).set_value(true);
			}
		}

		set_selected_from_string(
				find_widget<menu_button>(get_window(), "race_list", false),
				race_list_,
				type->race_id());

		set_selected_from_string(
				find_widget<menu_button>(get_window(), "alignment_list", false),
				align_list_,
				unit_alignments::get_string(type->alignment()));

		update_image("unit_image");

		tabs.select_tab(1);
		find_widget<text_box>(get_window(), "path_small_profile_image", false).set_value(type->small_profile());

		set_selected_from_string(
				find_widget<menu_button>(get_window(), "movetype_list", false),
				movetype_list_,
				type->movement_type_id());

		config cfg;
		type->movement_type().write(cfg, false);
		movement_ = cfg.mandatory_child("movement_costs");
		defenses_ = cfg.mandatory_child("defense");
		resistances_ = cfg.mandatory_child("resistance");

		// Overrides for resistance/defense/movement costs
		for (unsigned i = 0; i < resistances_list_.size(); i++) {
			if (!type->get_cfg().has_child("resistance")) {
				break;
			}

			for (const auto& attr : type->get_cfg().mandatory_child("resistance").attribute_range()) {
				if (resistances_list_.at(i)["label"] == attr.first) {
					res_toggles_[i] = 1;
				}
			}
		}

		for (unsigned i = 0; i < defense_list_.size(); i++) {
			if (type->get_cfg().has_child("defense")) {
				for (const auto& attr : type->get_cfg().mandatory_child("defense").attribute_range()) {
					if (defense_list_.at(i)["label"] == attr.first) {
						def_toggles_[i] = 1;
					}
				}
			}

			if (type->get_cfg().has_child("movement_costs")) {
				for (const auto& attr : type->get_cfg().mandatory_child("movement_costs").attribute_range()) {
					if (defense_list_.at(i)["label"] == attr.first) {
						move_toggles_[i] = 1;
					}
				}
			}
		}

		update_resistances();
		update_defenses();
		update_resistances();

		set_selected_from_string(
				find_widget<menu_button>(get_window(), "usage_list", false),
				usage_type_list_,
				type->usage());

		update_image("small_profile_image");

		tabs.select_tab(2);
		attacks_.clear();
		for(const auto& atk : type->attacks())
		{
			config attack;
			boost::dynamic_bitset<> enabled(specials_list_.size());
			attack["name"] = atk.id();
			attack["description"] = atk.name().base_str();
			attack["icon"] = atk.icon();
			attack["range"] = atk.range();
			attack["damage"] = atk.damage();
			attack["number"] = atk.num_attacks();
			attack["type"] = atk.type();
			attacks_.push_back(std::make_pair(enabled, attack));
		}

		selected_attack_ = 1;
		update_attacks();
		update_index();

		tabs.select_tab(0);

		button_state_change();
		get_window()->invalidate_layout();
	}
}

void editor_edit_unit::save_unit_type() {

	// Clear the config
	type_cfg_.clear();

	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);

	// Page 1
	grid* grid = tabs.get_tab_grid(0);


	config& utype = type_cfg_.add_child("unit_type");
	utype["id"] = find_widget<text_box>(grid, "id_box", false).get_value();
	utype["name"] = t_string(find_widget<text_box>(grid, "name_box", false).get_value(), current_textdomain);
	utype["image"] = find_widget<text_box>(grid, "path_unit_image", false).get_value();
	utype["profile"] = find_widget<text_box>(grid, "path_portrait_image", false).get_value();
	utype["level"] = find_widget<spinner>(grid, "level_box", false).get_value();
	utype["advances_to"] = find_widget<text_box>(grid, "adv_box", false).get_value();
	utype["hitpoints"] = find_widget<slider>(grid, "hp_slider", false).get_value();
	utype["experience"] = find_widget<slider>(grid, "xp_slider", false).get_value();
	utype["cost"] = find_widget<slider>(grid, "cost_slider", false).get_value();
	utype["movement"] = find_widget<slider>(grid, "move_slider", false).get_value();
	utype["description"] = t_string(find_widget<scroll_text>(grid, "desc_box", false).get_value(), current_textdomain);
	utype["race"] = find_widget<menu_button>(grid, "race_list", false).get_value_string();
	utype["alignment"] = unit_alignments::values[find_widget<menu_button>(grid, "alignment_list", false).get_value()];

	// Gender
	if (find_widget<toggle_button>(grid, "gender_male", false).get_value()) {
		if (find_widget<toggle_button>(grid, "gender_female", false).get_value()) {
			utype["gender"] = "male,female";
		} else {
			utype["gender"] = "male";
		}
	} else {
		if (find_widget<toggle_button>(grid, "gender_female", false).get_value()) {
			utype["gender"] = "female";
		}
	}

	// Page 2
	grid = tabs.get_tab_grid(1);

	utype["small_profile"] = find_widget<text_box>(grid, "path_small_profile_image", false).get_value();
	utype["movement_type"] = find_widget<menu_button>(grid, "movetype_list", false).get_value_string();
	utype["usage"] = find_widget<menu_button>(grid, "usage_list", false).get_value_string();

	if (res_toggles_.any()) {
		config& resistances = utype.add_child("resistance");
		int i = 0;
		for (const auto& attr : resistances_.attribute_range()) {
			if (res_toggles_[i]) {
				resistances[attr.first] = resistances_[attr.first];
			}
			i++;
		}
	}

	if (def_toggles_.any()) {
		config& defenses = utype.add_child("defense");
		int i = 0;
		for (const auto& attr : defenses_.attribute_range()) {
			if (def_toggles_[i]) {
				defenses[attr.first] = defenses_[attr.first];
			}
			i++;
		}
	}

	if (move_toggles_.any()) {
		config& movement_costs = utype.add_child("movement_costs");
		int i = 0;
		for (const auto& attr : movement_.attribute_range()) {
			if (move_toggles_[i]) {
				movement_costs[attr.first] = movement_[attr.first];
			}
			i++;
		}
	}

	const auto& abilities_states = find_widget<multimenu_button>(grid, "abilities_list", false).get_toggle_states();
	if (abilities_states.any()) {
		unsigned int i = 0;
		sel_abilities_.clear();
		for (const auto& x : abilities_map_) {
			if (i >= abilities_states.size()) {
				break;
			}

			if (abilities_states[i] == true) {
				sel_abilities_.push_back(x.first);
			}

			i++;
		}
	}

	// Note : attacks and abilities are not written to the config, since they have macros.
}

void editor_edit_unit::update_resistances() {
	find_widget<slider>(get_window(), "resistances_slider", false)
		.set_value(
			100 - resistances_[find_widget<menu_button>(get_window(), "resistances_list", false).get_value_string()]);

	find_widget<slider>(get_window(), "resistances_slider", false)
		.set_active(res_toggles_[find_widget<menu_button>(get_window(), "resistances_list", false).get_value()]);

	find_widget<toggle_button>(get_window(), "resistances_checkbox", false)
		.set_value(res_toggles_[find_widget<menu_button>(get_window(), "resistances_list", false).get_value()]);
}

void editor_edit_unit::store_resistances() {
	resistances_[find_widget<menu_button>(get_window(), "resistances_list", false).get_value_string()]
		= 100 - find_widget<slider>(get_window(), "resistances_slider", false).get_value();
}

void editor_edit_unit::enable_resistances_slider() {
	bool toggle = find_widget<toggle_button>(get_window(), "resistances_checkbox", false).get_value();
	res_toggles_[find_widget<menu_button>(get_window(), "resistances_list", false).get_value()] = toggle;
	find_widget<slider>(get_window(), "resistances_slider", false).set_active(toggle);
}

void editor_edit_unit::update_defenses() {
	find_widget<slider>(get_window(), "defense_slider", false)
		.set_value(
			100 - defenses_[find_widget<menu_button>(get_window(), "defense_list", false).get_value_string()]);

	find_widget<slider>(get_window(), "defense_slider", false)
		.set_active(def_toggles_[find_widget<menu_button>(get_window(), "defense_list", false).get_value()]);

	find_widget<toggle_button>(get_window(), "defense_checkbox", false)
		.set_value(def_toggles_[find_widget<menu_button>(get_window(), "defense_list", false).get_value()]);
}

void editor_edit_unit::store_defenses() {
	defenses_[find_widget<menu_button>(get_window(), "defense_list", false).get_value_string()]
		= 100 - find_widget<slider>(get_window(), "defense_slider", false).get_value();
}

void editor_edit_unit::enable_defense_slider() {
	bool toggle = find_widget<toggle_button>(get_window(), "defense_checkbox", false).get_value();
	def_toggles_[find_widget<menu_button>(get_window(), "defense_list", false).get_value()] = toggle;
	find_widget<slider>(get_window(), "defense_slider", false).set_active(toggle);
}

void editor_edit_unit::update_movement_costs() {
	find_widget<slider>(get_window(), "movement_costs_slider", false)
		.set_value(
			movement_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value_string()]);

	find_widget<slider>(get_window(), "movement_costs_slider", false)
		.set_active(move_toggles_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value()]);

	find_widget<toggle_button>(get_window(), "movement_costs_checkbox", false)
		.set_value(move_toggles_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value()]);
}

void editor_edit_unit::store_movement_costs() {
	movement_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value_string()]
		= find_widget<slider>(get_window(), "movement_costs_slider", false).get_value();
}

void editor_edit_unit::enable_movement_slider() {
	bool toggle = find_widget<toggle_button>(get_window(), "movement_costs_checkbox", false).get_value();
	move_toggles_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value()] = toggle;
	find_widget<slider>(get_window(), "movement_costs_slider", false).set_active(toggle);
}

void editor_edit_unit::store_attack() {
	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	// Save current attack data
	if (selected_attack_ < 1) {
		return;
	}

	config& attack = attacks_.at(selected_attack_-1).second;

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	attack["name"] = find_widget<text_box>(get_window(), "atk_id_box", false).get_value();
	attack["description"] = t_string(find_widget<text_box>(get_window(), "atk_name_box", false).get_value(), current_textdomain);
	attack["icon"] = find_widget<text_box>(get_window(), "path_attack_image", false).get_value();
	attack["type"] = find_widget<menu_button>(get_window(), "attack_type_list", false).get_value_string();
	attack["damage"] = find_widget<slider>(get_window(), "dmg_box", false).get_value();
	attack["number"] = find_widget<slider>(get_window(), "dmg_num_box", false).get_value();
	if (find_widget<toggle_button>(get_window(), "range_melee", false).get_value()) {
		attack["range"] = "melee";
	}
	if (find_widget<toggle_button>(get_window(), "range_ranged", false).get_value()) {
		attack["range"] = "ranged";
	}

	attacks_.at(selected_attack_-1).first = find_widget<multimenu_button>(get_window(), "weapon_specials_list", false).get_toggle_states();

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::update_attacks() {
	//Load data
	config& attack = attacks_.at(selected_attack_-1).second;

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	find_widget<text_box>(get_window(), "atk_id_box", false).set_value(attack["name"]);
	find_widget<text_box>(get_window(), "atk_name_box", false).set_value(attack["description"]);
	find_widget<text_box>(get_window(), "path_attack_image", false).set_value(attack["icon"]);
	update_image("attack_image");
	find_widget<slider>(get_window(), "dmg_box", false).set_value(attack["damage"]);
	find_widget<slider>(get_window(), "dmg_num_box", false).set_value(attack["number"]);

	if (attack["range"] == "melee") {
		find_widget<toggle_button>(get_window(), "range_melee", false).set_value(true);
		find_widget<toggle_button>(get_window(), "range_ranged", false).set_value(false);
	}
	if (attack["range"] == "ranged") {
		find_widget<toggle_button>(get_window(), "range_melee", false).set_value(false);
		find_widget<toggle_button>(get_window(), "range_ranged", false).set_value(true);
	}

	set_selected_from_string(
			find_widget<menu_button>(get_window(), "attack_type_list", false),
			resistances_list_,
			attack["type"]);

	find_widget<multimenu_button>(get_window(), "weapon_specials_list", false)
		.select_options(attacks_.at(selected_attack_-1).first);

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::update_index() {
	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	if (selected_attack_ <= 1) {
		find_widget<button>(get_window(), "atk_prev", false).set_active(false);
	} else {
		find_widget<button>(get_window(), "atk_prev", false).set_active(true);
	}

	if (selected_attack_ > 0) {
		find_widget<button>(get_window(), "atk_delete", false).set_active(true);
	} else {
		find_widget<button>(get_window(), "atk_delete", false).set_active(false);
	}

	if (selected_attack_ == attacks_.size()) {
		find_widget<button>(get_window(), "atk_next", false).set_active(false);
	} else {
		find_widget<button>(get_window(), "atk_next", false).set_active(true);
	}

	if (attacks_.size() > 0) {
		std::vector<config> atk_name_list;
		for(const auto& atk_data : attacks_) {
			atk_name_list.emplace_back("label", atk_data.second["name"]);
		}
		menu_button& atk_list = find_widget<menu_button>(get_window(), "atk_list", false);
		atk_list.set_values(atk_name_list);
		atk_list.set_selected(selected_attack_-1, false);
	}

	//Set index
	const std::string new_index_str = formatter() << selected_attack_ << "/" << attacks_.size();
	find_widget<label>(get_window(), "atk_number", false).set_label(new_index_str);

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::add_attack() {
	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	config attack;

	attack["name"] = find_widget<text_box>(get_window(), "atk_id_box", false).get_value();
	attack["description"] = t_string(find_widget<text_box>(get_window(), "atk_name_box", false).get_value(), current_textdomain);
	attack["icon"] = find_widget<text_box>(get_window(), "path_attack_image", false).get_value();
	attack["type"] = find_widget<menu_button>(get_window(), "attack_type_list", false).get_value_string();
	attack["damage"] = find_widget<slider>(get_window(), "dmg_box", false).get_value();
	attack["number"] = find_widget<slider>(get_window(), "dmg_num_box", false).get_value();
	if (find_widget<toggle_button>(get_window(), "range_melee", false).get_value()) {
		attack["range"] = "melee";
	}
	if (find_widget<toggle_button>(get_window(), "range_ranged", false).get_value()) {
		attack["range"] = "ranged";
	}

	selected_attack_++;

	attacks_.insert(attacks_.begin() + selected_attack_ - 1
			, std::make_pair(
					find_widget<multimenu_button>(get_window(), "weapon_specials_list", false).get_toggle_states()
					, attack));

	update_index();

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::delete_attack() {
	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	//remove attack
	if (attacks_.size() > 0) {
		attacks_.erase(attacks_.begin() + selected_attack_ - 1);
	}

	if (attacks_.size() == 0) {
		// clear fields instead since there are no attacks to show
		selected_attack_ = 0;
		find_widget<button>(get_window(), "atk_delete", false).set_active(false);
		update_index();
	} else {
		if (selected_attack_ == 1) {
			// 1st attack removed, show the next one
			next_attack();
		} else {
			// show previous attack otherwise
			prev_attack();
		}
	}

	update_index();

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::next_attack() {
	store_attack();

	if (attacks_.size() > 1) {
		selected_attack_++;
		update_attacks();
	}

	update_index();
}

void editor_edit_unit::prev_attack() {
	store_attack();

	if (selected_attack_ > 0) {
		selected_attack_--;
	}

	if (attacks_.size() > 1) {
		update_attacks();
	}

	update_index();
}

void editor_edit_unit::select_attack() {
	selected_attack_ = find_widget<menu_button>(get_window(), "atk_list", false).get_value()+1;
	update_attacks();
	update_index();
}

//TODO Check if works with non-mainline movetypes
void editor_edit_unit::load_movetype() {
	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(1);

	for(const auto& movetype : game_config_
		.mandatory_child("units")
		.child_range("movetype")) {
		if (movetype["name"] == find_widget<menu_button>(get_window(), "movetype_list", false).get_value_string()) {
			// Set resistances
			resistances_ = movetype.mandatory_child("resistance");
			update_resistances();
			// Set defense
			defenses_ = movetype.mandatory_child("defense");
			update_defenses();
			// Set movement
			movement_ = movetype.mandatory_child("movement_costs");
			update_movement_costs();
		}
	}

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::write_macro(std::ostream& out, unsigned level, const std::string macro_name)
{
	for(unsigned i = 0; i < level; i++)
	{
		out << "\t";
	}
	out << "{" << macro_name << "}\n";
}

void editor_edit_unit::update_wml_view() {
	store_attack();
	save_unit_type();

	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);
	tabs.select_tab(3);

	std::stringstream wml_stream;

	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	wml_stream
	    << "#textdomain " << current_textdomain << "\n"
		<< "#\n"
		<< "# This file was generated using the scenario editor.\n"
		<< "#\n";

	{
		config_writer out(wml_stream, false);
		int level = 0;

		out.open_child("unit_type");

		level++;
		for (const auto& attr : type_cfg_.mandatory_child("unit_type").attribute_range()) {
			::write_key_val(wml_stream, attr.first, attr.second, level, current_textdomain);
		}

		// Abilities
		if (!sel_abilities_.empty()) {
			out.open_child("abilities");
			level++;
			for (const std::string& ability : sel_abilities_) {
				write_macro(wml_stream, level, ability);
			}
			level--;
			out.close_child("abilities");
		}

		// Attacks
		if (!attacks_.empty()) {
			for (const auto& atk : attacks_) {
				out.open_child("attack");
				level++;
				for (const auto& attr : atk.second.attribute_range()) {
					if (!attr.second.empty()) {
						::write_key_val(wml_stream, attr.first, attr.second, level, current_textdomain);
					}
				}

				if(atk.first.any()) {
					out.open_child("specials");
					level++;
					int i = 0;
					for (const auto& attr : specials_map_) {
						if (atk.first[i]) {
							write_macro(wml_stream, level, attr.first);
						}
						i++;
					}
					level--;
					out.close_child("specials");

				}
				level--;
				out.close_child("attack");
			}
		}

		if (!movement_.empty() && (move_toggles_.size() <= movement_.attribute_count()) && move_toggles_.any())
		{
			out.open_child("movement_costs");
			level++;
			int i = 0;
			for (const auto& attr : movement_.attribute_range()) {
				if (move_toggles_[i] == 1) {
					::write_key_val(wml_stream, attr.first, attr.second, level, current_textdomain);
				}
				i++;
			}
			level--;
			out.close_child("movement_costs");
		}

		if (!defenses_.empty() && def_toggles_.any()  && (def_toggles_.size() <= defenses_.attribute_count()))
		{
			out.open_child("defense");
			level++;
			int i = 0;
			for (const auto& attr : defenses_.attribute_range()) {
				if (def_toggles_[i] == 1) {
					::write_key_val(wml_stream, attr.first, attr.second, level, current_textdomain);
				}
				i++;
			}
			level--;
			out.close_child("defense");
		}

		if (!resistances_.empty() && res_toggles_.any()  && (res_toggles_.size() <= resistances_.attribute_count()))
		{
			out.open_child("resistance");
			level++;
			int i = 0;
			for (const auto& attr : resistances_.attribute_range()) {
				if (res_toggles_[i] == 1) {
					::write_key_val(wml_stream, attr.first, attr.second, level, current_textdomain);
				}
				i++;
			}
			level--;
			out.close_child("resistance");
		}

		out.close_child("unit_type");
	}

	generated_wml = wml_stream.str();

	find_widget<scroll_text>(get_window(), "wml_view", false).set_label(generated_wml);
}

void editor_edit_unit::update_image(const std::string& id_stem) {
	std::string rel_path = find_widget<text_box>(get_window(), "path_"+id_stem, false).get_value();

	// remove IPF
	if (rel_path.find("~") != std::string::npos) {
		rel_path = rel_path.substr(0, rel_path.find("~"));
	}

	int scale_size = 200; // TODO: Arbitrary, can be changed later.
	if (rel_path.size() > 0) {
		point img_size = ::image::get_size(::image::locator{rel_path});
		float aspect_ratio = static_cast<float>(img_size.x)/img_size.y;
		if(img_size.x > scale_size) {
			rel_path.append("~SCALE(" + std::to_string(scale_size) + "," + std::to_string(scale_size*aspect_ratio) + ")");
		} else if (img_size.y > scale_size) {
			rel_path.append("~SCALE(" + std::to_string(scale_size/aspect_ratio) + "," + std::to_string(scale_size) + ")");
		}
	}

	if (id_stem == "portrait_image") {
		// portrait image uses same [image] as unit_image
		find_widget<image>(get_window(), "unit_image", false).set_label(rel_path);
	} else {
		find_widget<image>(get_window(), id_stem, false).set_label(rel_path);
	}

	get_window()->invalidate_layout();
	get_window()->queue_redraw();
}

bool editor_edit_unit::check_id(std::string id) {
	for(char c : id) {
		if (!(std::isalnum(c) || c == '_' || c == ' ')) {
			// One bad char means entire id string is invalid
			return false;
		}
	}
	return true;
}

void editor_edit_unit::button_state_change() {
	tab_container& tabs = find_widget<tab_container>(get_window(), "tabs", false);

	std::string id = find_widget<text_box>(tabs.get_tab_grid(0), "id_box", false).get_value();
	std::string name = find_widget<text_box>(tabs.get_tab_grid(0), "name_box", false).get_value();

	if (
		id.empty()
		|| name.empty()
		|| !check_id(id)
	) {
		find_widget<button>(get_window(), "ok", false).set_active(false);
	} else {
		find_widget<button>(get_window(), "ok", false).set_active(true);
	}

	get_window()->queue_redraw();
}

void editor_edit_unit::write() {
	// Write the file
	update_wml_view();

	std::string unit_name = type_cfg_.mandatory_child("unit_type")["name"];
	boost::algorithm::replace_all(unit_name, " ", "_");

	// Path to <unit_type_name>.cfg
	std::string unit_path = filesystem::get_current_editor_dir(addon_id_) + "/units/" + unit_name + filesystem::wml_extension;

	// Write to file
	try {
		filesystem::write_file(unit_path, generated_wml);
		gui2::show_transient_message("", _("Unit type saved."));
	} catch(const filesystem::io_exception& e) {
		gui2::show_transient_message("", e.what());
	}
}

void editor_edit_unit::signal_handler_sdl_key_down(const event::ui_event /*event*/,
										 bool& handled,
										 const SDL_Keycode key,
										 SDL_Keymod modifier)
{
	#ifdef __APPLE__
		// Idiomatic modifier key in macOS computers.
		const SDL_Keycode modifier_key = KMOD_GUI;
	#else
		// Idiomatic modifier key in Microsoft desktop environments. Common in
		// GNU/Linux as well, to some extent.
		const SDL_Keycode modifier_key = KMOD_CTRL;
	#endif

	// Ctrl+O shortcut for Load Unit Type
	switch(key) {
		case SDLK_o:
			if (modifier & modifier_key) {
				handled = true;
				load_unit_type();
			}
			break;
	}

}

}
