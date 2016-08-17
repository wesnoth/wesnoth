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

#include "config_assign.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/image.hpp"
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
#include "game_config.hpp"
#include "settings.hpp"
#include "formatter.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

namespace gui2
{

REGISTER_DIALOG(mp_create_game)

tmp_create_game::tmp_create_game(const config& cfg, ng::create_engine& create_eng)
	: cfg_(cfg)
	, create_engine_(create_eng)
	, config_engine_()
	, selected_game_index_(-1)
	, use_map_settings_(register_bool("use_map_settings",
			true, preferences::use_map_settings, preferences::set_use_map_settings,
				dialog_callback<tmp_create_game, &tmp_create_game::update_map_settings>))
	, fog_(register_bool("fog",
			true, preferences::fog, preferences::set_fog))
	, shroud_(register_bool("shroud",
			true, preferences::shroud, preferences::set_shroud))
	, start_time_(register_bool("random_start_time",
			true, preferences::random_start_time, preferences::set_random_start_time))
	, time_limit_(register_bool("time_limit",
			true, preferences::countdown, preferences::set_countdown,
				dialog_callback<tmp_create_game, &tmp_create_game::update_map_settings>))
	, turns_(register_integer("turn_count",
			true, preferences::turns, preferences::set_turns))
	, gold_(register_integer("village_gold",
			true, preferences::village_gold, preferences::set_village_gold))
	, support_(register_integer("village_support",
			false, preferences::village_support, preferences::set_village_support))
	, experience_(register_integer("experience_modifier",
			true, preferences::xp_modifier, preferences::set_xp_modifier))
	, init_turn_limit(register_integer("init_turn_limit",
			true, preferences::countdown_init_time, preferences::set_countdown_init_time))
	, turn_bonus_(register_integer("turn_bonus",
			true, preferences::countdown_turn_bonus, preferences::set_countdown_turn_bonus))
	, reservior_(register_integer("reservior",
			true, preferences::countdown_reservoir_time, preferences::set_countdown_reservoir_time))
	, action_bonus_(register_integer("action_bonus",
			true, preferences::countdown_action_bonus, preferences::set_countdown_action_bonus))
{
	level_types_ = {
		{ng::level::TYPE::SCENARIO, _("Scenarios")},
		{ng::level::TYPE::CAMPAIGN, _("Campaigns")},
		{ng::level::TYPE::USER_MAP, _("User Maps")},
		{ng::level::TYPE::USER_SCENARIO, _("User Scenarios")},
		{ng::level::TYPE::RANDOM_MAP, _("Random Maps")}
	};

	if(game_config::debug) {
		level_types_.push_back({ng::level::TYPE::SP_CAMPAIGN, _("SP Campaigns")});
	}

	level_types_.erase(std::remove_if(level_types_.begin(), level_types_.end(),
		[this](level_type_info& type_info) {
		return create_engine_.get_levels_by_type_unfiltered(type_info.first).empty();
	}), level_types_.end());

	create_engine_.init_active_mods();

	create_engine_.get_state() = saved_game();
	create_engine_.get_state().classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;
}

void tmp_create_game::pre_show(twindow& window)
{
	find_widget<tminimap>(&window, "minimap", false).set_config(&cfg_);

	find_widget<ttext_box>(&window, "game_name", false).set_value(config_engine_->game_name_default());

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "random_map_regenerate", false),
		std::bind(&tmp_create_game::regenerate_random_map, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "random_map_settings", false),
		std::bind(&tmp_create_game::show_generator_settings, this, std::ref(window)));

	tlistbox& list = find_widget<tlistbox>(&window, "games_list", false);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
		std::bind(&tmp_create_game::on_game_select,
			*this, std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::on_game_select>);
