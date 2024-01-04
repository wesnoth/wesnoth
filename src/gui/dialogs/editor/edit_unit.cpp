/*
	Copyright (C) 2023 - 2023
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
#include "units/types.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multiline_text.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/spinbox.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
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
		/** Don't add any macros that have INTERNAL */
		if (x.first.find("INTERNAL") == std::string::npos) {
			abilities_list_.emplace_back("label", x.first, "checkbox", false);
		}
	}
}

void editor_edit_unit::pre_show(window& win) {
	menu_button& alignments = find_widget<menu_button>(&win, "alignment_list", false);
	align_list_.emplace_back("label", _("lawful"));
	align_list_.emplace_back("label", _("chaotic"));
	align_list_.emplace_back("label", _("neutral"));
	align_list_.emplace_back("label", _("liminal"));
	alignments.set_values(align_list_);

	menu_button& races = find_widget<menu_button>(&win, "race_list", false);
//	combobox& race_box = find_widget<combobox>(&win, "race_box", false);

	for(const race_map::value_type &i : unit_types.races()) {
		const std::string& race_name = i.second.id();
		race_list_.emplace_back("label", race_name);
	}

	if (race_list_.size() > 0) {
		races.set_values(race_list_);
//		race_box.set_values(race_list_);
	}

	menu_button& movetypes = find_widget<menu_button>(&win, "movetype_list", false);
	for (const config &movetype : game_config_.mandatory_child("units").child_range("movetype")) {
		movetype_list_.emplace_back("label", movetype["name"]);
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
	menu_button& attack_types = find_widget<menu_button>(&win, "attack_type_list", false);

	const config& resistances_attr = game_config_
			.mandatory_child("units")
			.mandatory_child("movetype")
			.mandatory_child("resistance");
	for (const auto& attribute : resistances_attr.attribute_range()) {
		resistances_list_.emplace_back("label", attribute.first);
	}

	if (resistances_list_.size() > 0) {
		resistances.set_values(resistances_list_);
		attack_types.set_values(resistances_list_);
		res_toggles_.resize(resistances_list_.size());
	}

	menu_button& usage_types = find_widget<menu_button>(&win, "usage_list", false);
	usage_type_list_.emplace_back("label", _("scout"));
	usage_type_list_.emplace_back("label", _("fighter"));
	usage_type_list_.emplace_back("label", _("archer"));
	usage_type_list_.emplace_back("label", _("mixed fighter"));
	usage_type_list_.emplace_back("label", _("healer"));
	usage_types.set_values(usage_type_list_);

	multimenu_button& specials = find_widget<multimenu_button>(&win, "weapon_specials_list", false);
	specials.set_values(specials_list_);
	multimenu_button& abilities = find_widget<multimenu_button>(&win, "abilities_list", false);
	abilities.set_values(abilities_list_);

	range_group.add_member(find_widget<toggle_button>(&win, "range_melee", false, true), range::type::melee);
	range_group.add_member(find_widget<toggle_button>(&win, "range_ranged", false, true), range::type::ranged);
	range_group.set_member_states(range::type::melee);

	// Connect signals
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
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_small_profile_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/portraits", "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_small_profile_image", false),
		std::bind(&editor_edit_unit::update_image, this, "small_profile_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "browse_attack_image", false),
		std::bind(&editor_edit_unit::select_file, this, "data/core/images/attacks", "attack_image"));
	connect_signal_mouse_left_click(
		find_widget<button>(&win, "preview_attack_image", false),
		std::bind(&editor_edit_unit::update_image, this, "attack_image"));

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "load_unit_type", false),
			std::bind(&editor_edit_unit::load_unit_type, this));

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

//	connect_signal_notify_modified(
//		find_widget<menu_button>(&win, "race_list", false),
//		std::bind(&editor_edit_unit::combobox_set_value, this, "race"));
//	connect_signal_notify_modified(
//		find_widget<menu_button>(&win, "alignment_list", false),
//		std::bind(&editor_edit_unit::combobox_set_value, this, "alignment"));

	connect_signal_notify_modified(
		find_widget<menu_button>(&win, "atk_list", false),
		std::bind(&editor_edit_unit::select_attack, this));

	connect_signal_notify_modified(
		find_widget<text_box>(&win, "name_box", false),
		std::bind(&editor_edit_unit::button_state_change, this));
	connect_signal_notify_modified(
		find_widget<text_box>(&win, "id_box", false),
		std::bind(&editor_edit_unit::button_state_change, this));

	// Disable OK button at start, since ID and Name boxes are empty
	button_state_change();

	// Attack page
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

	// Setup tabs
	listbox& selector = find_widget<listbox>(&win, "tabs", false);
	connect_signal_notify_modified(selector,
		std::bind(&editor_edit_unit::on_page_select, this));
	stacked_widget& page = find_widget<stacked_widget>(&win, "page", false);
	win.keyboard_capture(&selector);

	int main_index = 0;
	selector.select_row(main_index);
	page.select_layer(main_index);
}

