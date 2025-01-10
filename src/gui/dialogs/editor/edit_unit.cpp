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
#include "gettext.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/units_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
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

// TODO properly support attributes from installed addons
editor_edit_unit::editor_edit_unit(const game_config_view& game_config, const std::string& addon_id)
	: modal_dialog(window_id())
	, game_config_(game_config)
	, addon_id_(addon_id)
{
	//TODO some weapon specials can have args (PLAGUE_TYPE)
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

void editor_edit_unit::pre_show() {
	tab_container& tabs = find_widget<tab_container>("tabs");
	connect_signal_notify_modified(tabs, std::bind(&editor_edit_unit::on_page_select, this));

	button& quit = find_widget<button>("exit");
	connect_signal_mouse_left_click(quit, std::bind(&editor_edit_unit::quit_confirmation, this));

	//
	// Main Stats tab
	//

	tabs.select_tab(0);

	menu_button& alignments = find_widget<menu_button>("alignment_list");
	for (auto& align : unit_alignments::values) {
		// Show the user the translated strings,
		// but use the untranslated align strings for generated WML
		const std::string& icon_path = "icons/alignments/alignment_" + std::string(align) + "_30.png";
		align_list_.emplace_back("label", t_string(static_cast<std::string>(align), "wesnoth"), "icon", icon_path);
	}
	alignments.set_values(align_list_);

	menu_button& races = find_widget<menu_button>("race_list");
	for(const race_map::value_type& i : unit_types.races()) {
		const std::string& race_name = i.second.id();
		race_list_.emplace_back("label", race_name, "icon", i.second.get_icon_path_stem() + "_30.png");
	}

	if (!race_list_.empty()) {
		races.set_values(race_list_);
	}

	button& load = find_widget<button>("load_unit_type");
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
		find_widget<button>("browse_unit_image"),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/units", "unit_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("preview_unit_image"),
		std::bind(&editor_edit_unit::update_image, this, "unit_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("browse_portrait_image"),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/portraits", "portrait_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("preview_portrait_image"),
		std::bind(&editor_edit_unit::update_image, this, "portrait_image"));

	connect_signal_notify_modified(
		find_widget<text_box>("name_box"),
		std::bind(&editor_edit_unit::button_state_change, this));
	connect_signal_notify_modified(
		find_widget<text_box>("id_box"),
		std::bind(&editor_edit_unit::button_state_change, this));

	//
	// Advanced Tab
	//
	tabs.select_tab(1);

	menu_button& movetypes = find_widget<menu_button>("movetype_list");
	for(const auto& mt : unit_types.movement_types()) {
		movetype_list_.emplace_back("label", mt.first);
	}

	if (!movetype_list_.empty()) {
		movetypes.set_values(movetype_list_);
	}

	menu_button& defenses = find_widget<menu_button>("defense_list");
	const config& defense_attr = game_config_
				.mandatory_child("units")
				.mandatory_child("movetype")
				.mandatory_child("defense");
	for (const auto& [key, _] : defense_attr.attribute_range()) {
		defense_list_.emplace_back("label", key);
	}

	menu_button& movement_costs = find_widget<menu_button>("movement_costs_list");
	if (!defense_list_.empty()) {
		defenses.set_values(defense_list_);
		def_toggles_.resize(defense_list_.size());
		movement_costs.set_values(defense_list_);
		move_toggles_.resize(defense_list_.size());
	}

	menu_button& resistances = find_widget<menu_button>("resistances_list");

	const config& resistances_attr = game_config_
				.mandatory_child("units")
				.mandatory_child("movetype")
				.mandatory_child("resistance");
	for (const auto& [key, _] : resistances_attr.attribute_range()) {
		resistances_list_.emplace_back("label", key, "icon", "icons/profiles/" + key + ".png");
	}

	if (!resistances_list_.empty()) {
		resistances.set_values(resistances_list_);
		res_toggles_.resize(resistances_list_.size());
	}

	menu_button& usage_types = find_widget<menu_button>("usage_list");
	usage_type_list_.emplace_back("label", _("scout"));
	usage_type_list_.emplace_back("label", _("fighter"));
	usage_type_list_.emplace_back("label", _("archer"));
	usage_type_list_.emplace_back("label", _("mixed fighter"));
	usage_type_list_.emplace_back("label", _("healer"));
	usage_types.set_values(usage_type_list_);

	multimenu_button& abilities = find_widget<multimenu_button>("abilities_list");
	abilities.set_values(abilities_list_);

	connect_signal_mouse_left_click(
		find_widget<button>("browse_small_profile_image"),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/portraits", "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("preview_small_profile_image"),
		std::bind(&editor_edit_unit::update_image, this, "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("load_movetype"),
		std::bind(&editor_edit_unit::load_movetype, this));
	connect_signal_notify_modified(
		find_widget<slider>("resistances_slider"),
		std::bind(&editor_edit_unit::store_resistances, this));
	connect_signal_notify_modified(
		find_widget<menu_button>("resistances_list"),
		std::bind(&editor_edit_unit::update_resistances, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>("resistances_checkbox"),
		std::bind(&editor_edit_unit::enable_resistances_slider, this));

	connect_signal_notify_modified(
		find_widget<slider>("defense_slider"),
		std::bind(&editor_edit_unit::store_defenses, this));
	connect_signal_notify_modified(
		find_widget<menu_button>("defense_list"),
		std::bind(&editor_edit_unit::update_defenses, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>("defense_checkbox"),
		std::bind(&editor_edit_unit::enable_defense_slider, this));

	connect_signal_notify_modified(
		find_widget<slider>("movement_costs_slider"),
		std::bind(&editor_edit_unit::store_movement_costs, this));
	connect_signal_notify_modified(
		find_widget<menu_button>("movement_costs_list"),
		std::bind(&editor_edit_unit::update_movement_costs, this));
	connect_signal_mouse_left_click(
		find_widget<toggle_button>("movement_costs_checkbox"),
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
	multimenu_button& specials = find_widget<multimenu_button>("weapon_specials_list");
	specials.set_values(specials_list_);

	combobox& attack_types = find_widget<combobox>("attack_type_list");
	if (!resistances_list_.empty()) {
		attack_types.set_values(resistances_list_);
	}

	// Connect signals
	connect_signal_mouse_left_click(
		find_widget<button>("browse_attack_image"),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/attacks", "attack_image"));
	connect_signal_mouse_left_click(
		find_widget<button>("preview_attack_image"),
		std::bind(&editor_edit_unit::update_image, this, "attack_image"));
	connect_signal_notify_modified(
		find_widget<menu_button>("atk_list"),
		std::bind(&editor_edit_unit::select_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>("atk_new"),
		std::bind(&editor_edit_unit::add_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>("atk_delete"),
		std::bind(&editor_edit_unit::delete_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>("atk_next"),
		std::bind(&editor_edit_unit::next_attack, this));
	connect_signal_mouse_left_click(
		find_widget<button>("atk_prev"),
		std::bind(&editor_edit_unit::prev_attack, this));

	update_index();

	tabs.select_tab(0);

	// Disable OK button at start, since ID and Name boxes are empty
	button_state_change();
}

void editor_edit_unit::on_page_select()
{
	save_unit_type();

	tab_container& tabs = find_widget<tab_container>("tabs");
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
						= _("This file is outside Wesnoth’s data dirs. Do you wish to copy it into your add-on?");

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

		find_widget<text_box>("path_"+id_stem).set_value(dn);
		update_image(id_stem);
	}
}

void editor_edit_unit::load_unit_type() {
	const auto& all_type_list = unit_types.types_list();
	const auto& type_select = gui2::dialogs::units_dialog::build_create_dialog(all_type_list);

	if (!type_select->show() && !type_select->is_selected()) {
		return;
	}

	const auto& type = all_type_list[type_select->get_selected_index()];

	tab_container& tabs = find_widget<tab_container>("tabs");
	tabs.select_tab(0);

	find_widget<text_box>("id_box").set_value(type->id());
	find_widget<text_box>("name_box").set_value(type->type_name().base_str());
	find_widget<spinner>("level_box").set_value(type->level());
	find_widget<slider>("cost_slider").set_value(type->cost());
	find_widget<text_box>("adv_box").set_value(utils::join(type->advances_to()));
	find_widget<slider>("hp_slider").set_value(type->hitpoints());
	find_widget<slider>("xp_slider").set_value(type->experience_needed());
	find_widget<slider>("move_slider").set_value(type->movement());
	find_widget<scroll_text>("desc_box").set_value(type->unit_description().base_str());
	find_widget<text_box>("path_unit_image").set_value(type->image());
	find_widget<text_box>("path_portrait_image").set_value(type->big_profile());

	for (const auto& gender : type->genders())
	{
		if (gender == unit_race::GENDER::MALE) {
			find_widget<toggle_button>("gender_male").set_value(true);
		}

		if (gender == unit_race::GENDER::FEMALE) {
			find_widget<toggle_button>("gender_female").set_value(true);
		}
	}

	set_selected_from_string(
			find_widget<menu_button>("race_list"),
			race_list_,
			type->race_id());

	set_selected_from_string(
			find_widget<menu_button>("alignment_list"),
			align_list_,
			unit_alignments::get_string(type->alignment()));

	update_image("unit_image");

	tabs.select_tab(1);
	find_widget<text_box>("path_small_profile_image").set_value(type->small_profile());

	set_selected_from_string(
			find_widget<menu_button>("movetype_list"),
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

		for (const auto& [key, _] : type->get_cfg().mandatory_child("resistance").attribute_range()) {
			if (resistances_list_.at(i)["label"] == key) {
				res_toggles_[i] = 1;
			}
		}
	}

	for (unsigned i = 0; i < defense_list_.size(); i++) {
		if (type->get_cfg().has_child("defense")) {
			for (const auto& [key, _] : type->get_cfg().mandatory_child("defense").attribute_range()) {
				if (defense_list_.at(i)["label"] == key) {
					def_toggles_[i] = 1;
				}
			}
		}

		if (type->get_cfg().has_child("movement_costs")) {
			for (const auto& [key, _] : type->get_cfg().mandatory_child("movement_costs").attribute_range()) {
				if (defense_list_.at(i)["label"] == key) {
					move_toggles_[i] = 1;
				}
			}
		}
	}

	update_resistances();
	update_defenses();
	update_resistances();

	set_selected_from_string(
			find_widget<menu_button>("usage_list"),
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

	if (!type->attacks().empty()) {
		selected_attack_ = 1;
		update_attacks();
	}

	update_index();

	tabs.select_tab(0);

	button_state_change();
	invalidate_layout();
}

void editor_edit_unit::save_unit_type() {

	// Clear the config
	type_cfg_.clear();

	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	tab_container& tabs = find_widget<tab_container>("tabs");

	// Page 1
	grid* grid = tabs.get_tab_grid(0);

	config& utype = type_cfg_.add_child("unit_type");
	utype["id"] = grid->find_widget<text_box>("id_box").get_value();
	utype["name"] = t_string(grid->find_widget<text_box>("name_box").get_value(), current_textdomain);
	utype["image"] = grid->find_widget<text_box>("path_unit_image").get_value();
	utype["profile"] = grid->find_widget<text_box>("path_portrait_image").get_value();
	utype["level"] = grid->find_widget<spinner>("level_box").get_value();
	utype["advances_to"] = grid->find_widget<text_box>("adv_box").get_value();
	utype["hitpoints"] = grid->find_widget<slider>("hp_slider").get_value();
	utype["experience"] = grid->find_widget<slider>("xp_slider").get_value();
	utype["cost"] = grid->find_widget<slider>("cost_slider").get_value();
	utype["movement"] = grid->find_widget<slider>("move_slider").get_value();
	utype["description"] = t_string(grid->find_widget<scroll_text>("desc_box").get_value(), current_textdomain);
	utype["race"] = grid->find_widget<menu_button>("race_list").get_value_string();
	utype["alignment"] = unit_alignments::values[grid->find_widget<menu_button>("alignment_list").get_value()];

	// Gender
	if (grid->find_widget<toggle_button>("gender_male").get_value()) {
		if (grid->find_widget<toggle_button>("gender_female").get_value()) {
			utype["gender"] = "male,female";
		} else {
			utype["gender"] = "male";
		}
	} else {
		if (grid->find_widget<toggle_button>("gender_female").get_value()) {
			utype["gender"] = "female";
		}
	}

	// Page 2
	grid = tabs.get_tab_grid(1);

	utype["small_profile"] = grid->find_widget<text_box>("path_small_profile_image").get_value();
	utype["movement_type"] = grid->find_widget<menu_button>("movetype_list").get_value_string();
	utype["usage"] = grid->find_widget<menu_button>("usage_list").get_value_string();

	if (res_toggles_.any()) {
		config& resistances = utype.add_child("resistance");
		int i = 0;
		for (const auto& [key, _] : resistances_.attribute_range()) {
			if (res_toggles_[i]) {
				resistances[key] = resistances_[key];
			}
			i++;
		}
	}

	if (def_toggles_.any()) {
		config& defenses = utype.add_child("defense");
		int i = 0;
		for (const auto& [key, _] : defenses_.attribute_range()) {
			if (def_toggles_[i]) {
				defenses[key] = defenses_[key];
			}
			i++;
		}
	}

	if (move_toggles_.any()) {
		config& movement_costs = utype.add_child("movement_costs");
		int i = 0;
		for (const auto& [key, _] : movement_.attribute_range()) {
			if (move_toggles_[i]) {
				movement_costs[key] = movement_[key];
			}
			i++;
		}
	}

	const auto& abilities_states = grid->find_widget<multimenu_button>("abilities_list").get_toggle_states();
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
	find_widget<slider>("resistances_slider")
		.set_value(
			100 - resistances_[find_widget<menu_button>("resistances_list").get_value_string()].to_int());

	find_widget<slider>("resistances_slider")
		.set_active(res_toggles_[find_widget<menu_button>("resistances_list").get_value()]);

	find_widget<toggle_button>("resistances_checkbox")
		.set_value(res_toggles_[find_widget<menu_button>("resistances_list").get_value()]);
}

void editor_edit_unit::store_resistances() {
	resistances_[find_widget<menu_button>("resistances_list").get_value_string()]
		= 100 - find_widget<slider>("resistances_slider").get_value();
}

void editor_edit_unit::enable_resistances_slider() {
	bool toggle = find_widget<toggle_button>("resistances_checkbox").get_value();
	res_toggles_[find_widget<menu_button>("resistances_list").get_value()] = toggle;
	find_widget<slider>("resistances_slider").set_active(toggle);
}

void editor_edit_unit::update_defenses() {
	find_widget<slider>("defense_slider")
		.set_value(
			100 - defenses_[find_widget<menu_button>("defense_list").get_value_string()].to_int());

	find_widget<slider>("defense_slider")
		.set_active(def_toggles_[find_widget<menu_button>("defense_list").get_value()]);

	find_widget<toggle_button>("defense_checkbox")
		.set_value(def_toggles_[find_widget<menu_button>("defense_list").get_value()]);
}

void editor_edit_unit::store_defenses() {
	defenses_[find_widget<menu_button>("defense_list").get_value_string()]
		= 100 - find_widget<slider>("defense_slider").get_value();
}

void editor_edit_unit::enable_defense_slider() {
	bool toggle = find_widget<toggle_button>("defense_checkbox").get_value();
	def_toggles_[find_widget<menu_button>("defense_list").get_value()] = toggle;
	find_widget<slider>("defense_slider").set_active(toggle);
}

void editor_edit_unit::update_movement_costs() {
	find_widget<slider>("movement_costs_slider")
		.set_value(
			movement_[find_widget<menu_button>("movement_costs_list").get_value_string()].to_int());

	find_widget<slider>("movement_costs_slider")
		.set_active(move_toggles_[find_widget<menu_button>("movement_costs_list").get_value()]);

	find_widget<toggle_button>("movement_costs_checkbox")
		.set_value(move_toggles_[find_widget<menu_button>("movement_costs_list").get_value()]);
}

void editor_edit_unit::store_movement_costs() {
	movement_[find_widget<menu_button>("movement_costs_list").get_value_string()]
		= find_widget<slider>("movement_costs_slider").get_value();
}

void editor_edit_unit::enable_movement_slider() {
	bool toggle = find_widget<toggle_button>("movement_costs_checkbox").get_value();
	move_toggles_[find_widget<menu_button>("movement_costs_list").get_value()] = toggle;
	find_widget<slider>("movement_costs_slider").set_active(toggle);
}

void editor_edit_unit::store_attack() {
	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	// Save current attack data
	if (selected_attack_ < 1) {
		return;
	}

	config& attack = attacks_.at(selected_attack_-1).second;

	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	attack["name"] = find_widget<text_box>("atk_id_box").get_value();
	attack["description"] = t_string(find_widget<text_box>("atk_name_box").get_value(), current_textdomain);
	attack["icon"] = find_widget<text_box>("path_attack_image").get_value();
	attack["type"] = find_widget<combobox>("attack_type_list").get_value();
	attack["damage"] = find_widget<slider>("dmg_box").get_value();
	attack["number"] = find_widget<slider>("dmg_num_box").get_value();
	attack["range"] = find_widget<combobox>("range_list").get_value();

	attacks_.at(selected_attack_-1).first = find_widget<multimenu_button>("weapon_specials_list").get_toggle_states();

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::update_attacks() {
	//Load data
	if (selected_attack_ < 1) {
		return;
	}

	config& attack = attacks_.at(selected_attack_-1).second;

	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	find_widget<text_box>("atk_id_box").set_value(attack["name"]);
	find_widget<text_box>("atk_name_box").set_value(attack["description"]);
	find_widget<text_box>("path_attack_image").set_value(attack["icon"]);
	update_image("attack_image");
	find_widget<slider>("dmg_box").set_value(attack["damage"].to_int());
	find_widget<slider>("dmg_num_box").set_value(attack["number"].to_int());
	find_widget<combobox>("range_list").set_value(attack["range"]);

	set_selected_from_string(
		find_widget<combobox>("attack_type_list"), resistances_list_, attack["type"]);

	find_widget<multimenu_button>("weapon_specials_list")
		.select_options(attacks_.at(selected_attack_-1).first);

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::update_index() {
	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	find_widget<button>("atk_prev").set_active(selected_attack_ > 1);
	find_widget<button>("atk_delete").set_active(selected_attack_ > 0);
	find_widget<button>("atk_next").set_active(selected_attack_ != attacks_.size());

	if (!attacks_.empty()) {
		std::vector<config> atk_name_list;
		for(const auto& atk_data : attacks_) {
			atk_name_list.emplace_back("label", atk_data.second["name"]);
		}
		menu_button& atk_list = find_widget<menu_button>("atk_list");
		atk_list.set_values(atk_name_list);
		atk_list.set_selected(selected_attack_-1, false);
	}

	//Set index
	const std::string new_index_str = formatter() << selected_attack_ << "/" << attacks_.size();
	find_widget<label>("atk_number").set_label(new_index_str);

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::add_attack() {
	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	config attack;

	attack["name"] = find_widget<text_box>("atk_id_box").get_value();
	attack["description"] = t_string(find_widget<text_box>("atk_name_box").get_value(), current_textdomain);
	attack["icon"] = find_widget<text_box>("path_attack_image").get_value();
	attack["type"] = find_widget<combobox>("attack_type_list").get_value();
	attack["damage"] = find_widget<slider>("dmg_box").get_value();
	attack["number"] = find_widget<slider>("dmg_num_box").get_value();
	attack["range"] = find_widget<combobox>("range_list").get_value();

	selected_attack_++;

	attacks_.insert(
		attacks_.begin() + selected_attack_ - 1
		, std::make_pair(find_widget<multimenu_button>("weapon_specials_list").get_toggle_states(), attack));

	update_index();

	tabs.select_tab(prev_tab);
}

void editor_edit_unit::delete_attack() {
	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(2);

	//remove attack
	if (!attacks_.empty()) {
		attacks_.erase(attacks_.begin() + selected_attack_ - 1);
	}

	if (attacks_.empty()) {
		// clear fields instead since there are no attacks to show
		selected_attack_ = 0;
		find_widget<button>("atk_delete").set_active(false);
	} else if (selected_attack_ == 1) {
		// 1st attack removed, show the next one
		next_attack();
	} else {
		// show previous attack otherwise
		prev_attack();
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
	selected_attack_ = find_widget<menu_button>("atk_list").get_value()+1;
	update_attacks();
	update_index();
}

//TODO Check if works with non-mainline movetypes
void editor_edit_unit::load_movetype() {
	tab_container& tabs = find_widget<tab_container>("tabs");
	int prev_tab = tabs.get_active_tab_index();
	tabs.select_tab(1);

	for(const auto& movetype : game_config_
		.mandatory_child("units")
		.child_range("movetype")) {
		if (movetype["name"] == find_widget<menu_button>("movetype_list").get_value_string()) {
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

void editor_edit_unit::write_macro(std::ostream& out, unsigned level, const std::string& macro_name)
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

	tab_container& tabs = find_widget<tab_container>("tabs");
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
		for (const auto& [key, value] : type_cfg_.mandatory_child("unit_type").attribute_range()) {
			::write_key_val(wml_stream, key, value, level, current_textdomain);
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
				for (const auto& [key, value] : atk.second.attribute_range()) {
					if (!value.empty()) {
						::write_key_val(wml_stream, key, value, level, current_textdomain);
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
			for (const auto& [key, value] : movement_.attribute_range()) {
				if (move_toggles_[i] == 1) {
					::write_key_val(wml_stream, key, value, level, current_textdomain);
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
			for (const auto& [key, value] : defenses_.attribute_range()) {
				if (def_toggles_[i] == 1) {
					::write_key_val(wml_stream, key, value, level, current_textdomain);
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
			for (const auto& [key, value] : resistances_.attribute_range()) {
				if (res_toggles_[i] == 1) {
					::write_key_val(wml_stream, key, value, level, current_textdomain);
				}
				i++;
			}
			level--;
			out.close_child("resistance");
		}

		out.close_child("unit_type");
	}

	generated_wml = wml_stream.str();

	find_widget<scroll_text>("wml_view").set_label(generated_wml);
}

void editor_edit_unit::update_image(const std::string& id_stem) {
	std::string rel_path = find_widget<text_box>("path_"+id_stem).get_value();

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
		find_widget<image>("unit_image").set_label(rel_path);
	} else {
		find_widget<image>(id_stem).set_label(rel_path);
	}

	invalidate_layout();
	queue_redraw();
}

bool editor_edit_unit::check_id(const std::string& id) {
	for(char c : id) {
		if (!(std::isalnum(c) || c == '_' || c == ' ')) {
			// One bad char means entire id string is invalid
			return false;
		}
	}
	return true;
}

void editor_edit_unit::button_state_change() {
	grid* grid = find_widget<tab_container>("tabs").get_tab_grid(0);

	std::string id = grid->find_widget<text_box>("id_box").get_value();
	std::string name = grid->find_widget<text_box>("name_box").get_value();

	find_widget<button>("ok").set_active(!id.empty() && !name.empty() && check_id(id));

	queue_redraw();
}

void editor_edit_unit::quit_confirmation() {
	const std::string& message
		= _("Unsaved changes will be lost. Do you want to leave?");
	if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
		set_retval(gui2::retval::CANCEL);
	}
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
