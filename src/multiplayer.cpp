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

#include "cursor.hpp"
#include "events.hpp"
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
                              const config& cfg, game_state& state, bool server)
        : disp_(disp), units_data_(units_data), cfg_(cfg), state_(state), server_(server), level_(NULL), map_selection_(-1),
		  maps_menu_(NULL), turns_slider_(NULL), village_gold_slider_(NULL), xp_modifier_slider_(NULL),
		  fog_game_(NULL), shroud_game_(NULL), observers_game_(NULL),
          cancel_game_(NULL), launch_game_(NULL), regenerate_map_(NULL), generator_settings_(NULL),
		  era_combo_(NULL), name_entry_(NULL), generator_(NULL)
{
	std::cerr << "setup dialog ctor\n";

	state_.available_units.clear();
	state_.variables.clear();
	state_.can_recruit.clear();

	//build the list of scenarios to play
	std::vector<std::string> options;
	const config::child_list& levels = cfg.get_children("multiplayer");
	std::map<int,std::string> res_to_id;
	for(config::child_list::const_iterator i = levels.begin(); i != levels.end(); ++i){
		const std::string& id = (**i)["id"];
		res_to_id[i - levels.begin()] = id;

		const std::string& lang_name = string_table[id];
		if(lang_name.empty() == false)
			options.push_back(lang_name);
		else
			options.push_back((**i)["name"]);
	}

	//add the 'load game' option
	options.push_back(string_table["load_game"] + "...");

	//create the scenarios menu
	maps_menu_.assign(new gui::menu(disp_,options));
	maps_menu_->set_numeric_keypress_selection(false);

	SDL_Rect rect = {0,0,0,0};

	turns_slider_.assign(new gui::slider(disp_,rect));
	turns_slider_->set_min(20);
	turns_slider_->set_max(100);
	turns_slider_->set_value(50);

	village_gold_slider_.assign(new gui::slider(disp_,rect));
	village_gold_slider_->set_min(1);
	village_gold_slider_->set_max(10);
	village_gold_slider_->set_value(1);

	xp_modifier_slider_.assign(new gui::slider(disp_,rect));
	xp_modifier_slider_->set_min(25);
	xp_modifier_slider_->set_max(200);
	xp_modifier_slider_->set_value(100);

	fog_game_.assign(new gui::button(disp_,string_table["fog_of_war"],gui::button::TYPE_CHECK));
	fog_game_->set_check(false);

	shroud_game_.assign(new gui::button(disp_,string_table["shroud"],gui::button::TYPE_CHECK));
	shroud_game_->set_check(false);

	observers_game_.assign(new gui::button(disp_,string_table["observers"],gui::button::TYPE_CHECK));
	observers_game_->set_check(true);

	cancel_game_.assign(new gui::button(disp_,string_table["cancel_button"]));
	launch_game_.assign(new gui::button(disp_,string_table["ok_button"]));

	regenerate_map_.assign(new gui::button(disp_,string_table["regenerate_map"]));

	generator_settings_.assign(new gui::button(disp_,string_table["generator_settings"]));

	//the possible eras to play
	const config::child_list& era_list = cfg.get_children("era");
	std::vector<std::string> eras;
	for(config::child_list::const_iterator er = era_list.begin(); er != era_list.end(); ++er) {
		eras.push_back(translate_string_default((**er)["id"],(**er)["name"]));
	}

	if(eras.empty()) {
		gui::show_dialog(disp_,NULL,"",string_table["error_no_mp_sides"],gui::OK_ONLY);
		std::cerr << "ERROR: no eras found\n";
		throw config::error("no eras found");
	}

	era_combo_.assign(new gui::combo(disp_,eras));

	std::cerr << "end setup dialog ctor\n";
}

