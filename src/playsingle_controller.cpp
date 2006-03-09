#include "global.hpp"

#include "ai_interface.hpp"
#include "config_adapter.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "intro.hpp"
#include "key.hpp"
#include "log.hpp"
#include "map_create.hpp"
#include "marked-up_text.hpp"
#include "playsingle_controller.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"

#define LOG_NG LOG_STREAM(info, engine)

LEVEL_RESULT playsingle_scenario(const game_data& gameinfo, const config& game_config,
		const config* level, CVideo& video, game_state& state_of_game,
		const std::vector<config*>& story, upload_log& log, bool skip_replay)
{
//	try{
		const int ticks = SDL_GetTicks();
		const int num_turns = atoi((*level)["turns"].c_str());
		playsingle_controller playcontroller(*level, state_of_game, ticks, num_turns);
		return playcontroller.play_scenario(gameinfo, game_config, video, story, log, skip_replay);
	//return LEVEL_CONTINUE;
		
		//play event-loop
//		for (;;){
//			replaycontroller.replay_slice();
//		}
		
//	}
//	catch(end_level_exception&){}

}

playsingle_controller::playsingle_controller(const config& level, game_state& state_of_game, const int ticks, const int num_turns)
	: play_controller(level, state_of_game, ticks, num_turns), 
	generator_setter(&recorder), cursor_setter(cursor::NORMAL)
{}

