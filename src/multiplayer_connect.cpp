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
#include "util.hpp"
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
		       const config &cfg, game_data& data, game_state& state,
		       bool join) : 
	    disp_(&disp), cfg_(&cfg), data_(&data), state_(&state),
	    show_replay_(false), save_(false), join_(join),
	    player_types_(), player_races_(), player_teams_(),
	    player_colors_(), combos_type_(), combos_race_(),
	    combos_team_(), combos_color_(), sliders_gold_(),
	    launch_(gui::button(disp, string_table["im_ready"])),
	    cancel_(gui::button(disp, string_table["cancel"])),
	    ai_(gui::button(disp, string_table["ai_players"])),
		message_full_(true)
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

int mp_connect::load_map(const std::string& era, config& scenario_data, int num_turns, int village_gold, int xpmodifier,
                         bool fog_game, bool shroud_game, bool allow_observers, bool share_view, bool share_maps)
{
	log_scope("load_map");
	// Setup the game
	config* level_ptr;

	era_ = era;

	if(scenario_data.child("side") == NULL) {
		//Load a saved game
		save_ = true;
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(*disp_, *cfg_, *data_, &show_replay);
		if(game == "") {
			return -1;
		}

		log_scope("loading save");

		load_game(*data_, game, *state_);

		state_->available_units.clear();
		state_->can_recruit.clear();

		if(state_->campaign_type != "multiplayer") {
			gui::show_dialog(*disp_, NULL, "", 
					 string_table["not_multiplayer_save_message"],
					 gui::OK_ONLY);
			return -1;
		}

		if(state_->version != game_config::version) {
			const int res = gui::show_dialog(*disp_, NULL, "",
						string_table["version_save_message"],
						gui::YES_NO);
			if(res == 1) {
				return -1;
			}
		}

		scenario_data = state_->snapshot;
		level_ptr = &scenario_data;

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
		if(scenario_data["snapshot"] == "yes") {
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
		level_ptr = &scenario_data;

		//set the number of turns here
		std::stringstream turns;
		turns << num_turns;
		(*level_ptr)["turns"] = turns.str();
	}

	assert(level_ptr != NULL);

	//this will force connecting clients to be using the same version number as us.
	(*level_ptr)["version"] = game_config::version;

	level_ = level_ptr;
	state_->label = level_->values["name"];
	state_->gold = -10000;

	state_->scenario = scenario_data["id"];

	level_->values["observer"] = allow_observers ? "yes" : "no";

	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);

	if(era_cfg == NULL) {
		std::cerr << "ERROR: era '" << era_ << "' not found\n";
		return -1;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	if(sides.first == sides.second || possible_sides.empty()) {
		gui::show_dialog(*disp_, NULL, "", 
				 string_table["error_no_mp_sides"],
				 gui::OK_ONLY);
		std::cerr << "no multiplayer sides found\n";
		return -1;
	}

	config::child_iterator sd;

	bool first = true;
	for(sd = sides.first; sd != sides.second; ++sd) {
		config& side = (**sd);

		if(save_ == false)
		{
			std::stringstream svillage_gold;
			svillage_gold << village_gold;
			side["village_gold"] = svillage_gold.str();
			side["gold"] = "100";
			if (first == true) {
				(**sd)["controller"] = "human";
				(**sd)["description"] = preferences::login();
				first = false;
			} else {
				(**sd)["controller"] = "ai";
				(**sd)["description"] = "";
			}
		}

		if(side["fog"].empty())
			side["fog"] = fog_game ? "yes" : "no";

		if(side["shroud"].empty())
			side["shroud"] = shroud_game ? "yes" : "no";
		
		if(side["share_maps"].empty())
			side["share_maps"] = share_maps ? "yes" : "no";
		
		if(side["share_view"].empty())
			side["share_view"] = share_view ? "yes" : "no";

		if(side["name"].empty())
			side["name"] = (*possible_sides.front())["name"];

		if(side["type"].empty() && save_ == false)
			side["type"] = (*possible_sides.front())["type"];

		if(side["recruit"].empty())
			side["recruit"] = (*possible_sides.front())["recruit"];

		if(side["music"].empty())
			side["music"] = (*possible_sides.front())["music"];

		if(side["terrain_liked"].empty())
			side["terrain_liked"] = (*possible_sides.front())["terrain_liked"];

		if(side["recruitment_pattern"].empty())
			side["recruitment_pattern"] = possible_sides.front()->values["recruitment_pattern"];
	}

	if((*level_)["objectives"] == "") {
		(*level_)["objectives"] = string_table["mp_objectives"];
	}

	if(save_ == false) {
		(*level_)["experience_modifier"] = lexical_cast<std::string>(xpmodifier);
		(*level_)["era"] = era;
	}

	lists_init();

	//if we have any connected players when we are created, send them the data
	network::send_data(*level_);

	return 0;
}

void mp_connect::lists_init()
{
	//Options
	player_types_.push_back(string_table["network_controlled"]);
	player_types_.push_back(string_table["human_controlled"]);
	player_types_.push_back(string_table["ai_controlled"]);
	player_types_.push_back(string_table["null_controlled"]);
	player_types_.push_back("-----");
	player_types_.push_back(preferences::login());

	//Races
	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);
	if(era_cfg == NULL) {
		return;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	for(std::vector<config*>::const_iterator race = possible_sides.begin();
	    race != possible_sides.end(); ++race) {
		player_races_.push_back(translate_string((**race)["name"]));
	}

	//Teams
	config::child_iterator sd;
	for(sd = sides.first; sd != sides.second; ++sd) {
		const int team_num = sd - sides.first;
		std::string& team_name = (**sd)["team_name"];
		if(team_name.empty()) {
			team_name = lexical_cast<std::string>(team_num+1);
		}

		player_teams_.push_back(string_table["team"] + " " + team_name);
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

void mp_connect::set_area(const SDL_Rect& rect)
{
	rect_ = rect;

	const int left = rect.x;
	const int right = rect.x + rect.w;
	const int center_x = rect.x + rect.w/2;
	const int top = rect.y;
	const int bottom = rect.y + rect.h;
	const int center_y = rect.y + rect.h/2;

	const int width = rect.w;
	const int height = rect.h;

	// Wait to players, Configure players
	//gui::draw_dialog_background(left, right, width, height, *disp_, "menu");

	//Buttons
	cancel_.set_location(right - cancel_.width() - gui::ButtonHPadding,bottom-cancel_.height()-gui::ButtonVPadding);
	launch_.set_location(right - cancel_.width() - launch_.width() - gui::ButtonHPadding*2,bottom-launch_.height()-gui::ButtonVPadding);
	
	ai_.set_location(left+30,bottom-60);

	//Title and labels
	gui::draw_dialog_title(left,top,disp_,string_table["game_lobby"]);

	SDL_Rect labelr = font::draw_text(NULL,rect,14,font::GOOD_COLOUR,
			                          string_table["player_type"],0,0);
	font::draw_text(disp_,rect,14,font::GOOD_COLOUR,
	                string_table["player_type"],(left+30)+(launch_.width()/2)-(labelr.w/2),top+35);
	labelr = font::draw_text(NULL,rect,14,font::GOOD_COLOUR,string_table["race"],0,0);

	font::draw_text(disp_,rect,14,font::GOOD_COLOUR,
	                string_table["race"],(left+145)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,14,font::GOOD_COLOUR,string_table["team"],0,0);

	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["team"],(left+260)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,14,font::GOOD_COLOUR,string_table["color"],0,0);

	font::draw_text(disp_,disp_->screen_area(),14,font::GOOD_COLOUR,
	                string_table["color"],(left+375)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,14,font::GOOD_COLOUR,string_table["gold"],0,0);
	font::draw_text(disp_,rect,14,font::GOOD_COLOUR,
	                string_table["gold"],(left+480)+(launch_.width()/2)-(labelr.w/2),top+35);

	//Per player settings
	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);
	if(era_cfg == NULL) {
		return;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	combos_type_.clear();
	combos_race_.clear();
	combos_team_.clear();
	combos_color_.clear();
	sliders_gold_.clear();

	config::child_iterator sd;

	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = sd - sides.first;

		//Player number
		font::draw_text(disp_,rect, 24, font::GOOD_COLOUR,
		                (*sd)->values["side"], left+10, top+53+(30*side_num));

		//Player type
		combos_type_.push_back(gui::combo(*disp_, player_types_));
		combos_type_.back().set_location(left+30,top+55+(30*side_num));

		//Player race
		combos_race_.push_back(gui::combo(*disp_, player_races_));
		combos_race_.back().set_location(left+145,top+55+(30*side_num));

		//Player team
		combos_team_.push_back(gui::combo(*disp_, player_teams_));
		combos_team_.back().set_location(left+260,top+55+(30*side_num));
		combos_team_.back().set_selected(side_num);

		//Player color
		combos_color_.push_back(gui::combo(*disp_, player_colors_));
		combos_color_.back().set_location(left+375,top+55+(30*side_num));
		combos_color_.back().set_selected(side_num);

		SDL_Rect r;

		//Player gold
		r.x = left+490;
		r.y = top+55+(30*side_num);
		r.w = launch_.width()-5;
		r.h = launch_.height();
		sliders_gold_.push_back(gui::slider(*disp_, r));
		sliders_gold_.back().set_min(20);
		sliders_gold_.back().set_max(1000);
		sliders_gold_.back().set_value(lexical_cast_default<int>((**sd)["gold"],100));
		r.w = 30;
		r.x = left+603;
		gold_bg_.push_back(surface_restorer(&disp_->video(),r));
		font::draw_text(disp_, disp_->screen_area(), 12, font::GOOD_COLOUR,
		                "100", r.x, r.y);
	}

	std::cerr << "done set_area()\n";

	update_whole_screen();
}

void mp_connect::gui_update()
{
	//Update the GUI based on current config settings.
	//Settings may change based on other networked
	//players.

	const config* const era_cfg = cfg_->find_child("era","id",era_);
	if(era_cfg == NULL) {
		return;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	const config::child_itors sides = level_->child_range("side");
	SDL_Rect rect;

	for(size_t n = 0; n != combos_type_.size(); ++n) {
		config& side = **(sides.first+n);

		//Player type
		if (side["controller"] == "network") {
			if (side["description"] == "") {
				combos_type_[n].set_selected(0);
			} else if (side["description"] == string_table["ai_controlled"]) {
				//When loading a game you did not create AI players are marked
				//as network players, set back to AI
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
				combos_type_[n].set_selected(5);
			} else if (side["description"] != "") {
				//When loading a game and you use a name not originally used during
				//the initial game, mark that original slot as network
				combos_type_[n].set_selected(0);
			} else {
				combos_type_[n].set_selected(1);
			}
		} else if (side["controller"] == "ai") {
			combos_type_[n].set_selected(2);
		} else if(side["controller"] == "null") {
			combos_type_[n].set_selected(3);
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
		sliders_gold_[n].set_value(atoi(str.c_str()));
		rect.x = rect_.x + 603;
		rect.y = rect_.y + 55 + (30 * n);
		rect.w = 30;
		rect.h = launch_.height();
		gold_bg_[n].restore();
		font::draw_text(disp_, disp_->screen_area(), 12,
				font::GOOD_COLOUR,
		                side["gold"],
				rect.x, rect.y);
		update_rect(rect);
	}

	const bool full = is_full();
	if(full != message_full_) {
		message_full_ = full;
		if(full) {
			message_bg_.restore();
			message_bg_ = surface_restorer();
		} else {
			SDL_Rect rect = font::draw_text(NULL,rect_,12,font::NORMAL_COLOUR,string_table["waiting_other_players"],0,0);
			rect.x = ai_.location().x + ai_.location().w + 10;
			rect.y = ai_.location().y;
			message_bg_ = surface_restorer(&disp_->video(),rect);
			font::draw_text(disp_,rect,12,font::NORMAL_COLOUR,string_table["waiting_other_players"],rect.x,rect.y);
		}
	}
}

lobby::RESULT mp_connect::process()
{
	if(old_level_.empty()) {
		old_level_ = *level_;
	}

	const config* const era_cfg = cfg_->find_child("era","id",era_);

	if(era_cfg == NULL) {
		std::cerr << "ERROR: cannot find era '" << era_ << "'\n";
		return lobby::QUIT;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	const config::child_itors sides = level_->child_range("side");

	int mousex, mousey;
	const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
	const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

	bool start_game = false;

	bool level_changed = false;

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
			} else if(combos_type_[n].selected() == 1){
				side["controller"] = "human";
				side["description"] = "";
			} else if(combos_type_[n].selected() == 2){
				side["controller"] = "ai";
				side["description"] = string_table["ai_controlled"];
			} else if(combos_type_[n].selected() == 3) {
				side["controller"] = "null";
				side["description"] = "";
			} else if(combos_type_[n].selected() == 4){
				combos_type_[n].set_selected(old_select);
			} else if(combos_type_[n].selected() == 5){
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

			level_changed = true;
		}

		//Player race
		combos_race_[n].enable(!save_);
		combos_team_[n].enable(!save_);
		combos_color_[n].enable(!save_);
		
		if(combos_race_[n].process(mousex, mousey, left_button)) {
			const string_map& values =  possible_sides[combos_race_[n].selected()]->values;
			for(string_map::const_iterator i = values.begin(); i != values.end(); ++i) {
				std::cerr << "value: " << i->first << " , " << i->second<< std::endl;
				side[i->first] = i->second;
			}
			level_changed = true;
		}

		//Player team
		if(combos_team_[n].process(mousex, mousey, left_button)) {
			std::stringstream str;
			str << (combos_team_[n].selected()+1);
			side["team_name"] = str.str();
			level_changed = true;
		}

		if(combos_color_[n].process(mousex, mousey, left_button)) {
			level_changed = true;
		}

		sliders_gold_[n].process();
		if(!save_){
			const int cur_playergold = sliders_gold_[n].value();
			std::stringstream playergold;
			playergold << cur_playergold;
			if (side["gold"] != playergold.str())
			{
				side["gold"] = playergold.str();

				SDL_Rect rect;
				rect.x = rect_.x + 603;
				rect.y = rect_.y + 55 + (30 * n);
				rect.w = 30;
				rect.h = launch_.height();
				gold_bg_[n].restore();
				font::draw_text(disp_, rect_, 12,font::GOOD_COLOUR,(*sides.first[n])["gold"],
						        rect.x, rect.y);
				update_rect(rect);
				level_changed = true;
			}
		}
	}

	if(cancel_.process(mousex,mousey,left_button)) {
		if(network::nconnections() > 0) {
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg);
		}

		return lobby::QUIT;
	}

	if(ai_.process(mousex,mousey,left_button)) {
		for(size_t m = 0; m != combos_team_.size(); ++m) {
			config& si = **(sides.first+m);
			si["controller"] = "ai";
			si["description"] = string_table["ai_controlled"];
			combos_type_[m].set_selected(2);
		}
		level_changed = true;
	}

	launch_.enable(is_full());

	if(launch_.process(mousex,mousey,left_button)) {
		const config::child_list& real_sides = era_cfg->get_children("multiplayer_side");

		for(config::child_iterator side = sides.first; side != sides.second; ++side) {
			int ntry = 0;
			while((**side)["type"] == "random" && ntry < 1000) {
				const int choice = rand()%real_sides.size();

				(**side)["name"] = (*real_sides[choice])["name"];
				(**side)["type"] = (*real_sides[choice])["type"];
				(**side)["recruit"] = (*real_sides[choice])["recruit"];
				(**side)["music"] = (*real_sides[choice])["music"];

				(**side)["recruitment_pattern"] = real_sides[choice]->values["recruitment_pattern"];
				(**side)["terrain_liked"] = (*real_sides[choice])["terrain_liked"];
				level_changed = true;

				++ntry;
			}
		}

		start_game = true;
	}

	if(level_changed) {
		config diff;
		diff.add_child("scenario_diff",level_->get_diff(old_level_));

		network::send_data(diff);

		old_level_ = *level_;
	}

	if(start_game) {
		std::cerr << "multiplayer_connect returning create...\n";
		return lobby::CREATE;
	}

	gui_update();
	update_positions();
	update_network();

	return lobby::CONTINUE;
}

bool mp_connect::get_network_data(config& cfg)
{
	if(network_data_.empty() == false) {
		cfg = network_data_.front();
		network_data_.pop_front();
		return true;
	} else {
		return false;
	}
}

void mp_connect::start_game()
{
	combos_type_.clear();
	combos_race_.clear();
	combos_team_.clear();
	combos_color_.clear();
	sliders_gold_.clear();

	ai_.hide();
	launch_.hide();
	cancel_.hide();
	gold_bg_.clear();


	//Tell everyone to start
	config cfg;
	cfg.add_child("start_game");
	network::send_data(cfg);

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

	state_->snapshot = *level_;

	if(save_ == false) {
		state_->starting_pos = *level_;
	}

	//any replay data isn't meant to hang around under the level,
	//it was just there to tell clients about the replay data
	level_->clear_children("replay");
	std::vector<config*> story;
	state_->can_recruit.clear();
	play_level(*data_, *cfg_, level_, disp_->video(), *state_, story);
	recorder.clear();

	if(network::nconnections() > 0) {
		config cfg;
		cfg.add_child("leave_game");
		network::send_data(cfg);
	}
}

void mp_connect::update_positions()
{
	const config::child_itors sides = level_->child_range("side");
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
		//check if this is a message that might be useful to our caller
		if(cfg.child("message") || cfg.child("gamelist")) {
			network_data_.push_back(cfg);
			return;
		}

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

		int side_taken = atoi(cfg["side"].c_str())-1;
		if(side_taken >= 0 && side_taken < int(sides.size())) {
			std::map<config*,network::connection>::iterator pos = positions_.find(sides[side_taken]);
			if(pos != positions_.end()) {
				//see if we can reassign the player to a different position
				if(pos->second && pos->second != sock) {
					side_taken = 0;
					for(pos = positions_.begin(); pos != positions_.end(); ++pos, ++side_taken) {
						if(pos->first->values["controller"] == "network" &&
							pos->first->values["taken"] != "yes") {
							break;
						}
					}

					if(pos == positions_.end()) {
						config response;
						response.values["failed"] = "yes";
						network::send_data(response,sock);
						return;
					}

					config reassign;
					config& cfg = reassign.add_child("reassign_side");
					cfg["from"] = cfg["side"];
					cfg["to"] = lexical_cast<std::string>(side_taken+1);
					network::send_data(reassign,sock);
				}

				std::cerr << "client has taken a valid position\n";

				//does the client already own the side, and is just updating
				//it, or is it taking a vacant slot?
				const bool update_only = pos->second == sock;

				//broadcast to everyone the new game status
				pos->first->values["controller"] = "network";
				pos->first->values["taken"] = "yes";

				if(cfg["description"].empty() == false) {
					pos->first->values["description"] = cfg["description"];
				}

				if(cfg["name"].empty() == false) {
					pos->first->values["name"] = cfg["name"];
				}

				if(cfg["type"].empty() == false) {
					pos->first->values["type"] = cfg["type"];
				}

				if(cfg["recruit"].empty() == false) {
					pos->first->values["recruit"] = cfg["recruit"];
				}

				if(cfg["music"].empty() == false) {
					pos->first->values["music"] = cfg["music"];
				}

				if(cfg["terrain_liked"].empty() == false) {
					pos->first->values["terrain_liked"] = cfg["terrain_liked"];
				}

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
				std::cerr << "tried to take illegal side: " << side_taken << "\n";
			}
		} else {
			std::cerr << "tried to take unknown side: " << side_taken << "\n";
		}
	}
}

bool mp_connect::is_full()
{
	//see if all positions are now filled
	bool full = true;
	const config::const_child_itors sides = level_->child_range("side");

	for(config::const_child_iterator sd = sides.first; sd != sides.second; ++sd) {
		if((**sd)["controller"] == "network" && positions_[*sd] == 0) {
			full = false;
		}
	}

	return full;
}