void editor_edit_unit::on_page_select()
{
	save_unit_type();

	const int selected_row =
		std::max(0, find_widget<listbox>(get_window(), "tabs", false).get_selected_row());
	find_widget<stacked_widget>(get_window(), "page", false).select_layer(static_cast<unsigned int>(selected_row));

	if (selected_row == 3) {
		update_wml_view();
	}

	get_window()->queue_redraw();
}

void editor_edit_unit::select_file(const std::string& default_dir, const std::string& id_stem)
{
	gui2::dialogs::file_dialog dlg;
	dlg.set_title(_("Choose File"))
		.set_ok_label(_("Select"))
		.set_path(default_dir)
		.set_read_only(true);

	if (dlg.show()) {
		std::string images_dir = filesystem::get_core_images_dir();
		std::string addons_dir = filesystem::get_current_editor_dir(addon_id_) + "/images";
		std::stringstream path;

		if ((dlg.path().find(images_dir) == std::string::npos)
				&& (dlg.path().find(addons_dir) == std::string::npos)) {
			/* choosen file is outside wesnoth's images dir,
			 * copy image to addons directory */
			std::string filename = boost::filesystem::path(dlg.path()).filename().string();

			if (id_stem == "unit_image") {
				path << addons_dir + "/units/" + filename;
			} else if ((id_stem == "portrait_image")||(id_stem == "small_profile_image")) {
				path << addons_dir + "/portraits/" + filename;
			} else if (id_stem == "attack_image") {
				path << addons_dir + "/attacks/" + filename;
			}
			filesystem::copy_file(dlg.path(), path.str());
		} else {
			path << dlg.path();
		}

		/* Scale if too big by attaching ~SCALE()
		 * Create SDL surface to get width and height of image.
		 * &*dlg.path().begin() is used to convert std::string to char*
		 */
		SDL_Surface * img_surf = IMG_Load(&*dlg.path().begin());
		int w, h;
		bool big_image;
		if (img_surf->w > 300) {
			w = (img_surf->w > 200) ? 200 : img_surf->w;
			h = (img_surf->h > 200) ? 200 : img_surf->h;
			big_image = true;
		} else {
			big_image = false;
		}

		if (path.str().find(images_dir) != std::string::npos) {
			// Image in Wesnoth core dir
			path.str(path.str().replace(0, images_dir.size()+1, ""));
		} else if (path.str().find(addons_dir) != std::string::npos) {
			// Image in addons dir
			path.str(path.str().replace(0, addons_dir.size()+1, ""));
		}

		if (big_image) {
			path << "~SCALE(" << w << "," << h << ")";
		}
		find_widget<text_box>(get_window(), "path_"+id_stem, false).set_value(path.str());
		update_image(id_stem);
	}

}

void editor_edit_unit::load_unit_type() {
	gui2::dialogs::unit_create dlg_uc;
	if (dlg_uc.show()) {
		const unit_type *type = unit_types.find(dlg_uc.choice());
		find_widget<text_box>(get_window(), "id_box", false).set_value(type->id());
		find_widget<text_box>(get_window(), "name_box", false).set_value(type->type_name());
		find_widget<spinbox>(get_window(), "level_box", false).set_value(type->level());
		find_widget<slider>(get_window(), "cost_slider", false).set_value(type->cost());
		find_widget<text_box>(get_window(), "adv_box", false).set_value(utils::join(type->advances_to()));
		find_widget<slider>(get_window(), "hp_slider", false).set_value(type->hitpoints());
		find_widget<slider>(get_window(), "xp_slider", false).set_value(type->experience_needed());
		find_widget<slider>(get_window(), "move_slider", false).set_value(type->movement());
		find_widget<multiline_text>(get_window(), "desc_box", false).set_value(type->unit_description());
		find_widget<combobox>(get_window(), "race_box", false).set_value(type->race_id());
		find_widget<text_box>(get_window(), "alignment_box", false).set_value(unit_alignments::get_string(type->alignment()));
	}

	// FIXME: Complete this.
}

