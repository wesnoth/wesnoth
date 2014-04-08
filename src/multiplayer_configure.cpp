/*
   Copyright (C) 2013 - 2014 Boldizsár Lipka <lipkab@zoho.com>
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
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "construct_dialog.hpp"
#include "settings.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_create_game_set_password.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "minimap.hpp"
#include "multiplayer_configure.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_configure("mp/configure");
#define DBG_MP LOG_STREAM(debug, log_mp_configure)

namespace mp {

configure::configure(game_display& disp, const config &cfg, chat& c, config& gamelist, const mp_game_settings& params, bool local_players_only) :
	ui(disp, _("Configure Game"), cfg, c, gamelist),

	local_players_only_(local_players_only),
	tooltip_manager_(disp.video()),
	mp_countdown_init_time_(270),
	mp_countdown_reservoir_time_(330),

	turns_slider_(disp.video()),
	turns_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_game_(disp.video(), _("Time limit"), gui::button::TYPE_CHECK),
	countdown_init_time_slider_(disp.video()),
	countdown_init_time_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_reservoir_time_slider_(disp.video()),
	countdown_reservoir_time_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_turn_bonus_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_turn_bonus_slider_(disp.video()),
	countdown_action_bonus_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	countdown_action_bonus_slider_(disp.video()),
	village_gold_slider_(disp.video()),
	village_gold_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	village_support_slider_(disp.video()),
	village_support_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	xp_modifier_slider_(disp.video()),
	xp_modifier_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	generic_label_(disp.video(), "`~ " + std::string(_("Generic")), font::SIZE_PLUS, font::LOBBY_COLOR),
	name_entry_label_(disp.video(), _("Name of game:"), font::SIZE_PLUS, font::LOBBY_COLOR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	map_size_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	use_map_settings_(disp.video(), _("Use map settings"), gui::button::TYPE_CHECK),
	random_start_time_(disp.video(), _("Random start time"), gui::button::TYPE_CHECK),
	fog_game_(disp.video(), _("Fog of war"), gui::button::TYPE_CHECK),
	shroud_game_(disp.video(), _("Shroud"), gui::button::TYPE_CHECK),
	observers_game_(disp.video(), _("Observers"), gui::button::TYPE_CHECK),
	shuffle_sides_(disp.video(), _("Shuffle sides"), gui::button::TYPE_CHECK),
	cancel_game_(disp.video(), _("Back")),
	launch_game_(disp.video(), _("OK")),
	password_button_(disp.video(), _("Set Password...")),
	vision_combo_(disp, std::vector<std::string>()),
	name_entry_(disp.video(), 32),
	entry_points_label_(disp.video(), _("Select an entry point:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	entry_points_combo_(disp, std::vector<std::string>()),
	options_pane_left_(disp.video()),
	options_pane_right_(disp.video()),
	entry_points_(),
	show_entry_points_(false),
	force_use_map_settings_check_(true),
	parameters_(params),
	options_manager_(cfg, disp, &options_pane_right_, preferences::options())
{
	// Build the list of scenarios to play

	DBG_MP << "constructing multiplayer configure dialog" << std::endl;

	turns_slider_.set_min(settings::turns_min);
	turns_slider_.set_max(settings::turns_max);
	turns_slider_.set_increment(settings::turns_step);
	turns_slider_.set_value(preferences::turns());
	turns_slider_.set_help_string(_("The maximum number of turns the game can last"));

	countdown_game_.set_check(preferences::countdown());
	countdown_game_.set_help_string(_("Enables user time limit"));

	countdown_init_time_slider_.set_min(30);
	countdown_init_time_slider_.set_max(1500);
	countdown_init_time_slider_.set_increment(30);
	countdown_init_time_slider_.set_value(preferences::countdown_init_time());
	countdown_init_time_slider_.set_help_string(_("Longest time allowed for first turn (seconds)"));

	countdown_reservoir_time_slider_.set_min(30);
	countdown_reservoir_time_slider_.set_max(1500);
	countdown_reservoir_time_slider_.set_increment(30);
	countdown_reservoir_time_slider_.set_value(preferences::countdown_reservoir_time());
	countdown_reservoir_time_slider_.set_help_string(_("Longest time possible for any turn (seconds)"));

	countdown_turn_bonus_slider_.set_min(10);
	countdown_turn_bonus_slider_.set_max(300);
	countdown_turn_bonus_slider_.set_increment(5);
	countdown_turn_bonus_slider_.set_value(preferences::countdown_turn_bonus());
	countdown_turn_bonus_slider_.set_help_string(_("Time for general tasks each turn (seconds)"));

	countdown_action_bonus_slider_.set_min(0);
	countdown_action_bonus_slider_.set_max(30);
	countdown_action_bonus_slider_.set_increment(1);
	countdown_action_bonus_slider_.set_value(preferences::countdown_action_bonus());
	countdown_action_bonus_slider_.set_help_string(_("Time for each attack, recruit, and capture"));

	village_gold_slider_.set_min(1);
	village_gold_slider_.set_max(5);
	village_gold_slider_.set_value(preferences::village_gold());
	village_gold_slider_.set_help_string(_("The amount of income each village yields per turn"));

	village_support_slider_.set_min(0);
	village_support_slider_.set_max(4);
	village_support_slider_.set_value(preferences::village_support());
	village_support_slider_.set_help_string(_("The number of unit levels each village can support"));

	xp_modifier_slider_.set_min(30);
	xp_modifier_slider_.set_max(200);
	xp_modifier_slider_.set_value(preferences::xp_modifier());
	xp_modifier_slider_.set_increment(10);
	xp_modifier_slider_.set_help_string(_("The amount of experience a unit needs to advance"));

	if (parameters_.scenario_data["force_lock_settings"].to_bool()) {
		use_map_settings_.enable(false);
		use_map_settings_.set_check(true);
	} else {
		use_map_settings_.set_check(preferences::use_map_settings());
	}
	use_map_settings_.set_help_string(_("Use scenario specific settings"));

	random_start_time_.set_check(preferences::random_start_time());
	random_start_time_.set_help_string(_("Randomize time of day in begin"));

	fog_game_.set_check(preferences::fog());
	fog_game_.set_help_string(_("Enemy units cannot be seen unless they are in range of your units"));

	shroud_game_.set_check(preferences::shroud());
	shroud_game_.set_help_string(_("The map is unknown until your units explore it"));

	observers_game_.set_check(preferences::allow_observers());
	observers_game_.set_help_string(_("Allow users who are not playing to watch the game"));

	shuffle_sides_.set_check(preferences::shuffle_sides());
	shuffle_sides_.set_help_string(_("Assign sides to players at random"));

	// The possible vision settings
	std::vector<std::string> vision_types;
	vision_types.push_back(_("Share View"));
	vision_types.push_back(_("Share Maps"));
	vision_types.push_back(_("Share None"));
	vision_combo_.set_items(vision_types);
	vision_combo_.set_selected(0);

	// The starting points for campaign.
	std::vector<std::string> entry_point_titles;

	BOOST_FOREACH(const config& scenario,
		game_config().child_range("multiplayer")) {

		if (!scenario["campaign_id"].empty() &&
			(scenario["allow_new_game"].to_bool(true) || game_config::debug)) {

			const std::string& title = (!scenario["new_game_title"].empty()) ?
				scenario["new_game_title"] : scenario["name"];

			entry_points_.push_back(&scenario);
			entry_point_titles.push_back(title);
		}
	}

	if (entry_point_titles.size() > 1) {
		entry_points_combo_.set_items(entry_point_titles);
		entry_points_combo_.set_selected(0);

		show_entry_points_ = true;
	}

	options_manager_.set_era(parameters_.mp_era);
	options_manager_.set_scenario(parameters_.mp_scenario);
	options_manager_.set_modifications(parameters_.active_mods);
	options_manager_.init_widgets();

	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	name_entry_.set_text(vgettext("$login|’s game", i18n_symbols));

	gamelist_updated();
}

configure::~configure()
{
	// Only save the settings if the dialog was 'accepted'
	if(get_result() != CREATE) {
		DBG_MP << "destructing multiplayer configure dialog - aborted game creation" << std::endl;
		return;
	}
	DBG_MP << "destructing multiplayer configure dialog - a game will be configured" << std::endl;

	// Save values for next game
	DBG_MP << "storing parameter values in preferences" << std::endl;
	preferences::set_allow_observers(parameters_.allow_observers);
	preferences::set_shuffle_sides(parameters_.shuffle_sides);
	preferences::set_use_map_settings(parameters_.use_map_settings);
	preferences::set_countdown(parameters_.mp_countdown);
	preferences::set_countdown_init_time(parameters_.mp_countdown_init_time);
	preferences::set_countdown_turn_bonus(parameters_.mp_countdown_turn_bonus);
	preferences::set_countdown_reservoir_time(parameters_.mp_countdown_reservoir_time);
	preferences::set_countdown_action_bonus(parameters_.mp_countdown_action_bonus);
	preferences::set_options(parameters_.options);

	// When using map settings, the following variables are determined by the map,
	// so don't store them as the new preferences.
	if(!parameters_.use_map_settings) {
		preferences::set_fog(parameters_.fog_game);
		preferences::set_shroud(parameters_.shroud_game);
		preferences::set_turns(parameters_.num_turns);
		preferences::set_random_start_time(parameters_.random_start_time);
		preferences::set_village_gold(parameters_.village_gold);
		preferences::set_village_support(parameters_.village_support);
		preferences::set_xp_modifier(parameters_.xp_modifier);
	}
}

const mp_game_settings& configure::get_parameters()
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
	const int num_turns_val = turns_slider_.value() <
		turns_slider_.max_value() ? turns_slider_.value() : -1;

	// Updates the values in the "parameters_" member to match
	// the values selected by the user with the widgets:
	parameters_.name = name_entry_.text();

	// CHECK
	parameters_.mp_countdown_init_time = mp_countdown_init_time_val;
	parameters_.mp_countdown_turn_bonus = mp_countdown_turn_bonus_val;
	parameters_.mp_countdown_reservoir_time = mp_countdown_reservoir_time_val;
	parameters_.mp_countdown_action_bonus = mp_countdown_action_bonus_val;
	parameters_.mp_countdown = countdown_game_.checked();
	parameters_.num_turns = num_turns_val;
	parameters_.village_gold = village_gold_slider_.value();
	parameters_.village_support = village_support_slider_.value();
	parameters_.xp_modifier = xp_modifier_slider_.value();
	parameters_.use_map_settings = use_map_settings_.checked();
	parameters_.random_start_time = random_start_time_.checked();
	parameters_.fog_game = fog_game_.checked();
	parameters_.shroud_game = shroud_game_.checked();
	parameters_.allow_observers = observers_game_.checked();
	parameters_.shuffle_sides = shuffle_sides_.checked();
	parameters_.share_view = vision_combo_.selected() == 0;
	parameters_.share_maps = vision_combo_.selected() == 1;

	parameters_.options = options_manager_.get_values();

	return parameters_;
}

void configure::process_event()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	if(cancel_game_.pressed()) {
		set_result(QUIT);
		return;
	}

	if(launch_game_.pressed()) {
		// check if the map is valid
		if (name_entry_.text() == "") {
			gui2::show_transient_message(disp_.video(), "", _("You must enter a name."));
		} else {
			set_result(CREATE);
			return;
		}
	}

	if(password_button_.pressed()) {
		gui2::tmp_create_game_set_password::execute(
				  parameters_.password
				, disp_.video());
	}

	if (entry_points_combo_.changed()) {
		const config& scenario = *entry_points_[entry_points_combo_.selected()];

		parameters_.hash = scenario.hash();
		parameters_.scenario_data = scenario;
		parameters_.mp_scenario = scenario["id"].str();
		parameters_.mp_scenario_name = scenario["name"].str();

		force_use_map_settings_check_ = true;
	}

	// Turns per game
	const int cur_turns = turns_slider_.value();

	std::stringstream buf;
	if(cur_turns < 100) {
		buf << _("Turns: ") << cur_turns;
	} else {
		buf << _("Unlimited turns");
	}
	turns_label_.set_text(buf.str());

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


	// Villages can produce between 1 and 5 gold a turn
	const int village_gold = village_gold_slider_.value();
	buf.str("");
	buf << _("Village gold: ") << village_gold;
	village_gold_label_.set_text(buf.str());

	// Unit levels supported per village
	const int village_support = village_support_slider_.value();
	buf.str("");
	buf << _("Village support: ") << village_support;
	village_support_label_.set_text(buf.str());

	// Experience modifier
	const int xpmod = xp_modifier_slider_.value();
	buf.str("");
	buf << _("Experience modifier: ") << xpmod << "%";

	xp_modifier_label_.set_text(buf.str());

	if(use_map_settings_.pressed() || force_use_map_settings_check_) {
		force_use_map_settings_check_ = false;

		const bool map_settings = use_map_settings_.checked();

		// If the map settings are wanted use them,
		// if not properly defined fall back to the default settings
		turns_slider_.set_value(map_settings ?
			settings::get_turns(parameters_.scenario_data["turns"]) :
			preferences::turns());

		xp_modifier_slider_.set_value(map_settings ?
			settings::get_xp_modifier(parameters_.scenario_data["experience_modifier"]) :
			preferences::xp_modifier());

		random_start_time_.set_check(map_settings ?
			parameters_.scenario_data["random_start_time"].to_bool(true) :
			preferences::random_start_time());

		// These are per player, always show values of player 1.
		/**
		 * @todo This might not be 100% correct, but at the moment
		 * it is not possible to show the fog and shroud per player.
		 * This might change in the future.
		 * NOTE when 'load game' is selected there are no sides.
		 */
		config::const_child_itors sides = parameters_.scenario_data.child_range("side");
		if (sides.first != sides.second)
		{
			const config &cfg = *sides.first;

			village_gold_slider_.set_value(map_settings ?
				settings::get_village_gold(cfg["village_gold"]) :
				preferences::village_gold());

			village_support_slider_.set_value(map_settings ?
				settings::get_village_support(cfg["village_support"]) :
				preferences::village_support());

			fog_game_.set_check(map_settings ?
				cfg["fog"].to_bool(true) :
				preferences::fog());

			shroud_game_.set_check(map_settings ?
				cfg["shroud"].to_bool(false) :
				preferences::shroud());
		}

		// Set the widget states
		turns_slider_.enable(!map_settings);
		village_gold_slider_.enable(!map_settings);
		village_support_slider_.enable(!map_settings);
		xp_modifier_slider_.enable(!map_settings);
		random_start_time_.enable(!map_settings);
		fog_game_.enable(!map_settings);
		shroud_game_.enable(!map_settings);
	}

	options_manager_.process_event();
}

