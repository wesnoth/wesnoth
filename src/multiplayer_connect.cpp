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
#include "wassert.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"

#include <sstream>
#include <string>
#include <vector>

#define ERR_CF lg::err(lg::config)
#define LOG_CF lg::info(lg::config)
#define ERR_NW lg::err(lg::network)
#define LOG_NW lg::info(lg::network)
#define LOG_G lg::info(lg::general)

mp_connect::mp_connect(display& disp, std::string game_name,
		       const config &cfg, game_data& data, game_state& state,
		       bool join, const std::string& default_controller) : 
	    disp_(&disp), cfg_(&cfg), data_(&data), state_(&state),
	    show_replay_(false), save_(false), join_(join),
	    scroll_pane_(disp),
	    ai_(disp, _(" Computer vs Computer ")),
	    launch_(disp, _("I'm Ready")),
	    cancel_(disp, _("Cancel")),
	    waiting_label_(disp, ""),
	    message_full_(true), default_controller_(default_controller),
	    team_prefix_(_("Team") + std::string(" "))
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

	if(scenario_data.child("side") == NULL) {
		//Load a saved game
		save_ = true;
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(*disp_, *cfg_, *data_, &show_replay);
		if(game == "") {
			return -1;
		}

		log_scope("loading save");

		state_->players.clear();
		load_game(*data_, game, *state_);

		if(state_->campaign_type != "multiplayer") {
			gui::show_dialog(*disp_, NULL, "", 
					 _("This is not a multiplayer save"),
					 gui::OK_ONLY);
			return -1;
		}

		if(state_->version != game_config::version) {
			const int res = gui::show_dialog(*disp_, NULL, "",
						_("This save is from a different version of the game. Do you want to try to load it?"),
						gui::YES_NO);
			if(res == 1) {
				return -1;
			}
		}

		scenario_data = state_->snapshot;
		level_ptr = &scenario_data;

		//make all sides untaken
		for(config::child_itors j = level_ptr->child_range("side"); j.first != j.second; ++j.first) {
			(**j.first)["taken"] = "";

			//tell clients not to change their race
			(**j.first)["allow_changes"] = "no";
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

	wassert(level_ptr != NULL);

	if(scenario_data["era"].empty()) {
		era_ = era;
	} else { 
		era_ = scenario_data["era"];
	}

	//this will force connecting clients to be using the same version number as us.
	(*level_ptr)["version"] = game_config::version;

	level_ = level_ptr;
	state_->label = level_->values["name"];
	state_->players.clear();

	state_->scenario = scenario_data["id"];

	level_->values["observer"] = allow_observers ? "yes" : "no";

	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);

	if(era_cfg == NULL) {
		ERR_CF << "era '" << era_ << "' not found\n";
		return -1;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	if(sides.first == sides.second || possible_sides.empty()) {
		gui::show_dialog(*disp_, NULL, "", 
				 _("No multiplayer sides."),
				 gui::OK_ONLY);
		ERR_CF << "no multiplayer sides found\n";
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
				(**sd)["controller"] = default_controller_;
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

		if(side["id"].empty())
			side["id"] = (*possible_sides.front())["id"];

		if(side["name"].empty())
			side["name"] = (*possible_sides.front())["name"];

		if(side["leader"].empty())
			side["leader"] = (*possible_sides.front())["leader"];

		if(side["type"].empty() && save_ == false) {
			side["random_faction"] = (*possible_sides.front())["random_faction"];
			side["type"] = (*possible_sides.front())["type"];
		}

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
		(*level_)["objectives"] = _("Victory\n\
@Defeat enemy leader(s)");
	}

	if(save_ == false) {
		(*level_)["experience_modifier"] = lexical_cast<std::string>(xpmodifier);
		(*level_)["era"] = era_;
	}

	lists_init();

	//if we have any connected players when we are created, send them the data
	network::send_data(*level_);

	return 0;
}

void mp_connect::lists_init()
{
	//Options
	player_types_.push_back(_("Network Player"));
	player_types_.push_back(_("Local Player"));
	player_types_.push_back(_("Computer Player"));
	player_types_.push_back(_("Empty"));
	player_types_.push_back("-----");
	player_types_.push_back(preferences::login());

	//Factions
	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);
	if(era_cfg == NULL) {
		return;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	for(std::vector<config*>::const_iterator race = possible_sides.begin(); race != possible_sides.end(); ++race) {
		player_races_.push_back((**race)["name"]);
		possible_faction_ids_.push_back((**race)["id"]);
	}

	//Teams
	config::child_iterator sd;
	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = (sd - sides.first) + 1;
		(**sd)["colour"] = lexical_cast_default<std::string>(side_num);
      
		std::string& team_name = (**sd)["team_name"];
		if (team_name.empty())
			team_name = lexical_cast<std::string>(side_num);
      
		const std::vector<std::string>::iterator result = 
			std::find(player_teams_.begin(), player_teams_.end(),
			          team_prefix_ + team_name);
		
		if (result == player_teams_.end()) {
			player_teams_.push_back(team_prefix_ + team_name);
			team_names_.push_back(team_name);
			team_indices_.push_back(player_teams_.size() - 1);
		} else {
			team_indices_.push_back(result - player_teams_.begin());
		}

		player_leaders_.push_back(leader_list_manager(possible_sides, data_));
	}

	std::string prefix;
	prefix.resize(1);
	//Colors
	prefix[0] = 1;
	player_colors_.push_back(prefix + _("Red"));
	prefix[0] = 2;
	player_colors_.push_back(prefix + _("Blue"));
	prefix[0] = 3;
	player_colors_.push_back(prefix + _("Green"));
	prefix[0] = 4;
	player_colors_.push_back(prefix + _("Yellow"));
	prefix[0] = 5;
	player_colors_.push_back(prefix + _("Purple"));
	prefix[0] = 6;
	player_colors_.push_back(prefix + _("Orange"));
	prefix[0] = 7;
	player_colors_.push_back(prefix + _("Grey"));
	prefix[0] = 8;
	player_colors_.push_back(prefix + _("White"));
	prefix[0] = 9;
	player_colors_.push_back(prefix + _("Brown"));
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

	for(size_t n = 0; n != combos_type_.size(); ++n) {
		combos_type_[n].set_items(player_types_);
	}
}

