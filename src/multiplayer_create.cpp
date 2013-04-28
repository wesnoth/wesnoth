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
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "construct_dialog.hpp"
#include "settings.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_create_game_choose_mods.hpp"
#include "gui/dialogs/mp_create_game_set_password.hpp"
#include "gui/dialogs/transient_message.hpp"
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
	ui(disp, _("Create Game"), cfg, c, gamelist, preferences::resolution().second < 768),

	local_players_only_(local_players_only),
	tooltip_manager_(disp.video()),
	era_selection_(-1),
	map_selection_(-1),
	user_maps_(),
	map_options_(),
	available_mods_(),
	map_index_(),
	maps_menu_(disp.video(), std::vector<std::string>()),
	map_size_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	era_label_(disp.video(), _("Era:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_label_(disp.video(), _("Map to play:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	choose_mods_(disp.video(), _("Modifications")),
	launch_game_(disp.video(), _("OK")),
	cancel_game_(disp.video(), _("Cancel")),
	regenerate_map_(disp.video(), _("Regenerate")),
	generator_settings_(disp.video(), _("Settings...")),
	era_combo_(disp, std::vector<std::string>()),
	minimap_restorer_(NULL),
	minimap_rect_(null_rect),
	generator_(NULL),
	parameters_(),
	dependency_manager_(cfg, disp.video())
{
	// Build the list of scenarios to play

	DBG_MP << "constructing multiplayer create dialog" << std::endl;

	// Add the 'load game' option
	std::string markup_txt = "`~";
	std::string help_sep = " ";
	help_sep[0] = HELP_STRING_SEPARATOR;
	std::string menu_help_str = help_sep + _("Load Game");
	map_options_.push_back(markup_txt + _("Load Game...") + menu_help_str);

	// Treat the Load game option as a scenario
	config load_game_info;
	load_game_info["id"] = "multiplayer_load_game";
	load_game_info["name"] = "Load Game";
	dependency_manager_.insert_element(depcheck::SCENARIO, load_game_info, 0);


	// User maps
	get_files_in_dir(get_user_data_dir() + "/editor/maps",&user_maps_,NULL,FILE_NAME_ONLY);

	size_t i = 0;
	for(i = 0; i < user_maps_.size(); i++)
	{
		menu_help_str = help_sep + user_maps_[i];
		map_options_.push_back(user_maps_[i] + menu_help_str);

		// Since user maps are treated as scenarios,
		// some dependency info is required
		config depinfo;

		depinfo["id"] = user_maps_[i];
		depinfo["name"] = user_maps_[i];

		dependency_manager_.insert_element(depcheck::SCENARIO, depinfo, i+1);
	}

	// Standard maps
	i = 0;
	BOOST_FOREACH(const config &j, cfg.child_range("multiplayer"))
	{
		if (j["allow_new_game"].to_bool(true))
		{
			std::string name = j["name"];
			menu_help_str = help_sep + name;
			map_options_.push_back(name + menu_help_str);
			map_index_.push_back(i);
		}
		++i;
	}

	// Create the scenarios menu
	maps_menu_.set_items(map_options_);
	if (size_t(preferences::map()) < map_options_.size()) {
		maps_menu_.move_selection(preferences::map());
		dependency_manager_.try_scenario_by_index(preferences::map(), true);
	}
	maps_menu_.set_numeric_keypress_selection(false);

	// The possible eras to play
	std::vector<std::string> eras;
	BOOST_FOREACH(const config &er, cfg.child_range("era")) {
		eras.push_back(er["name"]);
	}
	if(eras.empty()) {
		gui2::show_transient_message(disp.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}
	era_combo_.set_items(eras);

	if (size_t(preferences::era()) < eras.size()) {
		era_combo_.set_selected(preferences::era());
	} else {
		era_combo_.set_selected(preferences::era());
	}

	dependency_manager_.try_era_by_index(era_selection_, true);

	// Available modifications
	BOOST_FOREACH (const config& mod, cfg.child_range("modification")) {
		available_mods_.add_child("modification", mod);
	}

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

	// Updates the values in the "parameters_" member to match
	// the values selected by the user with the widgets:

	config::const_child_itors era_list = game_config().child_range("era");
	for (int num = era_combo_.selected(); num > 0; --num) {
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

	if(cancel_game_.pressed()) {
		set_result(QUIT);
		return;
	}

	if(launch_game_.pressed() || maps_menu_.double_clicked()) {
		// check if the map is valid
		const std::string& map_data = parameters_.scenario_data["map_data"];
		util::unique_ptr<gamemap> map;
		try {
			map.reset(new gamemap(game_config(), map_data));
		} catch(incorrect_map_format_error&) {
		} catch(twml_exception&) {}

		if (map.get() == NULL) {
			gui2::show_transient_message(disp_.video(), "", _("The map is invalid."));
		} else {
			set_result(CREATE);
			return;
		}
	}



	if(choose_mods_.pressed()) {
		if (available_mods_.empty()) {
			gui2::show_transient_message(disp_.video(), "",
			_(	"There are no modifications currently installed." \
				" To download modifications, connect to the add-ons server" \
				" by choosing the 'Add-ons' option on the main screen."		));
		} else {

			gui2::tmp_create_game_choose_mods
						dialog(available_mods_, parameters_.active_mods);

			dialog.show(disp_.video());

			dependency_manager_.try_modifications(parameters_.active_mods);
			synchronize_selections();
		}
	}

	bool era_changed = era_selection_ != era_combo_.selected();
	era_selection_ = era_combo_.selected();

	if (era_changed) {
		dependency_manager_.try_era_by_index(era_selection_);
		synchronize_selections();
	}

	bool map_changed = map_selection_ != maps_menu_.selection();
	map_selection_ = maps_menu_.selection();

	if (map_changed) {
		dependency_manager_.try_scenario_by_index(map_selection_);
		synchronize_selections();
	}

	if(map_changed) {
		generator_.assign(NULL);

		tooltips::clear_tooltips(minimap_rect_);

		const size_t select = size_t(maps_menu_.selection());

		if(select > 0 && select <= user_maps_.size()) {
			parameters_.saved_game = false;
			if (const config &generic_multiplayer = game_config().child("generic_multiplayer")) {
				parameters_.scenario_data = generic_multiplayer;
				parameters_.scenario_data["map_data"] = read_map(user_maps_[select-1]);
			}

		} else if(select > user_maps_.size() && select <= maps_menu_.number_of_items()-1) {
			parameters_.saved_game = false;
			size_t index = select - user_maps_.size() - 1;
			assert(index < map_index_.size());
			index = map_index_[index];

			config::const_child_itors levels = game_config().child_range("multiplayer");
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

				// If the map should be randomly generated.
				if (!level["map_generation"].empty()) {
					generator_.assign(create_map_generator(level["map_generation"], level.child("generator")));
				}

				if (!level["description"].empty()) {
					tooltips::add_tooltip(minimap_rect_, level["description"], "", false);
				}
			}
		} else {
			parameters_.scenario_data.clear();
			parameters_.saved_game = true;

			if (minimap_restorer_ != NULL)
				minimap_restorer_->restore();
		}
	}

	if(generator_ != NULL && generator_->allow_user_config() && generator_settings_.pressed()) {
		generator_->user_config(disp_);
		map_changed = true;
	}

	if(generator_ != NULL && (map_changed || regenerate_map_.pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);

		// Generate the random map
		cursor::setter cur(cursor::WAIT);
		parameters_.scenario_data = generator_->create_scenario(std::vector<std::string>());
		map_changed = true;

		if (!parameters_.scenario_data["error_message"].empty())
			gui2::show_message(disp().video(), "map generation error", parameters_.scenario_data["error_message"]);

		// Set the scenario to have placing of sides
		// based on the terrain they prefer
		parameters_.scenario_data["modify_placing"] = "true";
	}

	if(map_changed) {
		generator_settings_.hide(generator_ == NULL);
		regenerate_map_.hide(generator_ == NULL);

		const std::string& map_data = parameters_.scenario_data["map_data"];
		parameters_.hash = parameters_.scenario_data.hash();
		util::unique_ptr<gamemap> map;
		try {
			map.reset(new gamemap(game_config(), map_data));
		} catch(incorrect_map_format_error& e) {
			ERR_CF << "map could not be loaded: " << e.message << '\n';

			tooltips::clear_tooltips(minimap_rect_);
			tooltips::add_tooltip(minimap_rect_,e.message);
		} catch(twml_exception& e) {
			ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
		}

		launch_game_.enable(map.get() != NULL);

		// If there are less sides in the configuration than there are
		// starting positions, then generate the additional sides
		const int map_positions = map.get() != NULL ? map->num_valid_starting_positions() : 0;

		for (int pos = parameters_.scenario_data.child_count("side"); pos < map_positions; ++pos) {
			config& side = parameters_.scenario_data.add_child("side");
			side["side"] = pos + 1;
			side["team_name"] = pos + 1;
			side["canrecruit"] = true;
			side["controller"] = "human";
		}

		if(map.get() != NULL) {
			const surface mini(image::getMinimap(minimap_rect_.w,minimap_rect_.h,*map,0));
			SDL_Color back_color = {0,0,0,255};
			draw_centered_on_background(mini, minimap_rect_, back_color, video().getSurface());
		}

		int nsides = 0;
		BOOST_FOREACH(const config &k, parameters_.scenario_data.child_range("side")) {
			if (k["allow_player"].to_bool(true)) ++nsides;
		}

		std::stringstream players;
		std::stringstream map_size;
		if(map.get() != NULL) {
			players << _("Players: ") << nsides;
			map_size << _("Size: ") << map.get()->w() << utils::unicode_multiplication_sign << map.get()->h();
		} else {
			players << _("Error");
			map_size << "";
		}
		map_size_label_.set_text(map_size.str());
		num_players_label_.set_text(players.str());
	}
}

void create::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) << " children widgets" << std::endl;

	ui::hide_children(hide);

	maps_menu_.hide(hide);

	map_size_label_.hide(hide);
	era_label_.hide(hide);
	map_label_.hide(hide);
	num_players_label_.hide(hide);

	cancel_game_.hide(hide);
	launch_game_.hide(hide);

	era_combo_.hide(hide);
	choose_mods_.hide(hide);

	regenerate_map_.hide(hide || generator_ == NULL);
	generator_settings_.hide(hide || generator_ == NULL);

	if (hide) {
		minimap_restorer_.assign(NULL);
	} else {
		minimap_restorer_.assign(new surface_restorer(&video(), minimap_rect_));

		const std::string& map_data = parameters_.scenario_data["map_data"];

		try {
			gamemap map(game_config(), map_data);

			const surface mini(image::getMinimap(minimap_rect_.w,minimap_rect_.h,map,0));
			SDL_Color back_color = {0,0,0,255};
			draw_centered_on_background(mini, minimap_rect_, back_color, video().getSurface());
		} catch(incorrect_map_format_error& e) {
			ERR_CF << "map could not be loaded: " << e.message << "\n";
		} catch(twml_exception& e) {
			ERR_CF <<  "map could not be loaded: " << e.dev_message << '\n';
		}

	}
}

void create::layout_children(const SDL_Rect& rect)
{
	DBG_MP << "laying out the children" << std::endl;

	ui::layout_children(rect);

	std::pair<int,int> resolution = preferences::resolution();
	const bool low_hres = resolution.first <= 840;
	const bool low_vres = resolution.second < 768;

	const int border_size = low_vres ? 4 : 6;
	const int column_border_size = low_hres ? 8 : 10;

	SDL_Rect ca = client_area();
	int xpos = ca.x;
	int ypos = ca.y;

	const int minimap_width = !low_vres ? 200 : 100;
	const int maps_menu_width = !low_hres ? 200 : 175;

	// Dialog title
	ypos += low_vres ? 0 : title().height() + border_size;

	// Save ypos here (column top)
	int ypos_columntop = ypos;

	// First column: minimap & random map options
	minimap_rect_ = create_rect(xpos, ypos, minimap_width, minimap_width);
	ypos += minimap_width + border_size;

	num_players_label_.set_location(xpos, ypos);
	ypos += num_players_label_.height() + border_size;

	map_size_label_.set_location(xpos, ypos);
	ypos += map_size_label_.height() + 2 * border_size;

	regenerate_map_.set_location(xpos, ypos);
	ypos += regenerate_map_.height() + border_size;
	generator_settings_.set_location(xpos, ypos);
	ypos += generator_settings_.height() + 2 * border_size;

	era_label_.set_location(xpos, ypos);
	ypos += era_label_.height() + border_size;
	era_combo_.set_location(xpos, ypos);
	ypos += era_combo_.height() + border_size;
	choose_mods_.set_location(xpos, ypos);
	ypos += choose_mods_.height() + border_size;

	// Second column: map menu
	ypos = ypos_columntop;
	xpos += minimap_width + column_border_size;
	map_label_.set_location(xpos, ypos);
	ypos += map_label_.height() + border_size;

	maps_menu_.set_max_width(maps_menu_width);
	maps_menu_.set_max_height(ca.h + ca.y - ypos);
	maps_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int mapsel_save = maps_menu_.selection();
	maps_menu_.set_items(map_options_);
	maps_menu_.move_selection(mapsel_save);

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

void create::synchronize_selections()
{
	DBG_MP << "Synchronizing with the dependency manager" << std::endl;
	if (era_selection_ != dependency_manager_.get_era_index()) {
		era_combo_.set_selected(dependency_manager_.get_era_index());
		process_event();
	}

	if (map_selection_ != dependency_manager_.get_scenario_index()) {
		maps_menu_.move_selection(dependency_manager_.get_scenario_index());
		process_event();
	}

	parameters_.active_mods = dependency_manager_.get_modifications();
}

} // namespace mp