void editor_edit_unit::save_unit_type() {

	// Clear the config
	type_cfg_.clear();

	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	stacked_widget& page = find_widget<stacked_widget>(get_window(), "page", false);

	// Page 1
	page.select_layer(0);

	config& utype = type_cfg_.add_child("unit_type");
	utype["id"] = find_widget<text_box>(get_window(), "id_box", false).get_value();
	utype["name"] = t_string(find_widget<text_box>(get_window(), "name_box", false).get_value(), current_textdomain);
	utype["image"] = find_widget<text_box>(get_window(), "path_unit_image", false).get_value();
	utype["profile"] = find_widget<text_box>(get_window(), "path_portrait_image", false).get_value();
	utype["level"] = find_widget<spinbox>(get_window(), "level_box", false).get_value();
	utype["advances_to"] = find_widget<text_box>(get_window(), "adv_box", false).get_value();
	utype["hitpoints"] = find_widget<slider>(get_window(), "hp_slider", false).get_value();
	utype["experience"] = find_widget<slider>(get_window(), "xp_slider", false).get_value();
	utype["cost"] = find_widget<slider>(get_window(), "cost_slider", false).get_value();
	utype["movement"] = find_widget<slider>(get_window(), "move_slider", false).get_value();
	utype["description"] = t_string(find_widget<multiline_text>(get_window(), "desc_box", false).get_value(), current_textdomain);
	utype["race"] = find_widget<combobox>(get_window(), "race_box", false).get_value();
	utype["alignment"] = find_widget<text_box>(get_window(), "alignment_box", false).get_value();

	// Gender
	if (find_widget<toggle_button>(get_window(), "gender_male", false).get_value()) {
		if (find_widget<toggle_button>(get_window(), "gender_female", false).get_value()) {
			utype["gender"] = "male,female";
		} else {
			utype["gender"] = "male";
		}
	} else {
		if (find_widget<toggle_button>(get_window(), "gender_female", false).get_value()) {
			utype["gender"] = "female";
		}
	}

	// Page 2
	page.select_layer(1);

	utype["small_profile"] = find_widget<text_box>(get_window(), "path_small_profile_image", false).get_value();
	utype["movement_type"] = find_widget<menu_button>(get_window(), "movetype_list", false).get_value_string();
	utype["usage"] = find_widget<menu_button>(get_window(), "usage_list", false).get_value_string();

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

	const auto& abilities_states = find_widget<multimenu_button>(get_window(), "abilities_list", false).get_toggle_states();
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
}

void editor_edit_unit::store_defenses() {
	defenses_[find_widget<menu_button>(get_window(), "defense_list", false).get_value_string()]
		= 100 - find_widget<slider>(get_window(), "defense_slider", false).get_value();
}

void editor_edit_unit::enable_defense_slider() {
	if (find_widget<toggle_button>(get_window(), "defense_checkbox", false).get_value()) {
		find_widget<slider>(get_window(), "defense_slider", false).set_active(true);
	} else {
		find_widget<slider>(get_window(), "defense_slider", false).set_active(false);
	}
}

void editor_edit_unit::update_movement_costs() {
	find_widget<slider>(get_window(), "movement_costs_slider", false)
		.set_value(
			movement_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value_string()]);
}

void editor_edit_unit::store_movement_costs() {
	movement_[find_widget<menu_button>(get_window(), "movement_costs_list", false).get_value_string()]
		= find_widget<slider>(get_window(), "movement_costs_slider", false).get_value();
}

void editor_edit_unit::enable_movement_slider() {
	if (find_widget<toggle_button>(get_window(), "movement_costs_checkbox", false).get_value()) {
		find_widget<slider>(get_window(), "movement_costs_slider", false).set_active(true);
	} else {
		find_widget<slider>(get_window(), "movement_costs_slider", false).set_active(false);
	}
}

void editor_edit_unit::store_attack() {
	// Save current attack data

	config& attack = attacks_.at(selected_attack_-1).second;
	stacked_widget& page = find_widget<stacked_widget>(get_window(), "page", false);
	page.select_layer(2);

	attack["id"] = find_widget<text_box>(get_window(), "atk_id_box", false).get_value();
	attack["name"] = find_widget<text_box>(get_window(), "atk_name_box", false).get_value();
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
}

void editor_edit_unit::update_attacks() {
	//Load data
	config& attack = attacks_.at(selected_attack_-1).second;

	stacked_widget& page = find_widget<stacked_widget>(get_window(), "page", false);
	page.select_layer(2);

	find_widget<text_box>(get_window(), "atk_id_box", false).set_value(attack["id"]);
	find_widget<text_box>(get_window(), "atk_name_box", false).set_value(attack["name"]);
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

	for (unsigned int i = 0; i < resistances_list_.size(); i++) {
		if (resistances_list_.at(i)["label"] == attack["type"]) {
			find_widget<menu_button>(get_window(), "attack_type_list", false).set_value(i);
			break;
		}
	}

	find_widget<multimenu_button>(get_window(), "weapon_specials_list", false)
		.select_options(attacks_.at(selected_attack_-1).first);

}

