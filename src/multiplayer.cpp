/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "cursor.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "language.hpp"
#include "log.hpp"
#include "image.hpp"
#include "key.hpp"
#include "mapgen.hpp"
#include "multiplayer.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "scoped_resource.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
#include "widgets/widget.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

network_game_manager::~network_game_manager()
{
	if(network::nconnections() > 0) {
		std::cerr << "sending leave_game\n";
		config cfg;
		cfg.add_child("leave_game");
		network::send_data(cfg);
		std::cerr << "sent leave_game\n";
	}
}

multiplayer_game_setup_dialog::multiplayer_game_setup_dialog(
                              display& disp, game_data& units_data,
                              const config& cfg, game_state& state, bool server, const std::string& controller)
        : disp_(disp), units_data_(units_data), cfg_(cfg), state_(state), server_(server), level_(NULL), map_selection_(-1),
		  maps_menu_(NULL), turns_slider_(NULL), village_gold_slider_(NULL), xp_modifier_slider_(NULL),
		  fog_game_(NULL), shroud_game_(NULL), observers_game_(NULL),
          cancel_game_(NULL), launch_game_(NULL), regenerate_map_(NULL), generator_settings_(NULL),
		  era_combo_(NULL), vision_combo_(NULL), name_entry_(NULL), generator_(NULL), controller_(controller)
{
	std::cerr << "setup dialog ctor\n";

	state_.players.clear();
	state_.variables.clear();

	//build the list of scenarios to play
	get_files_in_dir(get_user_data_dir() + "/editor/maps",&user_maps_,NULL,FILE_NAME_ONLY);

	map_options_ = user_maps_;

	const config::child_list& levels = cfg.get_children("multiplayer");
	for(config::child_list::const_iterator j = levels.begin(); j != levels.end(); ++j){
		map_options_.push_back((**j)["name"]);
	}

	//add the 'load game' option
	map_options_.push_back(_("Load Game") + std::string("..."));

	//create the scenarios menu
	maps_menu_.assign(new gui::menu(disp_,map_options_));
	maps_menu_->set_numeric_keypress_selection(false);

	turns_slider_.assign(new gui::slider(disp_));
	turns_slider_->set_min(20);
	turns_slider_->set_max(100);
	turns_slider_->set_value(preferences::turns());
	turns_slider_->set_help_string(_("The maximum turns the game will go for"));

	village_gold_slider_.assign(new gui::slider(disp_));
	village_gold_slider_->set_min(1);
	village_gold_slider_->set_max(5);
	village_gold_slider_->set_value(preferences::village_gold());
	village_gold_slider_->set_help_string(_("The amount of income each village yields per turn"));

	xp_modifier_slider_.assign(new gui::slider(disp_));
	xp_modifier_slider_->set_min(25);
	xp_modifier_slider_->set_max(200);
	xp_modifier_slider_->set_value(preferences::xp_modifier());
	xp_modifier_slider_->set_increment(10);
	xp_modifier_slider_->set_help_string(_("The amount of experience a unit needs to advance"));

	fog_game_.assign(new gui::button(disp_,_("Fog Of War"),gui::button::TYPE_CHECK));
	fog_game_->set_check(preferences::fog());
	fog_game_->set_help_string(_("Enemy units cannot be seen unless they are in range of your units"));

	shroud_game_.assign(new gui::button(disp_,_("Shroud"),gui::button::TYPE_CHECK));
	shroud_game_->set_check(preferences::shroud());
	shroud_game_->set_help_string(_("The map is unknown until your units explore it"));

	observers_game_.assign(new gui::button(disp_,_("Observers"),gui::button::TYPE_CHECK));
	observers_game_->set_check(preferences::allow_observers());
	observers_game_->set_help_string(_("Allow users who are not playing to watch the game"));

	cancel_game_.assign(new gui::button(disp_,_("Cancel")));
	launch_game_.assign(new gui::button(disp_,_("Ok")));

	regenerate_map_.assign(new gui::button(disp_,_("Regenerate")));

	generator_settings_.assign(new gui::button(disp_,_("Settings...")));

	//The possible vision settings
	std::vector<std::string> vision_types;
	vision_types.push_back(_("Share View"));
	vision_types.push_back(_("Share Maps"));
	vision_types.push_back(_("Share None"));
	vision_combo_.assign(new gui::combo(disp_,vision_types));

	//the possible eras to play
	const config::child_list& era_list = cfg.get_children("era");
	std::vector<std::string> eras;
	for(config::child_list::const_iterator er = era_list.begin(); er != era_list.end(); ++er) {
		eras.push_back((**er)["name"]);
	}

	if(eras.empty()) {
		gui::show_dialog(disp_,NULL,"",_("No multiplayer sides."),gui::OK_ONLY);
		std::cerr << "ERROR: no eras found\n";
		throw config::error("no eras found");
	}

	era_combo_.assign(new gui::combo(disp_,eras));
	era_combo_->set_selected(preferences::era());

	std::cerr << "end setup dialog ctor\n";
}