#endif

	window.keyboard_capture(&list);

	//
	// Set up filtering
	//
	connect_signal_notify_modified(find_widget<tslider>(&window, "num_players", false),
		std::bind(&tmp_create_game::num_players_filter_callback, this, std::ref(window)));

	//
	// Set up game types combobox
	//
	std::vector<std::string> game_types;

	for(level_type_info& type_info : level_types_) {
		game_types.push_back(type_info.second);
	}

	if(game_types.empty()) {
		gui2::show_transient_message(window.video(), "", _("No games found."));
		throw game::error(_("No games found."));
	}

	tcombobox& game_combobox = find_widget<tcombobox>(&window, "game_types", false);

	game_combobox.set_values(game_types);
	game_combobox.connect_click_handler(std::bind(&tmp_create_game::update_games_list, this, std::ref(window)));

	//
	// Set up eras combobox
	//
	tcombobox& eras_combobox = find_widget<tcombobox>(&window, "eras", false);

	const std::vector<std::string>& era_names = create_engine_.extras_menu_item_names(ng::create_engine::ERA, false);
	if(era_names.empty()) {
		gui2::show_transient_message(window.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}

	eras_combobox.set_values(era_names);
	eras_combobox.connect_click_handler(std::bind(&tmp_create_game::on_era_select, this, std::ref(window)));

	on_era_select(window);

	//
	// Set up mods list
	//
	tlistbox& mod_list = find_widget<tlistbox>(&window, "mod_list", false);

	//std::vector<std::string> mods = create_engine_.extras_menu_item_names(ng::create_engine::MOD, false);
	for(const auto& mod : create_engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = mod->name;
		data.emplace("mod_name", item);

		mod_list.add_row(data);

		const int index = mod_list.get_item_count() - 1;
		ttoggle_button& mog_toggle = find_widget<ttoggle_button>(mod_list.get_row_grid(index), "mod_active_state", false);

		// TODO
		//mog_toggle.set_active(true);
		mog_toggle.set_callback_state_change(std::bind(&tmp_create_game::on_mod_toggle, this, index, _1));
	}

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*mod_list,
			std::bind(&tmp_create_game::on_mod_select,
				*this, std::ref(window)));
#else
	mod_list.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::on_mod_select>);
#endif

	on_mod_select(window);

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

	display_games_of_type(window, ng::level::TYPE::SCENARIO);

	on_tab_select(window);
}

void tmp_create_game::on_game_select(twindow& window)
{
	if(find_widget<tlistbox>(&window, "games_list", false).get_selected_row() != selected_game_index_) {
		update_details(window);
	}
}

void tmp_create_game::on_tab_select(twindow& window)
{
	const int i = find_widget<tlistbox>(&window, "tab_bar", false).get_selected_row();

	find_widget<tstacked_widget>(&window, "pager", false).select_layer(i);

	switch(i) {
		case 0:
			// TODO: are there any General panel settings that need to be updated here?
			break;

		case 1:
			update_options_list(window);
			break;

		case 2:
			update_map_settings(window);
			break;
	}
}

void tmp_create_game::num_players_filter_callback(twindow& window)
{
	create_engine_.apply_level_filter(find_widget<tslider>(&window, "num_players", false).get_value());

	// TODO: actually display the filtered options
}

void tmp_create_game::update_games_list(twindow& window)
{
	const int index = find_widget<tcombobox>(&window, "game_types", false).get_value();

	display_games_of_type(window, level_types_[index].first);
}

void tmp_create_game::display_games_of_type(twindow& window, ng::level::TYPE type)
{
	create_engine_.set_current_level_type(type);

	tlistbox& list = find_widget<tlistbox>(&window, "games_list", false);

	list.clear();

	for(const auto& game : create_engine_.get_levels_by_type(type)) {
		if(!game.get()->can_launch_game()) {
			continue;
		}

		std::map<std::string, string_map> data;
		string_map item;

		if(type == ng::level::TYPE::CAMPAIGN || type == ng::level::TYPE::SP_CAMPAIGN) {
			item["label"] = game.get()->icon();
			data.emplace("game_icon", item);
		}

		item["label"] = game.get()->name();
		data.emplace("game_name", item);

		list.add_row(data);
	}

	// TODO: move to click handler?
	const bool is_random_map = type == ng::level::TYPE::RANDOM_MAP;

	find_widget<tbutton>(&window, "random_map_regenerate", false).set_active(is_random_map);
	find_widget<tbutton>(&window, "random_map_settings", false).set_active(is_random_map);

	update_details(window);
}

void tmp_create_game::on_mod_select(twindow& window)
{
	create_engine_.set_current_mod_index(find_widget<tlistbox>(&window, "mod_list", false).get_selected_row());

	find_widget<tcontrol>(&window, "description", false).set_label(
		create_engine_.current_extra(ng::create_engine::MOD).description);
}

