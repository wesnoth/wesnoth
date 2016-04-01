/*
   Copyright (C) 2013 - 2016 Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "gettext.hpp"
#include "game_preferences.hpp"
#include "construct_dialog.hpp"
#include "settings.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_create_game_set_password.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "minimap.hpp"
#include "mp_game_settings.hpp"
#include "multiplayer_configure.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "saved_game.hpp"
#include "scripting/plugins/context.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "formula/string_utils.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_configure("mp/configure");
#define DBG_MP LOG_STREAM(debug, log_mp_configure)

namespace mp {

configure::nolock_settings::nolock_settings(CVideo& video)
	: turns_slider_(video)
	, turns_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR)
	, village_gold_slider_(video)
	, village_gold_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR)
	, village_support_slider_(video)
	, village_support_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR)
	, xp_modifier_slider_(video)
	, xp_modifier_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR)
	, generic_label_(video, "`~ " + std::string(_("Generic")), font::SIZE_PLUS, font::LOBBY_COLOR)
	, use_map_settings_(video, _("Use map settings"), gui::button::TYPE_CHECK)
	, random_start_time_(video, _("Random start time"), gui::button::TYPE_CHECK)
	, fog_game_(video, _("Fog of war"), gui::button::TYPE_CHECK)
	, shroud_game_(video, _("Shroud"), gui::button::TYPE_CHECK)
{

}
configure::configure(CVideo& video, const config &cfg, chat& c, config& gamelist, saved_game& game, bool local_players_only) :
	ui(video, _("Configure Game"), cfg, c, gamelist),

	local_players_only_(local_players_only),
	tooltip_manager_(video),
	mp_countdown_init_time_(270),
	mp_countdown_reservoir_time_(330),

	countdown_game_(video, _("Time limit"), gui::button::TYPE_CHECK),
	countdown_init_time_slider_(video),
	countdown_init_time_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_reservoir_time_slider_(video),
	countdown_reservoir_time_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_turn_bonus_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_turn_bonus_slider_(video),
	countdown_action_bonus_label_(video, "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_action_bonus_slider_(video),
	name_entry_label_(video, _("Name of game:"), font::SIZE_PLUS, font::LOBBY_COLOR),
	observers_game_(video, _("Observers"), gui::button::TYPE_CHECK),
	oos_debug_(video, _("Debug OOS"), gui::button::TYPE_CHECK),
	shuffle_sides_(video, _("Shuffle sides"), gui::button::TYPE_CHECK),
	random_faction_mode_label_(video, _("Random factions:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	random_faction_mode_(video, std::vector<std::string>()),
	cancel_game_(video, _("Back")),
	launch_game_(video, _("OK")),
	password_button_(video, _("Set Password...")),
	name_entry_(video, 32),
	entry_points_label_(video, _("Select an entry point:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	entry_points_combo_(video, std::vector<std::string>()),
	options_pane_left_(video),
	options_pane_right_(video),
	entry_points_(),
	show_entry_points_(false),
	force_use_map_settings_check_(false),
	state_(game),
	parameters_(state_.mp_settings()),
	engine_(state_),
	options_manager_(cfg, video, &options_pane_right_, engine_.options_default()),
	nolock_settings_(engine_.force_lock_settings() ? 0 : new nolock_settings(video))
{
	// Build the list of scenarios to play

	DBG_MP << "constructing multiplayer configure dialog" << std::endl;

	countdown_game_.set_check(engine_.mp_countdown_default());
	countdown_game_.set_help_string(_("Enables user time limit"));

	countdown_init_time_slider_.set_min(30);
	countdown_init_time_slider_.set_max(1500);
	countdown_init_time_slider_.set_increment(30);
	countdown_init_time_slider_.set_value(engine_.mp_countdown_init_time_default());
	countdown_init_time_slider_.set_help_string(_("Longest time allowed for first turn (seconds)"));

	countdown_reservoir_time_slider_.set_min(30);
	countdown_reservoir_time_slider_.set_max(1500);
	countdown_reservoir_time_slider_.set_increment(30);
	countdown_reservoir_time_slider_.set_value(engine_.mp_countdown_reservoir_time_default());
	countdown_reservoir_time_slider_.set_help_string(_("Longest time possible for any turn (seconds)"));

	countdown_turn_bonus_slider_.set_min(10);
	countdown_turn_bonus_slider_.set_max(300);
	countdown_turn_bonus_slider_.set_increment(5);
	countdown_turn_bonus_slider_.set_value(engine_.mp_countdown_turn_bonus_default());
	countdown_turn_bonus_slider_.set_help_string(_("Time for general tasks each turn (seconds)"));

	countdown_action_bonus_slider_.set_min(0);
	countdown_action_bonus_slider_.set_max(30);
	countdown_action_bonus_slider_.set_increment(1);
	countdown_action_bonus_slider_.set_value(engine_.mp_countdown_action_bonus_default());
	countdown_action_bonus_slider_.set_help_string(_("Time for each attack, recruit, and capture"));

	observers_game_.set_check(engine_.allow_observers_default());
	observers_game_.set_help_string(_("Allow users who are not playing to watch the game"));
	observers_game_.enable(state_.classification().campaign_type != game_classification::CAMPAIGN_TYPE::SCENARIO);

	oos_debug_.set_check(false);
	oos_debug_.set_help_string(_("More checks for OOS errors but also more network traffic"));
	oos_debug_.enable(true);

	shuffle_sides_.set_check(engine_.shuffle_sides_default());
	shuffle_sides_.set_help_string(_("Assign sides to players at random"));

	random_faction_mode_label_.set_help_string(_("Allow for mirror matchups when random factions are chosen"));

	std::vector<std::string> translated_modes;
	for(size_t i = 0; i < mp_game_settings::RANDOM_FACTION_MODE::count; ++i) {
		std::string mode_str = mp_game_settings::RANDOM_FACTION_MODE::from_int(i).to_string();
		translated_modes.push_back(translation::gettext(mode_str.c_str()));
	}
	random_faction_mode_.set_items(translated_modes);
	random_faction_mode_.set_selected(engine_.random_faction_mode().cast<int>());
	random_faction_mode_.set_help_string(_("Independent: Random factions assigned independently\nNo Mirror: No two players will get the same faction\nNo Ally Mirror: No two allied players will get the same faction"));

	if(nolock_settings_) {
		nolock_settings_->use_map_settings_.enable(!engine_.force_lock_settings());
		nolock_settings_->use_map_settings_.set_check(engine_.use_map_settings());
		nolock_settings_->use_map_settings_.set_help_string(_("Use scenario specific settings"));

		nolock_settings_->turns_slider_.set_min(settings::turns_min);
		nolock_settings_->turns_slider_.set_max(settings::turns_max);
		nolock_settings_->turns_slider_.set_increment(settings::turns_step);
		nolock_settings_->turns_slider_.set_value(engine_.num_turns_default());
		nolock_settings_->turns_slider_.set_help_string(_("The maximum number of turns the game can last"));
		nolock_settings_->turns_slider_.enable(!engine_.use_map_settings());

		nolock_settings_->village_gold_slider_.set_min(1);
		nolock_settings_->village_gold_slider_.set_max(5);
		nolock_settings_->village_gold_slider_.set_value(engine_.village_gold_default());
		nolock_settings_->village_gold_slider_.set_help_string(_("The amount of income each village yields per turn"));
		nolock_settings_->village_gold_slider_.enable(!engine_.use_map_settings());

		nolock_settings_->village_support_slider_.set_min(0);
		nolock_settings_->village_support_slider_.set_max(4);
		nolock_settings_->village_support_slider_.set_value(engine_.village_support_default());
		nolock_settings_->village_support_slider_.set_help_string(_("The number of unit levels each village can support"));
		nolock_settings_->village_support_slider_.enable(!engine_.use_map_settings());

		nolock_settings_->xp_modifier_slider_.set_min(30);
		nolock_settings_->xp_modifier_slider_.set_max(200);
		nolock_settings_->xp_modifier_slider_.set_value(engine_.xp_modifier_default());
		nolock_settings_->xp_modifier_slider_.set_increment(10);
		nolock_settings_->xp_modifier_slider_.set_help_string(_("The amount of experience a unit needs to advance"));
		nolock_settings_->xp_modifier_slider_.enable(!engine_.use_map_settings());

		nolock_settings_->random_start_time_.set_check(engine_.random_start_time_default());
		nolock_settings_->random_start_time_.set_help_string(_("Randomize time of day in begin"));
		nolock_settings_->random_start_time_.enable(!engine_.use_map_settings());

		nolock_settings_->fog_game_.set_check(engine_.fog_game_default());
		nolock_settings_->fog_game_.set_help_string(_("Enemy units cannot be seen unless they are in range of your units"));
		nolock_settings_->fog_game_.enable(!engine_.use_map_settings());

		nolock_settings_->shroud_game_.set_check(engine_.shroud_game_default());
		nolock_settings_->shroud_game_.set_help_string(_("The map is unknown until your units explore it"));
		nolock_settings_->shroud_game_.enable(!engine_.use_map_settings());
	}
#if 0
	// The possible vision settings
	std::vector<std::string> vision_types;
	vision_types.push_back(_("Share View"));
	vision_types.push_back(_("Share Maps"));
	vision_types.push_back(_("Share None"));
#endif
	// The starting points for campaign.

	if (engine_.entry_point_titles().size() > 1) {
		entry_points_combo_.set_items(engine_.entry_point_titles());
		entry_points_combo_.set_selected(0);

		show_entry_points_ = true;
	}

	options_manager_.set_era(parameters_.mp_era);
	if(state_.classification().campaign.empty()) {
		options_manager_.set_scenario(state_.get_scenario_id());
	}
	else {
		options_manager_.set_campaign(state_.classification().campaign);
	}
	options_manager_.set_modifications(parameters_.active_mods);
	options_manager_.init_widgets();

	name_entry_.set_text(engine_.game_name_default());

	gamelist_updated();

	plugins_context_.reset(new plugins_context("Multiplayer Configure"));

	//These structure initializers create a lobby::process_data_event
	plugins_context_->set_callback("launch", 	boost::bind(&configure::plugin_event_helper, this, process_event_data (true, false)));
	plugins_context_->set_callback("quit", 		boost::bind(&configure::plugin_event_helper, this, process_event_data (false, true)));
	plugins_context_->set_callback("set_name",	boost::bind(&gui::textbox::set_text, &name_entry_, boost::bind(get_str, _1, "name"), font::NORMAL_COLOR), true);

	if(!options_manager_.has_options() && engine_.force_lock_settings() && state_.classification().campaign_type != game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		set_result(CREATE);
	}
}

configure::~configure()
{
	try {
	// Only save the settings if the dialog was 'accepted'
	if(get_result() != CREATE) {
		DBG_MP << "destructing multiplayer configure dialog - aborted game creation" << std::endl;
		return;
	}
	DBG_MP << "destructing multiplayer configure dialog - a game will be configured" << std::endl;

	// Save values for next game
	DBG_MP << "storing parameter values in preferences" << std::endl;
	preferences::set_shuffle_sides(engine_.shuffle_sides());
	preferences::set_random_faction_mode(engine_.random_faction_mode().to_string());
	preferences::set_use_map_settings(engine_.use_map_settings());
	preferences::set_countdown(engine_.mp_countdown());
	preferences::set_countdown_init_time(engine_.mp_countdown_init_time());
	preferences::set_countdown_turn_bonus(engine_.mp_countdown_turn_bonus());
	preferences::set_countdown_reservoir_time(engine_.mp_countdown_reservoir_time());
	preferences::set_countdown_action_bonus(engine_.mp_countdown_action_bonus());
	preferences::set_options(engine_.options());
	// don't set observers preference if disabled (for singleplayer)
	if (observers_game_.enabled())
		preferences::set_allow_observers(engine_.allow_observers());

	// When using map settings, the following variables are determined by the map,
	// so don't store them as the new preferences.
	if(!engine_.use_map_settings()) {
		preferences::set_fog(engine_.fog_game());
		preferences::set_shroud(engine_.shroud_game());
		preferences::set_turns(engine_.num_turns());
		preferences::set_random_start_time(engine_.random_start_time());
		preferences::set_village_gold(engine_.village_gold());
		preferences::set_village_support(engine_.village_support());
		preferences::set_xp_modifier(engine_.xp_modifier());
	}
	} catch (...) {}
}

void configure::get_parameters()
{
	DBG_MP << "getting parameter values from widgets" << std::endl;

	const int mp_countdown_turn_bonus_val = countdown_turn_bonus_slider_.value() <= countdown_turn_bonus_slider_.max_value() ?
		countdown_turn_bonus_slider_.value() : -1;
	const int mp_countdown_action_bonus_val = countdown_action_bonus_slider_.value() <= countdown_action_bonus_slider_.max_value() ?
		countdown_action_bonus_slider_.value() : -1;
	const int mp_countdown_reservoir_time_val = countdown_reservoir_time_slider_.value() <= countdown_reservoir_time_slider_.max_value() ?
		countdown_reservoir_time_slider_.value() : -1;
	int mp_countdown_init_time_val = countdown_init_time_slider_.value() <= countdown_init_time_slider_.max_value() ?
		countdown_init_time_slider_.value() : -1;
	if(mp_countdown_reservoir_time_val > 0 && mp_countdown_init_time_val > mp_countdown_reservoir_time_val)
		mp_countdown_init_time_val = mp_countdown_reservoir_time_val;

	// Updates the values in the configure_engine to match
	// the values selected by the user with the widgets:
	engine_.set_game_name(name_entry_.text());

	// CHECK
	engine_.set_mp_countdown_init_time(mp_countdown_init_time_val);
	engine_.set_mp_countdown_turn_bonus(mp_countdown_turn_bonus_val);
	engine_.set_mp_countdown_reservoir_time(mp_countdown_reservoir_time_val);
	engine_.set_mp_countdown_action_bonus(mp_countdown_action_bonus_val);
	engine_.set_mp_countdown(countdown_game_.checked());
	if(nolock_settings_) {
		const int num_turns_val = nolock_settings_->turns_slider_.value() <
			nolock_settings_->turns_slider_.max_value() ? nolock_settings_->turns_slider_.value() : -1;
		engine_.set_num_turns(num_turns_val);
		engine_.set_village_gold(nolock_settings_->village_gold_slider_.value());
		engine_.set_village_support(nolock_settings_->village_support_slider_.value());
		engine_.set_xp_modifier(nolock_settings_->xp_modifier_slider_.value());
		engine_.set_use_map_settings(nolock_settings_->use_map_settings_.checked());
		engine_.set_random_start_time(nolock_settings_->random_start_time_.checked());
		engine_.set_fog_game(nolock_settings_->fog_game_.checked());
		engine_.set_shroud_game(nolock_settings_->shroud_game_.checked());
		engine_.write_parameters();
	}
	engine_.set_allow_observers(observers_game_.checked());
	engine_.set_oos_debug(oos_debug_.checked());
	engine_.set_shuffle_sides(shuffle_sides_.checked());
	engine_.set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE::from_int(random_faction_mode_.selected()));

	engine_.set_options(options_manager_.get_values());

}

bool configure::plugin_event_helper(const process_event_data & data)
{
	process_event_impl(data);
	return get_result() == mp::ui::CONTINUE;
}

void configure::process_event()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	process_event_data data;
	data.launch = launch_game_.pressed();
	data.quit = cancel_game_.pressed();

	process_event_impl(data);
}

void configure::process_event_impl(const process_event_data & data)
{
	if(data.quit) {
		set_result(QUIT);
		return;
	}

	if(data.launch) {
		// check if the map is valid
		if (name_entry_.text() == "") {
			gui2::show_transient_message(video(), "", _("You must enter a name."));
		} else {
			set_result(CREATE);
			return;
		}
	}

	if(password_button_.pressed()) {
		gui2::tmp_create_game_set_password::execute(
				  parameters_.password
				, video());
	}

	if (entry_points_combo_.changed()) {
		engine_.set_scenario(entry_points_combo_.selected());
		force_use_map_settings_check_ = true;
	}
	std::stringstream buf;
	if(nolock_settings_) {
		// Turns per game
		const int cur_turns = nolock_settings_->turns_slider_.value();

		if(cur_turns < 100) {
			buf << _("Turns: ") << cur_turns;
		} else {
			buf << _("Unlimited turns");
		}
		nolock_settings_->turns_label_.set_text(buf.str());


		// Villages can produce between 1 and 5 gold a turn
		const int village_gold = nolock_settings_->village_gold_slider_.value();
		buf.str("");
		buf << _("Village gold: ") << village_gold;
		nolock_settings_->village_gold_label_.set_text(buf.str());

		// Unit levels supported per village
		const int village_support = nolock_settings_->village_support_slider_.value();
		buf.str("");
		buf << _("Village support: ") << village_support;
		nolock_settings_->village_support_label_.set_text(buf.str());

		// Experience modifier
		const int xpmod = nolock_settings_->xp_modifier_slider_.value();
		buf.str("");
		buf << _("Experience modifier: ") << xpmod << "%";

		nolock_settings_->xp_modifier_label_.set_text(buf.str());


		if(nolock_settings_->use_map_settings_.pressed() || force_use_map_settings_check_) {
			force_use_map_settings_check_ = false;

			engine_.set_use_map_settings(nolock_settings_->use_map_settings_.checked());

			// If the map settings are wanted use them,
			// if not properly defined fall back to the default settings
			nolock_settings_->turns_slider_.set_value(engine_.num_turns_default());
			nolock_settings_->xp_modifier_slider_.set_value(engine_.xp_modifier_default());
			nolock_settings_->random_start_time_.set_check(engine_.random_start_time_default());
			nolock_settings_->village_gold_slider_.set_value(engine_.village_gold_default());
			nolock_settings_->village_support_slider_.set_value(engine_.village_support_default());
			nolock_settings_->fog_game_.set_check(engine_.fog_game_default());
			nolock_settings_->shroud_game_.set_check(engine_.shroud_game_default());

			// Set the widget states
			nolock_settings_->turns_slider_.enable(!engine_.use_map_settings());
			nolock_settings_->village_gold_slider_.enable(!engine_.use_map_settings());
			nolock_settings_->village_support_slider_.enable(!engine_.use_map_settings());
			nolock_settings_->xp_modifier_slider_.enable(!engine_.use_map_settings());
			nolock_settings_->random_start_time_.enable(!engine_.use_map_settings());
			nolock_settings_->fog_game_.enable(!engine_.use_map_settings());
			nolock_settings_->shroud_game_.enable(!engine_.use_map_settings());
		}
	}
	countdown_init_time_label_.enable(countdown_game_.checked());
	countdown_init_time_slider_.enable(countdown_game_.checked());
	countdown_turn_bonus_label_.enable(countdown_game_.checked());
	countdown_turn_bonus_slider_.enable(countdown_game_.checked());

	countdown_reservoir_time_label_.enable(countdown_game_.checked());
	countdown_reservoir_time_slider_.enable(countdown_game_.checked());
	countdown_action_bonus_label_.enable(countdown_game_.checked());
	countdown_action_bonus_slider_.enable(countdown_game_.checked());

	if(mp_countdown_init_time_ != countdown_init_time_slider_.value()
		&& countdown_init_time_slider_.value() > countdown_reservoir_time_slider_.value())
	{
		countdown_reservoir_time_slider_.set_value(countdown_init_time_slider_.value());
	}
	if(mp_countdown_reservoir_time_ != countdown_reservoir_time_slider_.value()
		&& countdown_reservoir_time_slider_.value() < countdown_init_time_slider_.value())
	{
		countdown_init_time_slider_.set_value(countdown_reservoir_time_slider_.value());
	}
	mp_countdown_init_time_ = countdown_init_time_slider_.value();
	mp_countdown_reservoir_time_ = countdown_reservoir_time_slider_.value();

	buf.str("");
	buf <<  _("Init. limit: ") << mp_countdown_init_time_; // << _(" sec.");
	countdown_init_time_label_.set_text(buf.str());

	const int mp_countdown_turn_bonus_val = countdown_turn_bonus_slider_.value();
	buf.str("");
	buf <<  _("Turn bonus: ") << mp_countdown_turn_bonus_val; // << _(" sec.");
	countdown_turn_bonus_label_.set_text(buf.str());

	buf.str("");
	buf <<  _("Reservoir: ") << mp_countdown_reservoir_time_; // << _(" sec.");
	countdown_reservoir_time_label_.set_text(buf.str());

	const int mp_countdown_action_bonus_val = countdown_action_bonus_slider_.value();
	buf.str("");
	buf <<  _("Action bonus: ") << mp_countdown_action_bonus_val; // << _(" sec.");
	countdown_action_bonus_label_.set_text(buf.str());

	options_manager_.process_event();
}

void configure::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) << " children widgets" << std::endl;

	ui::hide_children(hide);
	if (nolock_settings_)
	{
		nolock_settings_->turns_slider_.hide(hide);
		nolock_settings_->turns_label_.hide(hide);
		nolock_settings_->village_gold_slider_.hide(hide);
		nolock_settings_->village_gold_label_.hide(hide);
		nolock_settings_->village_support_slider_.hide(hide);
		nolock_settings_->village_support_label_.hide(hide);
		nolock_settings_->xp_modifier_slider_.hide(hide);
		nolock_settings_->xp_modifier_label_.hide(hide);
		nolock_settings_->generic_label_.hide(hide);
		nolock_settings_->use_map_settings_.hide(hide);
		nolock_settings_->random_start_time_.hide(hide);
		nolock_settings_->fog_game_.hide(hide);
		nolock_settings_->shroud_game_.hide(hide);
	}

	countdown_init_time_slider_.hide(hide);
	countdown_init_time_label_.hide(hide);
	countdown_turn_bonus_slider_.hide(hide);
	countdown_turn_bonus_label_.hide(hide);
	countdown_reservoir_time_slider_.hide(hide);
	countdown_reservoir_time_label_.hide(hide);
	countdown_action_bonus_slider_.hide(hide);
	countdown_action_bonus_label_.hide(hide);
	countdown_game_.hide(hide);

	name_entry_label_.hide(hide);

	observers_game_.hide(hide);
	oos_debug_.hide(hide);
	shuffle_sides_.hide(hide);
	random_faction_mode_label_.hide(hide);
	random_faction_mode_.hide(hide);
	cancel_game_.hide(hide);
	launch_game_.hide(hide);

	password_button_.hide(hide);
	name_entry_.hide(hide);

	entry_points_label_.hide(hide);
	entry_points_combo_.hide(hide);

	options_pane_left_.hide(hide);
	options_pane_right_.hide(hide);

	options_manager_.hide_children(hide);
}

void configure::layout_children(const SDL_Rect& rect)
{
	DBG_MP << "laying out the children" << std::endl;

	ui::layout_children(rect);

	const int border_size = 6;
	const int column_border_size = 10;

	SDL_Rect ca = client_area();
	int xpos = ca.x;
	int ypos = ca.y;
	const int column_width = (ca.w - ca.x) / 2 - column_border_size;

	// Dialog title
	ypos += title().height() + border_size;

	// Name Entry
	name_entry_label_.set_location(xpos, ypos);
	name_entry_.set_location(xpos + name_entry_label_.width() + border_size, ypos);
	name_entry_.set_width(ca.w - name_entry_label_.width() - border_size);
	ypos += std::max<int>(name_entry_.height(), name_entry_label_.height()) + border_size;

	// Save ypos here (column top)
	int ypos_columntop = ypos;

	const int right_pane_height =
		ca.h - (ypos_columntop - ca.y + launch_game_.height() + border_size);

	// First column: non-gameplay settings
	options_pane_left_.set_location(xpos, ypos);
	options_pane_left_.set_width(column_width);
	options_pane_left_.set_height(right_pane_height - entry_points_label_.height());

	int slider_width = options_pane_left_.width() - 40;

	int xpos_left = 0;
	int ypos_left = 0;

	ypos_left += 2 * border_size;
	options_pane_left_.add_widget(&shuffle_sides_, xpos_left, ypos_left);
	options_pane_left_.add_widget(&observers_game_,
		xpos_left + (options_pane_left_.width() - xpos_left) / 2 + border_size, ypos_left);
	ypos_left += shuffle_sides_.height() + border_size;

	options_pane_left_.add_widget(&random_faction_mode_label_, xpos_left, ypos_left);
	xpos_left += random_faction_mode_label_.width() + border_size;

	options_pane_left_.add_widget(&random_faction_mode_, xpos_left, ypos_left);
	xpos_left += random_faction_mode_.width() + border_size;

	if(!local_players_only_) {
		options_pane_left_.add_widget(&password_button_,
			std::max(xpos_left + 2* border_size, (options_pane_left_.width() / 2) + border_size), ypos_left);
	} else {
		password_button_.hide(true);
	}
	xpos_left = 0;
	ypos_left += random_faction_mode_.height() + border_size;

	options_pane_left_.add_widget(&countdown_game_, xpos_left, ypos_left);
	ypos_left += countdown_game_.height() + border_size;

	options_pane_left_.add_widget(&countdown_init_time_label_, xpos_left, ypos_left	);
	ypos_left += countdown_init_time_label_.height() + border_size;
	countdown_init_time_slider_.set_width(slider_width);
	options_pane_left_.add_widget(&countdown_init_time_slider_, xpos_left, ypos_left);
	ypos_left += countdown_init_time_slider_.height() + border_size;

	options_pane_left_.add_widget(&countdown_turn_bonus_label_, xpos_left, ypos_left);
	ypos_left += countdown_turn_bonus_label_.height() + border_size;
	countdown_turn_bonus_slider_.set_width(slider_width);
	options_pane_left_.add_widget(&countdown_turn_bonus_slider_, xpos_left, ypos_left);
	ypos_left += countdown_turn_bonus_slider_.height() + border_size;

	options_pane_left_.add_widget(&countdown_reservoir_time_label_, xpos_left, ypos_left);
	ypos_left += countdown_reservoir_time_label_.height() + border_size;
	countdown_reservoir_time_slider_.set_width(slider_width);
	options_pane_left_.add_widget(&countdown_reservoir_time_slider_, xpos_left, ypos_left);
	ypos_left += countdown_reservoir_time_slider_.height() + border_size;
	options_pane_left_.add_widget(&countdown_action_bonus_label_, xpos_left, ypos_left);
	ypos_left += countdown_action_bonus_label_.height() + border_size;
	countdown_action_bonus_slider_.set_width(slider_width);
	options_pane_left_.add_widget(&countdown_action_bonus_slider_, xpos_left, ypos_left);
	ypos_left += countdown_action_bonus_slider_.height() + border_size;

	options_pane_left_.add_widget(&oos_debug_, xpos_left, ypos_left	);
	ypos_left += oos_debug_.height() + border_size;

	if (show_entry_points_) {
		int x = ca.x;
		int y = ca.y + ca.h - entry_points_combo_.height();
		entry_points_combo_.set_location(x, y);
		y -= entry_points_label_.height() + border_size;
		entry_points_label_.set_location(x, y);
	}

	// Second column: gameplay settings
	xpos += column_width + column_border_size;
	ypos = ypos_columntop;

	options_pane_right_.set_location(xpos, ypos);
	options_pane_right_.set_width(ca.w - (xpos - ca.x));
	options_pane_right_.set_height(right_pane_height);

	slider_width = options_pane_right_.width() - 40;

	int xpos_right = 0;
	int ypos_right = 0;
	if(nolock_settings_)
	{
		options_pane_right_.add_widget(&nolock_settings_->generic_label_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->generic_label_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->use_map_settings_, xpos_right, ypos_right);
		options_pane_right_.add_widget(&nolock_settings_->fog_game_,
			xpos_right + (options_pane_right_.width() - xpos_right)/2 + 5, ypos_right);
		ypos_right += nolock_settings_->use_map_settings_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->random_start_time_, xpos_right, ypos_right);
		options_pane_right_.add_widget(&nolock_settings_->shroud_game_,
			xpos_right + (options_pane_right_.width() - xpos_right)/2 + 5, ypos_right);
		ypos_right += nolock_settings_->random_start_time_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->turns_label_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->turns_label_.height() + border_size;
		nolock_settings_->turns_slider_.set_width(slider_width);
		options_pane_right_.add_widget(&nolock_settings_->turns_slider_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->turns_slider_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->xp_modifier_label_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->xp_modifier_label_.height() + border_size;
		nolock_settings_->xp_modifier_slider_.set_width(slider_width);
		options_pane_right_.add_widget(&nolock_settings_->xp_modifier_slider_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->xp_modifier_slider_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->village_support_label_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->village_support_label_.height() + border_size;
		nolock_settings_->village_support_slider_.set_width(slider_width);
		options_pane_right_.add_widget(&nolock_settings_->village_support_slider_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->village_support_slider_.height() + border_size;

		options_pane_right_.add_widget(&nolock_settings_->village_gold_label_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->village_gold_label_.height() + border_size;
		nolock_settings_->village_gold_slider_.set_width(slider_width);

		options_pane_right_.add_widget(&nolock_settings_->village_gold_slider_, xpos_right, ypos_right);
		ypos_right += nolock_settings_->village_gold_slider_.height() + 3 * border_size;

	}

	options_manager_.layout_widgets(xpos_right, ypos_right, column_width);

	// OK / Cancel buttons
	gui::button* left_button = &launch_game_;
	gui::button* right_button = &cancel_game_;

#ifdef OK_BUTTON_ON_RIGHT
	std::swap(left_button,right_button);
#endif

	// Buttons
	right_button->set_location(ca.x + ca.w - right_button->width(),
	                           ca.y + ca.h - right_button->height());
	left_button->set_location(right_button->location().x - left_button->width() -
	                          gui::ButtonHPadding, ca.y + ca.h - left_button->height());
}

} // namespace mp


