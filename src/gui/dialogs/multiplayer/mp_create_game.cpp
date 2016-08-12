/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/multiplayer/mp_create_game.hpp"

#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/combobox.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "settings.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

namespace gui2
{

REGISTER_DIALOG(mp_create_game)

tmp_create_game::tmp_create_game(const config& cfg, ng::create_engine& eng)
	: cfg_(cfg)
	, scenario_(nullptr)
	, engine_(eng)
	, use_map_settings_(register_bool(
			  "use_map_settings",
			  true,
			  preferences::use_map_settings,
			  preferences::set_use_map_settings,
			  dialog_callback<tmp_create_game,
							  &tmp_create_game::update_map_settings>))
	, fog_(register_bool("fog", true, preferences::fog, preferences::set_fog))
	, shroud_(register_bool("shroud", true, preferences::shroud, preferences::set_shroud))
	, start_time_(register_bool("random_start_time",
								true,
								preferences::random_start_time,
								preferences::set_random_start_time))
	, turns_(register_integer(
			  "turn_count", true, preferences::turns, preferences::set_turns))
	, gold_(register_integer("village_gold",
							 true,
							 preferences::village_gold,
							 preferences::set_village_gold))
	, support_(register_integer("village_support",
								false,
								preferences::village_support,
								preferences::set_village_support))
	, experience_(register_integer("experience_modifier",
								   true,
								   preferences::xp_modifier,
								   preferences::set_xp_modifier))
	//, options_manager_()
{
}

void tmp_create_game::pre_show(twindow& window)
{
	find_widget<tminimap>(&window, "minimap", false).set_config(&cfg_);

	tlistbox& list = find_widget<tlistbox>(&window, "map_list", false);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
		std::bind(&tmp_create_game::update_map,
			*this, std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::update_map>);
#endif

	// Load option (might turn it into a button later).
	//std::map<std::string, string_map> data;
	//string_map item;

	//item["label"] = _("Load Game");
	//item["tooltip"] = _("Load Game...");
	//data.emplace("game_name", item);

	list.clear();

	window.keyboard_capture(&list);

	//list.add_row(data);

	// User maps
	/*	FIXME implement user maps
		std::vector<std::string> maps;
		filesystem::get_files_in_dir(filesystem::get_user_data_dir() + "/editor/maps", &maps, nullptr,
	   filesystem::FILE_NAME_ONLY);

		for(const auto& map : maps) {
			std::map<std::string, t_string> item;
			item.emplace("label", map);
			list->add_row(item);
		}
	*/

	// Standard maps
	for(const auto & map : cfg_.child_range("multiplayer"))
	{
		if(map["allow_new_game"].to_bool(true)) {
			std::map<std::string, string_map> data;
			string_map item;

			item["label"] = map["name"].str();
			item["tooltip"] = map["name"].str();
			data.emplace("game_name", item);

			list.add_row(data);
		}
	}

	update_map_settings(window);

	//
	// Set up eras combobox
	//

	const std::vector<std::string>& era_names = engine_.extras_menu_item_names(ng::create_engine::ERA, false);
	if(era_names.empty()) {
		gui2::show_transient_message(window.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}

	find_widget<tcombobox>(&window, "eras", false).set_values(era_names);

	//
	// Set up mods list
	//
	tlistbox& mod_list = find_widget<tlistbox>(&window, "mod_list", false);

	//std::vector<std::string> mods = engine_.extras_menu_item_names(ng::create_engine::MOD, false);
	for(const auto& mod : engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = mod->name;
		data.emplace("mod_name", item);

		mod_list.add_row(data);
	}

	//
	// Set up tab control
	//
	tlistbox& tab_bar = find_widget<tlistbox>(&window, "tab_bar", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*tab_bar,
			std::bind(&tmp_create_game::on_tab_select,
				*this, std::ref(window)));
#else
	tab_bar.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::on_tab_select>);
#endif

	std::map<std::string, string_map> list_data;

	list_data["tab_label"]["label"] = _("General");
	tab_bar.add_row(list_data);

	list_data["tab_label"]["label"] = _("Custom Options");
	tab_bar.add_row(list_data);

	list_data["tab_label"]["label"] = _("Game Settings");
	tab_bar.add_row(list_data);

	tab_bar.select_row(0);

	on_tab_select(window);

	update_map(window);
}

void tmp_create_game::on_tab_select(twindow& window)
{
	const int i = find_widget<tlistbox>(&window, "tab_bar", false).get_selected_row();

	find_widget<tstacked_widget>(&window, "pager", false).select_layer(i);

	// TODO: don't hardcode
	if(i == 1) {
		update_options_list(window);
	}
}

void tmp_create_game::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		find_widget<tlistbox>(&window, "map_list", false);
	}
}