void tmp_create_game::on_mod_toggle(const int index, twidget&)
{
	create_engine_.set_current_mod_index(index);
	create_engine_.toggle_current_mod();
}

void tmp_create_game::on_era_select(twindow& window)
{
	create_engine_.set_current_era_index(find_widget<tcombobox>(&window, "eras", false).get_value());

	find_widget<tcontrol>(&window, "description", false).set_label(
		create_engine_.current_extra(ng::create_engine::ERA).description);
}

template<typename T>
static config::attribute_value cav(const T& v)
{
	config::attribute_value res;
	res = v;
	return res;
}

void tmp_create_game::display_custom_options(ttree_view& tree, std::string&& type, const std::string& id, const config& cfg)
{
	auto& map = visible_options_[{type, id}];
	static const std::map<std::string, string_map> empty;

	for(const auto& options : cfg.child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = cfg["name"];
		data.emplace("tree_view_node_label", item);

		ttree_view_node& option_node = tree.add_node("option_node", data);

		for(const auto& checkbox_option : options.child_range("checkbox")) {
			data.clear();

			item["label"] = checkbox_option["name"];
			data.emplace("option_checkbox", item);

			ttree_view_node& node = option_node.add_child("option_checkbox_node", data);

			ttoggle_button* checkbox = dynamic_cast<ttoggle_button*>(node.find("option_checkbox", true));

			VALIDATE(checkbox, missing_widget("option_checkbox"));

			checkbox->set_value(checkbox_option["default"].to_bool());
			//checkbox->set_label(checkbox_option["name"].str());
			map[checkbox_option["id"]] = [checkbox]() { return cav(checkbox->get_value_bool()); };
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("checkbox")) {
			option_node.add_child("options_spacer_node", empty);
		}

		for(const auto& combobox_option : options.child_range("combo")) {
			data.clear();

			item["label"] = combobox_option["name"];
			data.emplace("combobox_label", item);

			std::vector<std::string> combo_items;
			std::vector<std::string> combo_values;

			for(const auto& item : combobox_option.child_range("item")) {
				combo_items.push_back(item["name"]);
				combo_values.push_back(item["value"]);
			}

			if(combo_items.empty()) {
				continue;
			}

			ttree_view_node& node = option_node.add_child("option_combobox_node", data);

			tcombobox* combobox = dynamic_cast<tcombobox*>(node.find("option_combobox", true));

			VALIDATE(combobox, missing_widget("option_combobox"));

			combobox->set_values(combo_items);

			map[combobox_option["id"]] = [combobox, combo_values]() { return cav(combo_values[combobox->get_value()]); };
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("combo")) {
			option_node.add_child("options_spacer_node", empty);
		}

		for(const auto& slider_option : options.child_range("slider")) {
			data.clear();

			item["label"] = slider_option["name"];
			data.emplace("slider_label", item);

			ttree_view_node& node = option_node.add_child("option_slider_node", data);

			tslider* slider = dynamic_cast<tslider*>(node.find("option_slider", true));

			VALIDATE(slider, missing_widget("option_slider"));

			slider->set_maximum_value(slider_option["max"].to_int());
			slider->set_minimum_value(slider_option["min"].to_int());
			slider->set_step_size(slider_option["step"].to_int(1));
			slider->set_value(slider_option["default"].to_int());

			map[slider_option["id"]] = [slider]() { return cav(slider->get_value()); };
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("slider")) {
			option_node.add_child("options_spacer_node", empty);
		}

		for(const auto& text_entry_option : options.child_range("entry")) {
			data.clear();

			item["label"] = text_entry_option["name"];
			data.emplace("text_entry_label", item);

			ttree_view_node& node = option_node.add_child("option_text_entry_node", data);

			ttext_box* textbox = dynamic_cast<ttext_box*>(node.find("option_text_entry", true));

			VALIDATE(textbox, missing_widget("option_text_entry"));

			textbox->set_value(text_entry_option["default"].str());

			map[text_entry_option["id"]] = [textbox]() { return cav(textbox->get_value()); };
		}

		// Add the Defaults button at the end
		option_node.add_child("options_default_button", empty);
	}
}

