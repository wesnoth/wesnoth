/*
   Copyright (C) 2007 - 2013
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Create a multiplayer-game: select map, players, options etc.
 */

#include "global.hpp"

#include "gettext.hpp"
#include "game_config_manager.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "construct_dialog.hpp"
#include "settings.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/mp_create_game_choose_mods.hpp"
#include "gui/dialogs/mp_create_game_set_password.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "minimap.hpp"
#include "multiplayer_create.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_create("mp/create");
#define DBG_MP LOG_STREAM(debug, log_mp_create)

namespace {
const SDL_Rect null_rect = {0, 0, 0, 0};
}

namespace mp {

create::create(game_display& disp, const config &cfg, chat& c, config& gamelist, bool local_players_only) :
	ui(disp, _("Create Game"), cfg, c, gamelist),

	local_players_only_(local_players_only),
	tooltip_manager_(disp.video()),
	era_selection_(-1),
	mod_selection_(-1),
	level_selection_(-1),
	eras_menu_(disp.video(), std::vector<std::string>()),
	levels_menu_(disp.video(), std::vector<std::string>()),
	mods_menu_(disp.video(), std::vector<std::string>()),
	filter_name_label_(disp.video(), _("Filter:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	filter_num_players_label_(disp.video(), _("Number of players: any"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_generator_label_(disp.video(), _("Random map options:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	era_label_(disp.video(), _("Era:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	level_label_(disp.video(), _("Maps to play:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	mod_label_(disp.video(), _("Modifications:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_size_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	launch_game_(disp.video(), _("OK")),
	cancel_game_(disp.video(), _("Cancel")),
	regenerate_map_(disp.video(), _("Regenerate")),
	generator_settings_(disp.video(), _("Settings...")),
	load_game_(disp.video(), _("Load game...")),
	switch_levels_menu_(disp.video(), _("Switch to campaigns")),
	filter_num_players_slider_(disp.video()),
	description_(disp.video(), 100, "", false),
	filter_name_(disp.video(), 100, "", true),
	image_restorer_(NULL),
	image_rect_(null_rect),
	parameters_(),
	dependency_manager_(cfg, disp.video()),
	engine_(level::USER_MAP, parameters_, dependency_manager_)
{
	filter_num_players_slider_.set_min(0);
	filter_num_players_slider_.set_max(9);
	filter_num_players_slider_.set_increment(1);

	DBG_MP << "constructing multiplayer create dialog" << std::endl;

	const std::vector<std::string>& level_names = levels_menu_item_names();

	levels_menu_.set_items(level_names);
	levels_menu_.set_numeric_keypress_selection(false);

	if (size_t(preferences::map()) < level_names.size()) {
		levels_menu_.move_selection(preferences::map());
		dependency_manager_.try_scenario_by_index(preferences::map(), true);
	}

	sync_current_level_with_engine();

	const std::vector<std::string>& era_names = engine_.eras_menu_item_names();
	if(era_names.empty()) {
		gui2::show_transient_message(disp.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}
	eras_menu_.set_items(era_names);

	if (size_t(preferences::era()) < era_names.size()) {
		eras_menu_.move_selection(preferences::era());
	} else {
		eras_menu_.move_selection(0);
	}

	dependency_manager_.try_era_by_index(era_selection_, true);

	mods_menu_.set_items(engine_.mods_menu_item_names());

	BOOST_FOREACH (const std::string& str, preferences::modifications()) {
		if (cfg.find_child("modification", "id", str))
			parameters_.active_mods.push_back(str);
	}

	dependency_manager_.try_modifications(parameters_.active_mods, true);


	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();

	gamelist_updated();
}

create::~create()
{
	// Only save the settings if the dialog was 'accepted'
	if(get_result() != CREATE) {
		DBG_MP << "destructing multiplayer create dialog - aborted game creation" << std::endl;
		return;
	}
	DBG_MP << "destructing multiplayer create dialog - a game will be created" << std::endl;

	// Retrieve values
	get_parameters();

	// Save values for next game
	DBG_MP << "storing parameter values in preferences" << std::endl;
}

mp_game_settings& create::get_parameters()
{
	DBG_MP << "getting parameter values from widgets" << std::endl;

	parameters_.mp_era = engine_.current_era_id();

	return parameters_;
}

void create::process_event()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	if (cancel_game_.pressed()) {
		set_result(QUIT);
		return;
	}

	if (launch_game_.pressed() || levels_menu_.double_clicked()) {
		if (engine_.current_level().can_launch_game()) {
			if (engine_.current_level_type() == level::CAMPAIGN) {
				std::string difficulty = select_campaign_difficulty();
				if (difficulty == "CANCEL") {
					return;
				}

				engine_.prepare_for_campaign(difficulty);
			}

			engine_.prepare_for_new_level();

			set_result(CREATE);
			return;
		} else {
			gui2::show_transient_message(disp_.video(), "",
				_("The level is invalid."));
		}
	}

	if (switch_levels_menu_.pressed()) {
		if (engine_.current_level_type() == level::CAMPAIGN) {
			switch_levels_menu_.set_label(_("Switch to campaigns"));
			level_label_.set_text(_("Maps to play:"));

			engine_.set_current_level_type(level::USER_MAP);
			sync_current_level_with_engine();
		} else {
			switch_levels_menu_.set_label(_("Switch to maps"));
			level_label_.set_text(_("Campaigns to play:"));

			engine_.set_current_level_type(level::CAMPAIGN);
			sync_current_level_with_engine();
		}

		levels_menu_.set_items(levels_menu_item_names());
		level_selection_ = -1;
	}

	if (load_game_.pressed()) {
		engine_.prepare_for_saved_game();

		set_result(LOAD_GAME);

		return;
	}

	bool era_changed = era_selection_ != eras_menu_.selection();
	era_selection_ = eras_menu_.selection();

	if (era_changed) {
		engine_.set_current_era_index(era_selection_);

		description_.set_text(engine_.current_era_description());
		dependency_manager_.try_era_by_index(era_selection_);
		synchronize_selections();
	}

	bool level_changed = level_selection_ != levels_menu_.selection();
	level_selection_ = levels_menu_.selection();

	if (level_changed) {
		sync_current_level_with_engine();

		description_.set_text(engine_.current_level().description());
		dependency_manager_.try_scenario_by_index(levels_menu_.selection());
		synchronize_selections();
	}

	if(level_changed) {
		tooltips::clear_tooltips(image_rect_);

		engine_.init_current_level_data();

		if (!engine_.current_level().description().empty()) {
			tooltips::add_tooltip(image_rect_,
				engine_.current_level().description(), "", false);
		}
	}

	if (engine_.generator_assigned() && generator_settings_.pressed()) {
		engine_.generator_user_config(disp_);

		level_changed = true;
	}

	if(engine_.generator_assigned() &&
		(level_changed || regenerate_map_.pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);
		cursor::setter cur(cursor::WAIT);

		engine_.init_generated_level_data();

		if (!parameters_.scenario_data["error_message"].empty())
			gui2::show_message(disp().video(), "map generation error",
				parameters_.scenario_data["error_message"]);

		level_changed = true;
	}

	if(level_changed) {
		parameters_.hash = parameters_.scenario_data.hash();

		std::stringstream players;
		std::stringstream map_size;

		engine_.current_level().set_metadata();

		draw_level_image();

		switch (engine_.current_level_type()) {
		case level::SCENARIO:
		case level::USER_MAP: {
			scenario* current_scenario =
				dynamic_cast<scenario*>(&engine_.current_level());

			players << _("Players: ") << current_scenario->num_players();
			map_size << _("Size: ") << current_scenario->map_size();

			break;
		}
		case level::CAMPAIGN: {
			//TODO: add a way to determine
			// the information about number of players for mp campaigns.

			break;
		}
		} // end switch

		map_size_label_.set_text(map_size.str());
		num_players_label_.set_text(players.str());

		launch_game_.enable(engine_.current_level().can_launch_game());
		generator_settings_.enable(engine_.generator_assigned());
		regenerate_map_.enable(engine_.generator_assigned());
	}

	bool mod_selection_changed = mod_selection_ != mods_menu_.selection();
	mod_selection_ = mods_menu_.selection();

	if (mod_selection_changed) {
		engine_.set_current_mod_index(mod_selection_);

		description_.set_text(engine_.current_mod_description());
	}
}

void create::sync_current_level_with_engine()
{
	if (engine_.current_level_type() == level::CAMPAIGN) {
		engine_.set_current_level_index(levels_menu_.selection());
	} else if ((size_t)levels_menu_.selection() < engine_.user_maps_count()) {
			engine_.set_current_level_type(level::USER_MAP);
			engine_.set_current_level_index(levels_menu_.selection());
	} else {
		engine_.set_current_level_type(level::SCENARIO);
		engine_.set_current_level_index(levels_menu_.selection()
			- engine_.user_maps_count());
	}
}

void create::synchronize_selections()
{
	DBG_MP << "Synchronizing with the dependency manager" << std::endl;
	if (era_selection_ != dependency_manager_.get_era_index()) {
		eras_menu_.move_selection(dependency_manager_.get_era_index());
		process_event();
	}

	if (level_selection_ !=	dependency_manager_.get_scenario_index()) {
		levels_menu_.move_selection(dependency_manager_.get_scenario_index());
		process_event();
	}

	parameters_.active_mods = dependency_manager_.get_modifications();
}

std::vector<std::string> create::levels_menu_item_names() const
{
	switch (engine_.current_level_type()) {
	case level::SCENARIO:
	case level::USER_MAP: {
		const std::vector<std::string>& scenarios =
			engine_.levels_menu_item_names(level::SCENARIO);
		const std::vector<std::string>& user_maps =
			engine_.levels_menu_item_names(level::USER_MAP);

		std::vector<std::string> names;
		names.reserve(scenarios.size() + user_maps.size());
		names.insert(names.end(), user_maps.begin(), user_maps.end());
		names.insert(names.end(), scenarios.begin(), scenarios.end());

		return names;
	}
	case level::CAMPAIGN:
	default: {
		return engine_.levels_menu_item_names(level::CAMPAIGN);
	}
	} // end switch
}

void create::draw_level_image() const
{
	boost::scoped_ptr<surface> image(
		engine_.current_level().create_image_surface(image_rect_));

	SDL_Color back_color = {0,0,0,255};
	draw_centered_on_background(*image, image_rect_, back_color,
		video().getSurface());
}

std::string create::select_campaign_difficulty()
{
	const std::string difficulty_descriptions =
		engine_.current_level().data()["difficulty_descriptions"];
	std::vector<std::string> difficulty_options =
		utils::split(difficulty_descriptions, ';');
	const std::vector<std::string> difficulties =
		utils::split(engine_.current_level().data()["difficulties"]);

	if(difficulties.empty() == false) {
		int difficulty = 0;
		if(difficulty_options.size() != difficulties.size()) {
			difficulty_options.resize(difficulties.size());
			std::copy(difficulties.begin(), difficulties.end(),
				difficulty_options.begin());
		}

		gui2::tcampaign_difficulty dlg(difficulty_options);
		dlg.show(disp().video());

		if(dlg.selected_index() == -1) {
			return "CANCEL";
		}
		difficulty = dlg.selected_index();

		return difficulties[difficulty];
	}

	return "";
}

void create::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) << " children widgets" << std::endl;

	ui::hide_children(hide);

	eras_menu_.hide(hide),
	levels_menu_.hide(hide);
	mods_menu_.hide(hide);

	filter_name_.hide(hide);
	filter_num_players_label_.hide(hide);
	map_generator_label_.hide(hide);
	map_size_label_.hide(hide);
	era_label_.hide(hide);
	level_label_.hide(hide);
	mod_label_.hide(hide);
	num_players_label_.hide(hide);

	cancel_game_.hide(hide);
	launch_game_.hide(hide);

	load_game_.hide(hide);
	switch_levels_menu_.hide(hide);

	regenerate_map_.hide(hide);
	generator_settings_.hide(hide);

	filter_num_players_slider_.hide(hide);

	description_.hide(hide);
	filter_name_.hide(hide);

	if (hide) {
		image_restorer_.assign(NULL);
	} else {
		image_restorer_.assign(new surface_restorer(&video(), image_rect_));

		engine_.current_level().set_metadata();
		draw_level_image();
	}
}

void create::layout_children(const SDL_Rect& rect)
{
	DBG_MP << "laying out the children" << std::endl;

	ui::layout_children(rect);

	const int border_size =  6;
	const int column_border_size = 10;

	SDL_Rect ca = client_area();
	int xpos = ca.x;
	int ypos = ca.y;

	// 222 is two times a button's minimal width plus one time border_size.
	// Instead of binding this value to the actual button widths, I chose this
	// because it makes no difference for most languages, and where it does, I
	// guess we'd prefer having the buttons less neatly aligned to having a
	// potentially giant image.
	const int image_width = ca.h < 500 ? 111 : 222;
	const int menu_width = (ca.w - 3*column_border_size - image_width)/3;
	const int eras_menu_height = (ca.h / 2 - era_label_.height() - 2*border_size - cancel_game_.height());

	// Dialog title
	ypos += title().height() + border_size;

	// Save ypos here (column top)
	int ypos_columntop = ypos;

	// First column: image & random map options
	image_rect_ = create_rect(xpos, ypos, image_width, image_width);
	ypos += image_width + border_size;

	num_players_label_.set_location(xpos, ypos);
	ypos += num_players_label_.height() + border_size;

	map_size_label_.set_location(xpos, ypos);
	ypos += map_size_label_.height() + 2 * border_size;

	const int ypos1 = ypos;
	const int xpos1 = xpos;
	// The description box is set later

	// Second column: filtering options
	ypos = ypos_columntop;
	xpos += image_width + column_border_size;
	filter_name_label_.set_location(xpos, ypos);
	filter_name_.set_location(xpos + filter_name_label_.width() + border_size, ypos);
	filter_name_.set_measurements(menu_width - border_size - filter_name_label_.width(), filter_name_label_.height());
	ypos += filter_name_.height() + border_size;
	filter_num_players_label_.set_location(xpos, ypos);
	ypos += filter_num_players_label_.height() + border_size;
	filter_num_players_slider_.set_location(xpos, ypos);
	filter_num_players_slider_.set_width(menu_width);
	ypos += filter_num_players_slider_.height() + border_size;
	map_generator_label_.set_location(xpos, ypos);
	ypos += map_generator_label_.height() + border_size;
	regenerate_map_.set_location(xpos, ypos);
	ypos += regenerate_map_.height() + border_size;
	generator_settings_.set_location(xpos, ypos);
	ypos += generator_settings_.height() + 4 * border_size;
	load_game_.set_location(xpos, ypos);

	// And now the description box
	description_.set_location(xpos1, std::max(ypos,ypos1));
	description_.set_measurements(image_width + border_size + menu_width, ca.h + ca.y - std::max(ypos,ypos1) - border_size);
	description_.set_wrap(true);
	ypos += description_.height() + border_size;

	//Third column: maps menu
	ypos = ypos_columntop;
	xpos += menu_width + column_border_size;
	const int x_offset = (menu_width - switch_levels_menu_.width()) / 2;
	switch_levels_menu_.set_location(xpos + x_offset, ypos);
	ypos += switch_levels_menu_.height() + 2 * border_size;
	level_label_.set_location(xpos, ypos);
	ypos += level_label_.height() + border_size;

	levels_menu_.set_max_width(menu_width);
	levels_menu_.set_max_height(ca.h + ca.y - ypos);
	levels_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int levelsel = levels_menu_.selection();
	levels_menu_.set_items(levels_menu_item_names());
	levels_menu_.move_selection(levelsel);

	//Fourth column: eras & mods menu
	ypos = ypos_columntop;
	xpos += menu_width + column_border_size;
	era_label_.set_location(xpos, ypos);
	ypos += era_label_.height() + border_size;
	eras_menu_.set_max_width(menu_width);
	eras_menu_.set_max_height(eras_menu_height);
	eras_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int erasel_save = eras_menu_.selection();
	eras_menu_.set_items(engine_.eras_menu_item_names());
	eras_menu_.move_selection(erasel_save);
	ypos += eras_menu_height;
	mod_label_.set_location(xpos, ypos);
	ypos += mod_label_.height() + border_size;
	mods_menu_.set_max_width(menu_width);
	mods_menu_.set_max_height(ca.h - eras_menu_height);
	mods_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int modsel_save = mods_menu_.selection();
	mods_menu_.set_items(engine_.mods_menu_item_names());
	mods_menu_.move_selection(modsel_save);

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


