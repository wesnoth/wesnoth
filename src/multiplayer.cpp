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
		config cfg;
		cfg.add_child("leave_game");
		network::send_data(cfg);
	}
}

namespace {

class connection_acceptor : public gui::dialog_action
{
public:

	typedef std::map<config*,network::connection> positions_map;

	connection_acceptor(config& players);
	int do_action();

	bool is_complete() const;

	std::vector<std::string> get_positions_status() const;

	enum { CONNECTIONS_PART_FILLED=1, CONNECTIONS_FILLED=2 };

private:
	positions_map positions_;
	config& players_;
};

connection_acceptor::connection_acceptor(config& players)
                   : players_(players)
{
	const config::child_list& sides = players.get_children("side");
	for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
		if((**i)["controller"] == "network") {
			positions_[*i] = 0;
		}
	}

	//if we have any connected players when we are created, send them the data
	network::send_data(players_);
}

int connection_acceptor::do_action()
{
	network::connection sock = network::accept_connection();
	if(sock) {
		std::cerr << "Received connection\n";
		network::send_data(players_,sock);
	}

	config cfg;

	const config::child_list& sides = players_.get_children("side");

	try {
		sock = network::receive_data(cfg);
	} catch(network::error& e) {

		std::cerr << "caught networking error. we are " << (network::is_server() ? "" : "NOT") << " a server\n";

		sock = 0;

		//if the problem isn't related to any specific connection,
		//it's a general error and we should just re-throw the error
		//likewise if we are not a server, we cannot afford any connection
		//to go down, so also re-throw the error
		if(!e.socket || !network::is_server()) {
			e.disconnect();
			throw network::error(e.message);
		}

		bool changes = false;

		//a socket has disconnected. Remove its positions.
		for(positions_map::iterator i = positions_.begin();
		    i != positions_.end(); ++i) {
			if(i->second == e.socket) {
				changes = true;
				i->second = 0;
				i->first->values.erase("taken");
			}
		}

		//now disconnect the socket
		e.disconnect();

		//if there have been changes to the positions taken,
		//then notify other players
		if(changes) {
			network::send_data(players_);
		}
	}

	if(sock) {
		const int side_drop = atoi(cfg["side_drop"].c_str())-1;
		if(side_drop >= 0 && side_drop < int(sides.size())) {
			positions_map::iterator pos = positions_.find(sides[side_drop]);
			if(pos != positions_.end()) {
				pos->second = 0;
				pos->first->values.erase("taken");
				network::send_data(players_);
			}
		}

		const int side_taken = atoi(cfg["side"].c_str())-1;
		if(side_taken >= 0 && side_taken < int(sides.size())) {
			positions_map::iterator pos = positions_.find(sides[side_taken]);
			if(pos != positions_.end()) {
				if(!pos->second) {
					std::cerr << "client has taken a valid position\n";

					//broadcast to everyone the new game status
					pos->first->values["taken"] = "yes";
					pos->first->values["description"] = cfg["description"];
					pos->first->values["name"] = cfg["name"];
					pos->first->values["type"] = cfg["type"];
					pos->first->values["recruit"] = cfg["recruit"];
					pos->first->values["music"] = cfg["music"];
					positions_[sides[side_taken]] = sock;
					network::send_data(players_);

					std::cerr << "sent player data\n";

					//send a reply telling the client they have secured
					//the side they asked for
					std::stringstream side;
					side << (side_taken+1);
					config reply;
					reply.values["side_secured"] = side.str();
					std::cerr << "going to send data...\n";
					network::send_data(reply,sock);

					//see if all positions are now filled
					bool unclaimed = false;
					for(positions_map::const_iterator p = positions_.begin();
					    p != positions_.end(); ++p) {
						if(!p->second) {
							unclaimed = true;
							break;
						}
					}

					if(!unclaimed) {
						std::cerr << "starting game now...\n";
						return CONNECTIONS_FILLED;
					}
				} else {
					config response;
					response.values["failed"] = "yes";
					network::send_data(response,sock);
				}
			} else {
				std::cerr << "tried to take illegal side: " << side_taken
				          << "\n";
			}
		} else {
			std::cerr << "tried to take unknown side: " << side_taken << "\n";
		}

		return CONNECTIONS_PART_FILLED;
	}

	return CONTINUE_DIALOG;
}

bool connection_acceptor::is_complete() const
{
	for(positions_map::const_iterator i = positions_.begin();
	    i != positions_.end(); ++i) {
		if(!i->second) {
			return false;
		}
	}

	return true;
}