void multiplayer_game_setup_dialog::set_area(const SDL_Rect& area)
{
	std::cerr << "setup dialog set_area\n";

	area_ = area;

	// Dialog width and height
	int width = int(area.w);
	int height = int(area.h);
	const int left = area.x;
	const int top = area.y;
	const int border_size = 5;
	const int right = left + width;
	const int bottom = top + height;

	std::cerr << "a\n";

	//gui::draw_dialog_background(left,top,width,height,disp_,"menu");
	int xpos = left + border_size;
	int ypos = top + gui::draw_dialog_title(left,top,&disp_,string_table["create_new_game"]).h + border_size;

	std::cerr << "b\n";

	//Name Entry
	ypos += font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
	                        string_table["name_of_game"] + ":",xpos,ypos).h + border_size;
	name_entry_.assign(new gui::textbox(disp_,width-20,string_table["game_prefix"] + preferences::login() + string_table["game_postfix"]));
	name_entry_->set_location(xpos,ypos);

	ypos += name_entry_->location().h + border_size;

	std::cerr << "c\n";

	const int minimap_width = 200;

	//the map selection menu goes near the middle of the dialog, to the right of
	//the minimap
	const int map_label_height = font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
	                                             string_table["map_to_play"] + ":",xpos + minimap_width + border_size,ypos).h;

	maps_menu_->set_loc(xpos + minimap_width + border_size,ypos + map_label_height + border_size);

	std::cerr << "d\n";

	SDL_Rect rect;

	//the sliders and other options on the right side of the dialog
	rect.x = xpos + minimap_width + maps_menu_->width() + border_size*2;
	rect.y = ypos;
	rect.w = maximum<int>(0,right - border_size - rect.x);
	//a font sized "12" isn't necessarily 12 pixel high.
	rect.h = font::get_max_height(12);

	turns_restorer_ = surface_restorer(&disp_.video(),rect);

	rect.y += rect.h + border_size*2;

	turns_slider_->set_location(rect);

	std::cerr << "e\n";

	//Village Gold
	rect.y += rect.h + border_size*2;

	village_gold_restorer_ = surface_restorer(&disp_.video(),rect);

	rect.y += rect.h + border_size*2;

	village_gold_slider_->set_location(rect);

	std::cerr << "f\n";

	//Experience Modifier
	rect.y += rect.h + border_size*2;

	std::cerr << "fa\n";

	xp_restorer_ = surface_restorer(&disp_.video(),rect);

	std::cerr << "fb\n";

	rect.y += rect.h + border_size*2;

	xp_modifier_slider_->set_location(rect);

	std::cerr << "fc\n";

	//FOG of war
	rect.y += rect.h + border_size*2;

	fog_game_->set_location(rect.x,rect.y);

	rect.y += fog_game_->location().h + border_size;

	std::cerr << "g\n";

	//Shroud
	shroud_game_->set_location(rect.x,rect.y);

	rect.y += shroud_game_->location().h + border_size;

	//Observers
	observers_game_->set_location(rect.x,rect.y);

	rect.y += observers_game_->location().h + border_size;

	std::cerr << "h\n";

	//Buttons
	cancel_game_->set_location(right - cancel_game_->width() - gui::ButtonHPadding,
	                     bottom - cancel_game_->height() - gui::ButtonVPadding);
	launch_game_->set_location(right - cancel_game_->width() - launch_game_->width() - gui::ButtonHPadding*2,
	                     bottom - launch_game_->height() - gui::ButtonVPadding);
	

	regenerate_map_->set_location(rect.x,rect.y);
	regenerate_map_->bg_backup();

	rect.y += regenerate_map_->location().h + border_size;

	generator_settings_->set_location(rect.x,rect.y);
	generator_settings_->bg_backup();

	std::cerr << "i\n";

	//player amount number background
	SDL_Rect player_num_rect = {xpos+minimap_width/2 - 30,ypos+minimap_width,100,25};
	playernum_restorer_ = surface_restorer(&disp_.video(),player_num_rect);

	SDL_Rect era_rect = {xpos,player_num_rect.y+player_num_rect.h + border_size,50,20};
	era_rect = font::draw_text(&disp_,era_rect,12,font::GOOD_COLOUR,translate_string("Era") + ":",
	                           era_rect.x,era_rect.y);

	std::cerr << "j\n";
	
	era_combo_->set_location(era_rect.x+era_rect.w+border_size,era_rect.y);

	SDL_Rect minimap_rect = {xpos,ypos,minimap_width,minimap_width};
	minimap_restorer_ = surface_restorer(&disp_.video(),minimap_rect);

	std::cerr << "setup dialog end set_area\n";
}

