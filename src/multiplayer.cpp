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
#include "multiplayer.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
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
	std::vector<config*>& sides = players.children["side"];
	for(std::vector<config*>::const_iterator i = sides.begin();
	    i != sides.end(); ++i) {
		if((*i)->values["controller"] == "network") {
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

	std::vector<config*>& sides = players_.children["side"];

	try {
		sock = network::receive_data(cfg);
	} catch(network::error& e) {

		sock = 0;

		//if the problem isn't related to any specific connection,
		//it's a general error and we should just re-throw the error
		//likewise if we are not a server, we cannot afford any connection
		//to go down, so also re-throw the error
		if(!e.socket || !network::is_server()) {
			throw e;
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
	start_game.children["start_game"].push_back(new config());
	network::send_data(start_game);

	return true;
}

}

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
	int cur_villagegold = 2;
	int new_villagegold = 2;
	int cur_turns = 50;
	int new_turns = 50;

	gui::draw_dialog_frame((disp.x()-width)/2, (disp.y()-height)/2,
			       width, height, disp);

	//Title
	font::draw_text(&disp,disp.screen_area(),24,font::NORMAL_COLOUR,
	                "Create Game",-1,(disp.y()-height)/2+5);

	//Name Entry
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                "Name of game:",(disp.x()-width)/2+10,(disp.y()-height)/2+38);
	gui::textbox name_entry(disp,width-20);
	name_entry.set_location((disp.x()-width)/2+10,(disp.y()-height)/2+55);

	//Maps
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                "Map to play:",(disp.x()-width)/2+(int)(width*0.4),
			(disp.y()-height)/2+83);
	std::vector<std::string> options;
	std::vector<config*>& levels = cfg.children["multiplayer"];
	std::map<int,std::string> res_to_id;
	for(std::vector<config*>::iterator i = levels.begin(); i!=levels.end();++i){
		const std::string& id = (**i)["id"];
		res_to_id[i - levels.begin()] = id;

		const std::string& lang_name = string_table[id];
		if(lang_name.empty() == false)
			options.push_back(lang_name);
		else
			options.push_back((**i)["name"]);
	}

	options.push_back("Load game...");
	gui::menu maps_menu(disp,options);
	maps_menu.set_loc((disp.x()-width)/2+(int)(width*0.4),(disp.y()-height)/2+100);

	//Game Turns
	rect.x = (disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19;
	rect.y = (disp.y()-height)/2+83;
	rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19)-10;
	rect.h = 12;
	SDL_Surface* village_bg=get_surface_portion(disp.video().getSurface(), rect);
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                "Turns: 50",rect.x,rect.y);
	rect.y = (disp.y()-height)/2+100;
	rect.h = name_entry.width();

	gui::slider turns_slider(disp,rect,0.38);

	//Village Gold
	rect.x = (disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19;
	rect.y = (disp.y()-height)/2+130;
	rect.w = ((disp.x()-width)/2+width)-((disp.x()-width)/2+(int)(width*0.4)+maps_menu.width()+19)-10;
	rect.h = 12;
	//SDL_Surface* village_bg=get_surface_portion(disp.video().getSurface(), rect);
	font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
	                "Village Gold: 2",rect.x,rect.y);
	rect.y = (disp.y()-height)/2+147;
	rect.h = name_entry.width();
	gui::slider villagegold_slider(disp,rect,0.1);

	//FOG of war
	gui::button fog_game(disp,"Fog Of War",gui::button::TYPE_CHECK);
	fog_game.set_check(false);
	fog_game.set_xy(rect.x+6,rect.y+30);
	fog_game.draw();

	//Shroud
	gui::button shroud_game(disp,"Shroud",gui::button::TYPE_CHECK);
	shroud_game.set_check(false);
	shroud_game.set_xy(rect.x+6,rect.y+60);
	shroud_game.draw();

	//Buttons
	gui::button cancel_game(disp,string_table["cancel_button"]);
	gui::button launch_game(disp,string_table["ok_button"]);
	launch_game.set_xy((disp.x()/2)-launch_game.width()*2-19,(disp.y()-height)/2+height-29);
	cancel_game.set_xy((disp.x()/2)+cancel_game.width()+19,(disp.y()-height)/2+height-29);

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

		if(cancel_game.process(mousex,mousey,left_button)) 
			return -1;

		if(launch_game.process(mousex,mousey,left_button)) {
			if(name_entry.text().empty() == false) {

				// Send Initial information
				config response;
				config create_game;
				create_game["name"] = name_entry.text();
				response.children["create_game"].push_back(
					new config(create_game));
				network::send_data(response);


				// Setup the game
				int res = maps_menu.selection();

				if(res == -1)
					break;

				//if we're loading a saved game
				config loaded_level;
				if(size_t(res) == options.size()-1) {
					const std::vector<std::string>& games = get_saves_list();
					if(games.empty()) {
						gui::show_dialog(disp,NULL,
						                 string_table["no_saves_heading"],
								 string_table["no_saves_message"],
						                 gui::OK_ONLY);
						break;
					}

					const int res = gui::show_dialog(disp,NULL,
			                                 string_table["load_game_heading"],
			                                 string_table["load_game_message"],
			                                 gui::OK_CANCEL, &games);
					if(res == -1)
						break;

					load_game(units_data,games[res],state);

					if(state.campaign_type != "multiplayer") {
						gui::show_dialog(disp,NULL,"",
				                 "This is not a multiplayer save",gui::OK_ONLY);
						break;
					}

					if(state.version != game_config::version) {
						const int res = gui::show_dialog(disp,NULL,"",
					                        string_table["version_save_message"],
					                        gui::YES_NO);
						if(res == 1)
							break;
					}

					loaded_level = state.starting_pos;
					level_ptr = &loaded_level;

					//make all sides untaken
					for(config::child_itors i = level_ptr->child_range("side");
					    i.first != i.second; ++i.first) {
						(**i.first)["taken"] = "";
					}

					recorder = replay(state.replay_data);

					//add the replay data under the level data so clients can
					//receive it
					level_ptr->children["replay"].clear();
					level_ptr->add_child("replay") = state.replay_data;

				} else { //creating a new game
					level_ptr = levels[res];

					//set the number of turns here
					std::stringstream turns;
					turns << new_turns;
					(*level_ptr)["turns"] = turns.str();
				}

				assert(level_ptr != NULL);

				config& level = *level_ptr;
				state.label = level.values["name"];

				state.scenario = res_to_id[res];

				std::vector<config*>& sides = level.children["side"];
				std::vector<config*>& possible_sides = cfg.children["multiplayer_side"];
				if(sides.empty() || possible_sides.empty()) {
					std::cerr << "no multiplayer sides found\n";
					return -1;
				}

				for(std::vector<config*>::iterator sd = sides.begin();
				    sd != sides.end(); ++sd) {

					if((**sd)["fog"].empty())
						(**sd)["fog"] = fog_game.checked() ? "yes" : "no";

					if((**sd)["shroud"].empty())
						(**sd)["shroud"] = shroud_game.checked() ? "yes" : "no";

					if((**sd)["name"].empty())
						(**sd)["name"] = possible_sides.front()->values["name"];

					if((**sd)["type"].empty())
						(**sd)["type"] = possible_sides.front()->values["type"];

					if((**sd)["recruit"].empty())
						(**sd)["recruit"]=possible_sides.front()->values["recruit"];

					if((**sd)["music"].empty())
						(**sd)["music"] = possible_sides.front()->values["music"];

					if((**sd)["recruitment_pattern"].empty())
						(**sd)["recruitment_pattern"] =
					        possible_sides.front()->values["recruitment_pattern"];

					if((**sd)["description"].empty())
						(**sd)["description"] = preferences::login();
				}
	
				res = 0;
				while(size_t(res) != sides.size()) {
					std::vector<std::string> sides_list;
					for(std::vector<config*>::iterator sd = sides.begin();
					    sd != sides.end(); ++sd) {
						std::stringstream details;
						details << (*sd)->values["side"] << ","
							<< (*sd)->values["name"] << ",";

						const std::string& controller = (*sd)->values["controller"];
						if(controller == "human")
							details << string_table["human_controlled"];
						else if(controller == "network")
							details << string_table["network_controlled"];
						else
							details << string_table["ai_controlled"];

						sides_list.push_back(details.str());
					}
	
					sides_list.push_back(string_table["start_game"]);
	
					res = gui::show_dialog(disp,NULL,"",string_table["configure_sides"],
					                       gui::MESSAGE,&sides_list);
	
					if(size_t(res) < sides.size()) {
						std::vector<std::string> choices;
	
						for(int n = 0; n != 3; ++n) {
							for(std::vector<config*>::iterator i = possible_sides.begin();
							    i != possible_sides.end(); ++i) {
								std::stringstream choice;
								choice << (*i)->values["name"] << " - ";
								switch(n) {
									case 0: choice << string_table["human_controlled"];
									        break;
									case 1: choice << string_table["ai_controlled"];
									        break;
									case 2: choice << string_table["network_controlled"];
									        break;
									default: assert(false);
								}
							
								choices.push_back(choice.str());
							}
						}
	
						int result = gui::show_dialog(disp,NULL,"",
				                           string_table["choose_side"],
							   gui::MESSAGE,&choices);
						if(result >= 0) {
							std::string controller = "network";
							if(result < int(choices.size())/3) {
								controller = "human";
								sides[res]->values["description"] = preferences::login();
							} else if(result < int(choices.size()/3)*2) {
								controller = "ai";
								result -= choices.size()/3;
								sides[res]->values["description"] = "";
							} else {
								controller = "network";
								result -= (choices.size()/3)*2;
							}
	
							sides[res]->values["controller"] = controller;
	
							assert(result < int(possible_sides.size()));
	
							std::map<std::string,std::string>& values =
					                                possible_sides[result]->values;
							sides[res]->values["name"] = values["name"];
							sides[res]->values["type"] = values["type"];
							sides[res]->values["recruit"] = values["recruit"];
							sides[res]->values["recruitment_pattern"] =
					                          values["recruitment_pattern"];
							sides[res]->values["music"] = values["music"];
						}
					}
				}

				const network::manager net_manager;
				const network::server_manager server_man(15000,server);
	
				const bool network_state = accept_network_connections(disp,level);
				if(network_state == false)
					return -1;

				state.starting_pos = level;
	
				recorder.set_save_info(state);

				if(!recorder.empty()) {
					const int res = gui::show_dialog(disp,NULL,
			               "", string_table["replay_game_message"],
						   gui::YES_NO);
					//if yes, then show the replay, otherwise
					//skip showing the replay
					if(res == 0) {
						recorder.set_skip(0);
					} else {
						std::cerr << "skipping...\n";
						recorder.set_skip(-1);
					}
				}

				//any replay data isn't meant to hang around under the level,
				//it was just there to tell clients about the replay data
				level.children["replay"].clear();

				std::vector<config*> story;
				play_level(units_data,cfg,&level,disp.video(),state,story);
				recorder.clear();
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
		name_entry.process();

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
			sprintf(buf,"Village Gold: %d", cur_villagegold);
			font::draw_text(&disp,disp.screen_area(),12,font::GOOD_COLOUR,
			                buf,rect.x,rect.y);
			update_rect(rect);
		}
		
		if(maps_menu.selection() != cur_selection) {
			cur_selection = maps_menu.selection();
			if(size_t(maps_menu.selection()) != options.size()-1) {
				level_ptr = levels[maps_menu.selection()];

				gamemap map(cfg,read_file("data/maps/" + level_ptr->values["map"]));

				SDL_Surface* const mini = image::getMinimap(175,175,map);

				if(mini != NULL) {
					rect.x = ((disp.x()-width)/2+10)+20;
					rect.y = (disp.y()-height)/2+80;
					rect.w = 175;
					rect.h = 175;
					SDL_BlitSurface(mini, NULL, disp.video().getSurface(), &rect);
					SDL_FreeSurface(mini);
					update_rect(rect);
				}
			}else{
				//TODO: Load some other non-minimap
				//      image, ie a floppy disk
			}
		}

		events::pump();
		disp.video().flip();
		SDL_Delay(20);
	}
	return -1;
}