void tmp_create_game::update_options_list(twindow& window)
{
	ttree_view& options_tree = find_widget<ttree_view>(&window, "custom_options", false);

	// TODO: might be inefficient to regenerate this every single time this tab is selected
	// Maybe look into caching the result if no change has been made to the selection.
	visible_options_.clear();
	options_tree.clear();

	display_custom_options(options_tree,
		create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ? "campaign" : "multiplayer",
		create_engine_.current_level().data()["id"], create_engine_.current_level().data());

	display_custom_options(options_tree, "era", create_engine_.curent_era_cfg()["id"], create_engine_.curent_era_cfg());

	std::set<std::string> activemods(create_engine_.active_mods().begin(), create_engine_.active_mods().end());
	for(const auto& mod : create_engine_.get_const_extras_by_type(ng::create_engine::MP_EXTRA::MOD)) {
		if (activemods.find(mod->id) != activemods.end()) {
			display_custom_options(options_tree, "modification", mod->id, *mod->cfg);
		}
	}
}

void tmp_create_game::show_generator_settings(twindow& window)
{
	create_engine_.generator_user_config(window.video());
}

void tmp_create_game::regenerate_random_map(twindow& window)
{
	create_engine_.init_generated_level_data();

	update_details(window);
}

void tmp_create_game::update_details(twindow& window)
{
	tcontrol& description = find_widget<tcontrol>(&window, "description", false);
	tcontrol& players = find_widget<tcontrol>(&window, "map_num_players", false);
	tcontrol& map_size = find_widget<tcontrol>(&window, "map_size", false);

	selected_game_index_ = find_widget<tlistbox>(&window, "games_list", false).get_selected_row();

	create_engine_.set_current_level(selected_game_index_);

	create_engine_.current_level().set_metadata();

	config_engine_.reset(new ng::configure_engine(create_engine_.get_state()));
	config_engine_->update_initial_cfg(create_engine_.current_level().data());
	config_engine_->set_default_values();

	// TODO: display a message?
	if(tcombobox* eras = find_widget<tcombobox>(&window, "eras", false, false)) {
		eras->set_active(create_engine_.current_level().allow_era_choice());
	}

	// If the current random map doesn't have data, generate it
	if(create_engine_.current_level_type() == ng::level::TYPE::RANDOM_MAP) {
		if(create_engine_.generator_assigned() && create_engine_.current_level().data()["map_data"].empty()) {
			create_engine_.init_generated_level_data();
		}
	}

	description.set_label(create_engine_.current_level().description());
	description.set_use_markup(true);

	switch(create_engine_.current_level_type().v) {
		case ng::level::TYPE::SCENARIO:
		case ng::level::TYPE::USER_MAP:
		case ng::level::TYPE::USER_SCENARIO:
		case ng::level::TYPE::RANDOM_MAP: {
			ng::scenario* current_scenario = dynamic_cast<ng::scenario*>(&create_engine_.current_level());

			assert(current_scenario);

			create_engine_.get_state().classification().campaign = "";

			find_widget<tstacked_widget>(&window, "minimap_stack", false).select_layer(0);
			find_widget<tminimap>(&window, "minimap", false).set_map_data(current_scenario->data()["map_data"]);

			players.set_label(std::to_string(current_scenario->num_players()));
			map_size.set_label(current_scenario->map_size());

			break;
		}
		case ng::level::TYPE::CAMPAIGN:
		case ng::level::TYPE::SP_CAMPAIGN: {
			ng::campaign* current_campaign = dynamic_cast<ng::campaign*>(&create_engine_.current_level());

			assert(current_campaign);

			create_engine_.get_state().classification().campaign = current_campaign->data()["id"].str();

			const std::string image = formatter() << current_campaign->data()["image"] << "~SCALE_INTO(240,240)";

			find_widget<tstacked_widget>(&window, "minimap_stack", false).select_layer(1);
			find_widget<timage>(&window, "campaign_image", false).set_label(image);

			std::stringstream players_str;
			players_str << current_campaign->min_players();

			if(current_campaign->max_players() != current_campaign->min_players()) {
				players_str << " to " << current_campaign->max_players();
			}

			players.set_label(players_str.str());

			break;
		}
	}

	on_tab_select(window);
}