void configure::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) << " children widgets" << std::endl;

	ui::hide_children(hide);

	turns_slider_.hide(hide);
	turns_label_.hide(hide);

	countdown_init_time_slider_.hide(hide);
	countdown_init_time_label_.hide(hide);
	countdown_turn_bonus_slider_.hide(hide);
	countdown_turn_bonus_label_.hide(hide);
	countdown_reservoir_time_slider_.hide(hide);
	countdown_reservoir_time_label_.hide(hide);
	countdown_action_bonus_slider_.hide(hide);
	countdown_action_bonus_label_.hide(hide);
	countdown_game_.hide(hide);

	village_gold_slider_.hide(hide);
	village_gold_label_.hide(hide);
	village_support_slider_.hide(hide);
	village_support_label_.hide(hide);
	xp_modifier_slider_.hide(hide);
	xp_modifier_label_.hide(hide);

	generic_label_.hide(hide);
	name_entry_label_.hide(hide);
	num_players_label_.hide(hide);
	map_size_label_.hide(hide);

	use_map_settings_.hide(hide);
	random_start_time_.hide(hide);
	fog_game_.hide(hide);
	shroud_game_.hide(hide);
	observers_game_.hide(hide);
	shuffle_sides_.hide(hide);
	cancel_game_.hide(hide);
	launch_game_.hide(hide);

	password_button_.hide(hide);
	vision_combo_.hide(hide);
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
	const int first_column_width = (ca.w - ca.x) / 2 - column_border_size;

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
	options_pane_left_.set_width(first_column_width);
	options_pane_left_.set_height(right_pane_height - entry_points_label_.height());

	int slider_width = options_pane_left_.width() - 40;