void editor_edit_unit::update_index() {
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
		find_widget<menu_button>(get_window(), "atk_list", false).set_values(atk_name_list);
	}

	//Set index
	const std::string new_index_str = formatter() << selected_attack_ << "/" << attacks_.size();
	find_widget<label>(get_window(), "atk_number", false).set_label(new_index_str);
}

void editor_edit_unit::add_attack() {
	config attack;

	stacked_widget& page = find_widget<stacked_widget>(get_window(), "page", false);
	page.select_layer(2);
	attack["id"] = find_widget<text_box>(get_window(), "atk_id_box", false).get_value();
	attack["name"] = find_widget<text_box>(get_window(), "atk_name_box", false).get_value();
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
}

void editor_edit_unit::delete_attack() {
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

void editor_edit_unit::load_movetype() {
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
}

void editor_edit_unit::update_wml_view() {
	stacked_widget& page = find_widget<stacked_widget>(get_window(), "page", false);
	page.select_layer(3);

	std::stringstream wml_stream;

	wml_stream
		<< "#\n"
		<< "# This file was generated using the scenario editor.\n"
		<< "#\n";

	{
		config_writer out(wml_stream, false);
		out.write(type_cfg_);
	}

	// Abilities
	if (!sel_abilities_.empty()) {
		wml_stream <<  "[abilities]\n";
		for (const std::string& ability : sel_abilities_) {
			wml_stream << "\t{" << ability << "}\n";
		}
		wml_stream <<  "[/abilities]\n";
	}

	// Attacks
	if (!attacks_.empty()) {
		for (const auto& atk : attacks_) {
			wml_stream <<  "[attack]\n";
			for (const auto& attr : atk.second.attribute_range()) {
				if (!attr.second.empty()) {
					wml_stream << "\t" << attr.first << "=" << attr.second << "\n";
				}
			}

			if(atk.first.any()) {
				wml_stream <<  "\t[specials]\n";
				int i = 0;
				for (const auto& attr : specials_map_) {
					if (atk.first[i]) {
						wml_stream << "\t{" << attr.first << "}\n";
					}
					i++;
				}
				wml_stream <<  "\t[/specials]\n";
			}
			wml_stream <<  "[/attack]\n";
		}
	}

	generated_wml = wml_stream.str();

	find_widget<scroll_label>(get_window(), "wml_view", false).set_label(generated_wml);
}

void editor_edit_unit::update_image(const std::string& id_stem) {
	if (id_stem == "portrait_image") {
		// portrait image uses same [image] as unit_image
		find_widget<image>(get_window(), "unit_image", false).set_label(
			find_widget<text_box>(get_window(), "path_"+id_stem, false).get_value());
	} else {
		find_widget<image>(get_window(), id_stem, false).set_label(
			find_widget<text_box>(get_window(), "path_"+id_stem, false).get_value());
	}

	get_window()->invalidate_layout();
	get_window()->queue_redraw();
}

bool editor_edit_unit::check_id(std::string id) {
	for(char c : id) {
		if (!(std::isalnum(c) || c == '_')) {
			/* One bad char means entire id string is invalid */
			return false;
		}
	}
	return true;
}

void editor_edit_unit::button_state_change() {
	std::string id = find_widget<text_box>(get_window(), "id_box", false).get_value();
	std::string name = find_widget<text_box>(get_window(), "name_box", false).get_value();
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

void editor_edit_unit::button_state_change_id() {
	std::string id = find_widget<text_box>(get_window(), "id_box", false).get_value();
	find_widget<button>(get_window(), "ok", false).set_active( !id.empty() || check_id(id) );

	get_window()->queue_redraw();
}

void editor_edit_unit::write() {
	/** Write the file */
	save_unit_type();
	update_wml_view();

	std::string unit_name = type_cfg_.mandatory_child("unit_type")["name"];
	boost::algorithm::replace_all(unit_name, " ", "_");

	// Path to <unit_type_name>.cfg
	std::string unit_path = filesystem::get_current_editor_dir(addon_id_) + "/units/" + unit_name + ".cfg";

	// Write to file
	try {
		filesystem::write_file(unit_path, generated_wml);
	} catch(const filesystem::io_exception& e) {
		// TODO : Needs an error message
	}
}

}