void mp_connect::set_area(const SDL_Rect& rect)
{
	rect_ = rect;

	const int left = rect.x;
	const int right = rect.x + rect.w;
	const int top = rect.y;
	const int bottom = rect.y + rect.h;

	// Wait to players, Configure players
	//gui::draw_dialog_background(left, right, width, height, *disp_, "menu");

	gui::button* left_button = &launch_;
	gui::button* right_button = &cancel_;

#ifdef OK_BUTTON_ON_RIGHT
	std::swap(left_button,right_button);
#endif

	//Buttons
	right_button->set_location(right - right_button->width() - gui::ButtonHPadding,bottom-right_button->height()-gui::ButtonVPadding);
	left_button->set_location(right - right_button->width() - left_button->width() - gui::ButtonHPadding*2,bottom-left_button->height()-gui::ButtonVPadding);
	
	ai_.set_location(left+30, bottom-left_button->height()-gui::ButtonVPadding);
	waiting_label_.set_location(ai_.location().x + ai_.location().w + 10, bottom-left_button->height()-gui::ButtonVPadding);

	//Title and labels
	gui::draw_dialog_title(left,top,disp_,_("Game Lobby"));

	SDL_Rect labelr = font::draw_text(NULL,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,
			                          _("Player/Type"),0,0);
	font::draw_text(disp_,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,
	                _("Player/Type"),(left+30)+(launch_.width()/2)-(labelr.w/2),top+35);
	labelr = font::draw_text(NULL,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,_("Faction"),0,0);

	font::draw_text(disp_,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,
	                _("Faction"),(left+145)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,_("Team"),0,0);

	font::draw_text(disp_,disp_->screen_area(),font::SIZE_NORMAL,font::GOOD_COLOUR,
	                _("Team"),(left+260)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,_("Color"),0,0);

	font::draw_text(disp_,disp_->screen_area(),font::SIZE_NORMAL,font::GOOD_COLOUR,
	                _("Color"),(left+375)+(launch_.width()/2)-(labelr.w/2),top+35);
	
	labelr = font::draw_text(NULL,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,_("Gold"),0,0);
	font::draw_text(disp_,rect,font::SIZE_NORMAL,font::GOOD_COLOUR,
	                _("Gold"),(left+480)+(launch_.width()/2)-(labelr.w/2),top+35);

	//Per player settings
	const config::child_itors sides = level_->child_range("side");

	const config* const era_cfg = cfg_->find_child("era","id",era_);
	if(era_cfg == NULL) {
		return;
	}

	//Show buttons
	ai_.hide(false);
	launch_.hide(false);
	cancel_.hide(false);
	waiting_label_.hide(false);
	scroll_pane_.hide(false);

	SDL_Rect scroll_pane_rect;
	scroll_pane_rect.x = rect.x;
	scroll_pane_rect.y = rect.y + 50;
	scroll_pane_rect.w = rect.w;
	scroll_pane_rect.h = launch_.location().y - scroll_pane_rect.y - gui::ButtonVPadding;

	scroll_pane_.set_location(scroll_pane_rect);
	config::child_iterator sd;

	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = sd - sides.first;
		LOG_CF << "Side num: " << side_num << std::endl;

		//Player number
		player_numbers_.push_back(gui::label(*disp_, (*sd)->values["side"],
					font::SIZE_XLARGE, font::GOOD_COLOUR));

		//Player type
		combos_type_.push_back(gui::combo(*disp_, player_types_));

		//Player race
		combos_race_.push_back(gui::combo(*disp_, player_races_));

		//Player leader
		std::vector<std::string> dummy_leaders;
		dummy_leaders.push_back("-");
		combos_leader_.push_back(gui::combo(*disp_, dummy_leaders));

		//Player team
		combos_team_.push_back(gui::combo(*disp_, player_teams_));
		combos_team_.back().set_selected(team_indices_[ side_num ]);

		//Player color
		combos_color_.push_back(gui::combo(*disp_, player_colors_));
		combos_color_.back().set_selected(side_num);

		SDL_Rect r;

		//Player gold
		r.x = left+490;
		r.y = top+55+(60*side_num);
		r.w = launch_.width()-5;
		r.h = launch_.height();

		sliders_gold_.push_back(gui::slider(*disp_));
		sliders_gold_.back().set_min(20);
		sliders_gold_.back().set_max(1000);
		sliders_gold_.back().set_increment(25);
		sliders_gold_.back().set_value(lexical_cast_default<int>((**sd)["gold"],100));
		sliders_gold_.back().set_location(r);

		labels_gold_.push_back(gui::label(*disp_, "100", font::SIZE_NORMAL, font::GOOD_COLOUR));

		combos_race_.back().enable(!save_);
		combos_leader_.back().enable(!save_);
		combos_team_.back().enable(!save_);
		combos_color_.back().enable(!save_);
	}

		
	//Doing this after creating the combos, because growing vectors may
	//move their elements in memory, and we need a stable pointer
	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = sd - sides.first;
		const int spos = 60 * side_num;

		scroll_pane_.add_widget(&player_numbers_[side_num], 10, 3 + spos);
		scroll_pane_.add_widget(&combos_type_[side_num], 30, 5 + spos);
		scroll_pane_.add_widget(&combos_race_[side_num], 145, 5 + spos);
		scroll_pane_.add_widget(&combos_leader_[side_num], 145, 35 + spos);
		scroll_pane_.add_widget(&combos_team_[side_num], 260, 5 + spos);
		scroll_pane_.add_widget(&combos_color_[side_num], 375, 5 + spos);
		scroll_pane_.add_widget(&sliders_gold_[side_num], 490, 5 + spos);
		scroll_pane_.add_widget(&labels_gold_[side_num], 500 + sliders_gold_[side_num].width(), 5 + spos);

		player_leaders_[side_num].set_combo(&combos_leader_[side_num]);
	}

	LOG_CF << "done set_area()\n";

	update_whole_screen();
}

