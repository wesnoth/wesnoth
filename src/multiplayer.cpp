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

#include "events.hpp"
#include "font.hpp"
#include "language.hpp"
#include "log.hpp"
#include "image.hpp"
#include "mapgen.hpp"
#include "multiplayer.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"

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

// TODO: This function is way to big. It should be split into 2 functions,
//       one for each dialog.
int play_multiplayer(display& disp, game_data& units_data, config cfg,
                      game_state& state, bool server)
{
	state.available_units.clear();
	state.variables.clear();
	state.can_recruit.clear();

	SDL_Rect rect;
	char buf[100];
	log_scope("play multiplayer");

	//make sure the amount of gold we have for the game is 100
	//later allow configuration of amount of gold
	state.gold = 100;

	// Dialog width and height
	int width=640;
	int height=340;

	int cur_selection = -1;
	int cur_villagegold = 1;
	int new_villagegold = 1;
	int cur_turns = 50;
	int new_turns = 50;
	int cur_playergold = 100;
	int new_playergold = 100;

	gui::draw_dialog_frame((disp.x()-width)/2, (disp.y()-height)/2,
			       width, height, disp);

	//Title
	font::draw_text(&disp,disp.screen_area(),24,font::NORMAL_COLOUR,
	                string_table["create_new_game"],-1,(disp.y()-height)/2+5);

	//Name Entry
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                string_table["name_of_game"] + ":",(disp.x()-width)/2+10,(disp.y()-height)/2+38);
	gui::textbox name_entry(disp,width-20,string_table["game_prefix"] + preferences::login() + string_table["game_postfix"]);
	name_entry.set_position((disp.x()-width)/2+10,(disp.y()-height)/2+55);

	//Maps
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                string_table["map_to_play"] + ":",(disp.x()-width)/2+(int)(width*0.4),
			(disp.y()-height)/2+83);
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

	options.push_back(string_table["load_game"] + "...");
	gui::menu maps_menu(disp,options);
	maps_menu.set_loc((disp.x()-width)/2+(int)(width*0.4),(disp.y()-height)/2+100);

	//Game Turns
	rect.x = (disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19;
	rect.y = (disp.y()-height)/2+83;
	rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19)-10;
	rect.h = 12;
	SDL_Surface* village_bg=get_surface_portion(disp.video().getSurface(), rect);
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                string_table["turns"] + ": 50",rect.x,rect.y);
	rect.y = (disp.y()-height)/2+100;
	rect.h = name_entry.location().w;

	gui::slider turns_slider(disp,rect,0.38);

	//Village Gold
	rect.x = (disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19;
	rect.y = (disp.y()-height)/2+130;
	rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19)-10;
	rect.h = 12;
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                string_table["village_gold"] + ": 1",rect.x,rect.y);
	rect.y = (disp.y()-height)/2+147;
	rect.h = name_entry.location().w;
	gui::slider villagegold_slider(disp,rect,0.0);

	//FOG of war
	gui::button fog_game(disp,string_table["fog_of_war"],gui::button::TYPE_CHECK);
	fog_game.set_check(false);
	fog_game.set_xy(rect.x+6,rect.y+30);
	fog_game.draw();

	//Shroud
	gui::button shroud_game(disp,string_table["shroud"],gui::button::TYPE_CHECK);
	shroud_game.set_check(false);
	shroud_game.set_xy(rect.x+6,rect.y+30+fog_game.height()+2);
	shroud_game.draw();

	//Observers
	gui::button observers_game(disp,string_table["observers"],gui::button::TYPE_CHECK);
	observers_game.set_check(true);
	observers_game.set_xy(rect.x+6,rect.y+30+(2*fog_game.height())+4);
	observers_game.draw();

	//Buttons
	gui::button cancel_game(disp,string_table["cancel_button"]);
	gui::button launch_game(disp,string_table["ok_button"]);
	launch_game.set_xy((disp.x()/2)-launch_game.width()*2-19,(disp.y()-height)/2+height-29);
	cancel_game.set_xy((disp.x()/2)+cancel_game.width()+19,(disp.y()-height)/2+height-29);

	gui::button regenerate_map(disp,string_table["regenerate_map"]);
	regenerate_map.set_xy(rect.x+6,rect.y+30+(3*fog_game.height())+6);
	regenerate_map.backup_background();

	gui::button generator_settings(disp,string_table["generator_settings"]);
	generator_settings.set_xy(rect.x+6,rect.y+30+(4*fog_game.height())+8);
	generator_settings.backup_background();

	//player amount number background
	rect.x = ((disp.x()-width)/2+10)+35;
	rect.y = (disp.y()-height)/2+235;
	rect.w = 145;
	rect.h = 25;
	surface_restorer playernum_bg(&disp.video(),rect);

	update_whole_screen();

	//the current map generator
	map_generator* generator = NULL;

	CKey key;
	config* level_ptr = NULL;
	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		maps_menu.process(mousex,mousey,left_button,
		                  key[SDLK_UP],key[SDLK_DOWN],
		                  key[SDLK_PAGEUP],key[SDLK_PAGEDOWN]);

		if(cancel_game.process(mousex,mousey,left_button) || key[SDLK_ESCAPE]) 
			return -1;

		if(launch_game.process(mousex,mousey,left_button)) {
			if(name_entry.text().empty() == false) {
				//Connector
				mp_connect connector(disp, name_entry.text(), cfg, units_data, state);

				const int res = connector.load_map(maps_menu.selection(), cur_turns, cur_villagegold, fog_game.checked(), shroud_game.checked());
				if(res == -1)
					return -1;

				const network::manager net_manager;
				const network::server_manager server_man(15000,server);
				connector.gui_do();
				return -1;
			} else {
				rect.x=(disp.x()-width)/2;
				rect.y=(disp.y()-height)/2;
				rect.w=width;
				rect.h=height;
				SDL_Surface* dialog_bg=get_surface_portion(disp.video().getSurface(), rect);
				gui::show_dialog(disp,NULL,"",
				                 "You must enter a name.",gui::OK_ONLY);

				SDL_BlitSurface(dialog_bg, NULL, disp.video().getSurface(), &rect);
				SDL_FreeSurface(dialog_bg);
				update_whole_screen();
			}
		}

		fog_game.process(mousex,mousey,left_button);
		shroud_game.process(mousex,mousey,left_button);
		observers_game.process(mousex,mousey,left_button);

		events::raise_process_event();
		events::raise_draw_event();

		//Game turns are 20 to 99
		//FIXME: Should never be a - number, but it is sometimes
		int check_turns=20+int(79*turns_slider.process(mousex,mousey,left_button));		
		if(abs(check_turns) == check_turns)
			new_turns=check_turns;
		if(new_turns != cur_turns) {
			cur_turns = new_turns;
			rect.x = (disp.x()-width)/2+int(width*0.4)+maps_menu.width()+19;
			rect.y = (disp.y()-height)/2+83;
			rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+int(width*0.4)+maps_menu.width()+19)-10;
			rect.h = 12;
			SDL_BlitSurface(village_bg, NULL, disp.video().getSurface(), &rect);
			sprintf(buf,"Turns: %d", cur_turns);
			font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
			                buf,rect.x,rect.y);
			update_rect(rect);
		}

		//work out if we have to generate a new map
		bool map_changed = false;

		//Villages can produce between 1 and 10 gold a turn
		//FIXME: Should never be a - number, but it is sometimes
		int check_villagegold=1+int(9*villagegold_slider.process(mousex,mousey,left_button));
		if(abs(check_villagegold) == check_villagegold)
			new_villagegold=check_villagegold;
		if(new_villagegold != cur_villagegold) {
			cur_villagegold = new_villagegold;
			rect.x = (disp.x()-width)/2+int(width*0.4)+maps_menu.width()+19;
			rect.y = (disp.y()-height)/2+130;
			rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+int(width*0.4)+maps_menu.width()+19)-10;
			rect.h = 12;
			SDL_BlitSurface(village_bg, NULL, disp.video().getSurface(), &rect);
			sprintf(buf,": %d", cur_villagegold);
			font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
			                string_table["village_gold"] + buf,rect.x,rect.y);
			update_rect(rect);
		}
		
		if(maps_menu.selection() != cur_selection) {
			map_changed = true;
			generator = NULL;

			cur_selection = maps_menu.selection();
			if(size_t(maps_menu.selection()) != options.size()-1) {
				level_ptr = levels[maps_menu.selection()];

				std::string& map_data = (*level_ptr)["map_data"];
				if(map_data == "" && (*level_ptr)["map"] != "") {
					map_data = read_file("data/maps/" + (*level_ptr)["map"]);
				}

				//if the map should be randomly generated
				if((*level_ptr)["map_generation"] != "") {
					generator = get_map_generator((*level_ptr)["map_generation"]);
				}
			}else{
				const scoped_sdl_surface disk(image::get_image("misc/disk.png",image::UNSCALED));

				if(disk != NULL) {
					rect.x = ((disp.x()-width)/2+10)+35;
					rect.y = (disp.y()-height)/2+80;
					rect.w = 145;
					rect.h = 145;
					SDL_BlitSurface(disk, NULL, disp.video().getSurface(), &rect);
					update_rect(rect);
				}

				rect.x = ((disp.x()-width)/2+10)+35;
				rect.y = (disp.y()-height)/2+235;
				rect.w = 145;
				rect.h = 25;
				playernum_bg.restore();
				font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
					                " Load Map ",rect.x+45,rect.y);
				update_rect(rect);
			}
		}

		if(generator != NULL && generator->allow_user_config() && generator_settings.process(mousex,mousey,left_button)) {
			generator->user_config(disp);
			map_changed = true;
		}

		if(generator != NULL && (map_changed || regenerate_map.process(mousex,mousey,left_button))) {
			//generate the random map
			(*level_ptr)["map_data"] = generator->create_map(std::vector<std::string>());
			map_changed = true;

			//set the scenario to have placing of sides based on the terrain they prefer
			(*level_ptr)["modify_placing"] = "true";
		}

		if(map_changed) {
			if(generator != NULL) {
				generator_settings.draw();
				regenerate_map.draw();
			} else {
				generator_settings.hide();
				regenerate_map.hide();
			}

			const std::string& map_data = (*level_ptr)["map_data"];

			gamemap map(cfg,map_data);

			//if there are less sides in the configuration than there are starting
			//positions, then generate the additional sides
			const int map_positions = map.num_valid_starting_positions();

			for(int pos = level_ptr->get_children("side").size(); pos < map_positions; ++pos) {
				config& side = level_ptr->add_child("side");
				side["enemy"] = "1";
				char buf[50];
				sprintf(buf,"%d",(pos+1));
				side["side"] = buf;
				side["team_name"] = buf;
				side["canrecruit"] = "1";
				side["controller"] = "human";
			}

			//if there are too many sides, remove some
			while(level_ptr->get_children("side").size() > map_positions) {
				level_ptr->remove_child("side",level_ptr->get_children("side").size()-1);
			}

			const scoped_sdl_surface mini(image::getMinimap(145,145,map));

			if(mini != NULL) {
				rect.x = ((disp.x()-width)/2+10)+35;
				rect.y = (disp.y()-height)/2+80;
				rect.w = 145;
				rect.h = 145;
				SDL_BlitSurface(mini, NULL, disp.video().getSurface(), &rect);
				update_rect(rect);
			}

			//Display the number of players
			rect.x = ((disp.x()-width)/2+10)+35;
			rect.y = (disp.y()-height)/2+235;
			rect.w = 145;
			rect.h = 25;
			playernum_bg.restore();
			config& level = *level_ptr;
			const int nsides = level.get_children("side").size();

			std::stringstream players;
			players << "Players: " << nsides;
			font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
				                players.str(),rect.x+45,rect.y);
			update_rect(rect);
		}

		events::pump();
		disp.video().flip();
		SDL_Delay(20);
	}
	return -1;
}
