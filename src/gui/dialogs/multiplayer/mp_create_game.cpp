/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "game_config_manager.hpp"
#include "game_initialization/lobby_data.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/menu_button.hpp"
#include "preferences/game.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "formatter.hpp"
#include "game_config.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "log.hpp"
#include "savegame.hpp"
#include "settings.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

#include <boost/algorithm/string.hpp>


static lg::log_domain log_mp_create("mp/create");

#define DBG_MP LOG_STREAM(debug, log_mp_create)
#define WRN_MP LOG_STREAM(warn, log_mp_create)
#define ERR_MP LOG_STREAM(err, log_mp_create)

namespace gui2
{
namespace dialogs
{

// Special retval value for loading a game
static const int LOAD_GAME = 100;

// Shorthand
namespace prefs = preferences;

REGISTER_DIALOG(mp_create_game)

mp_create_game::mp_create_game(const config& cfg, saved_game& state, bool local_mode, mp::user_info* host_info)
	: cfg_(cfg)
	, create_engine_(state)
	, config_engine_()
	, options_manager_()
	, selected_game_index_(-1)
	, selected_rfm_index_(-1)
	, use_map_settings_(register_bool( "use_map_settings", true, prefs::use_map_settings, prefs::set_use_map_settings,
		dialog_callback<mp_create_game, &mp_create_game::update_map_settings>))
	, fog_(register_bool("fog", true, prefs::fog, prefs::set_fog))
	, shroud_(register_bool("shroud", true, prefs::shroud, prefs::set_shroud))
	, start_time_(register_bool("random_start_time", true, prefs::random_start_time, prefs::set_random_start_time))
	, time_limit_(register_bool("time_limit", true, prefs::countdown, prefs::set_countdown,
		dialog_callback<mp_create_game, &mp_create_game::update_map_settings>))
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
	, mod_list_()
	, eras_menu_button_()
	, local_mode_(local_mode)
	, host_info_(host_info)
{
	level_types_ = {
		{ng::level::TYPE::SCENARIO, _("Scenarios")},
		{ng::level::TYPE::CAMPAIGN, _("Multiplayer Campaigns")},
		{ng::level::TYPE::SP_CAMPAIGN, _("Singleplayer Campaigns")},
		{ng::level::TYPE::USER_MAP, _("Custom Maps")},
		{ng::level::TYPE::USER_SCENARIO, _("Custom Scenarios")},
		{ng::level::TYPE::RANDOM_MAP, _("Random Maps")},
	};

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

	create_engine_.get_state().clear();
	create_engine_.get_state().classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

	// Need to set this in the constructor, pre_show() is too late
	set_allow_plugin_skip(false);
}

void mp_create_game::pre_show(window& win)
{
	find_widget<minimap>(&win, "minimap", false).set_config(&cfg_);

	find_widget<text_box>(&win, "game_name", false).set_value(ng::configure_engine::game_name_default());

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "random_map_regenerate", false),
		std::bind(&mp_create_game::regenerate_random_map, this, std::ref(win)));

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "random_map_settings", false),
		std::bind(&mp_create_game::show_generator_settings, this, std::ref(win)));

	connect_signal_mouse_left_click(
		find_widget<button>(&win, "load_game", false),
		std::bind(&mp_create_game::load_game_callback, this, std::ref(win)));

	// Custom dialog close hook
	win.set_exit_hook_ok_only([this](window& w)->bool { return dialog_exit_hook(w); });

	//
	// Set up the options manager. Needs to be done before selecting an initial tab
	//
	options_manager_.reset(new mp_options_helper(win, create_engine_));

	//
	// Set up filtering
	//
	connect_signal_notify_modified(find_widget<slider>(&win, "num_players", false),
		std::bind(&mp_create_game::on_filter_change<slider>, this, std::ref(win), "num_players", true));

	text_box& filter = find_widget<text_box>(&win, "game_filter", false);

	filter.set_text_changed_callback(
		std::bind(&mp_create_game::on_filter_change<text_box>, this, std::ref(win), "game_filter", true));

	// Note this cannot be in the keyboard chain or it will capture focus from other text boxes
	win.keyboard_capture(&filter);

	//
	// Set up game types menu_button
	//
	std::vector<config> game_types;
	for(level_type_info& type_info : level_types_) {
		game_types.emplace_back(config {"label", type_info.second});
	}

	if(game_types.empty()) {
		gui2::show_transient_message("", _("No games found."));
		throw game::error(_("No games found."));
	}

	menu_button& game_menu_button = find_widget<menu_button>(&win, "game_types", false);

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
	game_menu_button.connect_click_handler(std::bind(&mp_create_game::update_games_list, this, std::ref(win)));

	//
	// Set up mods list
	//
	mod_list_ = &find_widget<listbox>(&win, "mod_list", false);

	const auto& activemods = preferences::modifications();
	for(const auto& mod : create_engine_.get_extras_by_type(ng::create_engine::MOD)) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = mod->name;
		data.emplace("mod_name", item);

		grid* row_grid = &mod_list_->add_row(data);

		find_widget<toggle_panel>(row_grid, "panel", false).set_tooltip(mod->description);

		toggle_button& mog_toggle = find_widget<toggle_button>(row_grid, "mod_active_state", false);

		const int i = mod_list_->get_item_count() - 1;
		if(std::find(activemods.begin(), activemods.end(), mod->id) != activemods.end()) {
			create_engine_.active_mods().push_back(mod->id);
			mog_toggle.set_value_bool(true);
		}

		connect_signal_notify_modified(mog_toggle, std::bind(&mp_create_game::on_mod_toggle, this, std::ref(win), i, &mog_toggle));
	}

	// No mods, hide the header
	if(mod_list_->get_item_count() <= 0) {
		find_widget<styled_widget>(&win, "mods_header", false).set_visible(widget::visibility::invisible);
	}

	//
	// Set up eras menu_button
	//
	eras_menu_button_ = &find_widget<menu_button>(&win, "eras", false);

	std::vector<config> era_names;
	for(const auto& era : create_engine_.get_const_extras_by_type(ng::create_engine::ERA)) {
		era_names.emplace_back(config {"label", era->name, "tooltip", era->description});
	}

	if(era_names.empty()) {
		gui2::show_transient_message("", _("No eras found."));
		throw config::error(_("No eras found"));
	}

	eras_menu_button_->set_values(era_names);
	eras_menu_button_->connect_click_handler(std::bind(&mp_create_game::on_era_select, this, std::ref(win)));

	const int era_selection = create_engine_.find_extra_by_id(ng::create_engine::ERA, preferences::era());
	if(era_selection >= 0) {
		eras_menu_button_->set_selected(era_selection);
	}

	on_era_select(win);

	//
	// Set up random faction mode menu_button
	//
	std::vector<config> rfm_options;
	for(const auto& type : rfm_types_) {
		rfm_options.emplace_back(config {"label", mp_game_settings::RANDOM_FACTION_MODE::enum_to_string(type)});
	};

	// Manually insert tooltips. Need to find a better way to do this
	rfm_options[0]["tooltip"] = _("Independent: Random factions assigned independently");
	rfm_options[1]["tooltip"] = _("No Mirror: No two players will get the same faction");
	rfm_options[2]["tooltip"] = _("No Ally Mirror: No two allied players will get the same faction");

	const int initial_index = std::find(rfm_types_.begin(), rfm_types_.end(),
		mp_game_settings::RANDOM_FACTION_MODE::string_to_enum(prefs::random_faction_mode(), mp_game_settings::RANDOM_FACTION_MODE::DEFAULT))
		- rfm_types_.begin();

	menu_button& rfm_menu_button = find_widget<menu_button>(&win, "random_faction_mode", false);

	rfm_menu_button.set_values(rfm_options, initial_index);
	rfm_menu_button.connect_click_handler(std::bind(&mp_create_game::on_random_faction_mode_select, this, std::ref(win)));

	on_random_faction_mode_select(win);

	//
	// Set up the setting status labels
	//
	bind_status_label<slider>(&win, turns_->id());
	bind_status_label<slider>(&win, gold_->id());
	bind_status_label<slider>(&win, support_->id());
	bind_status_label<slider>(&win, experience_->id());

	bind_status_label<slider>(&win, init_turn_limit_->id());
	bind_status_label<slider>(&win, turn_bonus_->id());
	bind_status_label<slider>(&win, reservoir_->id());
	bind_status_label<slider>(&win, action_bonus_->id());

	//
	// Disable certain settings if we're playing a local game.
	//

	// Don't allow a 'registered users only' game if the host themselves isn't registered.
	if(local_mode_ || (host_info_ && !host_info_->registered)) {
		registered_users_->widget_set_enabled(win, false, false);
	}

	if(local_mode_) {
		find_widget<text_box>(&win, "game_password", false).set_active(false);

		observers_->widget_set_enabled(win, false, false);
		strict_sync_->widget_set_enabled(win, false, false);
	}

	if(host_info_ && !host_info_->registered) {
		registered_users_->widget_set_enabled(win, false, false);
	}

	//
	// Set up tab control
	//
	listbox& tab_bar = find_widget<listbox>(&win, "tab_bar", false);

	connect_signal_notify_modified(tab_bar,
		std::bind(&mp_create_game::on_tab_select, this, std::ref(win)));

	// Allow the settings stack to find widgets in all pages, regardless of which is selected.
	// This ensures settings (especially game settings) widgets are appropriately updated when
	// a new game is selected, regardless of which settings tab is active at the time.
	find_widget<stacked_widget>(&win, "pager", false).set_find_in_all_layers(true);

	// We call on_tab_select farther down.

	//
	// Main games list
	//
	listbox& list = find_widget<listbox>(&win, "games_list", false);

	connect_signal_notify_modified(list,
		std::bind(&mp_create_game::on_game_select, this, std::ref(win)));

	win.add_to_keyboard_chain(&list);

	// This handles the initial game selection as well
	display_games_of_type(win, level_types_[get_initial_type_index()].first, preferences::level());

	// Initial tab selection must be done after game selection so the field widgets are set to their correct active state.
	on_tab_select(win);

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Create"));

	plugins_context_->set_callback("create", [&win](const config&) { win.set_retval(window::OK); }, false);
	plugins_context_->set_callback("quit",   [&win](const config&) { win.set_retval(window::CANCEL); }, false);
	plugins_context_->set_callback("load",   [this, &win](const config&) { load_game_callback(win); }, false);