#ifdef MP_VISION_OPTIONAL
	vision_combo_.set_location(xpos, ypos);
	ypos += vision_combo_.height() + border_size;
#endif

	int xpos_left = 0;
	int ypos_left = 0;

	ypos_left += 2 * border_size;
	options_pane_left_.add_widget(&observers_game_, xpos_left, ypos_left);
	options_pane_left_.add_widget(&shuffle_sides_,
		xpos_left + (options_pane_left_.width() - xpos_left) / 2 + border_size, ypos_left);
	ypos_left += shuffle_sides_.height() + border_size;

	options_pane_left_.add_widget(&countdown_game_, xpos_left, ypos_left);

	if(!local_players_only_) {
		options_pane_left_.add_widget(&password_button_,
			(ca.x + first_column_width / 2) - 40, ypos_left);
	} else {
		password_button_.hide(true);
	}

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

	if (show_entry_points_) {
		int x = ca.x;
		int y = ca.y + ca.h - entry_points_combo_.height();
		entry_points_combo_.set_location(x, y);
		y -= entry_points_label_.height() + border_size;
		entry_points_label_.set_location(x, y);
	}

	// Second column: gameplay settings
	xpos += first_column_width + column_border_size;
	ypos = ypos_columntop;

	options_pane_right_.set_location(xpos, ypos);
	options_pane_right_.set_width(ca.w - (xpos - ca.x));
	options_pane_right_.set_height(right_pane_height);

	slider_width = options_pane_right_.width() - 40;

	int xpos_right = 0;
	int ypos_right = 0;
	options_pane_right_.add_widget(&generic_label_, xpos_right, ypos_right);
	ypos_right += generic_label_.height() + border_size;

	options_pane_right_.add_widget(&use_map_settings_, xpos_right, ypos_right);
	options_pane_right_.add_widget(&fog_game_,
		xpos_right + (options_pane_right_.width() - xpos_right)/2 + 5, ypos_right);
	ypos_right += use_map_settings_.height() + border_size;

	options_pane_right_.add_widget(&random_start_time_, xpos_right, ypos_right);
	options_pane_right_.add_widget(&shroud_game_,
		xpos_right + (options_pane_right_.width() - xpos_right)/2 + 5, ypos_right);
	ypos_right += random_start_time_.height() + border_size;

	options_pane_right_.add_widget(&turns_label_, xpos_right, ypos_right);
	ypos_right += turns_label_.height() + border_size;
	turns_slider_.set_width(slider_width);
	options_pane_right_.add_widget(&turns_slider_, xpos_right, ypos_right);
	ypos_right += turns_slider_.height() + border_size;

	options_pane_right_.add_widget(&xp_modifier_label_, xpos_right, ypos_right);
	ypos_right += xp_modifier_label_.height() + border_size;
	xp_modifier_slider_.set_width(slider_width);
	options_pane_right_.add_widget(&xp_modifier_slider_, xpos_right, ypos_right);
	ypos_right += xp_modifier_slider_.height() + border_size;

	options_pane_right_.add_widget(&village_support_label_, xpos_right, ypos_right);
	ypos_right += village_support_label_.height() + border_size;
	village_support_slider_.set_width(slider_width);
	options_pane_right_.add_widget(&village_support_slider_, xpos_right, ypos_right);
	ypos_right += village_support_slider_.height() + border_size;

	options_pane_right_.add_widget(&village_gold_label_, xpos_right, ypos_right);
	ypos_right += village_gold_label_.height() + border_size;
	village_gold_slider_.set_width(slider_width);

	options_pane_right_.add_widget(&village_gold_slider_, xpos_right, ypos_right);
	ypos_right += village_gold_slider_.height() + 3 * border_size;

	options_manager_.layout_widgets(xpos_right, ypos_right);

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