lobby::RESULT multiplayer_game_setup_dialog::process()
{
	CKey key;

	int mousex, mousey;
	const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
	const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

	name_entry_->process();
	turns_slider_->process();
	village_gold_slider_->process();
	xp_modifier_slider_->process();
	era_combo_->process(mousex,mousey,left_button);

	maps_menu_->process(mousex,mousey,left_button,
	                    key[SDLK_UP],key[SDLK_DOWN],
	                    key[SDLK_PAGEUP],key[SDLK_PAGEDOWN]);

	if(cancel_game_->process(mousex,mousey,left_button) || key[SDLK_ESCAPE]) 
		return lobby::QUIT;

	if(launch_game_->process(mousex,mousey,left_button) || maps_menu_->double_clicked()) {
		if(name_entry_->text() != "") {
			return lobby::CREATE;
		} else {
			gui::show_dialog(disp_,NULL,"","You must enter a name.",gui::OK_ONLY);
		}
	}

	fog_game_->process(mousex,mousey,left_button);
	fog_game_->draw();
	shroud_game_->process(mousex,mousey,left_button);
	shroud_game_->draw();
	observers_game_->process(mousex,mousey,left_button);
	observers_game_->draw();

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
		turns_str = string_table["unlimited"];
	}

	font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
	                string_table["turns"] + ": " + turns_str,
					turns_restorer_.area().x,turns_restorer_.area().y);

	//Villages can produce between 1 and 10 gold a turn
	const int village_gold = village_gold_slider_->value();
	village_gold_restorer_.restore();
	sprintf(buf,": %d", village_gold);
	font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
	                string_table["village_gold"] + buf,
					village_gold_restorer_.area().x,village_gold_restorer_.area().y);



	//experience modifier
	const int xpmod = xp_modifier_slider_->value();
	xp_restorer_.restore();
	sprintf(buf,": %d%%", xpmod);

	const SDL_Rect& xp_rect = xp_restorer_.area();
	font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
	                string_table["xp_modifier"] + buf,xp_rect.x,xp_rect.y);

	bool map_changed = map_selection_ != maps_menu_->selection();
	map_selection_ = maps_menu_->selection();

	if(map_changed) {
		generator_.assign(NULL);

		const size_t select = size_t(maps_menu_->selection());
		if(select != maps_menu_->nitems()-1) {
			const config::child_list& levels = cfg_.get_children("multiplayer");

			assert(select < levels.size());
			config& scenario = *levels[select];
			level_ = &scenario;

			std::string& map_data = scenario["map_data"];
			if(map_data == "" && scenario["map"] != "") {
				map_data = read_file("data/maps/" + scenario["map"]);
			}

			//if the map should be randomly generated
			if(scenario["map_generation"] != "") {
				generator_.assign(create_map_generator(scenario["map_generation"],scenario.child("generator")));
			}
		} else {

			playernum_restorer_.restore();
			minimap_restorer_.restore();
			const SDL_Rect& player_num_rect = playernum_restorer_.area();
			font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
				                " Load Map ",player_num_rect.x,player_num_rect.y);
		}
	}

	if(generator_ != NULL && generator_->allow_user_config() && generator_settings_->process(mousex,mousey,left_button)) {
		generator_->user_config(disp_);
		map_changed = true;
	}

	if(generator_ != NULL && level_ != NULL && (map_changed || regenerate_map_->process(mousex,mousey,left_button))) {
		const cursor::setter cursor_setter(cursor::WAIT);

		//generate the random map
		(*level_)["map_data"] = generator_->create_map(std::vector<std::string>());
		map_changed = true;

		//set the scenario to have placing of sides based on the terrain they prefer
		(*level_)["modify_placing"] = "true";
	}

	if(map_changed) {
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
		const scoped_sdl_surface mini(image::getMinimap(rect.w,rect.h,map,0));

		if(mini != NULL) {
			SDL_BlitSurface(mini, NULL, disp_.video().getSurface(), &rect);
			update_rect(rect);
		
			//Display the number of players
			SDL_Rect players_rect = playernum_restorer_.area();

			playernum_restorer_.restore();			

			const int nsides = level_->get_children("side").size();

			std::stringstream players;
			players << string_table["num_players"] << ": " << nsides;
			font::draw_text(&disp_,disp_.screen_area(),12,font::GOOD_COLOUR,
			                players.str(),players_rect.x,players_rect.y);
		}
	}

	return lobby::CONTINUE;
}

void multiplayer_game_setup_dialog::start_game()
{
	turns_restorer_ = surface_restorer();
	village_gold_restorer_ = surface_restorer();
	xp_restorer_ = surface_restorer();
	playernum_restorer_ = surface_restorer();
	minimap_restorer_ = surface_restorer();

	//Connector
	mp_connect connector(disp_, name_entry_->text(), cfg_, units_data_, state_);

	const int turns = turns_slider_->value() < turns_slider_->max_value() ?
		turns_slider_->value() : -1;

	const config::child_list& era_list = cfg_.get_children("era");

	const int res = connector.load_map((*era_list[era_combo_->selected()])["id"],
	                   maps_menu_->selection(), turns, village_gold_slider_->value(),
					   xp_modifier_slider_->value(), fog_game_->checked(),
					   shroud_game_->checked(), observers_game_->checked());
	if(res == -1) {
		return;
	}

	const network::manager net_manager;
	const network::server_manager server_man(15000,server_);
	name_entry_->set_focus(false);

	//free up widget resources so they're not consumed while the game is on
	maps_menu_.assign(NULL);
	turns_slider_.assign(NULL);
	village_gold_slider_.assign(NULL);
	xp_modifier_slider_.assign(NULL);
	fog_game_.assign(NULL);
	shroud_game_.assign(NULL);
	observers_game_.assign(NULL);
	cancel_game_.assign(NULL);
	launch_game_.assign(NULL);
	regenerate_map_.assign(NULL);
	generator_settings_.assign(NULL);
	era_combo_.assign(NULL);
	name_entry_.assign(NULL);
	generator_.assign(NULL);

	std::vector<std::string> messages;
	config game_data;
	const lobby::RESULT result = lobby::enter(disp_,game_data,cfg_,&connector,messages);
	if(result == lobby::CREATE) {
		connector.start_game();
	}
}
