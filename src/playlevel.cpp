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
#include "game_events.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "tooltips.hpp"

#include <iostream>

LEVEL_RESULT play_level(game_data& gameinfo, config& terrain_config,
                        config* level, CVideo& video,
                        game_state& state_of_game,
						std::vector<config*>& story)
{
	const int num_turns = atoi(level->values["turns"].c_str());
	gamestatus status(*level,num_turns);

	gamemap map(terrain_config,read_file("data/maps/" + level->values["map"]));

	CKey key;
	unit_map units;

	std::vector<team> teams;

	int first_human_team = -1;

	std::vector<config*>& unit_cfg = level->children["side"];
	for(std::vector<config*>::iterator ui = unit_cfg.begin();
				ui != unit_cfg.end(); ++ui) {
		unit new_unit(gameinfo, **ui);
		if(ui == unit_cfg.begin()) {
			for(std::vector<unit>::iterator it =
							state_of_game.available_units.begin();
			    it != state_of_game.available_units.end(); ++it) {
				if(it->can_recruit()) {
					new_unit = *it;
					state_of_game.available_units.erase(it);
					break;
				}
			}
		}

		std::string gold = (*ui)->values["gold"];
		if(gold.empty())
			gold = "100";

		int ngold = ::atoi(gold.c_str());
		if(ui == unit_cfg.begin() && state_of_game.gold >= 0 &&
		   (*level)["disallow_recall"] != "yes")
			ngold = state_of_game.gold;

		const gamemap::location& start_pos =
		             map.starting_position(new_unit.side());

		if(!start_pos.valid()) {
			std::stringstream err;
			err << "No starting position for side " << new_unit.side();
			throw gamestatus::load_game_failed(err.str());
		}

		units.insert(std::pair<gamemap::location,unit>(
					     map.starting_position(new_unit.side()), new_unit));
		teams.push_back(team(**ui,ngold));

		if(first_human_team == -1 && teams.back().is_human()) {
			first_human_team = teams.size()-1;
		}

		//if there are additional starting units on this side
		std::vector<config*>& starting_units = (*ui)->children["unit"];
		for(std::vector<config*>::iterator su = starting_units.begin();
		    su != starting_units.end(); ++su) {
			unit new_unit(gameinfo,**su);
			const std::string& x = (*su)->values["x"];
			const std::string& y = (*su)->values["y"];

			const gamemap::location loc(**su);
			if(x.size() == 0 || y.size() == 0 || !map.on_board(loc)) {
				state_of_game.available_units.push_back(new_unit);
			} else {
				units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
			}
		}
	}

	display gui(units,video,map,status,teams);

	if(first_human_team != -1) {
		gui.set_team(first_human_team);
	}

	const preferences::display_manager prefs_disp_manager(&gui);
	const tooltips::manager tooltips_manager(gui);

	if(recorder.skipping() == false) {
		for(std::vector<config*>::iterator story_i = story.begin();
		    story_i != story.end(); ++story_i) {
			show_intro(gui,**story_i);
		}

		show_map_scene(gui,*level);
	}

	const std::string& music = level->values["music"];
	if(!music.empty()) {
		sound::play_music(music);
	}

	game_events::manager events_manager(*level,gui,map,units,teams,
	                                    state_of_game,gameinfo);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	std::vector<config*>& overlays = level->children["item"];
	for(std::vector<config*>::iterator overlay = overlays.begin();
	    overlay != overlays.end(); ++overlay) {
		gui.add_overlay(gamemap::location(**overlay),
		                (*overlay)->values["image"]);
	}

	for(unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		i->second.new_turn();
	}

	gui.scroll_to_tile(map.starting_position(1).x,map.starting_position(1).y,
	                   display::WARP);

	bool replaying = (recorder.empty() == false);

	int turn = 1;
	std::cout << "starting main loop\n";
	for(bool first_time = true; true; first_time = false) {
		try {

			if(first_time) {
				const hotkey::basic_handler key_events_handler(gui);
				clear_shroud(gui,map,gameinfo,units,teams,0);

				update_locker lock_display(gui,recorder.skipping());
				game_events::fire("start");
				gui.draw();
			}

			gui.new_turn();
			gui.invalidate_game_status();

			std::cerr << "turn: " << turn++ << "\n";

			for(std::vector<team>::iterator team_it = teams.begin();
			    team_it != teams.end(); ++team_it) {
				const int player_number = (team_it - teams.begin()) + 1;

				//if a side is dead, don't do their turn
				if(team_units(units,player_number) == 0)
					continue;

				if(!first_time) {
					team_it->new_turn();

					//if the expense is less than the number of villages owned,
					//then we don't have to pay anything at all
					const int expense = team_upkeep(units,player_number) -
										team_it->towers().size();
					if(expense > 0) {
						team_it->spend_gold(expense);
					}
				}

				gui.set_playing_team(size_t(player_number-1));

				clear_shroud(gui,map,gameinfo,units,teams,player_number-1);

				calculate_healing(gui,map,units,player_number);

				//scroll the map to the leader
				const unit_map::iterator leader =
				                   find_leader(units,player_number);

				if(leader != units.end() && !recorder.skipping()) {
					const hotkey::basic_handler key_events_handler(gui);
					gui.scroll_to_tile(leader->first.x,leader->first.y);
				}

				if(replaying) {
					const hotkey::basic_handler key_events_handler(gui);
					std::cerr << "doing replay " << player_number << "\n";
					replaying = do_replay(gui,map,gameinfo,units,teams,
					                      player_number,status,state_of_game);
					std::cerr << "result of replay: " << (replaying?"true":"false") << "\n";
				}

				if(!replaying && team_it->music().empty() == false) {
					sound::play_music(team_it->music());
				}

				if(!replaying && team_it->is_human()) {
					std::cerr << "is human...\n";

					if(first_time && team_it == teams.begin() &&
					   level->values["objectives"].empty() == false) {
						dialogs::show_objectives(gui,*level);
					}

					play_turn(gameinfo,state_of_game,status,terrain_config,
					          level, video, key, gui, events_manager, map,
							  teams, player_number, units);

					if(game_config::debug)
						display::clear_debug_highlights();

				} else if(!replaying && team_it->is_ai()) {
					const hotkey::basic_handler key_events_handler(gui);

					const int start_command = recorder.ncommands();

					update_locker lock(gui,!preferences::show_ai_moves());

					ai::do_move(gui,map,gameinfo,units,teams,
					            player_number,status);

					if(network::nconnections() > 0) {
						config cfg;
						cfg.children["turn"].push_back(
							  new config(recorder.get_data_range(start_command,
							                        recorder.ncommands())));
						network::send_data(cfg);
					}

					gui.invalidate_unit();
					gui.invalidate_game_status();
					gui.invalidate_all();
					gui.draw();
					SDL_Delay(500);
				} else if(!replaying && team_it->is_network()) {
					std::cerr << "is networked...\n";

					bool turn_end = false;

					while(!turn_end) {

						config cfg;
	
						turn_info turn_data(gameinfo,state_of_game,status,
						                    terrain_config,level,key,gui,
						                    map,teams,player_number,units,true);

						for(;;) {
							network::connection res =
							           network::receive_data(cfg);
							if(res && cfg.child("leave_game")) {
								throw network::error("");
							}

							if(res && cfg.children["turn"].empty() == false) {
								break;
							}

							turn_data.turn_slice();
							gui.draw();
						}

						std::cerr << "replay: '" << cfg.children["turn"].front()->write() << "'\n";
						replay replay_obj(*cfg.children["turn"].front());
						replay_obj.start_replay();

						try {
							turn_end = do_replay(gui,map,gameinfo,units,teams,
							   player_number,status,state_of_game,&replay_obj);
						} catch(replay::error&) {
							std::string output = "errorlog";
							const int res = gui::show_dialog(gui,NULL,"",
							               string_table["network_sync_error"],
							               gui::YES_NO,NULL,NULL,"",&output);
							if(res == 0 && output.empty() == false) {
								recorder.mark_current();
								recorder.save_game(gameinfo,output);
							}

							return QUIT;
						}

						recorder.add_config(*cfg.children["turn"].front());

						gui.invalidate_all();
						gui.draw();
					}

					std::cerr << "finished networked...\n";
				}

				for(unit_map::iterator uit = units.begin();
				    uit != units.end(); ++uit) {
					if(uit->second.side() == player_number)
						uit->second.end_turn();
				}

				game_events::pump();

				check_victory(units,teams);
			}

			//time has run out
			if(!status.next_turn()) {
				game_events::fire("time over");
				throw end_level_exception(DEFEAT);
			}

			std::stringstream event_stream;
			event_stream << "turn " << status.turn();

			{
				update_locker lock_display(gui,recorder.skipping());
				game_events::fire(event_stream.str());
			}

			for(unit_map::iterator i = units.begin();
			    i != units.end(); ++i) {
				i->second.new_turn();
			}

		} catch(end_level_exception& end_level) {

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
			} else if(end_level.result == VICTORY) {
				try {
					game_events::fire("victory");
				} catch(end_level_exception&) {
				}

				if((*level)["disallow_recall"] == "yes") {
					return VICTORY;
				}

				//add all the units that survived the scenario
				for(std::map<gamemap::location,unit>::iterator un =
				    units.begin(); un != units.end(); ++un) {
					if(un->second.side() == 1) {
						un->second.new_level();
						state_of_game.available_units.
						                push_back(un->second);
					}
				}

				const int remaining_gold = teams[0].gold();
				const int finishing_bonus_per_turn =
				      map.towers().size()*game_config::tower_income;
				const int turns_left = status.number_of_turns() - status.turn();
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

			std::string label = string_table["multiplayer_save_name"];
			const int res = gui::show_dialog(gui,NULL,"",
			                 string_table["save_game_error"],
			                 gui::OK_CANCEL,NULL,NULL,
			                 string_table["save_game_label"],&label);
			if(res == 0) {
				recorder.save_game(gameinfo,label);
			}

			return QUIT;
		}

	} //end for(;;)

	return QUIT;
}
