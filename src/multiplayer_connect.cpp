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
#include "multiplayer_client.hpp"
#include "multiplayer_connect.hpp"
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

mp_connect::mp_connect(display& disp, std::string game_name,
		       config &cfg, game_data& data, game_state& state,
		       bool join) : 
	    disp_(&disp), cfg_(&cfg), data_(&data), state_(&state),
	    show_replay_(false), save_(false), status_(0), join_(join),
	    player_types_(), player_races_(), player_teams_(),
	    player_colors_(), combos_type_(), combos_race_(),
	    combos_team_(), combos_color_(), sliders_gold_(),
	    launch_(gui::button(disp, string_table["im_ready"])),
	    cancel_(gui::button(disp, string_table["cancel"])),
	    ai_(gui::button(disp, " Computer vs Computer ")),
	    width_(630), height_(290), full_(false)
{
	// Send Initial information
	config response;
	config& create_game = response.add_child("create_game");
	create_game["name"] = game_name;
	network::send_data(response);

}

mp_connect::~mp_connect()
{
	if(network::nconnections() > 0) {
		config cfg;
		cfg.add_child("leave_game");
		network::send_data(cfg);
	}
}

int mp_connect::load_map(int map, int num_turns, int village_gold,
			 bool fog_game, bool shroud_game)
{
	log_scope("load_map");
	// Setup the game
	config* level_ptr;

	const config::child_list& levels = cfg_->get_children("multiplayer");

	if(map == levels.size() )
	{
		//Load a saved game
		save_ = true;
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(*disp_, &show_replay);
		if(game == "")
		{
			gui::show_dialog(*disp_, NULL, "", 
					 "Sae Game Error, game == \"\"",
					 gui::OK_ONLY);
			status_ = -1;
			return status_;
		}

		log_scope("loading save");

		load_game(*data_, game, *state_);

		state_->gold = -10000;
		state_->available_units.clear();
		state_->can_recruit.clear();

		if(state_->campaign_type != "multiplayer") {
			gui::show_dialog(*disp_, NULL, "", 
					 string_table["not_multiplayer_save_message"],
					 gui::OK_ONLY);
			status_ = -1;
			return status_;
		}

		if(state_->version != game_config::version) {
			const int res = gui::show_dialog(*disp_, NULL, "",
						string_table["version_save_message"],
						gui::YES_NO);
			if(res == 1) {
				status_ = -1;
				return status_;
			}
		}

		loaded_level_ = state_->starting_pos;
		level_ptr = &loaded_level_;

		//make all sides untaken
		for(config::child_itors i = level_ptr->child_range("side");
		    i.first != i.second; ++i.first) {
			(**i.first)["taken"] = "";

			//tell clients not to change their race
			(**i.first)["allow_changes"] = "no";
		}

		recorder = replay(state_->replay_data);

		config* const start = level_ptr->child("start");

		//if this is a snapshot save, we don't want to use the replay data
		if(loaded_level_["snapshot"] == "yes") {
			if(start != NULL)
				start->clear_children("replay");
			level_ptr->clear_children("replay");
			recorder.set_to_end();
		} else {
			//add the replay data under the level data so clients can
			//receive it
			level_ptr->clear_children("replay");
			level_ptr->add_child("replay") = state_->replay_data;
		}

	} else {
		//Load a new map
		save_ = false;
		level_ptr = levels[map];

		//set the number of turns here
		std::stringstream turns;
		turns << num_turns;
		(*level_ptr)["turns"] = turns.str();
	}

	assert(level_ptr != NULL);

	level_ = level_ptr;
	state_->label = level_->values["name"];

	std::map<int,std::string> res_to_id;
	for(config::child_list::const_iterator i = levels.begin(); i != levels.end(); ++i){
		const std::string& id = (**i)["id"];
		res_to_id[i - levels.begin()] = id;
	}

	state_->scenario = res_to_id[map];

	const config::child_itors sides = level_->child_range("side");
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");

	if(sides.first == sides.second || possible_sides.empty()) {
		gui::show_dialog(*disp_, NULL, "", 
				 "No multiplayer sides found.",
				 gui::OK_ONLY);
		std::cerr << "no multiplayer sides found\n";
		status_ = -1;
		return status_;
	}

	config::child_iterator sd;
	bool first = true;
	for(sd = sides.first; sd != sides.second; ++sd) {
		if(save_ == false)
		{
			std::stringstream svillage_gold;
			svillage_gold << village_gold;
			(**sd)["village_gold"] = svillage_gold.str();
			(**sd)["gold"] = "100";
			if (first == true) {
				(**sd)["controller"] = "human";
				(**sd)["description"] = preferences::login();
				first = false;
			} else {
				(**sd)["controller"] = "network";
				(**sd)["description"] = "";
			}
		}

		if((**sd)["fog"].empty())
			(**sd)["fog"] = fog_game ? "yes" : "no";

		if((**sd)["shroud"].empty())
			(**sd)["shroud"] = shroud_game ? "yes" : "no";

		if((**sd)["name"].empty())
			(**sd)["name"] = (*possible_sides.front())["name"];

		if((**sd)["type"].empty())
			(**sd)["type"] = (*possible_sides.front())["type"];

		if((**sd)["recruit"].empty())
			(**sd)["recruit"] = (*possible_sides.front())["recruit"];

		if((**sd)["music"].empty())
			(**sd)["music"] = (*possible_sides.front())["music"];

		if((**sd)["recruitment_pattern"].empty())
			(**sd)["recruitment_pattern"] =
		        possible_sides.front()->values["recruitment_pattern"];
	}

	if ((*level_)["objectives"] == "")
	{
		(*level_)["objectives"] = "Victory:\n@Defeat enemy leader(s)\n";
	}

	lists_init();
	gui_init();
	status_ = 0;

	//if we have any connected players when we are created, send them the data
	network::send_data(*level_);

	return status_;
}

