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

#include "actions.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "log.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "util.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

void play_turn(game_data& gameinfo, game_state& state_of_game,
               gamestatus& status, config& terrain_config, config* level,
			   CVideo& video, CKey& key, display& gui,
               game_events::manager& events_manager, gamemap& map,
			   std::vector<team>& teams, int team_num,
			   std::map<gamemap::location,unit>& units)
{
	log_scope("player turn");

	gui.set_team(team_num-1);
	gui.invalidate_all();
	gui.draw();
	gui.update_display();

	team& current_team = teams[team_num-1];

	const std::string menu_items[] = {"scenario_objectives","recruit",
	                                  "recall","unit_list","save_game",
									  "preferences","end_turn"};
	std::vector<std::string> menu;
	for(const std::string* menu_items_ptr = menu_items;
	    menu_items_ptr != menu_items + sizeof(menu_items)/sizeof(*menu_items);
		++menu_items_ptr) {
		menu.push_back(string_table[*menu_items_ptr]);
	}

	gamemap::location next_unit;

	bool left_button = false, right_button = false;

	const paths_wiper wiper(gui);
	paths current_paths;
	paths::route current_route;
	bool enemy_paths = false;

	gamemap::location last_hex;
	gamemap::location selected_hex;

	undo_list undo_stack, redo_stack;

	//execute gotos - first collect gotos in a list
	std::vector<gamemap::location> gotos;

	for(unit_map::iterator ui = units.begin(); ui != units.end(); ++ui) {
		if(ui->second.get_goto() == ui->first)
			ui->second.set_goto(gamemap::location());

		if(ui->second.side() == team_num && map.on_board(ui->second.get_goto()))
			gotos.push_back(ui->first);
	}

	for(std::vector<gamemap::location>::const_iterator g = gotos.begin();
	    g != gotos.end(); ++g) {

		const unit_map::iterator ui = units.find(*g);

		assert(ui != units.end());

		unit u = ui->second;
		const shortest_path_calculator calc(u,current_team,units,map);
		const bool can_teleport = u.type().teleports() &&
		          map[ui->first.x][ui->first.y] == gamemap::TOWER;

		const std::set<gamemap::location>* const teleports =
		     can_teleport ? &current_team.towers() : NULL;

		paths::route route = a_star_search(ui->first,ui->second.get_goto(),
		                                   10000.0,calc,teleports);
		if(route.steps.empty())
			continue;

		assert(route.steps.front() == *g);

		gui.set_route(&route);
		const size_t moves =
		    move_unit(&gui,map,units,teams,route.steps,&recorder,&undo_stack);
		if(moves > 0) {
			redo_stack.clear();
		}
	}

	std::cerr << "done gotos\n";

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool new_left_button = mouse_flags & SDL_BUTTON_LMASK;
		const bool new_right_button = mouse_flags & SDL_BUTTON_RMASK;

		gamemap::location new_hex = gui.hex_clicked_on(mousex,mousey);

		if(new_hex.valid() == false) {
			current_route.steps.clear();
			gui.set_route(NULL);
		}

		//highlight the hex that is currently moused over
		if(new_hex != last_hex) {
			gui.highlight_hex(new_hex);

			if(enemy_paths && new_hex != last_hex) {
				gui.set_paths(NULL);
				current_paths = paths();
				enemy_paths = false;
			}

			if(new_hex == selected_hex) {
				current_route.steps.clear();
				gui.set_route(NULL);
			} else if(!enemy_paths && new_hex != last_hex &&
			   !current_paths.routes.empty() && map.on_board(selected_hex) &&
			   map.on_board(new_hex)) {
				
				const unit_map::const_iterator un = units.find(selected_hex);
				if(un != units.end()) {
					const shortest_path_calculator calc(un->second,current_team,
					                                    units,map);
					const bool can_teleport = un->second.type().teleports() &&
					   map[selected_hex.x][selected_hex.y] == gamemap::TOWER;

					const std::set<gamemap::location>* const teleports =
					     can_teleport ? &current_team.towers() : NULL;

					current_route = a_star_search(selected_hex,new_hex,
					                              10000.0,calc,teleports);

					gui.set_route(&current_route);
				}
			}
		}

		HOTKEY_COMMAND command = HOTKEY_NULL;

		if(new_left_button) {
			const gamemap::location loc=gui.minimap_location_on(mousex,mousey);
			if(loc.valid()) {
				gui.scroll_to_tile(loc.x,loc.y,display::WARP);
			}
		} else if(new_hex != last_hex && current_paths.routes.empty()) {
			const unit_map::iterator u = units.find(new_hex);
			if(u != units.end() && u->second.side() != team_num) {
				const bool ignore_zocs = u->second.type().is_skirmisher();
				const bool teleport = u->second.type().teleports();
				current_paths = paths(map,gameinfo,units,new_hex,teams,
				                      ignore_zocs,teleport);
				gui.set_paths(&current_paths);
				enemy_paths = true;
			}
		}

		last_hex = new_hex;

		if(!left_button && new_left_button) {
			gamemap::location hex = gui.hex_clicked_on(mousex,mousey);

			unit_map::iterator u = units.find(selected_hex);

			//if the unit is selected and then itself clicked on,
			//any goto command is cancelled
			if(selected_hex == hex && u->second.side() == team_num) {
				u->second.set_goto(gamemap::location());
			}

			//if we can move to that tile
			std::map<gamemap::location,paths::route>::const_iterator
					route = enemy_paths ? current_paths.routes.end() :
			                              current_paths.routes.find(hex);
			unit_map::iterator enemy = units.find(hex);

			//see if we're trying to attack an enemy
			if(route != current_paths.routes.end() && enemy != units.end() &&
			   hex != selected_hex &&
			   enemy->second.side() != u->second.side()) {

				const std::vector<attack_type>& attacks = u->second.attacks();
				std::vector<std::string> items;

				std::vector<unit> units_list;

				for(size_t a = 0; a != attacks.size(); ++a) {
					const battle_stats stats = evaluate_battle_stats(
					                               map,selected_hex,hex,
												   a,units,status,gameinfo);

					const std::string& lang_attack_name =
					            string_table["weapon_name_"+stats.attack_name];
					const std::string& lang_attack_type =
					            string_table["weapon_type_"+stats.attack_type];
					const std::string& lang_range =
					            string_table[stats.range == "Melee" ?
					                            "short_range" : "long_range"];

					const std::string& lang_defend_name =
					            string_table["weapon_name_"+stats.defend_name];
					const std::string& lang_defend_type =
					            string_table["weapon_type_"+stats.defend_type];

					const std::string& attack_name = lang_attack_name.empty() ?
					                    stats.attack_name : lang_attack_name;
					const std::string& attack_type = lang_attack_type.empty() ?
					                    stats.attack_type : lang_attack_type;
					const std::string& defend_name = lang_defend_name.empty() ?
					                    stats.defend_name : lang_defend_name;
					const std::string& defend_type = lang_defend_type.empty() ?
					                    stats.defend_type : lang_defend_type;

					const std::string& range = lang_range.empty() ?
					                    stats.range : lang_range;

					std::stringstream att;
					att << attack_name << " (" << attack_type
					    << ") " << stats.damage_defender_takes << "-"
						<< stats.nattacks << " " << range << " "
						<< int(util::round(100.0*stats.chance_to_hit_defender))
					    << "%";

					att << "," << string_table["versus"] << ",";
					att << defend_name << " (" << defend_type
					    << ") " << stats.damage_attacker_takes << "-"
						<< stats.ndefends << " "
						<< int(util::round(100.0*stats.chance_to_hit_attacker))
					    << "%";

					items.push_back(att.str());
					units_list.push_back(enemy->second);
				}

				//make it so that when we attack an enemy, the attacking unit
				//is again shown in the status bar, so that we can easily
				//compare between the attacking and defending unit
				gui.highlight_hex(gamemap::location());
				gui.draw(true,true);

				const int res = gui::show_dialog(gui,NULL,"",
				                           string_table["choose_weapon"]+":\n",
				                           gui::OK_CANCEL,&items,&units_list);

				if(size_t(res) < attacks.size()) {
					u->second.set_goto(gamemap::location());
					undo_stack.clear();
					redo_stack.clear();

					current_paths = paths();
					gui.set_paths(NULL);

					game_events::fire("attack",selected_hex,hex);

					//the event could have killed either the attacker or
					//defender, so we have to make sure they still exist
					u = units.find(selected_hex);
					enemy = units.find(hex);

					if(u == units.end() || enemy == units.end() ||
					   size_t(res) >= u->second.attacks().size())
						continue;

					gui.invalidate_all();
					gui.draw();

					recorder.add_attack(selected_hex,hex,res);

					attack(gui,map,selected_hex,hex,res,units,
					       status,gameinfo,true);

					dialogs::advance_unit(gameinfo,units,selected_hex,gui);
					dialogs::advance_unit(gameinfo,units,hex,gui);

					selected_hex = gamemap::location();
					current_route.steps.clear();
					gui.set_route(NULL);

					gui.invalidate_unit();
					gui.draw(); //clear the screen

					const int winner = check_victory(units);

					//if the player has won
					if(winner == team_num) {
						throw end_level_exception(VICTORY);
					} else if(winner >= 1) {
						throw end_level_exception(DEFEAT);
					}
				}
			}

			//otherwise we're trying to move to a hex
			else if(selected_hex.valid() && selected_hex != hex &&
				     units.count(selected_hex) &&
				     enemy == units.end() && !current_route.steps.empty() &&
				     current_route.steps.front() == selected_hex) {

				const size_t moves = move_unit(&gui,map,units,teams,
				                   current_route.steps,&recorder,&undo_stack);

				redo_stack.clear();

				selected_hex = gamemap::location();
				gui.set_route(NULL);
				gui.select_hex(gamemap::location());
				gui.set_paths(NULL);
				current_paths = paths();

				assert(moves <= current_route.steps.size());
				const gamemap::location& dst = current_route.steps[moves-1];

				current_route.steps.clear();

				//if there is an enemy in a surrounding hex, then
				//highlight attack options
				gamemap::location adj[6];
				get_adjacent_tiles(dst,adj);

				int n;
				for(n = 0; n != 6; ++n) {
					const unit_map::const_iterator u_it = units.find(adj[n]);
					if(u_it != units.end() && u_it->second.side() != team_num
					   && current_team.is_enemy(u_it->second.side())){
						current_paths.routes[adj[n]] = paths::route();
					}
				}

				if(current_paths.routes.empty() == false) {
					current_paths.routes[dst] = paths::route();
					selected_hex = dst;
					gui.select_hex(dst);
					gui.set_paths(&current_paths);
				}

				if(clear_shroud(gui,map,gameinfo,units,teams,team_num-1)) {
					undo_stack.clear();
				}
			} else {
				gui.set_paths(NULL);
				current_paths = paths();

				selected_hex = hex;
				gui.select_hex(hex);
				current_route.steps.clear();
				gui.set_route(NULL);

				const unit_map::iterator it = units.find(hex);
				if(it != units.end() && it->second.side() == team_num) {
					const bool ignore_zocs = it->second.type().is_skirmisher();
					const bool teleport = it->second.type().teleports();
					current_paths = paths(map,gameinfo,units,hex,teams,
					                      ignore_zocs,teleport);
					gui.set_paths(&current_paths);

					unit u = it->second;
					const gamemap::location go_to = u.get_goto();
					if(map.on_board(go_to)) {
						const shortest_path_calculator calc(u,current_team,
						                                    units,map);

						const std::set<gamemap::location>* const teleports =
						        teleport ? &current_team.towers() : NULL;

						paths::route route = a_star_search(it->first,go_to,
						                               10000.0,calc,teleports);
						gui.set_route(&route);
					}
				}
			}
		}

		left_button = new_left_button;

		if(!right_button && new_right_button) {
			if(!current_paths.routes.empty()) {
				selected_hex = gamemap::location();
				gui.select_hex(gamemap::location());
				gui.set_paths(NULL);
				current_paths = paths();
				current_route.steps.clear();
				gui.set_route(NULL);
			} else {
				const unit_map::const_iterator un = units.find(
				                            gui.hex_clicked_on(mousex,mousey));
				if(un != units.end()) {
					menu.push_back(string_table["describe_unit"]);
				}

				const int res = gui::show_dialog(gui,NULL,"",
				                                 string_table["options"]+":\n",
				                                 gui::MESSAGE,&menu);

				const std::string result = res != -1 ? menu[res] : "";

				if(un != units.end()) {
					menu.pop_back();
				}

				if(result == string_table["describe_unit"]) {
					command = HOTKEY_UNIT_DESCRIPTION;
				}

				else if(result == string_table["preferences"]) {
					preferences::show_preferences_dialog(gui);
				}

				else if(result == string_table["end_turn"]) {
					command = HOTKEY_ENDTURN;
				}

				else if(result == string_table["scenario_objectives"]) {
					dialogs::show_objectives(gui,*level);
				}

				else if(result == string_table["recall"]) {
					command = HOTKEY_RECALL;
				}
				else if(result == string_table["recruit"]) {
					command = HOTKEY_RECRUIT;
				} else if(result == string_table["unit_list"]) {
					const std::string heading = string_table["name"] + "," +
					                            string_table["hp"] + "," +
					                            string_table["xp"] + "," +
					                            string_table["moves"] + "," +
					                            string_table["location"];

					std::vector<std::string> items;
					items.push_back(heading);

					std::vector<unit> units_list;
					for(unit_map::const_iterator i = units.begin();
					    i != units.end(); ++i) {
						if(i->second.side() != team_num)
							continue;

						std::stringstream row;
						row << i->second.name() << "," << i->second.hitpoints()
						    << "/" << i->second.max_hitpoints() << ","
							<< i->second.experience() << "/"
							<< i->second.max_experience() << ","
							<< i->second.movement_left() << "/"
							<< i->second.total_movement() << ","
							<< (i->first.x+1) << "-" << (i->first.y+1);

						items.push_back(row.str());

						//extra unit for the first row to make up the heading
						if(units_list.empty())
							units_list.push_back(i->second);

						units_list.push_back(i->second);
					}

					gui::show_dialog(gui,NULL,string_table["unit_list"],"",
					                 gui::OK_ONLY,&items,&units_list);
				} else if(result == string_table["save_game"]) {
					command = HOTKEY_SAVE_GAME;
				}
			}
		}

		right_button = new_right_button;

		if(key[KEY_UP] || mousey == 0)
			gui.scroll(0.0,-preferences::scroll_speed());

		if(key[KEY_DOWN] || mousey == gui.y()-1)
			gui.scroll(0.0,preferences::scroll_speed());

		if(key[KEY_LEFT] || mousex == 0)
			gui.scroll(-preferences::scroll_speed(),0.0);

		if(key[KEY_RIGHT] || mousex == gui.x()-1)
			gui.scroll(preferences::scroll_speed(),0.0);

		if(command == HOTKEY_NULL)
			command = check_keys(gui);

		if(command == HOTKEY_ENDTURN) {
			recorder.save_game(gameinfo,string_table["auto_save"]);
			recorder.end_turn();
			return;
		}

		if(command == HOTKEY_RECRUIT) {
			std::vector<unit> sample_units;

			gui.draw(); //clear the old menu
			std::vector<std::string> item_keys;
			std::vector<std::string> items;
			const std::set<std::string>& recruits
			                        = current_team.recruits();
			for(std::set<std::string>::const_iterator it =
			    recruits.begin(); it != recruits.end(); ++it) {
				item_keys.push_back(*it);
				const std::map<std::string,unit_type>::const_iterator
						u_type = gameinfo.unit_types.find(*it);
				if(u_type == gameinfo.unit_types.end()) {
					std::cerr << "could not find " << *it << std::endl;
					assert(false);
					continue;
				}

				const unit_type& type = u_type->second;
				std::stringstream description;

				description << type.language_name() << ","
				            << type.cost() << " gold";
				items.push_back(description.str());
				sample_units.push_back(unit(&type,team_num));
			}

			const int recruit_res =
			      gui::show_dialog(gui,NULL,"",
			                       string_table["recruit_unit"] + ":\n",
			                       gui::OK_CANCEL,&items,&sample_units);
			if(recruit_res != -1) {
				const std::string& name = item_keys[recruit_res];
				const std::map<std::string,unit_type>::const_iterator
						u_type = gameinfo.unit_types.find(name);
				assert(u_type != gameinfo.unit_types.end());

				if(u_type->second.cost() > current_team.gold()) {
					gui::show_dialog(gui,NULL,"",
					     string_table["not_enough_gold_to_recruit"],
						 gui::OK_ONLY);
				} else {
					//create a unit with traits
					recorder.add_recruit(recruit_res,last_hex);
					unit new_unit(&(u_type->second),team_num,true);
					const std::string& msg =
					   recruit_unit(map,team_num,units,new_unit,last_hex,&gui);
					if(msg.empty()) {
						current_team.spend_gold(u_type->second.cost());
					} else {
						recorder.undo();
						gui::show_dialog(gui,NULL,"",msg,gui::OK_ONLY);
					}

					undo_stack.clear();
					redo_stack.clear();

					gui.invalidate_game_status();
				}
			}
		}

		if(command == HOTKEY_RECALL) {
			//sort the available units into order by value
			//so that the most valuable units are shown first
			std::sort(state_of_game.available_units.begin(),
			          state_of_game.available_units.end(),
					  compare_unit_values());

			gui.draw(); //clear the old menu
			if(state_of_game.available_units.empty()) {
				gui::show_dialog(gui,NULL,"",string_table["no_recall"]);
			} else if(current_team.gold() < game_config::recall_cost) {
				std::stringstream msg;
				msg << string_table["not_enough_gold_to_recall_1"]
				    << " " << game_config::recall_cost << " "
					<< string_table["not_enough_gold_to_recall_2"];
				gui::show_dialog(gui, NULL,"",msg.str());
			} else {
				std::vector<std::string> options;
				for(std::vector<unit>::const_iterator unit =
				  state_of_game.available_units.begin();
				  unit != state_of_game.available_units.end(); ++unit) {
					std::stringstream option;
					option << unit->type().language_name() << ","
					       << string_table["level"] << ": "
					       << unit->type().level() << ","
					       << string_table["xp"] << ": "
					       << unit->experience() << "/"
					       << unit->max_experience();
					options.push_back(option.str());
				}

				const int res = gui::show_dialog(gui,NULL,"",
				           string_table["select_unit"] + ":\n",
				           gui::OK_CANCEL,&options,
						   &state_of_game.available_units);
				if(res >= 0) {

					const std::string err = recruit_unit(map,team_num,
					  units,state_of_game.available_units[res],
					  last_hex,&gui);
					if(!err.empty()) {
						gui::show_dialog(gui,NULL,"",err,gui::OK_ONLY);
					} else {
						current_team.spend_gold(
						             game_config::recall_cost);
						state_of_game.available_units.erase(
						   state_of_game.available_units.begin()+res);

						recorder.add_recall(res,last_hex);

						undo_stack.clear();
						redo_stack.clear();

						gui.invalidate_game_status();
					}
				}
			}
		}

		if(command == HOTKEY_SAVE_GAME) {
			std::stringstream stream;
			stream << string_table["scenario"]
			       << " " << (state_of_game.scenario+1)
			       << " " << string_table["turn"]
			       << " " << status.turn();
			std::string label = stream.str();

			const int res = gui::show_dialog(gui, NULL, "", "",
			                            gui::OK_CANCEL,NULL,NULL,
										string_table["save_game_label"],
										&label);

			if(res == 0) {
				recorder.save_game(gameinfo,label);
				gui::show_dialog(gui,NULL,"",
				   string_table["save_confirm_message"], gui::OK_ONLY);
			}
		}

		unit_map::const_iterator un = units.find(new_hex);
		if(un == units.end())
			un = units.find(selected_hex);

		if(command == HOTKEY_UNIT_DESCRIPTION && un != units.end()) {
			const std::string description = un->second.type().unit_description()
			                                + "\n\n" + string_table["see_also"];

			std::vector<std::string> options;

			options.push_back(string_table["terrain_info"]);
			options.push_back(string_table["attack_resistance"]);
			options.push_back(string_table["close_window"]);

			SDL_Surface* const unit_image = gui.getImage(
			           un->second.type().image_profile(), display::UNSCALED);

			const int res = gui::show_dialog(gui,unit_image,
		                                     un->second.type().language_name(),
			                                 description,gui::MESSAGE,
			                                 &options);

			//terrain table
			if(res == 0) {
				command = HOTKEY_TERRAIN_TABLE;
			}

			//attack resistance table
			else if(res == 1) {
				command = HOTKEY_ATTACK_RESISTANCE;
			}
		}

		if(un != units.end() && command == HOTKEY_ATTACK_RESISTANCE) {
			gui.draw();

			std::vector<std::string> items;
			items.push_back(string_table["attack_type"] + "," +
			                string_table["attack_resistance"]);
			const std::map<std::string,std::string>& table =
			     un->second.type().movement_type().damage_table();
			for(std::map<std::string,std::string>::const_iterator i
			    = table.begin(); i != table.end(); ++i) {
				double resistance = atof(i->second.c_str());

				//if resistance is less than 0, display in red
				std::string prefix = "";
				if(resistance > 1.0) {
					prefix = "#";
				}

				const int resist = 100 - int(util::round(100.0*resistance));

				const std::string& lang_weapon =
				               string_table["weapon_type_" + i->first];
				const std::string& weap = lang_weapon.empty() ? i->first :
				                                                lang_weapon;

				std::stringstream str;
				str << weap << "," << prefix << resist << "%";
				items.push_back(str.str());
			}

			const std::vector<unit> units_list(items.size(),
			                                   un->second);
			SDL_Surface* const unit_image =
		      gui.getImage(un->second.type().image_profile(),display::UNSCALED);
			gui::show_dialog(gui,unit_image,
			                 un->second.type().language_name(),
							 string_table["unit_resistance_table"],
							 gui::MESSAGE,&items,&units_list);
		}

		if(un != units.end() && command == HOTKEY_TERRAIN_TABLE) {
			gui.draw();

			std::vector<std::string> items;
			items.push_back(string_table["terrain"] + "," +
			                string_table["movement"] + "," +
							string_table["defense"]);

			const unit_type& type = un->second.type();
			const unit_movement_type& move_type =
			                        type.movement_type();
			const std::vector<gamemap::TERRAIN>& terrains =
			                    map.get_terrain_precedence();
			for(std::vector<gamemap::TERRAIN>::const_iterator t =
			    terrains.begin(); t != terrains.end(); ++t) {
				const terrain_type& info = map.get_terrain_info(*t);
				if(!info.is_alias()) {
					const std::string& name = map.terrain_name(*t);
					const std::string& lang_name = string_table[name];
					const int moves = move_type.movement_cost(map,*t);

					const double defense = move_type.defense_modifier(map,*t);

					const int def = 100-int(util::round(100.0*defense));

					std::stringstream str;
					str << lang_name << ",";
					if(moves < 10)
						str << moves;
					else
						str << "--";

					str << "," << def << "%";

					items.push_back(str.str());
				}
			}

			const std::vector<unit> units_list(items.size(),un->second);
			SDL_Surface* const unit_image =
		      gui.getImage(un->second.type().image_profile(),display::UNSCALED);
			gui::show_dialog(gui,unit_image,un->second.type().language_name(),
							 string_table["terrain_info"],
							 gui::MESSAGE,&items,&units_list);
		}

		if(command == HOTKEY_END_UNIT_TURN) {
			const unit_map::iterator un = units.find(selected_hex);
			if(un != units.end() && un->second.side() == team_num &&
			   un->second.movement_left() > 0) {
				std::vector<gamemap::location> steps;
				steps.push_back(selected_hex);
				undo_stack.push_back(undo_action(
				                      steps,un->second.movement_left(),-1));
				redo_stack.clear();
				un->second.set_movement(0);
				gui.draw_tile(selected_hex.x,selected_hex.y);

				gui.set_paths(NULL);
				current_paths = paths();
				recorder.add_movement(selected_hex,selected_hex);

				command = HOTKEY_CYCLE_UNITS;
			}
		}

		//look for the next unit that is unmoved on our side
		if(command == HOTKEY_CYCLE_UNITS) {
			unit_map::const_iterator it = units.find(next_unit);
			if(it != units.end()) {
				for(++it; it != units.end(); ++it) {
					if(it->second.side() == team_num &&
					   it->second.movement_left() > 0 &&
					   it->second.get_goto().valid() == false) {
						break;
					}
				}
			}

			if(it == units.end()) {
				for(it = units.begin(); it != units.end(); ++it) {
					if(it->second.side() == team_num &&
					   it->second.movement_left() > 0 &&
					   it->second.get_goto().valid() == false) {
						break;
					}
				}
			}

			if(it != units.end()) {
				const bool ignore_zocs =
				      it->second.type().is_skirmisher();
				const bool teleport = it->second.type().teleports();
				current_paths = paths(map,gameinfo,units,
				                 it->first,teams,ignore_zocs,teleport);
				gui.set_paths(&current_paths);

				gui.scroll_to_tile(it->first.x,it->first.y,
				                   display::WARP);
			}

			if(it != units.end()) {
				next_unit = it->first;
				selected_hex = next_unit;
				gui.select_hex(selected_hex);
				current_route.steps.clear();
				gui.set_route(NULL);
			} else
				next_unit = gamemap::location();
		}

		if(command == HOTKEY_LEADER) {
			for(unit_map::const_iterator i = units.begin(); i != units.end();
			    ++i) {
				if(i->second.side() == team_num && i->second.can_recruit()) {
					gui.scroll_to_tile(i->first.x,i->first.y,display::WARP);
					break;
				}
			}
		}

		//undo
		if(command == HOTKEY_UNDO && !undo_stack.empty()) {
			const int starting_moves = undo_stack.back().starting_moves;
			std::vector<gamemap::location> route = undo_stack.back().route;
			std::reverse(route.begin(),route.end());
			const unit_map::iterator u = units.find(route.front());
			if(u == units.end()) {
				assert(false);
				continue;
			}

			if(map[route.front().x][route.front().y] == gamemap::TOWER) {
				get_tower(route.front(),teams,
				          undo_stack.back().original_village_owner);
			}

			undo_stack.back().starting_moves = u->second.movement_left();

			unit un = u->second;
			un.set_goto(gamemap::location());
			units.erase(u);
			gui.move_unit(route,un);
			un.set_movement(starting_moves);
			units.insert(std::pair<gamemap::location,unit>(route.back(),un));
			gui.invalidate_unit();
			gui.draw_tile(route.back().x,route.back().y);

			redo_stack.push_back(undo_stack.back());
			undo_stack.pop_back();

			gui.set_paths(NULL);
			current_paths = paths();
			selected_hex = gamemap::location();
			current_route.steps.clear();
			gui.set_route(NULL);

			recorder.undo();
		}

		if(command == HOTKEY_REDO && !redo_stack.empty()) {

			//clear routes, selected hex, etc
			gui.set_paths(NULL);
			current_paths = paths();
			selected_hex = gamemap::location();
			current_route.steps.clear();
			gui.set_route(NULL);

			const int starting_moves = redo_stack.back().starting_moves;
			std::vector<gamemap::location> route = redo_stack.back().route;
			const unit_map::iterator u = units.find(route.front());
			if(u == units.end()) {
				assert(false);
				continue;
			}

			redo_stack.back().starting_moves = u->second.movement_left();

			unit un = u->second;
			un.set_goto(gamemap::location());
			units.erase(u);
			gui.move_unit(route,un);
			un.set_movement(starting_moves);
			units.insert(std::pair<gamemap::location,unit>(route.back(),un));
			gui.invalidate_unit();

			recorder.add_movement(route.front(),route.back());

			if(map[route.back().x][route.back().y] == gamemap::TOWER) {
				get_tower(route.back(),teams,un.side()-1);
			}

			gui.draw_tile(route.back().x,route.back().y);

			undo_stack.push_back(redo_stack.back());
			redo_stack.pop_back();
		}

		gui.draw();

		game_events::pump();

		pump_events();
	}
}