void tmp_create_game::update_options_list(twindow& window)
{
		const int index = find_widget<tlistbox>(&window, "map_list", false)
			.get_selected_row();

	scenario_ = &cfg_.child("multiplayer", index);

	ttree_view& options_tree = find_widget<ttree_view>(&window, "custom_options", false);

	//options_tree.clear();

	for(const auto& options : scenario_->child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = (*scenario_)["name"];
		data["tree_view_node_label"] = item;

		ttree_view_node& option_node = options_tree.add_node("option_node", data);

		data.clear();

		for(const auto& checkbox_option : options.child_range("checkbox")) {
			item["label"] = checkbox_option["name"];
			data["option_checkbox"] = item;

			ttree_view_node& node = option_node.add_child("option_checkbox_node", data);

			ttoggle_button* checkbox = dynamic_cast<ttoggle_button*>(node.find("option_checkbox", true));

			VALIDATE(checkbox, missing_widget("option_checkbox"));

			checkbox->set_value(checkbox_option["default"].to_bool());
			//checkbox->set_label(checkbox_option["name"].str());
		}

		for(const auto& combobox_option : options.child_range("combo")) {
			item["label"] = combobox_option["name"];
			data["combobox_label"] = item;

			ttree_view_node& node = option_node.add_child("option_combobox_node", data);

			std::vector<std::string> combo_items;

			for(const auto& item : combobox_option.child_range("item")) {
				combo_items.push_back(item["name"]);
			}

			if(!combo_items.empty()) {
				tcombobox* combobox = dynamic_cast<tcombobox*>(node.find("option_combobox", true));

				VALIDATE(combobox, missing_widget("option_combobox"));

				combobox->set_values(combo_items);
			}
		}

		for(const auto& slider_option : options.child_range("slider")) {
			item["label"] = slider_option["name"];
			data["slider_label"] = item;

			ttree_view_node& node = option_node.add_child("option_slider_node", data);

			tslider* slider = dynamic_cast<tslider*>(node.find("option_slider", true));

			VALIDATE(slider, missing_widget("option_slider"));

			slider->set_step_size(slider_option["step"].to_int());
			slider->set_minimum_value(slider_option["min"].to_int());
			slider->set_maximum_value(slider_option["max"].to_int());
			slider->set_value(slider_option["default"].to_int());
		}

		for(const auto& text_entry_option : options.child_range("entry")) {
			item["label"] = text_entry_option["name"];
			data["text_entry_label"] = item;

			ttree_view_node& node = option_node.add_child("option_text_entry_node", data);

			ttext_box* textbox = dynamic_cast<ttext_box*>(node.find("option_text_entry", true));

			VALIDATE(textbox, missing_widget("option_text_entry"));

			textbox->set_value(text_entry_option["default"].str());
		}

		// Add the Defaults button at the end
		option_node.add_child("options_default_button", {});
	}
}

void tmp_create_game::update_map(twindow& window)
{
	tminimap& minimap = find_widget<tminimap>(&window, "minimap", false);
	tcontrol& description = find_widget<tcontrol>(&window, "description", false);
	tcontrol& players = find_widget<tcontrol>(&window, "map_num_players", false);
	tcontrol& map_size = find_widget<tcontrol>(&window, "map_size", false);

	const int index = find_widget<tlistbox>(&window, "map_list", false)
			.get_selected_row();

	engine_.set_current_level(index);

	ng::scenario* current_scenario = dynamic_cast<ng::scenario*>(&engine_.current_level());

	//if(index >= 0) {
		scenario_ = &cfg_.child("multiplayer", index);

		minimap.set_map_data((*scenario_)["map_data"]);
		description.set_label((*scenario_)["description"]);
		players.set_label(std::to_string(current_scenario->num_players()));
		map_size.set_label(current_scenario->map_size());
	//} else {
	//	minimap.set_map_data("");
	//	description.set_label("");
	//	scenario_ = nullptr;
	//}

	update_map_settings(window);
	
	if(find_widget<tlistbox>(&window, "tab_bar", false).get_selected_row() == 1) {
		update_options_list(window);
	}
}

void tmp_create_game::update_map_settings(twindow& window)
{
	const bool use_map_settings = use_map_settings_->get_widget_value(window);

	fog_->widget_set_enabled(window, !use_map_settings, false);
	shroud_->widget_set_enabled(window, !use_map_settings, false);
	start_time_->widget_set_enabled(window, !use_map_settings, false);

	turns_->widget_set_enabled(window, !use_map_settings, false);
	gold_->widget_set_enabled(window, !use_map_settings, false);
	support_->widget_set_enabled(window, !use_map_settings, false);
	experience_->widget_set_enabled(window, !use_map_settings, false);

	if(use_map_settings) {
		if(scenario_) {
			const config& side = scenario_->child("side");

			fog_->set_widget_value(window, side["fog"].to_bool(true));
			shroud_->set_widget_value(window, side["shroud"].to_bool(false));
			start_time_->set_widget_value(window, (*scenario_)["random_start_time"].to_bool(true));

			turns_->set_widget_value(window, ::settings::get_turns((*scenario_)["turns"].str()));
			gold_->set_widget_value(window, ::settings::get_village_gold(side["village_gold"].str()));
			support_->set_widget_value(window,::settings::get_village_support(side["village_support"].str()));
			experience_->set_widget_value(window,::settings::get_xp_modifier((*scenario_)["experience_modifier"].str()));
		}
		// No scenario selected just leave the state unchanged for now.

	} else {

		// Fixme we should store the value and reuse it later...
		fog_->set_widget_value(window, preferences::fog());
		shroud_->set_widget_value(window, preferences::shroud());
		start_time_->set_widget_value(window, preferences::random_start_time());

		turns_->set_widget_value(window, preferences::turns());
		gold_->set_widget_value(window, preferences::village_gold());
		support_->set_widget_value(window, preferences::village_support());
		experience_->set_widget_value(window, preferences::xp_modifier());
	}
}

} // namespace gui2