void mp_connect::lists_init()
{
	//Options
	player_types_.push_back(string_table["network_controlled"]);
	player_types_.push_back(string_table["human_controlled"]);
	player_types_.push_back(string_table["ai_controlled"]);
	player_types_.push_back("-----");
	player_types_.push_back(preferences::login());

	//Races
	const config::child_itors sides = level_->child_range("side");
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	for(std::vector<config*>::const_iterator race = possible_sides.begin();
	    race != possible_sides.end(); ++race) {
		player_races_.push_back(translate_string((**race)["name"]));
	}

	//Teams
	config::child_iterator sd;
	height_ = 120;
	for(sd = sides.first; sd != sides.second; ++sd) {
		height_ = height_ + 30;
		const int team_num = sd - sides.first;
		const std::string& team_name = (**sd)["team_name"];
		std::stringstream str;
		str << string_table["team"] << " ";

		if(team_name.empty() == false)
			str << team_name;
		else
			str << (team_num+1);
		player_teams_.push_back(str.str());
	}

	//Colors
	player_colors_.push_back(string_table["red"]);
	player_colors_.push_back(string_table["blue"]);
	player_colors_.push_back(string_table["green"]);
	player_colors_.push_back(string_table["yellow"]);
	player_colors_.push_back(string_table["pink"]);
	player_colors_.push_back(string_table["purple"]);
}

void mp_connect::add_player(const std::string& name)
{
	player_types_.push_back(name);

	for(size_t n = 0; n != combos_type_.size(); ++n) {
		combos_type_[n].set_items(player_types_);
	}
}

void mp_connect::remove_player(const std::string& name)
{
	const std::vector<std::string>::iterator itor = std::find(player_types_.begin(),player_types_.end(),name);
	if(itor != player_types_.end())
		player_types_.erase(itor);
}