void mp_connect::clear_area()
{
	scroll_pane_.clear();

	combos_type_.clear();
	combos_race_.clear();
	combos_leader_.clear();
	combos_team_.clear();
	combos_color_.clear();
	sliders_gold_.clear();
	labels_gold_.clear();
	possible_faction_ids_.clear();

	ai_.hide();
	launch_.hide();
	cancel_.hide();
	waiting_label_.hide();
	scroll_pane_.hide();
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

	const config::child_itors sides = level_->child_range("side");

	for(size_t n = 0; n != combos_type_.size(); ++n) {
		config& side = **(sides.first+n);

		//Player type
		if (side["controller"] == "network") {
			if (side["description"] == "") {
				combos_type_[n].set_selected(0);
			} else if (side["description"] == _("Computer Player")) {
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

		//Player Faction
		for (size_t m = 0; m != player_races_.size(); ++m) {
			if (side["id"] == possible_faction_ids_[m]) {
				if (combos_race_[n].selected() != m) {
					combos_race_[n].set_selected(m);

					if (!save_)
						player_leaders_[n].update_leader_list(m);
				}
			}
		}

		// Player leader
		if (!save_) 
			player_leaders_[n].set_leader(side["type"]);

		//Player Gold
		labels_gold_[n].set_text(side["gold"]);
	}

	const bool full = is_full();
	if(full != message_full_) {
		message_full_ = full;
		if(full) {
			waiting_label_.set_text("");
		} else {
			waiting_label_.set_text(_("Waiting for network players to join"));
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
		ERR_CF << "cannot find era '" << era_ << "'\n";
		return lobby::QUIT;
	}

	const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

	const config::child_itors sides = level_->child_range("side");

	bool start_game = false;

	bool level_changed = false;

	for(size_t n = 0; n != combos_team_.size(); ++n) {
		config& side = **(sides.first+n);

		//Player type
		//Don't let user change this if a player is sitting
		combos_type_[n].enable(combos_type_[n].selected() < 4);

		if (combos_type_[n].changed()) {
			switch (combos_type_[n].selected()) {
			case 1:
				side["controller"] = "human";
				side["description"] = "";
				break;
			case 2:
				side["controller"] = "ai";
				side["description"] = _("Computer Player");
				break;
			case 3:
				side["controller"] = "null";
				side["description"] = "";
				break;
			case 5: {
				side["controller"] = "human";
				side["description"] = preferences::login();
				for(size_t m = 0; m != combos_type_.size(); ++m) {
					if(m != n) {
						if(combos_type_[m].selected() == 5){
							combos_type_[m].set_selected(2);
							config& si = **(sides.first+m);
							si["controller"] = "ai";
							si["description"] = "";
						}
					}
				}
				break;
			}
			case 4:
				combos_type_[n].set_selected(0);
			case 0:
				side["controller"] = "network";
				side["description"] = "";
				break;
			default:
				// Do nothing
				;
			}

			level_changed = true;
		}

		if (!save_ && combos_race_[n].changed()) {
			const string_map& values =  possible_sides[combos_race_[n].selected()]->values;
			side["random_faction"] = "";
			for(string_map::const_iterator i = values.begin(); i != values.end(); ++i) {
				LOG_CF << "value: " << i->first << " , " << i->second << std::endl;
				side[i->first] = i->second;
			}
			level_changed = true;

			player_leaders_[n].update_leader_list(combos_race_[n].selected());
		}
		
		//Player colour
		if (combos_color_[n].changed()) {
			side["colour"] = lexical_cast_default<std::string>(combos_color_[n].selected()+1);
			level_changed = true;
		}

		//Player leader
		if (!save_ && combos_leader_[n].changed()) {
			side["type"] = player_leaders_[n].get_leader();
			level_changed = true;
		}

		//Player team
		if (combos_team_[n].changed()) {
			side["team_name"] = team_names_[combos_team_[n].selected()];
			level_changed = true;
		}

		if(!save_){
			const int cur_playergold = sliders_gold_[n].value();
			std::stringstream playergold;
			playergold << cur_playergold;
			if (side["gold"] != playergold.str())
			{
				side["gold"] = playergold.str();

				//Player Gold
				labels_gold_[n].set_text(side["gold"]);
				level_changed = true;
			}
		}
	}

	if (cancel_.pressed()) {
		if(network::nconnections() > 0) {
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg);
		}

		return lobby::QUIT;
	}

	if (ai_.pressed()) {
		for(size_t m = 0; m != combos_team_.size(); ++m) {
			config& si = **(sides.first+m);
			//disconnect everyone except the host
			std::map<config*,network::connection>::iterator pos = positions_.find(&si);
			if (pos->first->values["description"] == si["description"]) {
				network::disconnect(pos->second);
				pos->second = 0;
			}
			pos->first->values.erase("taken");
			remove_player(pos->first->values["description"]);
			if(!save_) {
				si["description"] = _("Computer Player");
				si["controller"] = "ai";
				si["id"] = possible_faction_ids_.front();
				si["random_faction"] = "yes";
			}
		}
		level_changed = true;
	}

	launch_.enable(is_full());

	if (launch_.pressed()) {
		const config::child_list& real_sides = era_cfg->get_children("multiplayer_side");

		for(config::child_iterator side = sides.first; side != sides.second; ++side) {
			int ntry = 0;
			while((**side)["random_faction"] == "yes" && ntry < 1000) {
				const int choice = rand()%real_sides.size();

				(**side)["id"] = (*real_sides[choice])["id"];
				(**side)["name"] = (*real_sides[choice])["name"];
				(**side)["random_faction"] = (*real_sides[choice])["random_faction"];

				(**side)["type"] = "random";

				(**side)["leader"] = (*real_sides[choice])["leader"];
				(**side)["recruit"] = (*real_sides[choice])["recruit"];
				(**side)["music"] = (*real_sides[choice])["music"];

				(**side)["recruitment_pattern"] = real_sides[choice]->values["recruitment_pattern"];
				(**side)["terrain_liked"] = (*real_sides[choice])["terrain_liked"];
				level_changed = true;

				++ntry;
			}

			if ((**side)["type"] == "random" || (**side)["type"].empty()) {
				// Choose a random leader type.  
				std::vector<std::string> types = 
					utils::split((**side)["leader"]);
				if (!types.empty()) {
					const int lchoice = rand() % types.size();
					(**side)["type"] = types[lchoice];
				} else {
					ERR_CF << "Unable to find a type for side " << (**side)["name"] << "!\n";
				}
				level_changed = true;
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
		LOG_G << "multiplayer_connect returning create...\n";
		return lobby::CREATE;
	}

	update_positions();
	update_network();
	gui_update();

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
	clear_area();

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
			LOG_G << "skipping...\n";
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
	state_->players.clear();

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
	for(std::map<config*,network::connection>::const_iterator i = positions_.begin(); i != positions_.end(); ++i) {
		if(!i->second) {
			//We are waiting on someone
			network::connection sock = network::accept_connection();
			if(sock) {
				LOG_NW << "Received connection\n";
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
		ERR_NW << "caught networking error. we are " << (network::is_server() ? "" : "NOT") << " a server\n";
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
		for(std::map<config*,network::connection>::iterator i = positions_.begin(); i != positions_.end(); ++i) {
			if(i->second == e.socket) {
				changes = true;
				i->second = 0;
				i->first->values.erase("taken");
				remove_player(i->first->values["description"]);
				if(!save_) {
					i->first->values["description"] = "";
					i->first->values["id"] = possible_faction_ids_.front();
					i->first->values["random_faction"] = "yes";
					i->first->values["type"] = "";
				}
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
				remove_player(pos->first->values["description"]);
				if(!save_) {
					pos->first->values["description"] = "";
					pos->first->values["id"] = possible_faction_ids_.front();
					pos->first->values["random_faction"] = "yes";
					pos->first->values["type"] = "";
				}
				network::send_data(*level_);
			}
			return;
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

				LOG_CF << "client has taken a valid position\n";

				//broadcast to everyone the new game status
				pos->first->values["controller"] = "network";
				pos->first->values["taken"] = "yes";

				if(cfg["description"].empty() == false) {
					pos->first->values["description"] = cfg["description"];
				}

				if(cfg["id"].empty() == false) {
					const config* const era_cfg = cfg_->find_child("era","id",era_);
					const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");
					const std::vector<std::string>::const_iterator itor = 
						std::find(possible_faction_ids_.begin(), possible_faction_ids_.end(), cfg["id"] );
					if(itor == possible_faction_ids_.end()) {
						ERR_CF << "client sent unknown faction id: " << cfg["id"] << "\n";
					} else {
						const int index = itor - possible_faction_ids_.begin();
						const string_map& values = possible_sides[index]->values;
						pos->first->values["random_faction"] = "";
						for(string_map::const_iterator i = values.begin(); i != values.end(); ++i) {
							LOG_CF << "value: " << i->first << " , " << i->second << std::endl;
							pos->first->values[i->first] = i->second;
						}
						pos->first->values["id"] = cfg["id"];
					}
				}

				if(cfg["name"].empty() == false) {
					pos->first->values["name"] = cfg["name"];
				}

				if(cfg["type"].empty() == false) {
					pos->first->values["random_faction"] = "";
					pos->first->values["type"] = cfg["type"];
				}

				if(cfg["recruit"].empty() == false) {
					pos->first->values["recruit"] = cfg["recruit"];
				}

				if(cfg["music"].empty() == false) {
					pos->first->values["music"] = cfg["music"];
				}

				if(cfg["random_faction"].empty() == false) {
					pos->first->values["random_faction"] = cfg["random_faction"];
				}

				if(cfg["terrain_liked"].empty() == false) {
					pos->first->values["terrain_liked"] = cfg["terrain_liked"];
				}

				pos->second = sock;
				network::send_data(*level_);

				LOG_NW << "sent player data\n";

				//send a reply telling the client they have secured
				//the side they asked for
				std::stringstream side;
				side << (side_taken+1);
				config reply;
				reply.values["side_secured"] = side.str();
				LOG_NW << "going to send data...\n";
				network::send_data(reply,sock);

				// Add to combo list
				add_player(cfg["description"]);
			} else {
				ERR_CF << "tried to take illegal side: " << side_taken << '\n';
			}
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