#define UPDATE_ATTRIBUTE(field, convert) \
	do { if(cfg.has_attribute(#field)) { field##_->set_widget_value(win, cfg[#field].convert()); } } while(false) \

	plugins_context_->set_callback("update_settings", [this, &win](const config& cfg) {
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

	plugins_context_->set_callback("select_mod",   [this, &win](const config& cfg) {
		on_mod_toggle(win, cfg["index"].to_int(), nullptr);
	}, true);

	plugins_context_->set_accessor("game_config",  [this](const config&) {return cfg_; });
	plugins_context_->set_accessor("get_selected", [this](const config&) {
		const ng::level& current_level = create_engine_.current_level();
		return config {
			"id", current_level.id(),
			"name", current_level.name(),
			"icon", current_level.icon(),
			"description", current_level.description(),
			"allow_era_choice", current_level.allow_era_choice(),
			"type", create_engine_.current_level_type(),
		};
	});

	plugins_context_->set_accessor("find_level",   [this](const config& cfg) {
		const std::string id = cfg["id"].str();
		return config {
			"index", create_engine_.find_level_by_id(id).second,
			"type", create_engine_.find_level_by_id(id).first,
		};
	});

	plugins_context_->set_accessor_int("find_era", [this](const config& cfg) {
		return create_engine_.find_extra_by_id(ng::create_engine::ERA, cfg["id"]);
	});

	plugins_context_->set_accessor_int("find_mod", [this](const config& cfg) {
		return create_engine_.find_extra_by_id(ng::create_engine::MOD, cfg["id"]);
	});
}

void mp_create_game::sync_with_depcheck(window& window)
{
	DBG_MP << "sync_with_depcheck: start\n";

	if(static_cast<int>(create_engine_.current_era_index()) != create_engine_.dependency_manager().get_era_index()) {
		DBG_MP << "sync_with_depcheck: correcting era\n";
		const int new_era_index = create_engine_.dependency_manager().get_era_index();

		create_engine_.set_current_era_index(new_era_index, true);
		eras_menu_button_->set_value(new_era_index);
	}

	if(create_engine_.current_level().id() != create_engine_.dependency_manager().get_scenario()) {
		DBG_MP << "sync_with_depcheck: correcting scenario\n";

		// Match scenario and scenario type
		const auto new_level_index = create_engine_.find_level_by_id(create_engine_.dependency_manager().get_scenario());
		const bool different_type = new_level_index.first != create_engine_.current_level_type();

		if(new_level_index.second != -1) {
			create_engine_.set_current_level_type(new_level_index.first);
			create_engine_.set_current_level(new_level_index.second);
			selected_game_index_ = new_level_index.second;

			auto& game_types_list = find_widget<menu_button>(&window, "game_types", false);
			game_types_list.set_value(std::find_if(level_types_.begin(), level_types_.begin(), [&](const level_type_info& info){ return info.first == new_level_index.first; }) - level_types_.begin());

			if(different_type) {
				display_games_of_type(window, new_level_index.first, create_engine_.current_level().id());
			} else {
				// This function (or rather on_game_select) might be triggered by a listbox callback, in
				// which case we cannot use display_games_of_type since it destroys the list (and its
				// elements) which might result in segfaults. Instead, we assume that a listbox-triggered
				// sync_with_depcheck call never changes the game type and goes to this branch instead.
				find_widget<listbox>(&window, "games_list", false).select_row(new_level_index.second);

				// Override the last selection so on_game_select selects the new level
				selected_game_index_ = -1;

				on_game_select(window);
			}
		}
	}

	if(get_active_mods() != create_engine_.dependency_manager().get_modifications()) {
		DBG_MP << "sync_with_depcheck: correcting modifications\n";
		set_active_mods(create_engine_.dependency_manager().get_modifications());
	}

	create_engine_.init_active_mods();
	DBG_MP << "sync_with_depcheck: end\n";
}

template<typename widget>
void mp_create_game::on_filter_change(window& window, const std::string& id, bool do_select)
{
	create_engine_.apply_level_filter(find_widget<widget>(&window, id, false).get_value());

	listbox& game_list = find_widget<listbox>(&window, "games_list", false);

	boost::dynamic_bitset<> filtered(game_list.get_item_count());
	for(const size_t i : create_engine_.get_filtered_level_indices(create_engine_.current_level_type())) {
		filtered[i] = true;
	}

	game_list.set_row_shown(filtered);

	if(do_select) {
		on_game_select(window);
	}
}

void mp_create_game::on_game_select(window& window)
{
	const int selected_game = find_widget<listbox>(&window, "games_list", false).get_selected_row();

	if(selected_game == selected_game_index_) {
		return;
	}

	// Convert the absolute-index get_selected_row to a relative index for the create_engine to handle
	selected_game_index_ = convert_to_game_filtered_index(selected_game);

	create_engine_.set_current_level(selected_game_index_);

	sync_with_depcheck(window);

	update_details(window);

	// General settings
	const bool can_select_era = create_engine_.current_level().allow_era_choice();

	if(!can_select_era) {
		eras_menu_button_->set_label(_("No eras available for this game."));
	} else {
		eras_menu_button_->set_selected(eras_menu_button_->get_value());
	}

	eras_menu_button_->set_active(can_select_era);

	// Custom options
	options_manager_->update_game_options();

	// Game settings
	update_map_settings(window);
}

void mp_create_game::on_tab_select(window& window)
{
	const int i = find_widget<listbox>(&window, "tab_bar", false).get_selected_row();
	find_widget<stacked_widget>(&window, "pager", false).select_layer(i);
}

void mp_create_game::on_mod_toggle(window& window, const int index, toggle_button* sender)
{
	if(sender && (sender->get_value_bool() == create_engine_.dependency_manager().is_modification_active(index))) {
		ERR_MP << "ignoring on_mod_toggle that is already set\n";
		return;
	}

	create_engine_.toggle_mod(index);

	sync_with_depcheck(window);

	options_manager_->update_mod_options();
}

void mp_create_game::on_era_select(window& window)
{
	create_engine_.set_current_era_index(eras_menu_button_->get_value());

	eras_menu_button_->set_tooltip(create_engine_.current_era().description);

	sync_with_depcheck(window);

	options_manager_->update_era_options();
}

void mp_create_game::on_random_faction_mode_select(window& window)
{
	selected_rfm_index_ = find_widget<menu_button>(&window, "random_faction_mode", false).get_value();
}

void mp_create_game::show_description(window& window, const std::string& new_description)
{
	styled_widget& description = find_widget<styled_widget>(&window, "description", false);

	description.set_label(!new_description.empty() ? new_description : _("No description available."));
	description.set_use_markup(true);
}

void mp_create_game::update_games_list(window& window)
{
	const int index = find_widget<menu_button>(&window, "game_types", false).get_value();

	display_games_of_type(window, level_types_[index].first, create_engine_.current_level().id());
}

void mp_create_game::display_games_of_type(window& window, ng::level::TYPE type, const std::string& level)
{
	create_engine_.set_current_level_type(type);

	listbox& list = find_widget<listbox>(&window, "games_list", false);

	list.clear();

	for(const auto& game : create_engine_.get_levels_by_type_unfiltered(type)) {
		if(!game->can_launch_game()) {
			continue;
		}

		std::map<std::string, string_map> data;
		string_map item;

		if(type == ng::level::TYPE::CAMPAIGN || type == ng::level::TYPE::SP_CAMPAIGN) {
			item["label"] = game->icon();
			data.emplace("game_icon", item);
		}

		item["label"] = game->name();
		data.emplace("game_name", item);

		list.add_row(data);
	}

	if(!level.empty() && !list.get_rows_shown().empty()) {
		// Recalculate which rows should be visible
		on_filter_change<slider>(window, "num_players", false);
		on_filter_change<text_box>(window, "game_filter", false);

		int level_index = create_engine_.find_level_by_id(level).second;
		if(level_index >= 0 && size_t(level_index) < list.get_item_count()) {
			list.select_row(level_index);
		}
	}

	const bool is_random_map = type == ng::level::TYPE::RANDOM_MAP;

	find_widget<button>(&window, "random_map_regenerate", false).set_active(is_random_map);
	find_widget<button>(&window, "random_map_settings", false).set_active(is_random_map);

	// Override the last selection so on_game_select selects the new level
	selected_game_index_ = -1;

	on_game_select(window);
}

void mp_create_game::show_generator_settings(window& window)
{
	create_engine_.generator_user_config();

	regenerate_random_map(window);
}

void mp_create_game::regenerate_random_map(window& window)
{
	create_engine_.init_generated_level_data();

	update_details(window);
}

int mp_create_game::convert_to_game_filtered_index(const unsigned int initial_index)
{
	const std::vector<size_t>& filtered_indices = create_engine_.get_filtered_level_indices(create_engine_.current_level_type());
	return std::find(filtered_indices.begin(), filtered_indices.end(), initial_index) - filtered_indices.begin();
}

void mp_create_game::update_details(window& win)
{
	styled_widget& players = find_widget<styled_widget>(&win, "map_num_players", false);
	styled_widget& map_size = find_widget<styled_widget>(&win, "map_size", false);

	if(create_engine_.current_level_type() == ng::level::TYPE::RANDOM_MAP) {
		// If the current random map doesn't have data, generate it
		if(create_engine_.generator_assigned() && create_engine_.current_level().data()["map_data"].empty()) {
			create_engine_.init_generated_level_data();
		}

		find_widget<button>(&win, "random_map_settings", false).set_active(create_engine_.generator_has_settings());
	}

	create_engine_.current_level().set_metadata();

	// Reset the config_engine with new values
	config_engine_.reset(new ng::configure_engine(create_engine_.get_state()));
	config_engine_->update_initial_cfg(create_engine_.current_level().data());
	config_engine_->set_default_values();

	// Set the title, with newlines replaced. Newlines are sometimes found in SP Campaign names
	std::string title = create_engine_.current_level().name();
	boost::replace_all(title, "\n", " " + font::unicode_em_dash + " ");
	find_widget<styled_widget>(&win, "game_title", false).set_label(title);

	show_description(win, create_engine_.current_level().description());

	switch(create_engine_.current_level_type().v) {
		case ng::level::TYPE::SCENARIO:
		case ng::level::TYPE::USER_MAP:
		case ng::level::TYPE::USER_SCENARIO:
		case ng::level::TYPE::RANDOM_MAP: {
			ng::scenario* current_scenario = dynamic_cast<ng::scenario*>(&create_engine_.current_level());

			assert(current_scenario);

			create_engine_.get_state().classification().campaign = "";

			find_widget<stacked_widget>(&win, "minimap_stack", false).select_layer(0);
			find_widget<minimap>(&win, "minimap", false).set_map_data(current_scenario->data()["map_data"]);

			players.set_label(std::to_string(current_scenario->num_players()));
			map_size.set_label(current_scenario->map_size());

			break;
		}
		case ng::level::TYPE::CAMPAIGN:
		case ng::level::TYPE::SP_CAMPAIGN: {
			ng::campaign* current_campaign = dynamic_cast<ng::campaign*>(&create_engine_.current_level());

			assert(current_campaign);

			create_engine_.get_state().classification().campaign = current_campaign->data()["id"].str();

			const std::string img = formatter() << current_campaign->data()["image"] << "~SCALE_INTO(265,265)";

			find_widget<stacked_widget>(&win, "minimap_stack", false).select_layer(1);
			find_widget<image>(&win, "campaign_image", false).set_image(img);

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
}

void mp_create_game::update_map_settings(window& window)
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

void mp_create_game::load_game_callback(window& window)
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

std::vector<std::string> mp_create_game::get_active_mods()
{
	int i = 0;
	std::set<std::string> res;
	for(const auto& mod : create_engine_.get_extras_by_type(ng::create_engine::MOD)) {
		if(find_widget<toggle_button>(mod_list_->get_row_grid(i), "mod_active_state", false).get_value_bool()) {
			res.insert(mod->id);
		}
		++i;
	}
	return std::vector<std::string>(res.begin(), res.end());
}

void mp_create_game::set_active_mods(const std::vector<std::string>& val)
{
	std::set<std::string> val2(val.begin(), val.end());
	int i = 0;
	std::set<std::string> res;
	for(const auto& mod : create_engine_.get_extras_by_type(ng::create_engine::MOD)) {
		find_widget<toggle_button>(mod_list_->get_row_grid(i), "mod_active_state", false).set_value_bool(val2.find(mod->id) != val2.end());
		++i;
	}
}

bool mp_create_game::dialog_exit_hook(window& /*window*/)
{
	if(!create_engine_.current_level_has_side_data()) {
		gui2::show_transient_error_message(_("The selected game has no sides!"));
		return false;
	}

	if(create_engine_.current_level_type() != ng::level::TYPE::CAMPAIGN &&
	   create_engine_.current_level_type() != ng::level::TYPE::SP_CAMPAIGN
	) {
		return true;
	}

	return create_engine_.select_campaign_difficulty() != "CANCEL";
}

void mp_create_game::post_show(window& window)
{
	plugins_context_.reset();

	// Show all tabs so that find_widget works correctly
	find_widget<stacked_widget>(&window, "pager", false).select_layer(-1);

	if(get_retval() == LOAD_GAME) {
		create_engine_.prepare_for_saved_game();
		return;
	}

	if(get_retval() == window::OK) {
		prefs::set_modifications(create_engine_.active_mods());
		prefs::set_level_type(create_engine_.current_level_type().v);
		prefs::set_level(create_engine_.current_level().id());
		prefs::set_era(create_engine_.current_era().id);

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
			const std::string first_scenario = create_engine_.current_level().data()["first_scenario"];
			for(const config& scenario : game_config_manager::get()->game_config().child_range(tagname)) {
				const bool is_first = scenario["id"] == first_scenario;
				if(scenario["allow_new_game"].to_bool(false) || is_first || game_config::debug ) {
					const std::string& title = !scenario["new_game_title"].empty()
						? scenario["new_game_title"]
						: scenario["name"];

					entry_points.insert(is_first ? entry_points.begin() : entry_points.end(), &scenario);
					entry_point_titles.insert(is_first ? entry_point_titles.begin() : entry_point_titles.end(), title);
				}
			}
		}

		if(entry_points.size() > 1) {
			gui2::dialogs::simple_item_selector dlg(_("Choose Starting Scenario"), _("Select at which point to begin this campaign."), entry_point_titles);

			dlg.set_single_button(true);
			dlg.show();

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

		// Since we don't have a field handling this option, we need to save the value manually
		prefs::set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE::enum_to_string(rfm_types_[selected_rfm_index_]));

		// Save custom option settings
		const config options_config = options_manager_->get_options_config();

		config_engine_->set_options(options_config);
		prefs::set_options(options_config);

		// Set game name
		const std::string name = find_widget<text_box>(&window, "game_name", false).get_value();
		if(!name.empty() && (name != ng::configure_engine::game_name_default())) {
			config_engine_->set_game_name(name);
		}

		// Set game password
		const std::string password = find_widget<text_box>(&window, "game_password", false).get_value();
		if(!password.empty()) {
			config_engine_->set_game_password(password);
		}
	}
}

} // namespace dialogs
} // namespace gui2