void mp_connect::gui_init()
{

	// Wait to players, Configure players
	gui::draw_dialog_frame((disp_->x()-width_)/2, (disp_->y()-height_)/2,
			       width_, height_, *disp_);

	//Buttons
	launch_.set_xy((disp_->x()/2)-launch_.width()/2-100,
		       (disp_->y()-height_)/2+height_-30);
	cancel_.set_xy((disp_->x()/2)-launch_.width()/2+100,
		       (disp_->y()-height_)/2+height_-30);
	ai_.set_xy((disp_->x()-width_)/2+30,
		   (disp_->y()-height_)/2+height_-60);

	//Title and labels
	SDL_Rect labelr;
	font::draw_text(disp_,disp_->screen_area(),24,font::NORMAL_COLOUR,
	                string_table["game_lobby"],-1,(disp_->y()-height_)/2+5);
	labelr.x=0; labelr.y=0; labelr.w=disp_->x(); labelr.h=disp_->y();
	labelr = font::draw_text(NULL,labelr,14,font::GOOD_COLOUR,
			string_table["player_type"],0,0);
	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["player_type"],((disp_->x()-width_)/2+30)+(launch_.width()/2)-(labelr.w/2),
			(disp_->y()-height_)/2+35);
	labelr.x=0; labelr.y=0; labelr.w=disp_->x(); labelr.h=disp_->y();
	labelr = font::draw_text(NULL,labelr,14,font::GOOD_COLOUR,
			string_table["race"],0,0);
	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["race"],((disp_->x()-width_)/2+145)+(launch_.width()/2)-(labelr.w/2),
			(disp_->y()-height_)/2+35);
	labelr.x=0; labelr.y=0; labelr.w=disp_->x(); labelr.h=disp_->y();
	labelr = font::draw_text(NULL,labelr,14,font::GOOD_COLOUR,
			string_table["team"],0,0);
	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["team"],((disp_->x()-width_)/2+260)+(launch_.width()/2)-(labelr.w/2),
			(disp_->y()-height_)/2+35);
	labelr.x=0; labelr.y=0; labelr.w=disp_->x(); labelr.h=disp_->y();
	labelr = font::draw_text(NULL,labelr,14,font::GOOD_COLOUR,
			string_table["color"],0,0);
	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["color"],((disp_->x()-width_)/2+375)+(launch_.width()/2)-(labelr.w/2),
			(disp_->y()-height_)/2+35);
	labelr.x=0; labelr.y=0; labelr.w=disp_->x(); labelr.h=disp_->y();
	labelr = font::draw_text(NULL,labelr,14,font::GOOD_COLOUR,
			string_table["gold"],0,0);
	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["gold"],((disp_->x()-width_)/2+480)+(launch_.width()/2)-(labelr.w/2),
			(disp_->y()-height_)/2+35);

	//Per player settings
	const config::child_itors sides = level_->child_range("side");
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	config::child_iterator sd;
	SDL_Rect rect;

	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = sd - sides.first;

		//Player number
		font::draw_text(disp_,disp_->screen_area(), 24, font::GOOD_COLOUR,
		                (*sd)->values["side"], (disp_->x()-width_)/2+10,
				(disp_->y()-height_)/2+53+(30*side_num));

		//Player type
		combos_type_.push_back(gui::combo(*disp_, player_types_));
		combos_type_.back().set_xy((disp_->x()-width_)/2+30,
					   (disp_->y()-height_)/2+55+(30*side_num));

		//Player race
		combos_race_.push_back(gui::combo(*disp_, player_races_));
		combos_race_.back().set_xy((disp_->x()-width_)/2+145,
					   (disp_->y()-height_)/2+55+(30*side_num));

		//Player team
		combos_team_.push_back(gui::combo(*disp_, player_teams_));
		combos_team_.back().set_xy((disp_->x()-width_)/2+260,
					   (disp_->y()-height_)/2+55+(30*side_num));
		combos_team_.back().set_selected(side_num);

		//Player color
		combos_color_.push_back(gui::combo(*disp_, player_colors_));
		combos_color_.back().set_xy((disp_->x()-width_)/2+375,
					    (disp_->y()-height_)/2+55+(30*side_num));
		combos_color_.back().set_selected(side_num);

		//Player gold
		rect.x = (disp_->x()-width_)/2+490;
		rect.y = (disp_->y()-height_)/2+55+(30*side_num);
		rect.w = launch_.width()-5;
		rect.h = launch_.height();
		sliders_gold_.push_back(gui::slider(*disp_, rect, 0.0+((80.0)/979.0)));
		rect.w = 30;
		rect.x = (disp_->x()-width_)/2+603;
		gold_bg_.push_back(surface_restorer(&disp_->video(),rect));
		font::draw_text(disp_, disp_->screen_area(), 12, font::GOOD_COLOUR,
		                "100", rect.x, rect.y);
	}

	update_whole_screen();
}

void mp_connect::gui_update()
{
	//Update the GUI based on current config settings.
	//Settings may change based on other networked
	//players.

	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	const config::child_itors sides = level_->child_range("side");
	SDL_Rect rect;

	for(size_t n = 0; n != combos_type_.size(); ++n) {
		config& side = **(sides.first+n);

		//Player type
		if (side["controller"] == "network") {
			if (side["description"] == "") {
				combos_type_[n].set_selected(0);
			} else if (side["description"] == "Computer Player") {
				combos_type_[n].set_selected(2);
			} else {
				for (size_t m = 0; m != player_types_.size(); ++m) {
					if (side["description"] == player_types_[m]) {
						combos_type_[n].set_selected(m);
					}
				}
			}
		} else if (side["controller"] == "human") {
			if (side["description"] == preferences::login()) {
				combos_type_[n].set_selected(4);
			} else {
				combos_type_[n].set_selected(1);
			}
		} else if (side["controller"] == "ai") {
			combos_type_[n].set_selected(2);
		}

		//Player Race
		for (size_t m = 0; m != player_races_.size(); ++m) {
			if (translate_string(side["name"]) == player_races_[m]) {
				combos_race_[n].set_selected(m);
			}
		}

		//Player Team
		const std::string& team_name = side["team_name"];
		if(team_name != "" && isdigit(team_name[0]))
			combos_team_[n].set_selected(team_name[0] - '1');

		//Player Color

		//Player Gold
		std::string str = side["gold"];
		sliders_gold_[n].set_value((atoi(str.c_str()) - 20 + 0.0) / 979.0);
		rect.x = (disp_->x() - width_) / 2 + 603;
		rect.y = (disp_->y() - height_) / 2 + 55 + (30 * n);
		rect.w = 30;
		rect.h = launch_.height();
		gold_bg_[n].restore();
		font::draw_text(disp_, disp_->screen_area(), 12,
				font::GOOD_COLOUR,
		                side["gold"],
				rect.x, rect.y);
		update_rect(rect);

	}
}