LEVEL_RESULT playsingle_controller::play_scenario(const game_data& gameinfo, const config& game_config,
		CVideo& video,
		const std::vector<config*>& story, upload_log& log, bool skip_replay)
{
	const int ticks = SDL_GetTicks();
	LOG_NG << "in playsingle_controller::play_scenario()...\n";

	const statistics::scenario_context statistics_context(level_["name"]);

	gamemap map(game_config,level_["map_data"]);

	LOG_NG << "created objects... " << (SDL_GetTicks() - ticks) << "\n";

	CKey key;
	unit_map units;

	const verification_manager verify_manager(units);

	const int xp_modifier = atoi(level_["experience_modifier"].c_str());
	const unit_type::experience_accelerator xp_mod(xp_modifier > 0 ? xp_modifier : 100);

	std::vector<team> teams;

	teams_manager team_manager(teams);

	int first_human_team = -1;

	const config::child_list& unit_cfg = level_.get_children("side");

	if(level_["modify_placing"] == "true") {
		LOG_NG << "modifying placing...\n";
		play::place_sides_in_preferred_locations(map,unit_cfg);
	}

	LOG_NG << "initializing teams..." << unit_cfg.size() << "\n";;
	LOG_NG << (SDL_GetTicks() - ticks) << "\n";

	std::set<std::string> seen_save_ids;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::string save_id = get_unique_saveid(**ui, seen_save_ids);
		seen_save_ids.insert(save_id);
		if (first_human_team == -1){
			first_human_team = get_first_human_team(ui, unit_cfg);
		}
		get_player_info(**ui, gamestate_, save_id, teams, level_, gameinfo, map, units);
	}

	preferences::encounter_recruitable_units(teams);
	preferences::encounter_start_units(units);
	preferences::encounter_recallable_units(gamestate_);
	preferences::encounter_map_terrain(map);

	LOG_NG << "initialized teams... " << (SDL_GetTicks() - ticks) << "\n";

	const config* theme_cfg = NULL;
	if(level_["theme"] != "") {
		theme_cfg = game_config.find_child("theme","name",level_["theme"]);
	}

	if(theme_cfg == NULL) {
		theme_cfg = game_config.find_child("theme","name",preferences::theme());
	}

	LOG_NG << "initializing display... " << (SDL_GetTicks() - ticks) << "\n";
	const config dummy_cfg;
	display gui(units,video,map,status_,teams,theme_cfg != NULL ? *theme_cfg : dummy_cfg, game_config, level_);
	theme::set_known_themes(&game_config);

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
	game_events::manager events_manager(level_,gui,map,units,teams,
	                                    gamestate_,status_,gameinfo);

	if(!recorder.is_skipping()) {
		for(std::vector<config*>::const_iterator story_i = story.begin(); story_i != story.end(); ++story_i) {

			show_intro(gui,**story_i, level_);
		}
	}

	//object that will make sure that labels are removed at the end of the scenario
	const font::floating_label_context labels_manager;
	const halo::manager halo_manager(gui);
	gui.labels().read(level_);
	LOG_NG << "c... " << (SDL_GetTicks() - ticks) << "\n";

	const std::string& music = level_["music"];
	if(music != "") {
		sound::play_music_repeatedly(music);
	}

	LOG_NG << "d... " << (SDL_GetTicks() - ticks) << "\n";

	victory_conditions::set_victory_when_enemies_defeated(
						level_["victory_when_enemies_defeated"] != "no");

	LOG_NG << "initializing events manager... " << (SDL_GetTicks() - ticks) << "\n";

	help::help_manager help_manager(&game_config, &gameinfo, &map);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level_.get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui.add_overlay(gamemap::location(**overlay),(**overlay)["image"], (**overlay)["halo"]);
	}

	int turn = 1, player_number = 0;

	turn_info::floating_textbox textbox_info;

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks) << "\n";

	replay_network_sender replay_sender(recorder);

	try {
		//if a team is specified whose turn it is, it means we're loading a game
		//instead of starting a fresh one
		const bool loading_game = level_["playing_team"].empty() == false;

		//pre-start events must be executed before any GUI operation,
		//as those may cause the display to be refreshed.
		if(!loading_game) {
			update_locker lock_display(gui.video());
			game_events::fire("prestart");
		}

		gui.begin_game();
		gui.adjust_colours(0,0,0);

		LOG_NG << "scrolling... " << (SDL_GetTicks() - ticks) << "\n";
		if(first_human_team != -1) {
			LOG_NG << "b " << (SDL_GetTicks() - ticks) << "\n";
			gui.scroll_to_tile(map.starting_position(first_human_team + 1).x,
			                   map.starting_position(first_human_team + 1).y, display::WARP);
			LOG_NG << "c " << (SDL_GetTicks() - ticks) << "\n";
		}
		gui.scroll_to_tile(map.starting_position(1).x,map.starting_position(1).y,display::WARP);
		LOG_NG << "done scrolling... " << (SDL_GetTicks() - ticks) << "\n";

		bool replaying = (recorder.at_end() == false);

		int first_player = atoi(level_["playing_team"].c_str());
		if(first_player < 0 || first_player >= int(teams.size())) {
			first_player = 0;
		}

		for(std::vector<team>::iterator t = teams.begin(); t != teams.end(); ++t) {
			clear_shroud(gui,status_,map,gameinfo,units,teams,(t-teams.begin()));
		}

		std::deque<config> data_backlog;

		LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks) << "\n";
		for(bool first_time = true; true; first_time = false, first_player = 0) {
			if(first_time) {
				const hotkey::basic_handler key_events_handler(&gui);

				LOG_NG << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui.video(),recorder.is_skipping());
				events::raise_draw_event();
				gui.draw();
				for(std::vector<team>::iterator t = teams.begin(); t != teams.end(); ++t) {
					clear_shroud(gui,status_,map,gameinfo,units,teams,(t-teams.begin()));
				}

				if(!loading_game) {
					game_events::fire("start");
					gamestate_.set_variable("turn_number", "1");
				}

				gui.recalculate_minimap();
			}
			player_number = 0;

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

				if(team_manager.is_observer()) {
					gui.set_team(size_t(player_number-1));
				}

				std::stringstream player_number_str;
				player_number_str << player_number;
				gamestate_.set_variable("side_number",player_number_str.str());

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

					calculate_healing(gui,status_,map,units,player_number,teams, !skip_replay);
				}

				team_it->set_time_of_day(int(status_.turn()),status_.get_time_of_day());

				gui.set_playing_team(size_t(player_number-1));

				clear_shroud(gui,status_,map,gameinfo,units,teams,player_number-1);

				if (!skip_replay){
					gui.scroll_to_leader(units, player_number);
				}

				if(replaying) {
					const hotkey::basic_handler key_events_handler(&gui);
					LOG_NG << "doing replay " << player_number << "\n";
					try {
						replaying = do_replay(gui,map,gameinfo,units,teams,
						                      player_number,status_,gamestate_);
					} catch(replay::error&) {
						gui::show_dialog(gui,NULL,"",_("The file you have tried to load is corrupt"),gui::OK_ONLY);

						replaying = false;
					}
					LOG_NG << "result of replay: " << (replaying?"true":"false") << "\n";
				}

				if(!replaying && team_it->music().empty() == false && 
						(teams[gui.viewing_team()].knows_about_team(player_number-1) || teams[gui.viewing_team()].has_seen(player_number-1))) {
					LOG_NG << "playing music: '" << team_it->music() << "'\n";
					sound::play_music_repeatedly(team_it->music());
				} else if(!replaying && team_it->music().empty() == false){
					LOG_NG << "playing music: '" << game_config::anonymous_music<< "'\n";
					sound::play_music_repeatedly(game_config::anonymous_music);
				}
				// else leave old music playing, it's a scenario specific music