std::vector<std::string> connection_acceptor::get_positions_status() const
{
	std::vector<std::string> result;
	for(positions_map::const_iterator i = positions_.begin();
	    i != positions_.end(); ++i) {
		result.push_back(i->first->values["name"] + "," +
		                 (i->second ? ("@" + i->first->values["description"]) :
		                              string_table["position_vacant"]));
	}

	return result;
}

bool accept_network_connections(display& disp, config& players)
{
	connection_acceptor acceptor(players);

	while(acceptor.is_complete() == false) {
		const std::vector<std::string>& items = acceptor.get_positions_status();
		const int res = gui::show_dialog(disp,NULL,"",
		                                 string_table["awaiting_connections"],
		                       gui::CANCEL_ONLY,&items,NULL,"",NULL,&acceptor);
		if(res == 0) {
			return false;
		}
	}

	config start_game;
	start_game.add_child("start_game");
	network::send_data(start_game);

	return true;
}

}

// TODO: This function is way to big. It should be split into 2 functions,
//       one for each dialog.
int play_multiplayer(display& disp, game_data& units_data, config cfg,
                      game_state& state, bool server)
{
	SDL_Rect rect;
	char buf[100];
	log_scope("play multiplayer");

	//ensure we send a close game message to the server when we are done
	network_game_manager game_manager;

	//make sure the amount of gold we have for the game is 100
	//later allow configuration of amount of gold
	state.gold = 100;

	// Dialog width and height
	int width=600;
	int height=290;

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

	//player amount number background
	rect.x = ((disp.x()-width)/2+10)+35;
	rect.y = (disp.y()-height)/2+235;
	rect.w = 145;
	rect.h = 25;
	SDL_Surface* playernum_bg=get_surface_portion(disp.video().getSurface(), rect);

	update_whole_screen();

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

				connector.load_map(maps_menu.selection(), cur_turns, cur_villagegold, fog_game.checked(), shroud_game.checked());
				if (connector.gui_do() == 1)
				{
				const network::manager net_manager;
				const network::server_manager server_man(15000,server);
	
				config level = connector.get_level();
				const bool network_state = accept_network_connections(disp,level);
				if(network_state == false)
					return -1;

				state.starting_pos = level;
	
				recorder.set_save_info(state);

				//see if we should show the replay of the game so far
				if(!recorder.empty()) {
					if(false) {
						recorder.set_skip(0);
					} else {
						std::cerr << "skipping...\n";
						recorder.set_skip(-1);
					}
				}

				//any replay data isn't meant to hang around under the level,
				//it was just there to tell clients about the replay data
				level.clear_children("replay");
				std::vector<config*> story;
				play_level(units_data,cfg,&level,disp.video(),state,story);
				recorder.clear();
				}
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
			cur_selection = maps_menu.selection();
			if(size_t(maps_menu.selection()) != options.size()-1) {
				level_ptr = levels[maps_menu.selection()];

				std::string map_data = (*level_ptr)["map_data"];
				if(map_data == "" && (*level_ptr)["map"] != "") {
					map_data = read_file("data/maps/" + (*level_ptr)["map"]);
				}

				//if the map should be randomly generated
				if(map_data == "" && (*level_ptr)["map_generation"] != "") {
					map_data = random_generate_map((*level_ptr)["map_generation"]);

					//record the map data of the map, so that when we send to
					//remote clients, they will use the given map, and won't try
					//to generate their own.
					(*level_ptr)["map_data"] = map_data;
				}

				gamemap map(cfg,map_data);

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
				SDL_BlitSurface(playernum_bg, NULL, disp.video().getSurface(), &rect);
				config& level = *level_ptr;
				const config::child_itors sides = level.child_range("side");
				int sides_num;
				config::child_iterator sd;
				for(sd = sides.first; sd != sides.second; ++sd) {
					sides_num = sd - sides.first;
				}
				std::stringstream players;
				players << "Players: " << sides_num + 1;
				font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
					                players.str(),rect.x+45,rect.y);
				update_rect(rect);
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
				SDL_BlitSurface(playernum_bg, NULL, disp.video().getSurface(), &rect);
				font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
					                " Load Map ",rect.x+45,rect.y);
				update_rect(rect);
			}
		}

		events::pump();
		disp.video().flip();
		SDL_Delay(20);
	}
	return -1;
}
