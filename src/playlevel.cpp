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

#include "ai_interface.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "game_events.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "log.hpp"
#include "mapgen.hpp"
#include "map_create.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "video.hpp"

#include <iostream>
#include <iterator>

#define LOG_NG lg::info(lg::engine)

namespace {
	int placing_score(const config& side, const gamemap& map, const gamemap::location& pos)
	{
		int positions = 0, liked = 0;
		const std::string& terrain_liked = side["terrain_liked"];
		for(int i = pos.x-8; i != pos.x+8; ++i) {
			for(int j = pos.y-8; j != pos.y+8; ++j) {
				const gamemap::location pos(i,j);
				if(map.on_board(pos)) {
					++positions;
					if(std::count(terrain_liked.begin(),terrain_liked.end(),map[i][j])) {
						++liked;
					}
				}
			}
		}

		return (100*liked)/positions;
	}

	struct placing_info {
		int side, score;
		gamemap::location pos;
	};

	bool operator<(const placing_info& a, const placing_info& b) { return a.score > b.score; }
	bool operator==(const placing_info& a, const placing_info& b) { return a.score == b.score; }

	void place_sides_in_preferred_locations(gamemap& map, const config::child_list& sides)
	{
		std::vector<placing_info> placings;

		const int num_pos = map.num_valid_starting_positions();

		for(config::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
			const int side_num = s - sides.begin() + 1;
			for(int p = 1; p <= num_pos; ++p) {
				const gamemap::location& pos = map.starting_position(p);
				const int score = placing_score(**s,map,pos);
				placing_info obj;
				obj.side = side_num;
				obj.score = score;
				obj.pos = pos;
				placings.push_back(obj);
			}
		}

		std::sort(placings.begin(),placings.end());
		std::set<int> placed;
		std::set<gamemap::location> positions_taken;

		for(std::vector<placing_info>::const_iterator i = placings.begin(); i != placings.end() && placed.size() != sides.size(); ++i) {
			if(placed.count(i->side) == 0 && positions_taken.count(i->pos) == 0) {
				placed.insert(i->side);
				positions_taken.insert(i->pos);
				map.set_starting_position(i->side,i->pos);
				LOG_NG << "placing side " << i->side << " at " << i->pos.x << "," << i->pos.y << "\n";
			}
		}
	}

	bool is_observer(const std::vector<team>& teams)
	{
		for(std::vector<team>::const_iterator i = teams.begin(); i != teams.end(); ++i) {
			if(i->is_human()) {
				return false;
			}
		}

		return true;
	}
}

