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
#include "game_events.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "log.hpp"
#include "mapgen.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"
#include "util.hpp"

#include <iostream>
#include <iterator>

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

		for(std::vector<placing_info>::const_iterator i = placings.begin();
		    i != placings.end() && placed.size() != sides.size(); ++i) {
			if(placed.count(i->side) == 0 && positions_taken.count(i->pos) == 0) {
				placed.insert(i->side);
				positions_taken.insert(i->pos);
				map.set_starting_position(i->side,i->pos);
				std::cerr << "placing side " << i->side << " at " << i->pos.x << "," << i->pos.y << "\n";
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
	std::cerr << "starting level '" << string_table["defeat_message"] << "'\n";
	//if the entire scenario should be randomly generated
	if((*level)["scenario_generation"] != "") {
		std::cerr << "randomly generating scenario...\n";
		const cursor::setter cursor_setter(cursor::WAIT);
	
		static config scenario;
		scenario = random_generate_scenario((*level)["scenario_generation"],level->child("generator"));
		level = &scenario;

		state_of_game.starting_pos = scenario;
	}
	
	std::string map_data = (*level)["map_data"];
	if(map_data == "" && (*level)["map"] != "") {
		map_data = read_file("data/maps/" + (*level)["map"]);
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

	const statistics::scenario_context statistics_context(translate_string_default((*level)["id"],(*level)["name"]));

	const int num_turns = atoi(level->values["turns"].c_str());
	gamestatus status(*level,num_turns);

	gamemap map(game_config,map_data);

	CKey key;
	unit_map units;

	const verification_manager verify_manager(units);

	const int xp_modifier = atoi((*level)["experience_modifier"].c_str());
	const unit_type::experience_accelerator xp_mod(xp_modifier > 0 ? xp_modifier : 100);

	std::vector<team> teams;

	int first_human_team = -1;

	const config::child_list& unit_cfg = level->get_children("side");

	if((*level)["modify_placing"] == "true") {
		std::cerr << "modifying placing...\n";
		place_sides_in_preferred_locations(map,unit_cfg);
	}

	std::cerr << "initializing teams..." << unit_cfg.size() << "\n";;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::cerr << "initializing team...\n";

		if(first_human_team == -1 && (**ui)["controller"] == "human") {
			first_human_team = ui - unit_cfg.begin();
		}

		std::string gold = (**ui)["gold"];
		if(gold.empty())
			gold = "100";

		std::cerr << "found gold: '" << gold << "'\n";

		int ngold = lexical_cast_default<int>(gold);
		if(ui == unit_cfg.begin() && state_of_game.gold >= ngold) {
			ngold = state_of_game.gold;
		}

		std::cerr << "set gold to '" << ngold << "'\n";

		//if this side tag describes the leader of the side
		if((**ui)["no_leader"] != "yes") {
			unit new_unit(gameinfo, **ui);

			//search the recall list for leader units, and if there is
			//one, use it in place of the config-described unit
			if(ui == unit_cfg.begin()) {
				for(std::vector<unit>::iterator it = state_of_game.available_units.begin();
					it != state_of_game.available_units.end(); ++it) {
					if(it->can_recruit()) {
						new_unit = *it;
						state_of_game.available_units.erase(it);
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
				std::cerr << "initializing side '" << (**ui)["side"] << "' at " << start_pos.x << "," << start_pos.y << "\n";
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

		teams.push_back(team(**ui,ngold));

		//if the game state specifies units that can be recruited for the player
		//then add them
		if(teams.size() == 1 && state_of_game.can_recruit.empty() == false) {
			std::copy(state_of_game.can_recruit.begin(),state_of_game.can_recruit.end(),
				std::inserter(teams.back().recruits(),teams.back().recruits().end()));
		}
		
		if(teams.size() == 1) {
			state_of_game.can_recruit = teams.back().recruits();
		}

		//if there are additional starting units on this side
		const config::child_list& starting_units = (*ui)->get_children("unit");
		for(config::child_list::const_iterator su = starting_units.begin();
		    su != starting_units.end(); ++su) {
			unit new_unit(gameinfo,**su);
			const std::string& x = (**su)["x"];
			const std::string& y = (**su)["y"];

			const gamemap::location loc(**su);
			if(x.empty() || y.empty() || !map.on_board(loc)) {
				state_of_game.available_units.push_back(new_unit);
			} else {
				units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
				std::cerr << "inserting unit for side " << new_unit.side() << "\n";
			}
		}
	}

	const teams_manager team_manager(teams);

	const config* theme_cfg = NULL;
	if((*level)["theme"] != "") {
		theme_cfg = game_config.find_child("theme","name",(*level)["theme"]);
	}

	if(theme_cfg == NULL) {
		theme_cfg = game_config.find_child("theme","name",preferences::theme());
	}

	const config dummy_cfg;
	display gui(units,video,map,status,teams,theme_cfg != NULL ? *theme_cfg : dummy_cfg, game_config);

	//object that will make sure that labels are removed at the end of the scenario
	const font::floating_label_manager labels_manager;

	gui.labels().read(*level);

	if(first_human_team != -1) {
		gui.set_team(first_human_team);
	}

	const preferences::display_manager prefs_disp_manager(&gui);
	const tooltips::manager tooltips_manager(gui);

	if(recorder.skipping() == false) {
		for(std::vector<config*>::const_iterator story_i = story.begin();
		    story_i != story.end(); ++story_i) {
			show_intro(gui,**story_i, state_of_game);
		}

		show_map_scene(gui,*level);
	}

	const std::string& music = level->values["music"];
	if(music != "") {
		sound::play_music(music);
	}

	victory_conditions::set_victory_when_enemies_defeated(
						(*level)["victory_when_enemies_defeated"] != "no");

	game_events::manager events_manager(*level,gui,map,units,teams,
	                                    state_of_game,status,gameinfo);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level->get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui.add_overlay(gamemap::location(**overlay),(**overlay)["image"]);
	}

	int turn = 1, player_number = 0;

	turn_info::floating_textbox textbox_info;

	try {
		gui.create_buttons();
		gui.adjust_colours(0,0,0);
		game_events::fire("prestart");

		if(first_human_team != -1) {
			clear_shroud(gui,status,map,gameinfo,units,teams,first_human_team);
			gui.scroll_to_tile(map.starting_position(first_human_team+1).x,map.starting_position(first_human_team+1).y,display::WARP);
		}
	
		gui.scroll_to_tile(map.starting_position(1).x,map.starting_position(1).y,display::WARP);

		bool replaying = (recorder.at_end() == false);
	
		//if a team is specified whose turn it is, it means we're loading a game
		//instead of starting a fresh one
		const bool loading_game = (*level)["playing_team"].empty() == false;
		int first_player = atoi((*level)["playing_team"].c_str());
		if(first_player < 0 || first_player >= int(teams.size())) {
			first_player = 0;
		}

		std::cerr << "starting main loop\n";

		std::deque<config> data_backlog;
		
		for(bool first_time = true; true; first_time = false, first_player = 0) {
			player_number = 0;

			if(first_time) {
				const hotkey::basic_handler key_events_handler(&gui);

				std::cerr << "first_time..." << (recorder.skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui,recorder.skipping());
				events::raise_draw_event();
				game_events::fire("start");
				gui.draw();
			}

			gui.new_turn();
			gui.invalidate_game_status();
			events::raise_draw_event();

			std::cerr << "turn: " << turn++ << "\n";

			for(std::vector<team>::iterator team_it = teams.begin()+first_player;
			    team_it != teams.end(); ++team_it) {
				log_scope("player turn");
				player_number = (team_it - teams.begin()) + 1;

				//if a side is dead, don't do their turn
				if(team_units(units,player_number) == 0)
					continue;

				if(is_observer(teams)) {
					gui.set_team(size_t(player_number-1));
				}

				std::stringstream player_number_str;
				player_number_str << player_number;
				game_events::set_variable("side_number",player_number_str.str());
				game_events::fire("side turn");

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

					calculate_healing(gui,map,units,player_number,teams);
				}

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
					std::cerr << "doing replay " << player_number << "\n";
					try {
						replaying = do_replay(gui,map,gameinfo,units,teams,
						                      player_number,status,state_of_game);
					} catch(replay::error& e) {
						std::cerr << "caught replay::error\n";
						gui::show_dialog(gui,NULL,"",string_table["bad_save_message"],gui::OK_ONLY);

						replaying = false;
					}
					std::cerr << "result of replay: " << (replaying?"true":"false") << "\n";
				}

				if(!replaying && team_it->music().empty() == false) {
					std::cerr << "playing music: '" << team_it->music() << "'\n";
					sound::play_music(team_it->music());
				}

//goto this label if the type of a team (human/ai/networked) has changed mid-turn
redo_turn:

				if(!replaying && team_it->is_human()) {
					std::cerr << "is human...\n";

					if(first_time && team_it == teams.begin() &&
					   level->values["objectives"].empty() == false) {
						dialogs::show_objectives(gui,*level);
					}

					play_turn(gameinfo,state_of_game,status,game_config,
					          level, video, key, gui, events_manager, map,
							  teams, player_number, units, textbox_info);

					if(game_config::debug)
						display::clear_debug_highlights();

					std::cerr << "human finished turn...\n";

				} else if(!replaying && team_it->is_ai()) {
					std::cerr << "is ai...\n";
					gui.recalculate_minimap();

					const cursor::setter cursor_setter(cursor::WAIT);

					const int start_command = recorder.ncommands();

					update_locker lock(gui,!preferences::show_ai_moves());

					turn_info turn_data(gameinfo,state_of_game,status,
						                    game_config,level,key,gui,
						                    map,teams,player_number,units,true,textbox_info);

					ai_interface::info ai_info(gui,map,gameinfo,units,teams,player_number,status,turn_data,recorder);
					util::scoped_ptr<ai_interface> ai_obj(create_ai(team_it->ai_algorithm(),ai_info));
					ai_obj->play_turn();
					ai_obj->sync_network();

					gui.invalidate_unit();
					gui.invalidate_game_status();
					gui.invalidate_all();
					gui.draw();
					SDL_Delay(500);
				} else if(!replaying && team_it->is_network()) {
					std::cerr << "is networked...\n";

					turn_info turn_data(gameinfo,state_of_game,status,
					                    game_config,level,key,gui,
					                    map,teams,player_number,units,true,textbox_info);

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

						const int ncommand = recorder.ncommands();
						turn_data.turn_slice();
						turn_data.send_data(ncommand);
						gui.draw();
					}

					std::cerr << "finished networked...\n";
				}

				for(unit_map::iterator uit = units.begin(); uit != units.end(); ++uit) {
					if(uit->second.side() == player_number)
						uit->second.end_turn();
				}

				team_it->get_shared_maps();

				game_events::pump();

				check_victory(units,teams);
			}

			//time has run out
			if(!status.next_turn()) {
			
				if(non_interactive()) {
					std::cout << "time over (draw)\n";
				}

				std::cerr << "firing time over event...\n";
				game_events::fire("time over");
				std::cerr << "done firing time over event...\n";

				throw end_level_exception(DEFEAT);
			}

			std::stringstream event_stream;
			event_stream << status.turn();

			{
				std::cerr << "turn event..." << (recorder.skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui,recorder.skipping());
				const std::string turn_num = event_stream.str();
				game_events::set_variable("turn_number",turn_num);
				game_events::fire("turn " + turn_num);
				game_events::fire("new turn");
			}
		} //end for loop

	} catch(end_level_exception& end_level) {

		if((end_level.result == DEFEAT || end_level.result == VICTORY) && is_observer(teams)) {
			gui::show_dialog(gui,NULL,string_table["observer_endgame_heading"],
			                          string_table["observer_endgame"], gui::OK_ONLY);
		}

		if(end_level.result == QUIT || end_level.result == REPLAY) {
			return end_level.result;
		} else if(end_level.result == DEFEAT) {
			try {
				game_events::fire("defeat");
			} catch(end_level_exception&) {
			}

			gui::show_dialog(gui,NULL,
			                 string_table["defeat_heading"],
			                 string_table["defeat_message"],
			                 gui::OK_ONLY);
			return DEFEAT;
		} else if(end_level.result == VICTORY || end_level.result == CONTINUE) {
			try {
				game_events::fire("victory");
			} catch(end_level_exception&) {
			}

			//add all the units that survived the scenario
			for(std::map<gamemap::location,unit>::iterator un = units.begin(); un != units.end(); ++un) {
				if(un->second.side() == 1) {
					un->second.new_turn();
					un->second.new_level();
					state_of_game.available_units.push_back(un->second);
				}
			}

			//'continue' is like a victory, except it doesn't announce victory,
			//and the player returns 100% of gold.
			if(end_level.result == CONTINUE) {
				state_of_game.gold = teams[0].gold();
				return VICTORY;
			}

			const int remaining_gold = teams[0].gold();
			const int finishing_bonus_per_turn = map.villages().size()*game_config::village_income + game_config::base_income;
			const int turns_left = maximum<int>(0,status.number_of_turns() - status.turn());
			const int finishing_bonus = end_level.gold_bonus ?
			              (finishing_bonus_per_turn * turns_left) : 0;
			state_of_game.gold = ((remaining_gold+finishing_bonus)*80)/100;

			gui::show_dialog(gui,NULL,string_table["victory_heading"],
			                 string_table["victory_message"],gui::OK_ONLY);
			std::stringstream report;
			report << string_table["remaining_gold"] << ": "
			       << remaining_gold << "\n";
			if(end_level.gold_bonus) {
				report << string_table["early_finish_bonus"] << ": "
				       << finishing_bonus_per_turn
					   << " " << string_table["per_turn"] << "\n"
				       << string_table["turns_finished_early"] << ": "
				       << turns_left << "\n"
				       << string_table["bonus"] << ": "
					   << finishing_bonus << "\n"
				       << string_table["gold"] << ": "
					   << (remaining_gold+finishing_bonus);
			}

			report << "\n" << string_table["fifty_percent"] << "\n"
				   << string_table["retained_gold"] << ": "
				   << state_of_game.gold;

			gui::show_dialog(gui,NULL,"",report.str(),gui::OK_ONLY);
			return VICTORY;
		}
	} //end catch
	catch(replay::error& e) {
		std::cerr << "caught replay::error\n";
		gui::show_dialog(gui,NULL,"",string_table["bad_save_message"],
		                 gui::OK_ONLY);
		return QUIT;
	}
	catch(network::error& e) {
		if(e.socket) {
			e.disconnect();
		}

		turn_info turn_data(gameinfo,state_of_game,status,
					        game_config,level,key,gui,
					        map,teams,player_number,units,true,textbox_info);

		turn_data.save_game(string_table["save_game_error"]);
		throw network::error();
	}

	return QUIT;
}
