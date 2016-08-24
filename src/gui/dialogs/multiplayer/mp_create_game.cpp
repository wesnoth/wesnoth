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
#include "gui/widgets/menu_button.hpp"
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
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "game_config.hpp"
#include "savegame.hpp"
#include "settings.hpp"
#include "formatter.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

namespace gui2
{

// Shorthand
namespace prefs = preferences;

REGISTER_DIALOG(mp_create_game)

tmp_create_game::tmp_create_game(const config& cfg, ng::create_engine& create_eng)
	: cfg_(cfg)
	, create_engine_(create_eng)
	, config_engine_()
	, selected_game_index_(-1)
	, selected_rfm_index_(-1)
	, use_map_settings_(register_bool( "use_map_settings", true, prefs::use_map_settings, prefs::set_use_map_settings,
		dialog_callback<tmp_create_game, &tmp_create_game::update_map_settings>))
	, fog_(register_bool("fog", true, prefs::fog, prefs::set_fog))
	, shroud_(register_bool("shroud", true, prefs::shroud, prefs::set_shroud))
	, start_time_(register_bool("random_start_time", true, prefs::random_start_time, prefs::set_random_start_time))
	, time_limit_(register_bool("time_limit", true, prefs::countdown, prefs::set_countdown,
		dialog_callback<tmp_create_game, &tmp_create_game::update_map_settings>))
	, shuffle_sides_(register_bool("shuffle_sides", true, prefs::shuffle_sides, prefs::set_shuffle_sides))
	, observers_(register_bool("observers", true, prefs::allow_observers, prefs::set_allow_observers))
	, registered_users_(register_bool("registered_users", true, prefs::registered_users_only, prefs::set_registered_users_only))
	, strict_sync_(register_bool("strict_sync", true))
	, turns_(register_integer("turn_count", true, prefs::turns, prefs::set_turns))
	, gold_(register_integer("village_gold", true, prefs::village_gold, prefs::set_village_gold))
	, support_(register_integer("village_support", true, prefs::village_support, prefs::set_village_support))
	, experience_(register_integer("experience_modifier", true, prefs::xp_modifier, prefs::set_xp_modifier))
	, init_turn_limit(register_integer("init_turn_limit", true, prefs::countdown_init_time, prefs::set_countdown_init_time))
	, turn_bonus_(register_integer("turn_bonus", true, prefs::countdown_turn_bonus, prefs::set_countdown_turn_bonus))
	, reservior_(register_integer("reservior", true, prefs::countdown_reservoir_time, prefs::set_countdown_reservoir_time))
	, action_bonus_(register_integer("action_bonus", true, prefs::countdown_action_bonus, prefs::set_countdown_action_bonus))
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