int mp_connect::gui_do()
{
	SDL_Rect rect;
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	const config::child_itors sides = level_->child_range("side");
	int new_playergold = -1;
	int cur_playergold = -1;

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		for(size_t n = 0; n != combos_team_.size(); ++n) {
			config& side = **(sides.first+n);

			//Player type
			//Don't let user change this if a player is sitting
			combos_type_[n].enable(combos_type_[n].selected() < 4);

			int old_select = combos_type_[n].selected();
			if(combos_type_[n].process(mousex, mousey, left_button)) {
				if(combos_type_[n].selected() == 0) {
					side["controller"] = "network";
					side["description"] = "";
				}else if(combos_type_[n].selected() == 1){
					side["controller"] = "human";
					side["description"] = "";
				}else if(combos_type_[n].selected() == 2){
					side["controller"] = "ai";
					side["description"] = string_table["ai_controlled"];
				}else if(combos_type_[n].selected() == 3){
					combos_type_[n].set_selected(old_select);
				}else if(combos_type_[n].selected() == 4){
					side["controller"] = "human";
					side["description"] = preferences::login();
					for(size_t m = 0; m != combos_type_.size(); ++m) {
						if(m != n) {
							if(combos_type_[m].selected() == 4){
								combos_type_[m].set_selected(0);
								config& si = **(sides.first+m);
								si["controller"] = "network";
								si["description"] = "";
							}
						}
					}
				}else{
					side["controller"] = "network";
					side["description"] = "";
				}
				network::send_data(*level_);
			}

			//Player race
			combos_race_[n].enable(!save_);
			combos_team_[n].enable(!save_);
			combos_color_[n].enable(!save_);
			
			if(combos_race_[n].process(mousex, mousey, left_button)) {
				const string_map& values =  possible_sides[combos_race_[n].selected()]->values;
				for(string_map::const_iterator i = values.begin(); i != values.end(); ++i) {
					side[i->first] = i->second;
				}
				network::send_data(*level_);
			}

			//Player team
			if(combos_team_[n].process(mousex, mousey, left_button)) {
				std::stringstream str;
				str << (combos_team_[n].selected()+1);
				side["team_name"] = str.str();
				network::send_data(*level_);
			}

			if(combos_color_[n].process(mousex, mousey, left_button)) {
				network::send_data(*level_);
			}

			if(!save_){
				int check_playergold = 20 + int(979 *
					sliders_gold_[n].process(mousex, mousey, left_button));
				if(abs(check_playergold) == check_playergold)
					new_playergold = check_playergold;
				if(new_playergold != cur_playergold) {
					cur_playergold = new_playergold;
					std::stringstream playergold;
					playergold << cur_playergold;
					side["gold"] = playergold.str();
					rect.x = (disp_->x() - width_) / 2 + 603;
					rect.y = (disp_->y() - height_) / 2 + 55 + (30 * n);
					rect.w = 30;
					rect.h = launch_.height();
					gold_bg_[n].restore();
					font::draw_text(disp_, disp_->screen_area(), 12,
							font::GOOD_COLOUR,
					                (*sides.first[n])["gold"],
							rect.x, rect.y);
					update_rect(rect);
					network::send_data(*level_);
				}
			}else{
				sliders_gold_[n].draw();
			}
		}

		if(cancel_.process(mousex,mousey,left_button)) {
			if(network::nconnections() > 0) {
				config cfg;
				cfg.add_child("leave_game");
				network::send_data(cfg);
			}
			status_ = 0;
			return status_;
		}

		if(ai_.process(mousex,mousey,left_button)) {
			for(size_t m = 0; m != combos_team_.size(); ++m) {
				config& si = **(sides.first+m);
				si["controller"] = "ai";
				si["description"] = string_table["ai_controlled"];
				combos_type_[m].set_selected(2);
			}
			network::send_data(*level_);
		}

		launch_.enable(full_);

		if(launch_.process(mousex,mousey,left_button)) {
			//Tell everyone to start
			config cfg;
			cfg.add_child("start_game");
			network::send_data(cfg);

			state_->starting_pos = *level_;
	
			recorder.set_save_info(*state_);

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
			level_->clear_children("replay");
			std::vector<config*> story;
			play_level(*data_, *cfg_, level_, disp_->video(), *state_, story);
			recorder.clear();

			if(network::nconnections() > 0) {
				config cfg;
				cfg.add_child("leave_game");
				network::send_data(cfg);
			}

			status_ = 0;
			return status_;
		}

		gui_update();
		update_positions();
		update_network();
		is_full();

		events::pump();
		disp_->video().flip();
		SDL_Delay(20);
	}

	return status_;
}