void multiplayer_game_setup_dialog::set_area(const SDL_Rect& area)
{
	std::cerr << "setup dialog set_area\n";

	area_ = area;

	map_selection_ = -1;

	// Dialog width and height
	int width = int(area.w);
	int height = int(area.h);
	const int left = area.x;
	const int top = area.y;
	const int border_size = 6;
	const int right = left + width;
	const int bottom = top + height;

	int xpos = left + border_size;
	int ypos = top + gui::draw_dialog_title(left,top,&disp_,_("Create Game")).h + border_size;

	//Name Entry
	ypos += font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
	                        _("Name of game") + std::string(":"),xpos,ypos).h + border_size;
	string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	name_entry_.assign(new gui::textbox(disp_,width-20,vgettext("$login's game", i18n_symbols)));
	name_entry_->set_location(xpos,ypos);
	name_entry_->set_dirty();

	ypos += name_entry_->location().h + border_size;

	const int minimap_width = 200;

	//the map selection menu goes near the middle of the dialog, to the right of
	//the minimap
	const int map_label_height = font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
	                                             _("Map to play") + std::string(":"),xpos + minimap_width + border_size,ypos).h;

	maps_menu_->set_max_width(area.x + area.w - (xpos + minimap_width) - 250);
	maps_menu_->set_max_height(area.y + area.h - (ypos + map_label_height));
	maps_menu_->set_items(map_options_);
	maps_menu_->set_location(xpos + minimap_width + border_size,ypos + map_label_height + border_size);
	maps_menu_->move_selection(preferences::map());
	maps_menu_->set_dirty();

	SDL_Rect rect;

	//the sliders and other options on the right side of the dialog
	rect.x = xpos + minimap_width + maps_menu_->width() + border_size;
	rect.y = ypos;
	rect.w = maximum<int>(0,right - border_size - rect.x);
	//a font sized "12" isn't necessarily 12 pixel high.
	rect.h = font::get_max_height(font::SIZE_SMALL);

	turns_restorer_ = surface_restorer(&disp_.video(),rect);

	rect.y += rect.h + border_size + 1;

	turns_slider_->set_location(rect);
	turns_slider_->set_dirty();

	//Village Gold
	rect.y += rect.h + border_size + 1;

	village_gold_restorer_ = surface_restorer(&disp_.video(),rect);

	rect.y += rect.h + border_size + 1;

	village_gold_slider_->set_location(rect);
	village_gold_slider_->set_dirty();

	//Experience Modifier
	rect.y += rect.h + border_size + 1;

	xp_restorer_ = surface_restorer(&disp_.video(),rect);

	rect.y += rect.h + border_size + 1;

	xp_modifier_slider_->set_location(rect);
	xp_modifier_slider_->set_dirty();

	//FOG of war
	rect.y += rect.h + border_size + 1;

	fog_game_->set_location(rect.x,rect.y);
	fog_game_->set_dirty();

	rect.y += fog_game_->location().h + border_size + 1;

	//Shroud
	shroud_game_->set_location(rect.x,rect.y);
	shroud_game_->set_dirty();

	rect.y += shroud_game_->location().h + border_size;

	//Observers
	observers_game_->set_location(rect.x,rect.y);
	observers_game_->set_dirty();

	rect.y += observers_game_->location().h + border_size;

	//Ally shared view settings
	vision_combo_->set_location(rect.x,rect.y);

	rect.y += vision_combo_->height() + border_size;

	gui::button* left_button = launch_game_;
	gui::button* right_button = cancel_game_;