LEVEL_RESULT play_level(game_data& gameinfo, const config& game_config,
		config* level, CVideo& video,
		game_state& state_of_game,
		const std::vector<config*>& story)
{
	//guarantee the cursor goes back to 'normal' at the end of the level
	const cursor::setter cursor_setter(cursor::NORMAL);

	const int ticks = SDL_GetTicks();
	LOG_NG << "in play_level()...\n";

	//if the entire scenario should be randomly generated
	if((*level)["scenario_generation"] != "") {
		LOG_NG << "randomly generating scenario...\n";
		const cursor::setter cursor_setter(cursor::WAIT);
	
		static config scenario;
		scenario = random_generate_scenario((*level)["scenario_generation"],level->child("generator"));
		level = &scenario;

		state_of_game.starting_pos = scenario;
	}
	
	std::string map_data = (*level)["map_data"];
	if(map_data == "" && (*level)["map"] != "") {
		map_data = read_map((*level)["map"]);
	}

	//if the map should be randomly generated
	if(map_data == "" && (*level)["map_generation"] != "") {
		const cursor::setter cursor_setter(cursor::WAIT);
		map_data = random_generate_map((*level)["map_generation"],level->child("generator"));

		//since we've had to generate the map, make sure that when we save the game,
		//it will not ask for the map to be generated again on reload
		static config new_level;
		new_level = *level;
		new_level.values["map_data"] = map_data;
		level = &new_level;

		state_of_game.starting_pos = new_level;
	}

	const config& lvl = *level;

	LOG_NG << "generated map " << (SDL_GetTicks() - ticks) << "\n";

	const statistics::scenario_context statistics_context(lvl["name"]);

	const int num_turns = atoi(lvl["turns"].c_str());
	gamestatus status(*level,num_turns);

	gamemap map(game_config,map_data);

	LOG_NG << "created objects... " << (SDL_GetTicks() - ticks) << "\n";

	CKey key;
	unit_map units;

	const verification_manager verify_manager(units);

	const int xp_modifier = atoi(lvl["experience_modifier"].c_str());
	const unit_type::experience_accelerator xp_mod(xp_modifier > 0 ? xp_modifier : 100);

	std::vector<team> teams;

	const teams_manager team_manager(teams);

	int first_human_team = -1;

	const config::child_list& unit_cfg = level->get_children("side");

	if(lvl["modify_placing"] == "true") {
		LOG_NG << "modifying placing...\n";
		place_sides_in_preferred_locations(map,unit_cfg);
	}

	LOG_NG << "initializing teams..." << unit_cfg.size() << "\n";;
	LOG_NG << (SDL_GetTicks() - ticks) << "\n";

	std::set<std::string> seen_save_ids;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::string save_id = (**ui)["save_id"];

		if(save_id.empty()) {
			save_id=(**ui)["description"];
		}

		//make sure the 'save_id' is unique
		while(seen_save_ids.count(save_id)) {
			save_id += "_";
		}

		seen_save_ids.insert(save_id);

		player_info *player = NULL;

		if((**ui)["controller"] == "human" ||
		   (**ui)["controller"] == "network" ||
		   (**ui)["persistent"] == "1") {
			player = state_of_game.get_player(save_id);

			if(player == NULL && !save_id.empty()) {
				player = &state_of_game.players[save_id];
			}
		}

		LOG_NG << "initializing team...\n";

		if (first_human_team == -1 && ((**ui)["controller"] == "human" || (**ui)["persistent"] == "1")) {
			first_human_team = ui - unit_cfg.begin();
		}

		std::string gold = (**ui)["gold"];
		if(gold.empty())
			gold = "100";

		LOG_NG << "found gold: '" << gold << "'\n";

		int ngold = lexical_cast_default<int>(gold);
		if(player != NULL && player->gold >= ngold) {
			ngold = player->gold;
		}

		LOG_NG << "set gold to '" << ngold << "'\n";

		teams.push_back(team(**ui,ngold));

		//if this side tag describes the leader of the side
		if((**ui)["no_leader"] != "yes" && (**ui)["controller"] != "null") {
			unit new_unit(gameinfo, **ui);

			//search the recall list for leader units, and if there is
			//one, use it in place of the config-described unit
			if(player != NULL) {
				for(std::vector<unit>::iterator it = player->available_units.begin(); it != player->available_units.end(); ++it) {
					if(it->can_recruit()) {
						new_unit = *it;
						player->available_units.erase(it);
						break;
					}
				}
			}

			//see if the side specifies its location. Otherwise start it at the map-given
			//starting position
			const std::string& has_loc = (**ui)["x"];
			gamemap::location start_pos(**ui);

			if(has_loc.empty()) {
				start_pos = map.starting_position(new_unit.side());
				LOG_NG << "initializing side '" << (**ui)["side"] << "' at "
				       << start_pos.x << "," << start_pos.y << "\n";
			}

			if(map.empty()) {
				throw gamestatus::load_game_failed("Map not found");
			}

			if(!start_pos.valid() && new_unit.side() == 1) {
				throw gamestatus::load_game_failed("No starting position for side 1");
			}

			if(start_pos.valid()) {
				units.insert(std::pair<gamemap::location,unit>(
								map.starting_position(new_unit.side()), new_unit));
			}
		}

		//if the game state specifies units that can be recruited for the player
		//then add them
		if(player != NULL && player->can_recruit.empty() == false) {
			std::copy(player->can_recruit.begin(),player->can_recruit.end(),
					std::inserter(teams.back().recruits(),teams.back().recruits().end()));
		}
		
		if(player != NULL) {
			player->can_recruit = teams.back().recruits();
		}
		
		//if there are additional starting units on this side
		const config::child_list& starting_units = (*ui)->get_children("unit");
		for(config::child_list::const_iterator su = starting_units.begin(); su != starting_units.end(); ++su) {
			unit new_unit(gameinfo,**su);

			new_unit.set_side(lexical_cast_default<int>((**ui)["side"],1));

			const std::string& x = (**su)["x"];
			const std::string& y = (**su)["y"];

			const gamemap::location loc(**su);
			if(x.empty() || y.empty() || !map.on_board(loc)) {
				if(player) {
					player->available_units.push_back(new_unit);
				}
			} else {
				units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
				LOG_NG << "inserting unit for side " << new_unit.side() << "\n";
			}
		}

	}

	// Add all recruitable units as encountered so that information
	// about them are displayed to the user in the help system.
	for (std::vector<team>::const_iterator help_team_it = teams.begin();
		 help_team_it != teams.end(); help_team_it++) {
		LOG_NG << "Adding help units for team '" << help_team_it->name() << "'\n";
		const std::set<std::string> &recruitable = help_team_it->recruits();
		std::set<std::string> &enc_units = preferences::encountered_units();
		LOG_NG << "Adding recruitable units: \n";
		for (std::set<std::string>::const_iterator it = recruitable.begin();
			 it != recruitable.end(); it++) {
			LOG_NG << *it << std::endl;
		}
		LOG_NG << "Added all recruitable units\n";
		std::copy(recruitable.begin(), recruitable.end(), 
				  std::inserter(enc_units, enc_units.begin()));
	}
		
	// Add all units that exist at the start to the encountered units so
	// that information about them are displayed to the user in the help
	// system.
	for (unit_map::const_iterator help_unit_it = units.begin();
		 help_unit_it != units.end(); help_unit_it++) {
		const std::string name = help_unit_it->second.type().name();
		preferences::encountered_units().insert(name);
	}

	// Add all units that are recallable as encountred units.
	for(std::map<std::string, player_info>::iterator pi = state_of_game.players.begin(); pi!=state_of_game.players.end(); ++pi) {
		for(std::vector<unit>::iterator help_recall_it = pi->second.available_units.begin(); help_recall_it != pi->second.available_units.end(); help_recall_it++) {
			preferences::encountered_units().insert(help_recall_it->type().name());
		}
	}

	// Add all terrains on the map as encountered terrains.
	for (int map_x = 0; map_x < map.x(); map_x++) {
		for (int map_y = 0; map_y < map.y(); map_y++) {
			const gamemap::TERRAIN t = map.get_terrain(gamemap::location(map_x, map_y));
			std::string s;
			s += t;
			preferences::encountered_terrains().insert(s);
		}
	}
	LOG_NG << "initialized teams... " << (SDL_GetTicks() - ticks) << "\n";

	const config* theme_cfg = NULL;
	if(lvl["theme"] != "") {
		theme_cfg = game_config.find_child("theme","name",lvl["theme"]);
	}

	if(theme_cfg == NULL) {
		theme_cfg = game_config.find_child("theme","name",preferences::theme());
	}

	LOG_NG << "initializing display... " << (SDL_GetTicks() - ticks) << "\n";
	const config dummy_cfg;
	display gui(units,video,map,status,teams,theme_cfg != NULL ? *theme_cfg : dummy_cfg, game_config, *level);

	LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks) << "\n";

	LOG_NG << "a... " << (SDL_GetTicks() - ticks) << "\n";

	if(first_human_team != -1) {
		gui.set_team(first_human_team);
	}

	const preferences::display_manager prefs_disp_manager(&gui);
	const tooltips::manager tooltips_manager(gui.video());

	LOG_NG << "b... " << (SDL_GetTicks() - ticks) << "\n";

	//this *needs* to be created before the show_intro and show_map_scene
	//as that functions use the manager state_of_game 
	game_events::manager events_manager(*level,gui,map,units,teams,
	                                    state_of_game,status,gameinfo);

	if(recorder.skipping() == false) {
		for(std::vector<config*>::const_iterator story_i = story.begin(); story_i != story.end(); ++story_i) {
			show_intro(gui,**story_i, *level);
		}
	}

	//object that will make sure that labels are removed at the end of the scenario
	const font::floating_label_context labels_manager;
	const halo::manager halo_manager(gui);
	gui.labels().read(*level);
	LOG_NG << "c... " << (SDL_GetTicks() - ticks) << "\n";

	const std::string& music = lvl["music"];
	if(music != "") {
		sound::play_music(music);
	}

	LOG_NG << "d... " << (SDL_GetTicks() - ticks) << "\n";

	victory_conditions::set_victory_when_enemies_defeated(
						lvl["victory_when_enemies_defeated"] != "no");

	LOG_NG << "initializing events manager... " << (SDL_GetTicks() - ticks) << "\n";

	help::help_manager help_manager(&game_config, &gameinfo, &map);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level->get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui.add_overlay(gamemap::location(**overlay),(**overlay)["image"]);
	}

	int turn = 1, player_number = 0;

	turn_info::floating_textbox textbox_info;

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks) << "\n";

	replay_network_sender replay_sender(recorder);

	try {
		//if a team is specified whose turn it is, it means we're loading a game
		//instead of starting a fresh one
		const bool loading_game = lvl["playing_team"].empty() == false;
		
		gui.begin_game();
		gui.adjust_colours(0,0,0);

		if(!loading_game) {
			game_events::fire("prestart");
		}

		LOG_NG << "scrolling... " << (SDL_GetTicks() - ticks) << "\n";
		if(first_human_team != -1) {
			clear_shroud(gui,status,map,gameinfo,units,teams,first_human_team);
			LOG_NG << "b " << (SDL_GetTicks() - ticks) << "\n";
			gui.scroll_to_tile(map.starting_position(first_human_team + 1).x,
			                   map.starting_position(first_human_team + 1).y, display::WARP);
			LOG_NG << "c " << (SDL_GetTicks() - ticks) << "\n";
		}
	
		gui.scroll_to_tile(map.starting_position(1).x,map.starting_position(1).y,display::WARP);
		LOG_NG << "done scrolling... " << (SDL_GetTicks() - ticks) << "\n";

		bool replaying = (recorder.at_end() == false);
	
		int first_player = atoi(lvl["playing_team"].c_str());
		if(first_player < 0 || first_player >= int(teams.size())) {
			first_player = 0;
		}

		LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks) << "\n";

		std::deque<config> data_backlog;
		
		for(bool first_time = true; true; first_time = false, first_player = 0) {
			player_number = 0;

			if(first_time) {
				const hotkey::basic_handler key_events_handler(&gui);

				LOG_NG << "first_time..." << (recorder.skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui,recorder.skipping());
				events::raise_draw_event();
				if(!loading_game) {
					game_events::fire("start");
					game_events::set_variable("turn_number", "1");
				}

				gui.draw();
				
				for(std::vector<team>::iterator t = teams.begin(); t != teams.end(); ++t) {
					clear_shroud(gui,status,map,gameinfo,units,teams,(t-teams.begin()));
				}

				gui.recalculate_minimap();
			}

			gui.new_turn();
			gui.invalidate_game_status();
			events::raise_draw_event();

			LOG_NG << "turn: " << turn++ << "\n";

			for(std::vector<team>::iterator team_it = teams.begin()+first_player; team_it != teams.end(); ++team_it) {
				log_scope("player turn");
				player_number = (team_it - teams.begin()) + 1;

				//if a side is dead, don't do their turn
				if(team_it->is_empty() || team_units(units,player_number) == 0) {
					continue;
				}

				if(is_observer(teams)) {
					gui.set_team(size_t(player_number-1));
				}

				std::stringstream player_number_str;
				player_number_str << player_number;
				game_events::set_variable("side_number",player_number_str.str());

				//fire side turn event only if real side change occurs not counting changes from void to a side
				if (team_it != teams.begin()+first_player || !first_time) {
					game_events::fire("side turn");
				}

				//we want to work out if units for this player should get healed, and the
				//player should get income now. healing/income happen if it's not the first
				//turn of processing, or if we are loading a game, and this is not the
				//player it started with.
				const bool turn_refresh = !first_time || loading_game && team_it != teams.begin()+first_player;

				if(turn_refresh) {
					for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
						if(i->second.side() == player_number) {
							i->second.new_turn();
						}
					}

					team_it->new_turn();

					//if the expense is less than the number of villages owned,
					//then we don't have to pay anything at all
					const int expense = team_upkeep(units,player_number) -
										team_it->villages().size();
					if(expense > 0) {
						team_it->spend_gold(expense);
					}

					calculate_healing(gui,status,map,units,player_number,teams);
				}

				team_it->set_time_of_day(int(status.turn()),status.get_time_of_day());

				gui.set_playing_team(size_t(player_number-1));

				clear_shroud(gui,status,map,gameinfo,units,teams,player_number-1);

				//scroll the map to the leader
				const unit_map::iterator leader = find_leader(units,player_number);

				if(leader != units.end() && !recorder.skipping()) {
					const hotkey::basic_handler key_events_handler(&gui);
					gui.scroll_to_tile(leader->first.x,leader->first.y);
				}

				if(replaying) {
					const hotkey::basic_handler key_events_handler(&gui);
					LOG_NG << "doing replay " << player_number << "\n";
					try {
						replaying = do_replay(gui,map,gameinfo,units,teams,
						                      player_number,status,state_of_game);
					} catch(replay::error&) {
						gui::show_dialog(gui,NULL,"",_("The file you have tried to load is corrupt"),gui::OK_ONLY);

						replaying = false;
					}
					LOG_NG << "result of replay: " << (replaying?"true":"false") << "\n";
				}

				if(!replaying && team_it->music().empty() == false) {
					LOG_NG << "playing music: '" << team_it->music() << "'\n";
					sound::play_music(team_it->music());
				}

//goto this label if the type of a team (human/ai/networked) has changed mid-turn
redo_turn:

				if(!replaying && team_it->is_human()) {
					LOG_NG << "is human...\n";

					if(first_time && team_it == teams.begin()) {
						if(lvl["objectives"].empty() == false) {
							dialogs::show_objectives(gui,*level);
						}
					}

					try {
						play_turn(gameinfo,state_of_game,status,game_config,
						          level, key, gui, map, teams, player_number,
						          units, textbox_info, replay_sender);
					} catch(end_turn_exception& end_turn) {
						if (end_turn.redo == player_number)
							goto redo_turn;
					}

					if(game_config::debug)
						display::clear_debug_highlights();

					LOG_NG << "human finished turn...\n";

				} else if(!replaying && team_it->is_ai()) {
					LOG_NG << "is ai...\n";
					gui.recalculate_minimap();

					const cursor::setter cursor_setter(cursor::WAIT);

					turn_info turn_data(gameinfo,state_of_game,status,
						                game_config,level,key,gui,
						                map,teams,player_number,units,
										turn_info::BROWSE_AI,textbox_info,replay_sender);

					ai_interface::info ai_info(gui,map,gameinfo,units,teams,player_number,status,turn_data);
					util::scoped_ptr<ai_interface> ai_obj(create_ai(team_it->ai_algorithm(),ai_info));
					ai_obj->play_turn();

					gui.recalculate_minimap();
					clear_shroud(gui,status,map,gameinfo,units,teams,player_number-1);
					gui.invalidate_unit();
					gui.invalidate_game_status();
					gui.invalidate_all();
					gui.draw();
					SDL_Delay(500);
				} else if(!replaying && team_it->is_network()) {
					LOG_NG << "is networked...\n";

					turn_info turn_data(gameinfo,state_of_game,status,
					                    game_config,level,key,gui,
										map,teams,player_number,units,turn_info::BROWSE_NETWORKED,textbox_info,replay_sender);

					for(;;) {

						bool have_data = false;
						config cfg;

						network::connection from = network::null_connection;

						if(data_backlog.empty() == false) {
							have_data = true;
							cfg = data_backlog.front();
							data_backlog.pop_front();
						} else {
							from = network::receive_data(cfg);
							have_data = from != network::null_connection;
						}

						if(have_data) {
							const turn_info::PROCESS_DATA_RESULT result = turn_data.process_network_data(cfg,from,data_backlog);
							if(result == turn_info::PROCESS_RESTART_TURN) {
								goto redo_turn;
							} else if(result == turn_info::PROCESS_END_TURN) {
								break;
							}
						}

						turn_data.turn_slice();
						turn_data.send_data();
						gui.draw();
					}

					LOG_NG << "finished networked...\n";
				}

				for(unit_map::iterator uit = units.begin(); uit != units.end(); ++uit) {
					if(uit->second.side() == player_number)
						uit->second.end_turn();
				}

				//This implements "delayed map sharing." It's meant as an alternative to shared vision.
				if(team_it->copy_ally_shroud()) {
					gui.recalculate_minimap();
					gui.invalidate_all();
				}
				
				game_events::pump();

				check_victory(units,teams);
			}

			//time has run out
			if(!status.next_turn()) {
			
				if(non_interactive()) {
					std::cout << "time over (draw)\n";
				}

				LOG_NG << "firing time over event...\n";
				game_events::fire("time over");
				LOG_NG << "done firing time over event...\n";

				throw end_level_exception(DEFEAT);
			}

			std::stringstream event_stream;
			event_stream << status.turn();

			{
				LOG_NG << "turn event..." << (recorder.skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui,recorder.skipping());
				const std::string turn_num = event_stream.str();
				game_events::set_variable("turn_number",turn_num);
				game_events::fire("turn " + turn_num);
				game_events::fire("new turn");
			}
		} //end for loop

	} catch(end_level_exception& end_level) {
		bool obs = is_observer(teams);
		if (end_level.result == DEFEAT || end_level.result == VICTORY) {
			// if we're a player, and the result is victory/defeat, then send a message to notify
			// the server of the reason for the game ending
			if (!obs) {
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "game over";
				network::send_data(cfg);
			} else
				gui::show_dialog(gui, NULL, _("Game Over"),
				                 _("The game is over."), gui::OK_ONLY);
		}

		if(end_level.result == QUIT) {
			return end_level.result;
		} else if(end_level.result == DEFEAT) {
			try {
				game_events::fire("defeat");
			} catch(end_level_exception&) {
			}

			if (!obs)
				gui::show_dialog(gui, NULL,
				                 _("Defeat"),
				                 _("You have been defeated!"),
				                 gui::OK_ONLY);
			return DEFEAT;
		} else if (end_level.result == VICTORY || end_level.result == LEVEL_CONTINUE ||
		           end_level.result == LEVEL_CONTINUE_NO_SAVE) {
			try {
				game_events::fire("victory");
			} catch(end_level_exception&) {
			}

			if(state_of_game.scenario == (*level)["id"]) {
				state_of_game.scenario = (*level)["next_scenario"];
			}

			const bool has_next_scenario = !state_of_game.scenario.empty() &&
			                               state_of_game.scenario != "null";

			//add all the units that survived the scenario
			for(std::map<gamemap::location,unit>::iterator un = units.begin(); un != units.end(); ++un) {
				player_info *player=state_of_game.get_player(teams[un->second.side()-1].save_id());

				if(player) {
					un->second.new_turn();
					un->second.new_level();
					player->available_units.push_back(un->second);
				}
			}

			//'continue' is like a victory, except it doesn't announce victory,
			//and the player retains 100% of gold.
			if(end_level.result == LEVEL_CONTINUE || end_level.result == LEVEL_CONTINUE_NO_SAVE) {
				for(std::vector<team>::iterator i=teams.begin(); i!=teams.end(); ++i) {
					player_info *player=state_of_game.get_player(i->save_id());
					if(player) {
						player->gold = i->gold();
					}
				}

				return end_level.result == LEVEL_CONTINUE_NO_SAVE ? LEVEL_CONTINUE_NO_SAVE : VICTORY;
			}


			std::stringstream report;

			for(std::vector<team>::iterator i=teams.begin(); i!=teams.end(); ++i) {
				if (!i->is_persistent())
					continue;

				player_info *player=state_of_game.get_player(i->save_id());

				const int remaining_gold = i->gold();
				const int finishing_bonus_per_turn =
				             map.villages().size() * game_config::village_income +
				             game_config::base_income;
				const int turns_left = maximum<int>(0,status.number_of_turns() - status.turn());
				const int finishing_bonus = end_level.gold_bonus ?
				             (finishing_bonus_per_turn * turns_left) : 0;

				if(player) {
					player->gold = ((remaining_gold + finishing_bonus) * 80) / 100;

					if(state_of_game.players.size()>1) {
						if(i!=teams.begin()) {
							report << "\n";
						}

						report << font::BOLD_TEXT << i->save_id() << "\n";
					}

					report << _("Remaining gold: ")
					       << remaining_gold << "\n";
					if(end_level.gold_bonus) {
						report << _("Early finish bonus: ")
						       << finishing_bonus_per_turn
						       << " " << _("per turn") << "\n"
						       << _("Turns finished early: ")
						       << turns_left << "\n"
						       << _("Bonus: ")
						       << finishing_bonus << "\n"
						       << _("Gold: ")
						       << (remaining_gold+finishing_bonus);
					}

					// xgettext:no-c-format
					report << "\n" << _("80% of gold is retained for the next scenario") << "\n"
					       << _("Retained Gold: ") << player->gold;
				}
			}

			if (!obs)
				gui::show_dialog(gui, NULL, _("Victory"),
				                 _("You have emerged victorious!"), gui::OK_ONLY);

			if (state_of_game.players.size() > 0 && has_next_scenario)
				gui::show_dialog(gui, NULL, _("Scenario Report"), report.str(), gui::OK_ONLY);

			return VICTORY;
		}
	} //end catch
	catch(replay::error&) {
		gui::show_dialog(gui,NULL,"",_("The file you have tried to load is corrupt"),
		                 gui::OK_ONLY);
		return QUIT;
	}
	catch(network::error& e) {
		bool disconnect = false;
		if(e.socket) {
			e.disconnect();
			disconnect = true;
		}

		turn_info turn_data(gameinfo,state_of_game,status,
					        game_config,level,key,gui,
							map,teams,player_number,units,turn_info::BROWSE_NETWORKED,textbox_info,replay_sender);

		turn_data.save_game(_("A network disconnection has occured, and the game cannot continue. Do you want to save the game?"),gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return QUIT;
		}
	}

	return QUIT;
}