	rfm_types_ = {
		mp_game_settings::RANDOM_FACTION_MODE::DEFAULT,
		mp_game_settings::RANDOM_FACTION_MODE::NO_MIRROR,
		mp_game_settings::RANDOM_FACTION_MODE::NO_ALLY_MIRROR,
	};

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

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "load_game", false),
		std::bind(&tmp_create_game::load_game_callback, this, std::ref(window)));

	// Custom dialog close hook
	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "create_game", false),
		std::bind(&tmp_create_game::dialog_exit_hook, this, std::ref(window)));

	//
	// Set up filtering
	//
	connect_signal_notify_modified(find_widget<tslider>(&window, "num_players", false),
		std::bind(&tmp_create_game::on_filter_change<tslider>, this, std::ref(window), "num_players"));

	ttext_box& filter = find_widget<ttext_box>(&window, "game_filter", false);

	filter.set_text_changed_callback(
		std::bind(&tmp_create_game::on_filter_change<ttext_box>, this, std::ref(window), "game_filter"));

	// Note this cannot be in the keyboard chain or it will capture focus from other text boxes
	window.keyboard_capture(&filter);

	//
	// Set up game types menu_button
	//
	std::vector<config> game_types;
	for(level_type_info& type_info : level_types_) {
		game_types.push_back(config_of("label", type_info.second));
	}

	if(game_types.empty()) {
		gui2::show_transient_message(window.video(), "", _("No games found."));
		throw game::error(_("No games found."));
	}

	tmenu_button& game_menu_button = find_widget<tmenu_button>(&window, "game_types", false);

	game_menu_button.set_values(game_types);
	if(game_config::debug || preferences::level_type() != ng::level::TYPE::SP_CAMPAIGN) {
		game_menu_button.set_selected(preferences::level_type());
	}

	game_menu_button.connect_click_handler(std::bind(&tmp_create_game::update_games_list, this, std::ref(window)));

	//
	// Set up eras menu_button
	//
	tmenu_button& eras_menu_button = find_widget<tmenu_button>(&window, "eras", false);

	std::vector<config> era_names;
	for(const auto& era : create_engine_.extras_menu_item_names(ng::create_engine::ERA, false)) {
		era_names.push_back(config_of("label", era));
	}

	if(era_names.empty()) {
		gui2::show_transient_message(window.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}

	eras_menu_button.set_values(era_names);
	eras_menu_button.connect_click_handler(std::bind(&tmp_create_game::on_era_select, this, std::ref(window)));

	const int era_selection = create_engine_.find_extra_by_id(ng::create_engine::ERA, preferences::era());
	if(era_selection >= 0) {
		eras_menu_button.set_selected(era_selection);
	}

	on_era_select(window);

	//
	// Set up mods list
	//
	tlistbox& mod_list = find_widget<tlistbox>(&window, "mod_list", false);

	const auto& activemods = preferences::modifications();
	for(const auto& mod : create_engine_.get_extras_by_type(ng::create_engine::MOD)) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = mod->name;
		data.emplace("mod_name", item);

		tgrid* row_grid = &mod_list.add_row(data);

		ttoggle_button& mog_toggle = find_widget<ttoggle_button>(row_grid, "mod_active_state", false);

		const int i = mod_list.get_item_count() - 1;
		if(std::find(activemods.begin(), activemods.end(), mod->id) != activemods.end()) {
			create_engine_.active_mods().push_back(mod->id);
			mog_toggle.set_value_bool(true);
		}

		mog_toggle.set_callback_state_change(std::bind(&tmp_create_game::on_mod_toggle, this, i));
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
	// Set up random faction mode menu_button
	//
	std::vector<config> rfm_options;
	for(const auto& type : rfm_types_) {
		rfm_options.push_back(config_of("label", mp_game_settings::RANDOM_FACTION_MODE::enum_to_string(type)));
	};

	// Manually insert tooltips. Need to find a better way to do this
	rfm_options[0]["tooltip"] = _("Independent: Random factions assigned independently");
	rfm_options[1]["tooltip"] = _("No Mirror: No two players will get the same faction");
	rfm_options[2]["tooltip"] = _("No Ally Mirror: No two allied players will get the same faction");

	const int initial_index = std::find(rfm_types_.begin(), rfm_types_.end(),
		mp_game_settings::RANDOM_FACTION_MODE::string_to_enum(prefs::random_faction_mode())) - rfm_types_.begin();

	tmenu_button& rfm_menu_button = find_widget<tmenu_button>(&window, "random_faction_mode", false);

	rfm_menu_button.set_values(rfm_options);
	rfm_menu_button.set_selected(initial_index);
	rfm_menu_button.connect_click_handler(std::bind(&tmp_create_game::on_random_faction_mode_select, this, std::ref(window)));

	on_random_faction_mode_select(window);

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

	//
	// Main games list
	//
	tlistbox& list = find_widget<tlistbox>(&window, "games_list", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
		std::bind(&tmp_create_game::on_game_select,
			*this, std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::on_game_select>);
#endif

	window.add_to_keyboard_chain(&list);

	// This handles both the initial game and tab selection
	display_games_of_type(window, ng::level::TYPE::from_int(preferences::level_type()), preferences::level());

	// Set up the Lua plugin context
	plugins_context_.reset(new plugins_context("Multiplayer Create"));

	plugins_context_->set_callback("create", [&window](const config&) { window.set_retval(twindow::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(twindow::CANCEL); }, false);
	plugins_context_->set_callback("load",   [this, &window](const config&) { load_game_callback(window); }, false);

	plugins_context_->set_callback("select_level", [&window](const config& cfg) {
		find_widget<tlistbox>(&window, "games_list", false).select_row(cfg["index"].to_int()); }, true);
	plugins_context_->set_callback("select_type",  [this, &window](const config& cfg) {
		display_games_of_type(window, ng::level::TYPE::string_to_enum(cfg["type"]), cfg["level"]); }, true);

	plugins_context_->set_accessor("game_config",  [this](const config&) {return cfg_; });
	plugins_context_->set_accessor("get_selected", [this](const config&) {
		const ng::level& current_level = create_engine_.current_level();
		return config_of
			("id", current_level.id())
			("name", current_level.name())
			("icon", current_level.icon())
			("description", current_level.description())
			("allow_era_choice", current_level.allow_era_choice())
			("type", create_engine_.current_level_type());
	});

	plugins_context_->set_accessor("find_level", [this](const config& cfg) {
		const std::string id = cfg["id"].str();
		return config_of
			("index", create_engine_.find_level_by_id(id))
			("type", create_engine_.find_level_type_by_id(id));
	});
}

template<typename widget>
void tmp_create_game::on_filter_change(twindow& window, const std::string& id)
{
	create_engine_.apply_level_filter(find_widget<widget>(&window, id, false).get_value());

	tlistbox& game_list = find_widget<tlistbox>(&window, "games_list", false);

	std::vector<bool> filtered(game_list.get_item_count());
	for(const size_t i : create_engine_.get_filtered_level_indices(create_engine_.current_level_type())) {
		filtered[i] = true;
	}

	game_list.set_row_shown(filtered);

	update_details(window);
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

	if(i == tab::TAB_GENERAL) {
		const bool can_select_era = create_engine_.current_level().allow_era_choice();

		tmenu_button& era_combo = find_widget<tmenu_button>(&window, "eras", false);
		if(!can_select_era) {
			era_combo.set_label(_("No eras available for this game."));
		} else {
			era_combo.set_selected(era_combo.get_value());
		}

		era_combo.set_active(can_select_era);
	}

	if(i == tab::TAB_OPTIONS) {
		update_options_list(window);
	}

	// Map Settings can and should be updated every time
	update_map_settings(window);
}

void tmp_create_game::on_mod_select(twindow& window)
{
	if (find_widget<tlistbox>(&window, "mod_list", false).get_item_count() <= 0)	// no modifications installed
		return;

	create_engine_.set_current_mod_index(find_widget<tlistbox>(&window, "mod_list", false).get_selected_row());

	show_description(window, create_engine_.current_extra(ng::create_engine::MOD).description);
}

void tmp_create_game::on_mod_toggle(const int index)
{
	create_engine_.set_current_mod_index(index);
	create_engine_.toggle_current_mod();
}

void tmp_create_game::on_era_select(twindow& window)
{
	create_engine_.set_current_era_index(find_widget<tmenu_button>(&window, "eras", false).get_value());

	show_description(window, create_engine_.current_extra(ng::create_engine::ERA).description);
}

void tmp_create_game::on_random_faction_mode_select(twindow& window)
{
	selected_rfm_index_ = find_widget<tmenu_button>(&window, "random_faction_mode", false).get_value();
}

void tmp_create_game::show_description(twindow& window, const std::string& new_description)
{
	tcontrol& description = find_widget<tcontrol>(&window, "description", false);

	description.set_label(!new_description.empty() ? new_description : _("No description available"));
	description.set_use_markup(true);
}

void tmp_create_game::update_games_list(twindow& window)
{
	const int index = find_widget<tmenu_button>(&window, "game_types", false).get_value();

	display_games_of_type(window, level_types_[index].first, create_engine_.current_level().id());
}

void tmp_create_game::display_games_of_type(twindow& window, ng::level::TYPE type, const std::string& level)
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

		tgrid* row_grid = &list.add_row(data);

		find_widget<ttoggle_panel>(row_grid, "game_list_panel", false).set_callback_mouse_left_double_click(
			std::bind(&tmp_create_game::dialog_exit_hook, this, std::ref(window)));
	}

	if(!level.empty()) {
		int level_index = create_engine_.find_level_by_id(level);
		if(level_index >= 0) {
			list.select_row(level_index);
		}
	}

	const bool is_random_map = type == ng::level::TYPE::RANDOM_MAP;

	find_widget<tbutton>(&window, "random_map_regenerate", false).set_active(is_random_map);
	find_widget<tbutton>(&window, "random_map_settings", false).set_active(is_random_map);

	update_details(window);
}