void mp_connect::update_positions()
{
	const config::child_itors sides = level_->child_range("side");
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	config::child_iterator sd;
	for(sd = sides.first; sd != sides.second; ++sd) {
		if((**sd)["taken"] != "yes") {
			positions_[*sd] = 0;
		}
	}
}

void mp_connect::update_network()
{
	for(std::map<config*,network::connection>::const_iterator i = positions_.begin();
	    i != positions_.end(); ++i) {
		if(!i->second) {
			//We are waiting on someone
			network::connection sock = network::accept_connection();
			if(sock) {
				std::cerr << "Received connection\n";
				network::send_data(*level_,sock);
			}
		}
	}

	config cfg;
	const config::child_list& sides = level_->get_children("side");

	network::connection sock;

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
		for(std::map<config*,network::connection>::iterator i = positions_.begin();
		    i != positions_.end(); ++i) {
			if(i->second == e.socket) {
				changes = true;
				i->second = 0;
				i->first->values.erase("taken");

				remove_player(i->first->values["description"]);
				i->first->values["description"] = "";
			}
		}

		//now disconnect the socket
		e.disconnect();

		//if there have been changes to the positions taken,
		//then notify other players
		if(changes) {
			network::send_data(*level_);
		}
	}

	//No network errors
	if(sock) {
		const int side_drop = atoi(cfg["side_drop"].c_str())-1;
		if(side_drop >= 0 && side_drop < int(sides.size())) {
			std::map<config*,network::connection>::iterator pos = positions_.find(sides[side_drop]);
			if(pos != positions_.end()) {
				pos->second = 0;
				pos->first->values.erase("taken");
				pos->first->values["description"] = "";
				network::send_data(*level_);
			}
		}

		const int side_taken = atoi(cfg["side"].c_str())-1;
		if(side_taken >= 0 && side_taken < int(sides.size())) {
			std::map<config*,network::connection>::iterator pos = positions_.find(sides[side_taken]);
			if(pos != positions_.end()) {
				if(!pos->second || pos->second == sock) {
					std::cerr << "client has taken a valid position\n";

					//does the client already own the side, and is just updating
					//it, or is it taking a vacant slot?
					const bool update_only = pos->second == sock;

					//broadcast to everyone the new game status
					pos->first->values["controller"] = "network";
					pos->first->values["taken"] = "yes";
					pos->first->values["description"] = cfg["description"];
					pos->first->values["name"] = cfg["name"];
					pos->first->values["type"] = cfg["type"];
					pos->first->values["recruit"] = cfg["recruit"];
					pos->first->values["music"] = cfg["music"];
					pos->second = sock;
					network::send_data(*level_);

					std::cerr << "sent player data\n";

					//send a reply telling the client they have secured
					//the side they asked for
					std::stringstream side;
					side << (side_taken+1);
					config reply;
					reply.values["side_secured"] = side.str();
					std::cerr << "going to send data...\n";
					network::send_data(reply,sock);

					// Add to combo list
					add_player(cfg["description"]);
				} else {
					config response;
					response.values["failed"] = "yes";
					network::send_data(response,sock);
				}
			} else {
				std::cerr << "tried to take illegal side: " << side_taken << "\n";
			}
		} else {
			std::cerr << "tried to take unknown side: " << side_taken << "\n";
		}
	}
}

void mp_connect::is_full()
{
	//see if all positions are now filled
	full_ = true;
	const config::child_itors sides = level_->child_range("side");
	const config::child_list& possible_sides = cfg_->get_children("multiplayer_side");
	config::child_iterator sd;
	for(sd = sides.first; sd != sides.second; ++sd) {
		if((**sd)["controller"] == "network" &&
		   (**sd)["description"] == "") {
			if(positions_[*sd] == 0) {
				full_ = false;
			}
		}
	}
}