void tmp_create_game::update_map_settings(twindow& window)
{
	if(config_engine_->force_lock_settings()) {
		use_map_settings_->widget_set_enabled(window, false, false);
		use_map_settings_->set_widget_value(window, true);
	} else {
		use_map_settings_->widget_set_enabled(window, true, false);
	}

	const bool use_map_settings = use_map_settings_->get_widget_value(window);

	config_engine_->set_use_map_settings(use_map_settings);

	fog_            ->widget_set_enabled(window, !use_map_settings, false);
	shroud_         ->widget_set_enabled(window, !use_map_settings, false);
	start_time_     ->widget_set_enabled(window, !use_map_settings, false);

	turns_          ->widget_set_enabled(window, !use_map_settings, false);
	gold_           ->widget_set_enabled(window, !use_map_settings, false);
	support_        ->widget_set_enabled(window, !use_map_settings, false);
	experience_     ->widget_set_enabled(window, !use_map_settings, false);

	const bool time_limit = time_limit_->get_widget_value(window);

	init_turn_limit ->widget_set_enabled(window, time_limit, false);
	turn_bonus_     ->widget_set_enabled(window, time_limit, false);
	reservior_      ->widget_set_enabled(window, time_limit, false);
	action_bonus_   ->widget_set_enabled(window, time_limit, false);

	if(use_map_settings) {
		fog_       ->set_widget_value(window, config_engine_->fog_game_default());
		shroud_    ->set_widget_value(window, config_engine_->shroud_game_default());
		start_time_->set_widget_value(window, config_engine_->random_start_time_default());

		turns_     ->set_widget_value(window, config_engine_->num_turns_default());
		gold_      ->set_widget_value(window, config_engine_->village_gold_default());
		support_   ->set_widget_value(window, config_engine_->village_support_default());
		experience_->set_widget_value(window, config_engine_->xp_modifier_default());
	}
}

void tmp_create_game::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		create_engine_.prepare_for_era_and_mods();

		if(create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ||
		   create_engine_.current_level_type() == ng::level::TYPE::SP_CAMPAIGN) {

			std::string difficulty = create_engine_.select_campaign_difficulty();
			if(difficulty == "CANCEL") {
				window.set_retval(twindow::NONE);
			}

			create_engine_.prepare_for_campaign(difficulty);
		} else if(create_engine_.current_level_type() == ng::level::TYPE::SCENARIO) {
			create_engine_.prepare_for_scenario();
		} else {
			//This means define= doesn't work for random generated scenarios
			create_engine_.prepare_for_other();
		}

		create_engine_.prepare_for_new_level();

		create_engine_.get_parameters();

		config_engine_->set_use_map_settings(use_map_settings_->get_widget_value(window));

		if(!config_engine_->force_lock_settings()) {
			config_engine_->set_num_turns(turns_->get_widget_value(window));
			config_engine_->set_village_gold(gold_->get_widget_value(window));
			config_engine_->set_village_support(support_->get_widget_value(window));
			config_engine_->set_xp_modifier(experience_->get_widget_value(window));
			config_engine_->set_random_start_time(start_time_->get_widget_value(window));
			config_engine_->set_fog_game(fog_->get_widget_value(window));
			config_engine_->set_shroud_game(shroud_->get_widget_value(window));
			config_engine_->write_parameters();
		}

		config_engine_->set_mp_countdown(time_limit_->get_widget_value(window));
		config_engine_->set_mp_countdown_init_time(init_turn_limit->get_widget_value(window));
		config_engine_->set_mp_countdown_turn_bonus(turn_bonus_->get_widget_value(window));
		config_engine_->set_mp_countdown_reservoir_time(reservior_->get_widget_value(window));
		config_engine_->set_mp_countdown_action_bonus(action_bonus_->get_widget_value(window));

		config options;
		for(const auto& mod_pair : visible_options_) {
			config& mod = options.add_child(mod_pair.first[0]);
			mod["id"] = mod_pair.first[1];
			for(const auto& pair : mod_pair.second) {
				//TODO: change this to some key=value format as soon as we drop the old  mp configure screen.
				mod.add_child("option", config_of("id", pair.first)("value", pair.second()));
			}
		}

		config_engine_->set_options(options);

		// Set game name
		const std::string name = find_widget<ttext_box>(&window, "game_name", false).get_value();
		if(!name.empty() && (name != config_engine_->game_name_default())) {
			config_engine_->set_game_name(name);
		}

		// Set game password
		const std::string password = find_widget<ttext_box>(&window, "game_password", false).get_value();
		if(!password.empty()) {
			config_engine_->set_game_password(password);
		}
	}
}

} // namespace gui2
