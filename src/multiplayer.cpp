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

int play_multiplayer(display& disp, game_data& units_data, const config& cfg,
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

	int cur_selection = -1;
	int cur_villagegold = 1;
	int cur_turns = 50;
	int cur_xpmod = 100;

	// Dialog width and height
	int width = 740;
	int height = 440;
	const int left = (disp.x()-width)/2;
	const int top = (disp.y()-height)/2;
	const int border_size = 5;
	const int right = left + width;
	const int bottom = top + height;

	gui::draw_dialog_frame(left,top,width,height,disp);
	int xpos = left + border_size;
	int ypos = gui::draw_dialog_title(left,top,disp,string_table["create_new_game"]) + border_size;

	//Name Entry
	ypos += font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                        string_table["name_of_game"] + ":",xpos,ypos).h + border_size;
	gui::textbox name_entry(disp,width-20,string_table["game_prefix"] + preferences::login() + string_table["game_postfix"]);
	name_entry.set_position(xpos,ypos);

	ypos += name_entry.location().h + border_size;

	const int minimap_width = 200;

	//the map selection menu goes near the middle of the dialog, to the right of
	//the minimap
	const int map_label_height = font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                                             string_table["map_to_play"] + ":",xpos + minimap_width + border_size,ypos).h;

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
	gui::menu maps_menu(disp,options);
	maps_menu.set_loc(xpos + minimap_width + border_size,ypos + map_label_height + border_size);

	//the sliders and other options on the right side of the dialog
	rect.x = xpos + minimap_width + maps_menu.width() + border_size*2;
	rect.y = ypos;
	rect.w = maximum<int>(0,right - border_size - rect.x);
	rect.h = 12;

	SDL_Rect turns_rect = rect;

	const scoped_sdl_surface village_bg(get_surface_portion(disp.video().getSurface(), rect));

	rect.y += turns_rect.h + border_size*2;

	gui::slider turns_slider(disp,rect);
	turns_slider.set_min(20);
	turns_slider.set_max(100);
	turns_slider.set_value(cur_turns);

	//Village Gold
	rect.y += rect.h + border_size*2;

	SDL_Rect village_rect = rect;

	rect.y += village_rect.h + border_size*2;

	gui::slider villagegold_slider(disp,rect);
	villagegold_slider.set_min(1);
	villagegold_slider.set_max(10);
	villagegold_slider.set_value(cur_villagegold);

	//Experience Modifier
	rect.y += rect.h + border_size*2;

	SDL_Rect xp_rect = rect;

	rect.y += xp_rect.h + border_size*2;

	gui::slider xp_modifier_slider(disp,rect);
	xp_modifier_slider.set_min(25);
	xp_modifier_slider.set_max(200);
	xp_modifier_slider.set_value(cur_xpmod);

	//FOG of war
	rect.y += rect.h + border_size*2;

	gui::button fog_game(disp,string_table["fog_of_war"],gui::button::TYPE_CHECK);
	fog_game.set_check(false);
	fog_game.set_xy(rect.x,rect.y);
	fog_game.draw();

	rect.y += fog_game.location().h + border_size;

	//Shroud
	gui::button shroud_game(disp,string_table["shroud"],gui::button::TYPE_CHECK);
	shroud_game.set_check(false);
	shroud_game.set_xy(rect.x,rect.y);
	shroud_game.draw();

	rect.y += shroud_game.location().h + border_size;

	//Observers
	gui::button observers_game(disp,string_table["observers"],gui::button::TYPE_CHECK);
	observers_game.set_check(true);
	observers_game.set_xy(rect.x,rect.y);
	observers_game.draw();

	rect.y += observers_game.location().h + border_size;

	//Buttons
	gui::button cancel_game(disp,string_table["cancel_button"]);
	gui::button launch_game(disp,string_table["ok_button"]);
	launch_game.set_xy((disp.x()/2)-launch_game.width()*2-19,bottom-29);
	cancel_game.set_xy((disp.x()/2)+cancel_game.width()+19,bottom-29);

	gui::button regenerate_map(disp,string_table["regenerate_map"]);
	regenerate_map.set_xy(rect.x,rect.y);
	regenerate_map.backup_background();

	rect.y += regenerate_map.location().h + border_size;

	gui::button generator_settings(disp,string_table["generator_settings"]);
	generator_settings.set_xy(rect.x,rect.y);
	generator_settings.backup_background();

	//player amount number background
	SDL_Rect player_num_rect = {xpos+minimap_width/2 - 30,ypos+minimap_width,100,25};
	surface_restorer playernum_bg(&disp.video(),player_num_rect);

	//the possible eras to play
	const config::child_list& era_list = cfg.get_children("era");
	std::vector<std::string> eras;
	for(config::child_list::const_iterator er = era_list.begin(); er != era_list.end(); ++er) {
		eras.push_back(translate_string_default((**er)["id"],(**er)["name"]));
	}

	if(eras.empty()) {
		gui::show_dialog(disp,NULL,"",string_table["error_no_mp_sides"],gui::OK_ONLY);
		std::cerr << "ERROR: no eras found\n";
		return -1;
	}

	SDL_Rect era_rect = {xpos,player_num_rect.y+player_num_rect.h + border_size,50,20};
	era_rect = font::draw_text(&disp,era_rect,12,font::GOOD_COLOUR,translate_string("Era") + ":",
	                           era_rect.x,era_rect.y);
	gui::combo era_combo(disp,eras);
	era_combo.set_xy(era_rect.x+era_rect.w+border_size,era_rect.y);

	update_whole_screen();

	//the current map generator
	util::scoped_ptr<map_generator> generator(NULL);

	CKey key;
	config* level_ptr = NULL;
	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		name_entry.process();
		turns_slider.process();
		villagegold_slider.process();
		xp_modifier_slider.process();
		era_combo.process(mousex,mousey,left_button);

		maps_menu.process(mousex,mousey,left_button,
		                  key[SDLK_UP],key[SDLK_DOWN],
		                  key[SDLK_PAGEUP],key[SDLK_PAGEDOWN]);

		if(cancel_game.process(mousex,mousey,left_button) || key[SDLK_ESCAPE]) 
			return -1;

		if(launch_game.process(mousex,mousey,left_button)) {
			if(name_entry.text().empty() == false) {

				//Connector
				mp_connect connector(disp, name_entry.text(), cfg, units_data, state);

				const int res = connector.load_map((*era_list[era_combo.selected()])["id"],
				                   maps_menu.selection(), cur_turns < 100 ? cur_turns : -1, cur_villagegold, cur_xpmod, fog_game.checked(), shroud_game.checked(), observers_game.checked());
				if(res == -1)
					return -1;

				const network::manager net_manager;
				const network::server_manager server_man(15000,server);
				name_entry.set_focus(false);
				connector.gui_do();
				return -1;
			} else {
				gui::show_dialog(disp,NULL,"",
				                 "You must enter a name.",gui::OK_ONLY);
			}
		}

		fog_game.process(mousex,mousey,left_button);
		fog_game.draw();
		shroud_game.process(mousex,mousey,left_button);
		shroud_game.draw();
		observers_game.process(mousex,mousey,left_button);
		observers_game.draw();

		events::raise_process_event();
		events::raise_draw_event();

		//Turns per game
		cur_turns = turns_slider.value();
		SDL_BlitSurface(village_bg, NULL, disp.video().getSurface(), &turns_rect);

		if(cur_turns < 100) {
			sprintf(buf,"Turns: %d", cur_turns);
		} else {
			sprintf(buf,"Turns: %s", string_table["unlimited"].c_str());
		}

		font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
		                buf,turns_rect.x,turns_rect.y);
		update_rect(turns_rect);

		//work out if we have to generate a new map
		bool map_changed = false;

		//Villages can produce between 1 and 10 gold a turn
		cur_villagegold = villagegold_slider.value();
		SDL_BlitSurface(village_bg, NULL, disp.video().getSurface(), &village_rect);
		sprintf(buf,": %d", cur_villagegold);
		font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
		                string_table["village_gold"] + buf,village_rect.x,village_rect.y);
		update_rect(village_rect);


		//experience modifier
		cur_xpmod = xp_modifier_slider.value();
		SDL_BlitSurface(village_bg, NULL, disp.video().getSurface(), &xp_rect);
		sprintf(buf,": %d%%", cur_xpmod);
		font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
		                string_table["xp_modifier"] + buf,xp_rect.x,xp_rect.y);
		update_rect(xp_rect);
		
		if(maps_menu.selection() != cur_selection) {
			map_changed = true;
			generator.assign(NULL);

			cur_selection = maps_menu.selection();
			if(size_t(maps_menu.selection()) != options.size()-1) {
				level_ptr = levels[maps_menu.selection()];

				std::string& map_data = (*level_ptr)["map_data"];
				if(map_data == "" && (*level_ptr)["map"] != "") {
					map_data = read_file("data/maps/" + (*level_ptr)["map"]);
				}

				//if the map should be randomly generated
				if((*level_ptr)["map_generation"] != "") {
					generator.assign(create_map_generator((*level_ptr)["map_generation"],level_ptr->child("generator")));
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

				playernum_bg.restore();
				font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
					                " Load Map ",player_num_rect.x,player_num_rect.y);
				update_rect(rect);
			}
		}

		if(generator != NULL && generator.get()->allow_user_config() && generator_settings.process(mousex,mousey,left_button)) {
			generator.get()->user_config(disp);
			map_changed = true;
		}

		if(generator.get() != NULL && (map_changed || regenerate_map.process(mousex,mousey,left_button))) {
			const cursor::setter cursor_setter(cursor::WAIT);

			//generate the random map
			(*level_ptr)["map_data"] = generator.get()->create_map(std::vector<std::string>());
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

			const scoped_sdl_surface mini(image::getMinimap(minimap_width,minimap_width,map,0));

			if(mini != NULL) {
				SDL_Rect rect = {xpos,ypos,minimap_width,minimap_width};
				SDL_BlitSurface(mini, NULL, disp.video().getSurface(), &rect);
				update_rect(rect);
			
				//Display the number of players
				SDL_Rect players_rect = {xpos+minimap_width/2,ypos+minimap_width,145,25};

				rect.x = ((disp.x()-width)/2+10)+35;
				rect.y = (disp.y()-height)/2+235;
				rect.w = 145;
				rect.h = 25;
				
				playernum_bg.restore();
				config& level = *level_ptr;
				const int nsides = level.get_children("side").size();

				std::stringstream players;
				players << string_table["num_players"] << ": " << nsides;
				font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
				                players.str(),player_num_rect.x,player_num_rect.y);
				update_rect(rect);
			}
		}

		events::pump();
		disp.video().flip();
		SDL_Delay(20);
	}
	return -1;
}