#ifdef OK_BUTTON_ON_RIGHT
	std::swap(left_button,right_button);
#endif

	//Buttons
	right_button->set_location(right - right_button->width() - gui::ButtonHPadding,bottom - right_button->height() - gui::ButtonVPadding);
	left_button->set_location(right - right_button->width() - left_button->width() - gui::ButtonHPadding*2,bottom - left_button->height() - gui::ButtonVPadding);

	cancel_game_->set_dirty();
	launch_game_->set_dirty();
	

	regenerate_map_->set_location(rect.x,rect.y);

	rect.y += regenerate_map_->location().h + border_size;

	generator_settings_->set_location(rect.x,rect.y);

	//player amount number background
	SDL_Rect player_num_rect = {xpos+minimap_width/2 - 30,ypos+minimap_width,100,25};
	playernum_restorer_ = surface_restorer(&disp_.video(),player_num_rect);

	SDL_Rect era_rect = {xpos,player_num_rect.y+player_num_rect.h + border_size,50,20};
	era_rect = font::draw_text(&disp_,era_rect,font::SIZE_SMALL,font::GOOD_COLOUR,_("Era") + std::string(":"),
	                           era_rect.x,era_rect.y);
	
	era_combo_->set_location(era_rect.x+era_rect.w+border_size,era_rect.y);

	SDL_Rect minimap_rect = {xpos,ypos,minimap_width,minimap_width};
	minimap_restorer_ = surface_restorer(&disp_.video(),minimap_rect);

	name_entry_->hide(false);
	maps_menu_->hide(false);
	turns_slider_->hide(false);
	village_gold_slider_->hide(false);
	xp_modifier_slider_->hide(false);
	fog_game_->hide(false);
	shroud_game_->hide(false);
	observers_game_->hide(false);
	vision_combo_->hide(false);
	right_button->hide(false);
	left_button->hide(false);
	regenerate_map_->hide(false);
	generator_settings_->hide(false);
	era_combo_->hide(false);

	std::cerr << "setup dialog end set_area\n";
}

void multiplayer_game_setup_dialog::clear_area()
{
	name_entry_->hide();
	maps_menu_->hide();
	turns_slider_->hide();
	village_gold_slider_->hide();
	xp_modifier_slider_->hide();
	fog_game_->hide();
	shroud_game_->hide();
	observers_game_->hide();
	vision_combo_->hide();
	launch_game_->hide();
	cancel_game_->hide();
	regenerate_map_->hide();
	generator_settings_->hide();
	era_combo_->hide();
}