//goto this label if the type of a team (human/ai/networked) has changed mid-turn
redo_turn:

				if(!replaying && team_it->is_human()) {
					LOG_NG << "is human...\n";

					try {
						play_turn(gameinfo,gamestate_,status_,game_config,
						          level_, key, gui, map, teams, player_number,
						          units, textbox_info, replay_sender, skip_replay);
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

					turn_info turn_data(gameinfo,gamestate_,status_,
						                game_config,level_,key,gui,
						                map,teams,player_number,units,
								turn_info::BROWSE_AI,textbox_info,replay_sender);

					ai_interface::info ai_info(gui,map,gameinfo,units,teams,player_number,status_,turn_data);
					util::scoped_ptr<ai_interface> ai_obj(create_ai(team_it->ai_algorithm(),ai_info));
					ai_obj->play_turn();
					recorder.end_turn();
					ai_obj->sync_network();

					gui.recalculate_minimap();
					clear_shroud(gui,status_,map,gameinfo,units,teams,player_number-1);
					gui.invalidate_unit();
					gui.invalidate_game_status();
					gui.invalidate_all();
					gui.draw();
					SDL_Delay(500);
				} else if(!replaying && team_it->is_network()) {
					LOG_NG << "is networked...\n";

					turn_info turn_data(gameinfo,gamestate_,status_,
					                    game_config,level_,key,gui,
							    map,teams,player_number,units,
							    turn_info::BROWSE_NETWORKED,
							    textbox_info,replay_sender);

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
							const turn_info::PROCESS_DATA_RESULT result = turn_data.process_network_data(cfg,from,data_backlog,skip_replay);
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
			if(!status_.next_turn()) {

				if(non_interactive()) {
					std::cout << "time over (draw)\n";
				}

				LOG_NG << "firing time over event...\n";
				game_events::fire("time over");
				LOG_NG << "done firing time over event...\n";

				throw end_level_exception(DEFEAT);
			}

			std::stringstream event_stream;
			event_stream << status_.turn();

			{
				LOG_NG << "turn event..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";
				update_locker lock_display(gui.video(),recorder.is_skipping());
				const std::string turn_num = event_stream.str();
				gamestate_.set_variable("turn_number",turn_num);
				game_events::fire("turn " + turn_num);
				game_events::fire("new turn");
			}
		} //end for loop

	} catch(end_level_exception& end_level) {
		bool obs = team_manager.is_observer();
		if (end_level.result == DEFEAT || end_level.result == VICTORY) {
			// if we're a player, and the result is victory/defeat, then send a message to notify
			// the server of the reason for the game ending
			if (!obs) {
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "game over";
				network::send_data(cfg);
			} else {
				gui::show_dialog(gui, NULL, _("Game Over"),
				                 _("The game is over."), gui::OK_ONLY);
				return QUIT;
			}
		}

		if(end_level.result == QUIT) {
			return end_level.result;
		} else if(end_level.result == DEFEAT) {
			try {
				game_events::fire("defeat");
			} catch(end_level_exception&) {
			}

			if (!obs)
				return DEFEAT;
			else
				return QUIT;
		} else if (end_level.result == VICTORY || end_level.result == LEVEL_CONTINUE ||
		           end_level.result == LEVEL_CONTINUE_NO_SAVE) {
			try {
				game_events::fire("victory");
			} catch(end_level_exception&) {
			}

			if(gamestate_.scenario == (level_)["id"]) {
				gamestate_.scenario = (level_)["next_scenario"];
			}

			const bool has_next_scenario = !gamestate_.scenario.empty() &&
			                               gamestate_.scenario != "null";

			//add all the units that survived the scenario
			for(std::map<gamemap::location,unit>::iterator un = units.begin(); un != units.end(); ++un) {
				player_info *player=gamestate_.get_player(teams[un->second.side()-1].save_id());

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
					player_info *player=gamestate_.get_player(i->save_id());
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

				player_info *player=gamestate_.get_player(i->save_id());

				const int remaining_gold = i->gold();
				const int finishing_bonus_per_turn =
				             map.villages().size() * game_config::village_income +
				             game_config::base_income;
				const int turns_left = maximum<int>(0,status_.number_of_turns() - status_.turn());
				const int finishing_bonus = end_level.gold_bonus ?
				             (finishing_bonus_per_turn * turns_left) : 0;

				if(player) {
					player->gold = ((remaining_gold + finishing_bonus) * 80) / 100;

					if(gamestate_.players.size()>1) {
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
					report << '\n' << _("80% of gold is retained for the next scenario") << '\n'
					       << _("Retained Gold: ") << player->gold;
				}
			}

			if (!obs)
				gui::show_dialog(gui, NULL, _("Victory"),
				                 _("You have emerged victorious!"), gui::OK_ONLY);

			if (gamestate_.players.size() > 0 && has_next_scenario ||
					gamestate_.campaign_type == "test")
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

		turn_info turn_data(gameinfo,gamestate_,status_,
				game_config,level_,key,gui,
				map,teams,player_number,units,turn_info::BROWSE_NETWORKED,textbox_info,replay_sender);

		turn_data.save_game(_("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"),gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return QUIT;
		}
	}

	return QUIT;
}