template<typename T>
void tmp_create_game::update_options_data_map(T* widget, const option_source& source)
{
	options_data_[source][widget->id()] = widget->get_value();
}

template<>
void tmp_create_game::update_options_data_map(ttoggle_button* widget, const option_source& source)
{
	options_data_[source][widget->id()] = widget->get_value_bool();
}

void tmp_create_game::reset_options_data(twindow& window, const option_source& source, bool& handled, bool& halt)
{
	options_data_[source].clear();
	update_options_list(window);

	handled = true;
	halt = true;
}

void tmp_create_game::display_custom_options(twindow& window, ttree_view& tree, std::string&& type, const config& cfg)
{
	// Needed since some compilers don't like passing just {}
	static const std::map<std::string, string_map> empty_map;

	visible_options_.push_back({type, cfg["id"]});
	auto& data_map = options_data_[visible_options_.back()];

	for(const auto& options : cfg.child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = cfg["name"];
		data.emplace("tree_view_node_label", item);

		ttree_view_node& option_node = tree.add_node("option_node", data);

		for(const auto& checkbox_option : options.child_range("checkbox")) {
			data.clear();

			item["label"] = checkbox_option["name"];
			item["tooltip"] = checkbox_option["description"];
			data.emplace("option_checkbox", item);

			ttree_view_node& node = option_node.add_child("option_checkbox_node", data);

			ttoggle_button* checkbox = dynamic_cast<ttoggle_button*>(node.find("option_checkbox", true));

			VALIDATE(checkbox, missing_widget("option_checkbox"));

			const std::string widget_id = checkbox_option["id"];

			if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
				data_map[widget_id] = checkbox_option["default"];
			}

			checkbox->set_id(widget_id);
			checkbox->set_value(data_map[widget_id].to_bool());
			checkbox->set_callback_state_change(
				std::bind(&tmp_create_game::update_options_data_map<ttoggle_button>, this, checkbox, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("checkbox")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& menu_button_option : options.child_range("combo")) {
			data.clear();

			item["label"] = menu_button_option["name"];
			data.emplace("menu_button_label", item);
			item.clear();
			item["tooltip"] = menu_button_option["description"];
			data.emplace("option_menu_button", item);

			std::vector<config> combo_items;
			std::vector<std::string> combo_values;

			config::const_child_itors items = menu_button_option.child_range("item");
			for(auto item : items) {
				// Comboboxes expect this key to be 'label' not 'name'
				item["label"] = item["name"];

				combo_items.push_back(item);
				combo_values.push_back(item["value"]);
			}

			if(combo_items.empty()) {
				continue;
			}

			ttree_view_node& node = option_node.add_child("option_menu_button_node", data);

			tmenu_button* menu_button = dynamic_cast<tmenu_button*>(node.find("option_menu_button", true));

			VALIDATE(menu_button, missing_widget("option_menu_button"));

			const std::string widget_id = menu_button_option["id"];

			if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
				data_map[widget_id] = menu_button_option["default"];
			}

			menu_button->set_id(widget_id);
			menu_button->set_values(combo_items);

			config::attribute_value val = data_map[widget_id];
			auto iter = std::find_if(items.begin(), items.end(), [&val](const config& cfg) {
				return cfg["value"] == val;
			});
			if(iter != items.end()) {
				menu_button->set_selected(iter - items.begin());
			}

			menu_button->connect_click_handler(
				std::bind(&tmp_create_game::update_options_data_map<tmenu_button>, this, menu_button, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("combo")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& slider_option : options.child_range("slider")) {
			data.clear();

			item["label"] = slider_option["name"];
			data.emplace("slider_label", item);
			item.clear();
			item["tooltip"] = slider_option["description"];
			data.emplace("option_slider", item);

			ttree_view_node& node = option_node.add_child("option_slider_node", data);

			tslider* slider = dynamic_cast<tslider*>(node.find("option_slider", true));

			VALIDATE(slider, missing_widget("option_slider"));

			const std::string widget_id = slider_option["id"];

			if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
				data_map[widget_id] = slider_option["default"];
			}

			slider->set_id(widget_id);
			slider->set_maximum_value(slider_option["max"].to_int());
			slider->set_minimum_value(slider_option["min"].to_int());
			slider->set_step_size(slider_option["step"].to_int(1));
			slider->set_value(data_map[widget_id].to_int());

			connect_signal_notify_modified(*slider,
				std::bind(&tmp_create_game::update_options_data_map<tslider>, this, slider, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("slider")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& text_entry_option : options.child_range("entry")) {
			data.clear();

			item["label"] = text_entry_option["name"];
			data.emplace("text_entry_label", item);
			item.clear();
			item["tooltip"] = text_entry_option["description"];
			data.emplace("option_text_entry", item);

			ttree_view_node& node = option_node.add_child("option_text_entry_node", data);

			ttext_box* textbox = dynamic_cast<ttext_box*>(node.find("option_text_entry", true));

			VALIDATE(textbox, missing_widget("option_text_entry"));

			const std::string widget_id = text_entry_option["id"];

			if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
				data_map[widget_id] = text_entry_option["default"];
			}

			textbox->set_id(widget_id);
			textbox->set_value(data_map[widget_id].str());
			textbox->set_text_changed_callback(
				std::bind(&tmp_create_game::update_options_data_map<ttext_box>, this, textbox, visible_options_.back()));
		}

		// Add the Defaults button at the end
		ttree_view_node& node = option_node.add_child("options_default_button", empty_map);

		connect_signal_mouse_left_click(find_widget<tbutton>(&node, "reset_option_values", false),
			std::bind(&tmp_create_game::reset_options_data, this, std::ref(window), visible_options_.back(),
				std::placeholders::_3, std::placeholders::_4));
	}
}

void tmp_create_game::update_options_list(twindow& window)
{
	ttree_view& options_tree = find_widget<ttree_view>(&window, "custom_options", false);

	// TODO: might be inefficient to regenerate this every single time this tab is selected
	// Maybe look into caching the result if no change has been made to the selection.
	visible_options_.clear();
	options_tree.clear();

	display_custom_options(window, options_tree,
		create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ? "campaign" : "multiplayer",
		create_engine_.current_level().data());

	display_custom_options(window, options_tree, "era", create_engine_.curent_era_cfg());

	std::set<std::string> activemods(create_engine_.active_mods().begin(), create_engine_.active_mods().end());
	for(const auto& mod : create_engine_.get_const_extras_by_type(ng::create_engine::MP_EXTRA::MOD)) {
		if(activemods.find(mod->id) != activemods.end()) {
			display_custom_options(window, options_tree, "modification", *mod->cfg);
		}
	}

	// No custom options, display a message
	find_widget<tcontrol>(&window, "no_options_notice", false).set_visible(options_tree.empty() ? twindow::tvisible::visible : twindow::tvisible::invisible);
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

void tmp_create_game::load_game_callback(twindow& window)
{
	try {
		savegame::loadgame load(window.video(), cfg_, create_engine_.get_state());

		if(!load.load_multiplayer_game()) {
			return ;
		}

		if(load.cancel_orders()) {
			create_engine_.get_state().cancel_orders();
		}

		create_engine_.prepare_for_saved_game();

		dialog_exit_hook(window);
	} catch(config::error&) {}
}

int tmp_create_game::convert_to_game_filtered_index(const int initial_index)
{
	const std::vector<size_t>& filtered_indices = create_engine_.get_filtered_level_indices(create_engine_.current_level_type());
	return std::find(filtered_indices.begin(), filtered_indices.end(), initial_index) - filtered_indices.begin();
}

void tmp_create_game::update_details(twindow& window)
{
	tcontrol& players = find_widget<tcontrol>(&window, "map_num_players", false);
	tcontrol& map_size = find_widget<tcontrol>(&window, "map_size", false);

	// Convert the absolute-index get_selected_row to a relatve index for the create_engine to handle
	selected_game_index_ = convert_to_game_filtered_index(find_widget<tlistbox>(&window, "games_list", false).get_selected_row());

	create_engine_.set_current_level(selected_game_index_);

	// If the current random map doesn't have data, generate it
	if(create_engine_.current_level_type() == ng::level::TYPE::RANDOM_MAP) {
		if(create_engine_.generator_assigned() && create_engine_.current_level().data()["map_data"].empty()) {
			create_engine_.init_generated_level_data();
		}
	}

	create_engine_.current_level().set_metadata();

	// Reset the config_engine with new values
	config_engine_.reset(new ng::configure_engine(create_engine_.get_state()));
	config_engine_->update_initial_cfg(create_engine_.current_level().data());
	config_engine_->set_default_values();

	show_description(window, create_engine_.current_level().description());

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

			const std::string image = formatter() << current_campaign->data()["image"] << "~SCALE_INTO(265,265)";

			find_widget<tstacked_widget>(&window, "minimap_stack", false).select_layer(1);
			find_widget<timage>(&window, "campaign_image", false).set_label(image);

			std::stringstream players_str;
			players_str << current_campaign->min_players();

			if(current_campaign->max_players() != current_campaign->min_players()) {
				players_str << " to " << current_campaign->max_players();
			}

			players.set_label(players_str.str());
			map_size.set_label(utils::unicode_em_dash);

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

void tmp_create_game::dialog_exit_hook(twindow& window) {
	if(create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ||
		create_engine_.current_level_type() == ng::level::TYPE::SP_CAMPAIGN) {

		if(create_engine_.select_campaign_difficulty() == "CANCEL") {
			return;
		}
	}

	window.set_retval(twindow::OK);
}

void tmp_create_game::post_show(twindow& window)
{
	// Show all tabs so that find_widget works correctly
	find_widget<tstacked_widget>(&window, "pager", false).select_layer(-1);
	if(get_retval() == twindow::OK) {
		preferences::set_modifications(create_engine_.active_mods());
		preferences::set_level_type(create_engine_.current_level_type().v);
		preferences::set_level(create_engine_.current_level().id());
		preferences::set_era(create_engine_.current_extra(ng::create_engine::ERA).id);

		create_engine_.prepare_for_era_and_mods();

		if(create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ||
			create_engine_.current_level_type() == ng::level::TYPE::SP_CAMPAIGN) {
			create_engine_.prepare_for_campaign();
		} else if(create_engine_.current_level_type() == ng::level::TYPE::SCENARIO) {
			create_engine_.prepare_for_scenario();
		} else {
			// This means define= doesn't work for randomly generated scenarios
			create_engine_.prepare_for_other();
		}

		create_engine_.prepare_for_new_level();

		create_engine_.get_parameters();

		config_engine_->set_use_map_settings(use_map_settings_->get_widget_value(window));

		if(!config_engine_->force_lock_settings()) {
			// Max slider value (in this case, 100) means 'unlimited turns', so pass the value -1
			const int num_turns = turns_->get_widget_value(window);
			config_engine_->set_num_turns(num_turns < ::settings::turns_max ? num_turns : - 1);
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

		config_engine_->set_allow_observers(observers_->get_widget_value(window));
		config_engine_->set_registered_users_only(registered_users_->get_widget_value(window));
		config_engine_->set_oos_debug(strict_sync_->get_widget_value(window));
		config_engine_->set_shuffle_sides(shuffle_sides_->get_widget_value(window));

		config_engine_->set_random_faction_mode(rfm_types_[selected_rfm_index_]);

		// Since we don't have a tfield handling this option, we need to save the value manually
		prefs::set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE::enum_to_string(rfm_types_[selected_rfm_index_]));

		config options;
		for(const auto& source : visible_options_) {
			config& mod = options.add_child(source.level_type);
			mod["id"] = source.id;
			for(const auto& option : options_data_[source]) {
				// TODO: change this to some key=value format as soon as we drop the old mp configure screen.
				mod.add_child("option", config_of("id", option.first)("value", option.second));
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