lobby::RESULT multiplayer_game_setup_dialog::process()
{
	CKey key;

	name_entry_->process();
	maps_menu_->process();

	if(cancel_game_->pressed() || key[SDLK_ESCAPE]) 
		return lobby::QUIT;

	if(launch_game_->pressed() || maps_menu_->double_clicked()) {
		if(name_entry_->text() != "") {
			return lobby::CREATE;
		} else {
			gui::show_dialog(disp_,NULL,"","You must enter a name.",gui::OK_ONLY);
		}
	}

	events::raise_process_event();
	events::raise_draw_event();

	//Turns per game
	const int cur_turns = turns_slider_->value();
	turns_restorer_.restore();

	std::string turns_str;

	char buf[100];
	if(cur_turns < 100) {
		sprintf(buf,"%d", cur_turns);
		turns_str = buf;
	} else {
		turns_str = _("Unlimited");
	}

	font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
	                _("Turns") + std::string(": ") + turns_str,
					turns_restorer_.area().x,turns_restorer_.area().y);

	//Villages can produce between 1 and 10 gold a turn
	const int village_gold = village_gold_slider_->value();
	village_gold_restorer_.restore();
	sprintf(buf,": %d", village_gold);
	font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
	                _("Village Gold") + std::string(buf),
					village_gold_restorer_.area().x,village_gold_restorer_.area().y);



	//experience modifier
	const int xpmod = xp_modifier_slider_->value();
	xp_restorer_.restore();
	sprintf(buf,": %d%%", xpmod);

	const SDL_Rect& xp_rect = xp_restorer_.area();
	font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
	                _("Experience Requirements") + std::string(buf),xp_rect.x,xp_rect.y);

	bool map_changed = map_selection_ != maps_menu_->selection();
	map_selection_ = maps_menu_->selection();

	if(map_changed) {
		generator_.assign(NULL);

		SDL_Rect minimap_rect = minimap_restorer_.area();
		tooltips::clear_tooltips(minimap_rect);

		const size_t select = size_t(maps_menu_->selection());

		if(select < user_maps_.size()) {
			const config* const generic_multiplayer = cfg_.child("generic_multiplayer");
			if(generic_multiplayer != NULL) {
				scenario_data_ = *generic_multiplayer;
				scenario_data_["map_data"] = read_map(user_maps_[select]);
				level_ = &scenario_data_;
			}

		} else if(select != maps_menu_->nitems()-1) {
			const size_t index = select - user_maps_.size();
			const config::child_list& levels = cfg_.get_children("multiplayer");

			if(index < levels.size()) {

				scenario_data_ = *levels[index];
				level_ = &scenario_data_;

				std::string& map_data = scenario_data_["map_data"];
				if(map_data == "" && scenario_data_["map"] != "") {
					map_data = read_map(scenario_data_["map"]);
				}

				//if the map should be randomly generated
				if(scenario_data_["map_generation"] != "") {
					generator_.assign(create_map_generator(scenario_data_["map_generation"],scenario_data_.child("generator")));
				}

				if(scenario_data_["description"].empty() == false) {
					tooltips::add_tooltip(minimap_rect,scenario_data_["description"]);
				}
			}
		} else {

			scenario_data_.clear();

			playernum_restorer_.restore();
			minimap_restorer_.restore();
			const SDL_Rect& player_num_rect = playernum_restorer_.area();
			font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
				                _("Players") + std::string(": ?"),player_num_rect.x,player_num_rect.y);
		}
	}

	if(generator_ != NULL && generator_->allow_user_config() && generator_settings_->pressed()) {
		generator_->user_config(disp_);
		map_changed = true;
	}

	if(generator_ != NULL && level_ != NULL && (map_changed || regenerate_map_->pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);

		//generate the random map
		scenario_data_ = generator_->create_scenario(std::vector<std::string>());
		level_ = &scenario_data_;
		map_changed = true;

		//set the scenario to have placing of sides based on the terrain they prefer
		(*level_)["modify_placing"] = "true";
	}

	//if the map has changed and "Load Map" is not selected
	if(map_changed && level_ != NULL) {
		generator_settings_->hide(generator_ == NULL);
		regenerate_map_->hide(generator_ == NULL);

		const std::string& map_data = (*level_)["map_data"];

		gamemap map(cfg_,map_data);

		//if there are less sides in the configuration than there are starting
		//positions, then generate the additional sides
		const int map_positions = map.num_valid_starting_positions();

		for(int pos = level_->get_children("side").size(); pos < map_positions; ++pos) {
			config& side = level_->add_child("side");
			side["enemy"] = "1";
			char buf[50];
			sprintf(buf,"%d",(pos+1));
			side["side"] = buf;
			side["team_name"] = buf;
			side["canrecruit"] = "1";
			side["controller"] = "human";
		}

		//if there are too many sides, remove some
		while(level_->get_children("side").size() > map_positions) {
			level_->remove_child("side",level_->get_children("side").size()-1);
		}

		SDL_Rect rect = minimap_restorer_.area();
		const surface mini(image::getMinimap(rect.w,rect.h,map,0));

		if(mini != NULL) {
			SDL_BlitSurface(mini, NULL, disp_.video().getSurface(), &rect);
			update_rect(rect);
		
			//Display the number of players
			SDL_Rect players_rect = playernum_restorer_.area();

			playernum_restorer_.restore();			

			const int nsides = level_->get_children("side").size();

			std::stringstream players;
			players << _("Players") << ": " << nsides;
			font::draw_text(&disp_,disp_.screen_area(),font::SIZE_SMALL,font::GOOD_COLOUR,
			                players.str(),players_rect.x,players_rect.y);
		}
	}

	return lobby::CONTINUE;
}

