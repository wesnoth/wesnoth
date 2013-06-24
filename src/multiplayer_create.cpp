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
#include "gui/dialogs/campaign_selection.hpp"
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

mp_level::mp_level() :
	map_data(),
	image_label(),
	campaign(),
	type(SCENARIO)
{
}

void mp_level::reset()
{
	map_data = "";
	image_label = "";
	campaign.clear();
}

void mp_level::set_scenario()
{
	reset();

	type = SCENARIO;
}

void mp_level::set_campaign()
{
	reset();

	type = CAMPAIGN;
}

mp_level::TYPE mp_level::get_type() const
{
	return type;
}

create::create(game_display& disp, const config &cfg, chat& c, config& gamelist, bool local_players_only) :
	ui(disp, _("Create Game"), cfg, c, gamelist),

	local_players_only_(local_players_only),
	tooltip_manager_(disp.video()),
	era_selection_(-1),
	level_selection_(-1),
	mod_selection_(-1),
	user_maps_(),
	era_options_(),
	level_options_(),
	mod_options_(),
	era_descriptions_(),
	level_descriptions_(),
	mod_descriptions_(),
	level_index_(),
	eras_menu_(disp.video(), std::vector<std::string>()),
	levels_menu_(disp.video(), std::vector<std::string>()),
	mods_menu_(disp.video(), std::vector<std::string>()),
	filter_name_label_(disp.video(), _("Filter:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	filter_num_players_label_(disp.video(), _("Number of players: any"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_generator_label_(disp.video(), _("Random map options:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	era_label_(disp.video(), _("Era:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	level_label_(disp.video(), _("Map to play:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	mod_label_(disp.video(), _("Modifications:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_size_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	launch_game_(disp.video(), _("OK")),
	cancel_game_(disp.video(), _("Cancel")),
	regenerate_map_(disp.video(), _("Regenerate")),
	generator_settings_(disp.video(), _("Settings...")),
	switch_levels_menu_(disp.video(), _("Switch to campaigns")),
	filter_num_players_slider_(disp.video()),
	description_(disp.video(), 100, "", false),
	filter_name_(disp.video(), 100, "", true),
	image_restorer_(NULL),
	image_rect_(null_rect),
	map_(),
	generator_(NULL),
	parameters_(),
	mp_level_(),
	state_(),
	dependency_manager_(cfg, disp.video())
{
	filter_num_players_slider_.set_min(0);
	filter_num_players_slider_.set_max(9);
	filter_num_players_slider_.set_increment(1);

	// Build the list of scenarios to play

	DBG_MP << "constructing multiplayer create dialog" << std::endl;

	set_levels_menu(true);
	levels_menu_.set_numeric_keypress_selection(false);

	// The possible eras to play
	BOOST_FOREACH(const config &er, cfg.child_range("era")) {
		era_options_.push_back(er["name"]);
		era_descriptions_.push_back(er["description"]);
	}
	if(era_options_.empty()) {
		gui2::show_transient_message(disp.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}
	eras_menu_.set_items(era_options_);

	if (size_t(preferences::era()) < era_options_.size()) {
		eras_menu_.move_selection(preferences::era());
	} else {
		eras_menu_.move_selection(0);
	}

	dependency_manager_.try_era_by_index(era_selection_, true);

	// Available modifications
	BOOST_FOREACH (const config& mod, cfg.child_range("modification")) {
		mod_options_.push_back(mod["name"]);
		mod_descriptions_.push_back(mod["description"]);
	}
	mods_menu_.set_items(mod_options_);

	BOOST_FOREACH (const std::string& str, preferences::modifications()) {
		if (cfg.find_child("modification", "id", str))
			parameters_.active_mods.push_back(str);
	}

	dependency_manager_.try_modifications(parameters_.active_mods, true);


	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();

	gamelist_updated();

	get_level_image();
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

	// Updates the values in the "parameters_" member to match
	// the values selected by the user with the widgets:

	config::const_child_itors era_list = game_config().child_range("era");
	for (int num = eras_menu_.selection(); num > 0; --num) {
		if (era_list.first == era_list.second) {
			throw config::error(_("Invalid era selected"));
		}
		++era_list.first;
	}

	parameters_.mp_era = (*era_list.first)["id"].str();

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
		switch (mp_level_.get_type()) {
		case mp_level::SCENARIO: {
			if (map_.get() == NULL) {
				gui2::show_transient_message(disp_.video(), "",
					_("The map is invalid."));
			} else {
				set_result(CREATE);
				return;
			}

			break;
		}
		case mp_level::CAMPAIGN: {
			if (new_campaign()) {
				resources::config_manager->
					load_game_config_for_game(state_.classification());

				const config& level = game_config().find_child("scenario", "id",
					mp_level_.campaign["first_scenario"]);
				parameters_.scenario_data = level;

				set_result(CREATE);
				return;
			}

			break;
		}
		} // end switch
	}

	if (switch_levels_menu_.pressed()) {
		switch (mp_level_.get_type()) {
		case mp_level::SCENARIO: {
			mp_level_.set_campaign();

			switch_levels_menu_.set_label(_("Switch to maps"));

			break;
		}
		case mp_level::CAMPAIGN: {
			mp_level_.set_scenario();

			switch_levels_menu_.set_label(_("Switch to campaigns"));

			break;
		}
		} // end switch

		set_levels_menu();
	}

	bool era_changed = era_selection_ != eras_menu_.selection();
	era_selection_ = eras_menu_.selection();

	if (era_changed) {
		description_.set_text(era_descriptions_[era_selection_]);
		dependency_manager_.try_era_by_index(era_selection_);
		synchronize_selections();
	}

	bool level_changed = level_selection_ != levels_menu_.selection();
	level_selection_ = levels_menu_.selection();

	if (level_changed) {
		description_.set_text(level_descriptions_[level_selection_]);
		dependency_manager_.try_scenario_by_index(level_selection_);
		synchronize_selections();
	}

	if(level_changed) {
		generator_.assign(NULL);

		tooltips::clear_tooltips(image_rect_);

		const size_t select = size_t(levels_menu_.selection());

		switch (mp_level_.get_type()) {
		case mp_level::SCENARIO: {
			if (select > 0 && select <= user_maps_.size()) {
				set_level_data(GENERIC_MULTIPLAYER, select);
			} else if(select > user_maps_.size() &&
				select <= levels_menu_.number_of_items() - 1) {
				if (set_level_data(MULTIPLAYER, select)) {
					// If the map should be randomly generated.
					if (!parameters_.scenario_data["map_generation"].empty()) {
						generator_.assign(create_map_generator(
							parameters_.scenario_data["map_generation"],
							parameters_.scenario_data.child("generator")));
					}

					if (!parameters_.scenario_data["description"].empty()) {
						tooltips::add_tooltip(image_rect_,
							parameters_.scenario_data["description"], "", false);
					}
				}
			} else {
				set_level_data(SAVED_GAME, select);

				if (image_restorer_ != NULL) {
					image_restorer_->restore();
				}
			}
			break;
		}
		case mp_level::CAMPAIGN: {
			set_level_data(CAMPAIGN, select);

			if (!mp_level_.campaign["description"].empty()) {
				tooltips::add_tooltip(image_rect_,
					mp_level_.campaign["description"], "", false);
			}

			break;
		}
		} // end switch
	}

	if(generator_ != NULL && generator_->allow_user_config() &&
		generator_settings_.pressed()) {
		generator_->user_config(disp_);
		level_changed = true;
	}

	if(generator_ != NULL && (level_changed || regenerate_map_.pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);
		cursor::setter cur(cursor::WAIT);

		set_level_data(GENERATED_MAP, 0);

		if (!parameters_.scenario_data["error_message"].empty())
			gui2::show_message(disp().video(), "map generation error",
				parameters_.scenario_data["error_message"]);

		level_changed = true;
	}

	if(level_changed) {
		parameters_.hash = parameters_.scenario_data.hash();

		bool enable_launch_game = true;

		std::stringstream players;
		std::stringstream map_size;

		get_level_image();
		draw_level_image();

		switch (mp_level_.get_type()) {
		case mp_level::SCENARIO: {
			// If there are less sides in the configuration than there are
			// starting positions, then generate the additional sides
			const int map_positions = map_.get() != NULL ?
				map_->num_valid_starting_positions() : 0;
			set_level_sides(map_positions);

			if(map_.get() != NULL) {
				int nsides = 0;
				BOOST_FOREACH(const config &k,
					parameters_.scenario_data.child_range("side")) {
					if (k["allow_player"].to_bool(true)) ++nsides;
				}

				players << _("Players: ") << nsides;
				map_size << _("Size: ") << map_.get()->w() <<
					utils::unicode_multiplication_sign << map_.get()->h();
			} else {
				players << _("Error");
				map_size << "";
			}

			enable_launch_game = (map_.get() != NULL);

			break;
		}
		case mp_level::CAMPAIGN: {
			//TODO: add a way to determine
			// the information about number of players for mp campaigns.

			break;
		}
		} // end switch

		map_size_label_.set_text(map_size.str());
		num_players_label_.set_text(players.str());

		launch_game_.enable(enable_launch_game);
		generator_settings_.enable(generator_ != NULL);
		regenerate_map_.enable(generator_ != NULL);
	}

	bool mod_selection_changed = mod_selection_ != mods_menu_.selection();
	mod_selection_ = mods_menu_.selection();

	if (mod_selection_changed) {
		description_.set_text(mod_descriptions_[mod_selection_]);
	}
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
	ypos += generator_settings_.height() + 2 * border_size;

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
	levels_menu_.set_items(level_options_);
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
	eras_menu_.set_items(era_options_);
	eras_menu_.move_selection(erasel_save);
	ypos += eras_menu_height;
	mod_label_.set_location(xpos, ypos);
	ypos += mod_label_.height() + border_size;
	mods_menu_.set_max_width(menu_width);
	mods_menu_.set_max_height(ca.h - eras_menu_height);
	mods_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int modsel_save = mods_menu_.selection();
	mods_menu_.set_items(mod_options_);
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

void create::get_level_image()
{
	switch (mp_level_.get_type()) {
	case mp_level::SCENARIO: {
		const std::string& map_data = parameters_.scenario_data["map_data"];

		if ((mp_level_.map_data != map_data) || (mp_level_.map_data == "")) {
			try {
				map_.reset(new gamemap(game_config(), map_data));
			} catch(incorrect_map_format_error& e) {
				ERR_CF << "map could not be loaded: " << e.message << '\n';

				tooltips::clear_tooltips(image_rect_);
				tooltips::add_tooltip(image_rect_,e.message);
			} catch(twml_exception& e) {
				ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			}

			mp_level_.map_data = map_data;
		}
		break;
	}
	case mp_level::CAMPAIGN: {
		mp_level_.image_label = mp_level_.campaign["image"].str();

		break;
	}
	} // end switch
}

void create::draw_level_image()
{
	boost::scoped_ptr<surface> image;

	switch (mp_level_.get_type()) {
	case mp_level::SCENARIO: {

		if (map_.get() != NULL) {
			image.reset(new surface(image::getMinimap(image_rect_.w,
				image_rect_.h, *map_, 0)));
		}
		break;
	}
	case mp_level::CAMPAIGN: {
		surface campaign_image(
			image::get_image(image::locator(mp_level_.image_label)));

		image.reset(new surface(scale_surface(campaign_image, image_rect_.w,
			image_rect_.h)));

		break;
	}
	} // end switch

	SDL_Color back_color = {0,0,0,255};
	draw_centered_on_background(*image, image_rect_, back_color, video().getSurface());
}

bool create::set_level_data(SET_LEVEL set_level, const int select)
{
	switch (set_level) {
	case GENERIC_MULTIPLAYER: {
		parameters_.saved_game = false;
		if (const config &generic_multiplayer =
			game_config().child("generic_multiplayer")) {
			parameters_.scenario_data = generic_multiplayer;
			parameters_.scenario_data["map_data"] =
				read_map(user_maps_[select-1]);
		}

	break;
	}
	case MULTIPLAYER: {
		parameters_.saved_game = false;
		size_t index = select - user_maps_.size() - 1;
		assert(index < level_index_.size());
		index = level_index_[index];

		config::const_child_itors levels =
			game_config().child_range("multiplayer");
		for (; index > 0; --index) {
			if (levels.first == levels.second) break;
			++levels.first;
		}

		if (levels.first != levels.second)
		{
			const config &level = *levels.first;
			parameters_.scenario_data = level;
			std::string map_data = level["map_data"];

			if (map_data.empty() && !level["map"].empty()) {
				map_data = read_map(level["map"]);
			}
		} else {
			return false;
		}
		break;
	}
	case SAVED_GAME: {
		parameters_.scenario_data.clear();
		parameters_.saved_game = true;

		break;
	}
	case GENERATED_MAP: {
		parameters_.scenario_data =
			generator_->create_scenario(std::vector<std::string>());

		// Set the scenario to have placing of sides
		// based on the terrain they prefer
		parameters_.scenario_data["modify_placing"] = "true";

		break;
	}
	case CAMPAIGN: {
		parameters_.saved_game = false;

		size_t index = select;
		assert(index < level_index_.size());
		index = level_index_[index];

		config::const_child_itors levels =
			game_config().child_range("campaign");
		for (; index > 0; --index) {
			if (levels.first == levels.second) break;
			++levels.first;
		}

		if (levels.first != levels.second)
		{
			const config &level = *levels.first;
			mp_level_.campaign = level;
		}

		break;
	}
	} // end switch

	return true;
}

void create::set_level_sides(const int map_positions)
{
	for (int pos = parameters_.scenario_data.child_count("side");
		pos < map_positions; ++pos) {
		config& side = parameters_.scenario_data.add_child("side");
		side["side"] = pos + 1;
		side["team_name"] = pos + 1;
		side["canrecruit"] = true;
		side["controller"] = "human";
	}
}

bool create::new_campaign()
{
	state_ = game_state();
	state_.classification().campaign_type = "multiplayer";

	const std::string difficulty_descriptions =
		mp_level_.campaign["difficulty_descriptions"];
	std::vector<std::string> difficulty_options =
		utils::split(difficulty_descriptions, ';');
	const std::vector<std::string> difficulties =
		utils::split(mp_level_.campaign["difficulties"]);

	if(difficulties.empty() == false) {
		int difficulty = 0;
		if(difficulty_options.size() != difficulties.size()) {
			difficulty_options.resize(difficulties.size());
			std::copy(difficulties.begin(),difficulties.end(),difficulty_options.begin());
		}

		gui2::tcampaign_difficulty dlg(difficulty_options);
		dlg.show(disp().video());

		if(dlg.selected_index() == -1) {
			return false;
		}
		difficulty = dlg.selected_index();

		state_.classification().difficulty = difficulties[difficulty];
	}

	state_.classification().campaign_define =
		mp_level_.campaign["define"].str();
	state_.classification().campaign_xtra_defines =
		utils::split(mp_level_.campaign["extra_defines"]);

	return true;
}

void create::synchronize_selections()
{
	DBG_MP << "Synchronizing with the dependency manager" << std::endl;
	if (era_selection_ != dependency_manager_.get_era_index()) {
		eras_menu_.move_selection(dependency_manager_.get_era_index());
		process_event();
	}

	if (level_selection_ != dependency_manager_.get_scenario_index()) {
		levels_menu_.move_selection(dependency_manager_.get_scenario_index());
		process_event();
	}

	parameters_.active_mods = dependency_manager_.get_modifications();
}

void create::set_levels_menu(const bool init_dep_check)
{
	level_options_.clear();
	level_descriptions_.clear();
	level_index_.clear();
	user_maps_.clear();

	std::string markup_txt = "`~";
	std::string help_sep = " ";
	help_sep[0] = HELP_STRING_SEPARATOR;
	std::string menu_help_str;

	switch (mp_level_.get_type()) {
	case mp_level::SCENARIO: {
		// Add the 'load game' option
		menu_help_str = help_sep + _("Load Game");
		level_options_.push_back(markup_txt + _("Load Game...") + menu_help_str);
		level_descriptions_.push_back(_("Continue a saved game"));

		if (init_dep_check) {
			// Treat the Load game option as a scenario
			config load_game_info;
			load_game_info["id"] = "multiplayer_load_game";
			load_game_info["name"] = "Load Game";
			dependency_manager_.insert_element(depcheck::SCENARIO, load_game_info, 0);
		}

		// User maps
		get_files_in_dir(get_user_data_dir() + "/editor/maps",&user_maps_,NULL,FILE_NAME_ONLY);
		size_t i = 0;
		for(i = 0; i < user_maps_.size(); i++)
		{
			menu_help_str = help_sep + user_maps_[i];
			level_options_.push_back(user_maps_[i] + menu_help_str);
			level_descriptions_.push_back(_("User made map"));

			if (init_dep_check) {
				// Since user maps are treated as scenarios,
				// some dependency info is required
				config depinfo;
				depinfo["id"] = user_maps_[i];
				depinfo["name"] = user_maps_[i];
				dependency_manager_.insert_element(depcheck::SCENARIO, depinfo, i+1);
			}
		}

		// Standard maps
		i = 0;
		BOOST_FOREACH(const config &j, game_config().child_range("multiplayer"))
		{
			if (j["allow_new_game"].to_bool(true))
			{
				std::string name = j["name"];
				menu_help_str = help_sep + name;
				level_options_.push_back(name + menu_help_str);
				level_descriptions_.push_back(j["description"]);
				level_index_.push_back(i);
			}
			++i;
		}
		break;
	}
	case mp_level::CAMPAIGN: {
		// Campaigns
		size_t i = 0;
		BOOST_FOREACH(const config &j, game_config().child_range("campaign"))
		{
			std::string name = j["name"];
			menu_help_str = help_sep + name;
			level_options_.push_back(name + menu_help_str);
			level_descriptions_.push_back(j["description"]);
			level_index_.push_back(i);
			++i;
		}
		break;
	}
	} // end switch

	// Create the scenarios menu
	levels_menu_.set_items(level_options_);
	if (size_t(preferences::map()) < level_options_.size()) {
		levels_menu_.move_selection(preferences::map());
		dependency_manager_.try_scenario_by_index(preferences::map(), true);
	}
}

} // namespace mp


