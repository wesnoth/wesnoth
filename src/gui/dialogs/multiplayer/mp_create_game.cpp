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
#include "game_config_manager.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
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
#include "gui/widgets/status_label_helper.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/text_box.hpp"
#include "game_config.hpp"
#include "savegame.hpp"
#include "settings.hpp"
#include "formatter.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

#include <boost/algorithm/string.hpp>

namespace gui2
{

// Special retval value for loading a game
static const int LOAD_GAME = 100;

// Shorthand
namespace prefs = preferences;

REGISTER_DIALOG(mp_create_game)

tmp_create_game::tmp_create_game(const config& cfg, ng::create_engine& create_eng)
	: cfg_(cfg)
	, create_engine_(create_eng)
	, config_engine_()
	, options_manager_()
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
	, init_turn_limit_(register_integer("init_turn_limit", true, prefs::countdown_init_time, prefs::set_countdown_init_time))
	, turn_bonus_(register_integer("turn_bonus", true, prefs::countdown_turn_bonus, prefs::set_countdown_turn_bonus))
	, reservoir_(register_integer("reservoir", true, prefs::countdown_reservoir_time, prefs::set_countdown_reservoir_time))
	, action_bonus_(register_integer("action_bonus", true, prefs::countdown_action_bonus, prefs::set_countdown_action_bonus))
{
	level_types_ = {
		{ng::level::TYPE::SCENARIO, _("Scenarios")},
		{ng::level::TYPE::CAMPAIGN, _("Campaigns")},
		{ng::level::TYPE::USER_MAP, _("User Maps")},
		{ng::level::TYPE::USER_SCENARIO, _("User Scenarios")},
		{ng::level::TYPE::RANDOM_MAP, _("Random Maps")},
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

	set_show_even_without_video(true);

	create_engine_.init_active_mods();

	create_engine_.get_state() = saved_game();
	create_engine_.get_state().classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

	// Need to set this in the constructor, pre_show() is too late
	set_allow_plugin_skip(false);
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
	window.set_exit_hook_ok_only([this](twindow& w)->bool { return dialog_exit_hook(w); });

	//
	// Set up the options manager. Needs to be done before selecting an initial tab
	//
	options_manager_.reset(new tmp_options_helper(window, create_engine_));

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

	// Helper to make sure the initially selected level type is valid
	auto get_initial_type_index = [this]()->int {
		const auto index = std::find_if(level_types_.begin(), level_types_.end(), [](level_type_info& info) {
			return info.first == ng::level::TYPE::from_int(preferences::level_type());
		});

		if(index != level_types_.end()) {
			return index - level_types_.begin();
		}

		return 0;
	};

	game_menu_button.set_values(game_types, get_initial_type_index());
	game_menu_button.connect_click_handler(std::bind(&tmp_create_game::update_games_list, this, std::ref(window)));

	//
	// Set up eras menu_button
	//
	tmenu_button& eras_menu_button = find_widget<tmenu_button>(&window, "eras", false);

	std::vector<config> era_names;
	for(const auto& era : create_engine_.get_const_extras_by_type(ng::create_engine::ERA)) {
		era_names.push_back(config_of("label", era->name)("tooltip", era->description));
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

		find_widget<ttoggle_panel>(row_grid, "panel", false).set_tooltip(mod->description);

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

	// No mods, hide the header
	if(mod_list.get_item_count() <= 0) {
		find_widget<tcontrol>(&window, "mods_header", false).set_visible(twindow::tvisible::invisible);
	} else {
		on_mod_select(window);
	}

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
		mp_game_settings::RANDOM_FACTION_MODE::string_to_enum(prefs::random_faction_mode(), mp_game_settings::RANDOM_FACTION_MODE::DEFAULT))
		- rfm_types_.begin();

	tmenu_button& rfm_menu_button = find_widget<tmenu_button>(&window, "random_faction_mode", false);

	rfm_menu_button.set_values(rfm_options, initial_index);
	rfm_menu_button.connect_click_handler(std::bind(&tmp_create_game::on_random_faction_mode_select, this, std::ref(window)));

	on_random_faction_mode_select(window);

	//
	// Set up the setting status labels
	//
	bind_status_label<tslider>(window, turns_->id());
	bind_status_label<tslider>(window, gold_->id());
	bind_status_label<tslider>(window, support_->id());
	bind_status_label<tslider>(window, experience_->id());

	bind_status_label<tslider>(window, init_turn_limit_->id());
	bind_status_label<tslider>(window, turn_bonus_->id());
	bind_status_label<tslider>(window, reservoir_->id());
	bind_status_label<tslider>(window, action_bonus_->id());

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
	display_games_of_type(window, level_types_[get_initial_type_index()].first, preferences::level());

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Create"));

	plugins_context_->set_callback("create", [&window](const config&) { window.set_retval(twindow::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(twindow::CANCEL); }, false);
	plugins_context_->set_callback("load",   [this, &window](const config&) { load_game_callback(window); }, false);

#define UPDATE_ATTRIBUTE(field, convert) \
	do { if(cfg.has_attribute(#field)) { field##_->set_widget_value(window, cfg[#field].convert()); } } while(false) \

	plugins_context_->set_callback("update_settings", [this, &window](const config& cfg) {
		UPDATE_ATTRIBUTE(turns, to_int);
		UPDATE_ATTRIBUTE(gold, to_int);
		UPDATE_ATTRIBUTE(support, to_int);
		UPDATE_ATTRIBUTE(experience, to_int);
		UPDATE_ATTRIBUTE(start_time, to_bool);
		UPDATE_ATTRIBUTE(fog, to_bool);
		UPDATE_ATTRIBUTE(shroud, to_bool);
		UPDATE_ATTRIBUTE(time_limit, to_bool);
		UPDATE_ATTRIBUTE(init_turn_limit, to_int);
		UPDATE_ATTRIBUTE(turn_bonus, to_int);
		UPDATE_ATTRIBUTE(reservoir, to_int);
		UPDATE_ATTRIBUTE(action_bonus, to_int);
		UPDATE_ATTRIBUTE(observers, to_bool);
		UPDATE_ATTRIBUTE(registered_users, to_bool);
		UPDATE_ATTRIBUTE(strict_sync, to_bool);
		UPDATE_ATTRIBUTE(shuffle_sides, to_bool);
	}, true);

#undef UPDATE_ATTRIBUTE

	plugins_context_->set_callback("set_name",     [this](const config& cfg) {
		config_engine_->set_game_name(cfg["name"]); }, true);

	plugins_context_->set_callback("set_password", [this](const config& cfg) {
		config_engine_->set_game_password(cfg["password"]); }, true);

	plugins_context_->set_callback("select_level", [this](const config& cfg) {
		selected_game_index_ = convert_to_game_filtered_index(cfg["index"].to_int());
		create_engine_.set_current_level(selected_game_index_);
	}, true);

	plugins_context_->set_callback("select_type",  [this](const config& cfg) {
		create_engine_.set_current_level_type(ng::level::TYPE::string_to_enum(cfg["type"], ng::level::TYPE::SCENARIO)); }, true);

	plugins_context_->set_callback("select_era",   [this](const config& cfg) {
		create_engine_.set_current_era_index(cfg["index"].to_int()); }, true);

	plugins_context_->set_callback("select_mod",   [this](const config& cfg) {
		on_mod_toggle(cfg["index"].to_int()); }, true);

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

	plugins_context_->set_accessor("find_level",   [this](const config& cfg) {
		const std::string id = cfg["id"].str();
		return config_of
			("index", create_engine_.find_level_by_id(id))
			("type", create_engine_.find_level_type_by_id(id));
	});

	plugins_context_->set_accessor_int("find_era", [this](const config& cfg) {
		return create_engine_.find_extra_by_id(ng::create_engine::ERA, cfg["id"]);
	});

	plugins_context_->set_accessor_int("find_mod", [this](const config& cfg) {
		return create_engine_.find_extra_by_id(ng::create_engine::MOD, cfg["id"]);
	});
}

template<typename widget>
void tmp_create_game::on_filter_change(twindow& window, const std::string& id)
{
	create_engine_.apply_level_filter(find_widget<widget>(&window, id, false).get_value());

	tlistbox& game_list = find_widget<tlistbox>(&window, "games_list", false);

	boost::dynamic_bitset<> filtered(game_list.get_item_count());
	for(const size_t i : create_engine_.get_filtered_level_indices(create_engine_.current_level_type())) {
		filtered[i] = true;
	}

	game_list.set_row_shown(filtered);

	on_game_select(window);
}

void tmp_create_game::on_game_select(twindow& window)
{
	const int selected_game = find_widget<tlistbox>(&window, "games_list", false).get_selected_row();

	if(selected_game != selected_game_index_) {
		// Convert the absolute-index get_selected_row to a relatve index for the create_engine to handle
		selected_game_index_ = convert_to_game_filtered_index(selected_game);

		create_engine_.set_current_level(selected_game_index_);
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
		update_options_list();
	}

	// Map Settings can and should be updated every time
	update_map_settings(window);
}

void tmp_create_game::on_mod_select(twindow& window)
{
	create_engine_.set_current_mod_index(find_widget<tlistbox>(&window, "mod_list", false).get_selected_row());
}

void tmp_create_game::on_mod_toggle(const int index)
{
	create_engine_.set_current_mod_index(index);
	create_engine_.toggle_current_mod();
}

void tmp_create_game::on_era_select(twindow& window)
{
	create_engine_.set_current_era_index(find_widget<tmenu_button>(&window, "eras", false).get_value());

	find_widget<tmenu_button>(&window, "eras", false).set_tooltip(create_engine_.current_extra(ng::create_engine::ERA).description);
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

	for(const auto& game : create_engine_.get_levels_by_type_unfiltered(type)) {
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

	// Recalculate which rows should be visisble
	on_filter_change<tslider>(window, "num_players");
	on_filter_change<ttext_box>(window, "game_filter");

	if(!level.empty() && !list.get_rows_shown().empty()) {
		int level_index = create_engine_.find_level_by_id(level);
		if(level_index >= 0 && size_t(level_index) < list.get_item_count()) {
			list.select_row(level_index);
		}
	}

	const bool is_random_map = type == ng::level::TYPE::RANDOM_MAP;

	find_widget<tbutton>(&window, "random_map_regenerate", false).set_active(is_random_map);
	find_widget<tbutton>(&window, "random_map_settings", false).set_active(is_random_map);

	// Override the last selection so on_game_select selects the new level
	selected_game_index_ = -1;

	on_game_select(window);
}

void tmp_create_game::update_options_list()
{
	options_manager_->update_options_list();
}

void tmp_create_game::show_generator_settings(twindow& window)
{
	create_engine_.generator_user_config(window.video());

	regenerate_random_map(window);
}

void tmp_create_game::regenerate_random_map(twindow& window)
{
	create_engine_.init_generated_level_data();

	update_details(window);
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

	if(create_engine_.current_level_type() == ng::level::TYPE::RANDOM_MAP) {
		// If the current random map doesn't have data, generate it
		if(create_engine_.generator_assigned() && create_engine_.current_level().data()["map_data"].empty()) {
			create_engine_.init_generated_level_data();
		}

		find_widget<tbutton>(&window, "random_map_settings", false).set_active(create_engine_.generator_has_settings());
	}

	create_engine_.current_level().set_metadata();

	// Reset the config_engine with new values
	config_engine_.reset(new ng::configure_engine(create_engine_.get_state()));
	config_engine_->update_initial_cfg(create_engine_.current_level().data());
	config_engine_->set_default_values();

	// Set the title, with newlines replaced. Newlines are sometimes found in SP Campaign names
	std::string title = create_engine_.current_level().name();
	boost::replace_all(title, "\n", " " + font::unicode_em_dash + " ");
	find_widget<tcontrol>(&window, "game_title", false).set_label(title);

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
			map_size.set_label(font::unicode_em_dash);

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

	init_turn_limit_->widget_set_enabled(window, time_limit, false);
	turn_bonus_     ->widget_set_enabled(window, time_limit, false);
	reservoir_      ->widget_set_enabled(window, time_limit, false);
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

void tmp_create_game::load_game_callback(twindow& window)
{
	savegame::loadgame load(window.video(), cfg_, create_engine_.get_state());

	if(!load.load_multiplayer_game()) {
		return;
	}

	if(load.data().cancel_orders) {
		create_engine_.get_state().cancel_orders();
	}

	window.set_retval(LOAD_GAME);
}

bool tmp_create_game::dialog_exit_hook(twindow&) {
	if(create_engine_.current_level_type() != ng::level::TYPE::CAMPAIGN &&
	   create_engine_.current_level_type() != ng::level::TYPE::SP_CAMPAIGN
	) {
		return true;
	}

	return create_engine_.select_campaign_difficulty() != "CANCEL";
}

void tmp_create_game::post_show(twindow& window)
{
	plugins_context_.reset();

	// Show all tabs so that find_widget works correctly
	find_widget<tstacked_widget>(&window, "pager", false).select_layer(-1);

	if(get_retval() == LOAD_GAME) {
		create_engine_.prepare_for_saved_game();
		return;
	}

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

		create_engine_.get_parameters();

		create_engine_.prepare_for_new_level();

		std::vector<const config*> entry_points;
		std::vector<std::string> entry_point_titles;

		const auto& tagname = create_engine_.get_state().classification().get_tagname();

		if(tagname == "scenario") {
			for(const config& scenario : game_config_manager::get()->game_config().child_range(tagname)) {
				if(scenario["allow_new_game"].to_bool(true) || game_config::debug) {
					const std::string& title = !scenario["new_game_title"].empty()
						? scenario["new_game_title"]
						: scenario["name"];

					entry_points.push_back(&scenario);
					entry_point_titles.push_back(title);
				}
			}
		}

		if(entry_points.size() > 1) {
			gui2::tsimple_item_selector dlg(_("Choose Starting Scenario"), _("Select at which point to begin this campaign."), entry_point_titles);

			dlg.set_single_button(true);
			dlg.show(window.video());

			const config& scenario = *entry_points[dlg.selected_index()];

			create_engine_.get_state().mp_settings().hash = scenario.hash();
			create_engine_.get_state().set_scenario(scenario);
		}

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
		config_engine_->set_mp_countdown_init_time(init_turn_limit_->get_widget_value(window));
		config_engine_->set_mp_countdown_turn_bonus(turn_bonus_->get_widget_value(window));
		config_engine_->set_mp_countdown_reservoir_time(reservoir_->get_widget_value(window));
		config_engine_->set_mp_countdown_action_bonus(action_bonus_->get_widget_value(window));

		config_engine_->set_allow_observers(observers_->get_widget_value(window));
		config_engine_->set_registered_users_only(registered_users_->get_widget_value(window));
		config_engine_->set_oos_debug(strict_sync_->get_widget_value(window));
		config_engine_->set_shuffle_sides(shuffle_sides_->get_widget_value(window));

		config_engine_->set_random_faction_mode(rfm_types_[selected_rfm_index_]);

		// Since we don't have a tfield handling this option, we need to save the value manually
		prefs::set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE::enum_to_string(rfm_types_[selected_rfm_index_]));

		config_engine_->set_options(options_manager_->get_options_config());

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