void multiplayer_game_setup_dialog::start_game()
{
	std::cerr << "calling start_game()\n";
	const network::manager net_manager;
	const network::server_manager server_man(15000,server_ ? network::server_manager::TRY_CREATE_SERVER : network::server_manager::NO_SERVER);

	if(server_ && server_man.is_running() == false) {
		gui::show_dialog(disp_,NULL,_("Warning"),_("The game was unable to bind to the port needed to host games over the network. Network players will be unable to connect to this game"),
		                 gui::OK_ONLY);
	}

	turns_restorer_ = surface_restorer();
	village_gold_restorer_ = surface_restorer();
	xp_restorer_ = surface_restorer();
	playernum_restorer_ = surface_restorer();
	minimap_restorer_ = surface_restorer();

	std::cerr << "loading connector...\n";
	//Connector
	mp_connect connector(disp_, name_entry_->text(), cfg_, units_data_, state_, false, controller_);

	std::cerr << "done loading connector...\n";

	const int turns = turns_slider_->value() < turns_slider_->max_value() ?
		turns_slider_->value() : -1;

	const config::child_list& era_list = cfg_.get_children("era");

	const int share = vision_combo_->selected();

	//Save values for next game
	preferences::set_allow_observers(observers_game_->checked());
	preferences::set_fog(fog_game_->checked());
	preferences::set_shroud(shroud_game_->checked());
	preferences::set_turns(turns_slider_->value());
	preferences::set_village_gold(village_gold_slider_->value());
	preferences::set_xp_modifier(xp_modifier_slider_->value());
	preferences::set_era(era_combo_->selected());
	preferences::set_map(maps_menu_->selection());
	
	const int res = connector.load_map((*era_list[era_combo_->selected()])["id"],
	                   scenario_data_, turns, village_gold_slider_->value(),
					   xp_modifier_slider_->value(), fog_game_->checked(),
					   shroud_game_->checked(), observers_game_->checked(),
					   share == 0, share == 1);
	if(res == -1) {
		return;
	}

	name_entry_->set_focus(false);

	//free up widget resources so they're not consumed while the game is on
	maps_menu_.assign(NULL);
	turns_slider_.assign(NULL);
	village_gold_slider_.assign(NULL);
	xp_modifier_slider_.assign(NULL);
	fog_game_.assign(NULL);
	shroud_game_.assign(NULL);
	observers_game_.assign(NULL);
	vision_combo_.assign(NULL);
	cancel_game_.assign(NULL);
	launch_game_.assign(NULL);
	regenerate_map_.assign(NULL);
	generator_settings_.assign(NULL);
	era_combo_.assign(NULL);
	name_entry_.assign(NULL);
	generator_.assign(NULL);

	std::vector<std::string> messages;
	config game_data;
	lobby::RESULT result = lobby::CONTINUE;
	while(result == lobby::CONTINUE) {
		result = lobby::enter(disp_,game_data,cfg_,&connector,messages);
	}

	if(result == lobby::CREATE) {
		connector.start_game();
	}
}
