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
#include "events.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "language.hpp"
#include "log.hpp"
#include "mouse.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "tooltips.hpp"
#include "util.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

namespace {
	int commands_disabled = 0;
}

command_disabler::command_disabler() { ++commands_disabled; }
command_disabler::~command_disabler() { --commands_disabled; }

void play_turn(game_data& gameinfo, game_state& state_of_game,
               gamestatus& status, config& terrain_config, config* level,
			   CVideo& video, CKey& key, display& gui,
               game_events::manager& events_manager, gamemap& map,
			   std::vector<team>& teams, int team_num,
			   std::map<gamemap::location,unit>& units)
{
	log_scope("player turn");

	gui.set_team(team_num-1);
	gui.recalculate_minimap();
	gui.invalidate_all();
	gui.draw();
	gui.update_display();

	team& current_team = teams[team_num-1];

	const paths_wiper wiper(gui);

	if(preferences::turn_bell()) {
		sound::play_sound("bell.wav");
	}

	if(preferences::turn_dialog()) {
		gui::show_dialog(gui,NULL,"",string_table["your_turn"],gui::MESSAGE);
	}

	turn_info turn_data(gameinfo,state_of_game,status,terrain_config,level,
	                    key,gui,map,teams,team_num,units,false);

	int start_command = recorder.ncommands();

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

		const std::set<gamemap::location>* const teleports =
		     u.type().teleports() ? &current_team.towers() : NULL;

		paths::route route = a_star_search(ui->first,ui->second.get_goto(),
		                                   10000.0,calc,teleports);
		if(route.steps.empty())
			continue;

		assert(route.steps.front() == *g);

		route.move_left = route_turns_to_complete(ui->second,map,route);
		gui.set_route(&route);
		move_unit(&gui,gameinfo,map,units,teams,route.steps,
		          &recorder,&turn_data.undos());
		gui.invalidate_game_status();
	}

	std::cerr << "done gotos\n";

	while(!turn_data.turn_over()) {

		try {
			turn_data.turn_slice();
		} catch(end_level_exception& e) {
			turn_data.send_data(start_command);
			throw e;
		}

		gui.draw();

		start_command = turn_data.send_data(start_command);
	}
}

turn_info::turn_info(game_data& gameinfo, game_state& state_of_game,
                     gamestatus& status, config& terrain_config, config* level,
                     CKey& key, display& gui, gamemap& map,
                     std::vector<team>& teams, int team_num,
                     unit_map& units, bool browse)
  : paths_wiper(gui),
    gameinfo_(gameinfo), state_of_game_(state_of_game), status_(status),
    terrain_config_(terrain_config), level_(level),
    key_(key), gui_(gui), map_(map), teams_(teams), team_num_(team_num),
    units_(units), browse_(browse),
    left_button_(false), right_button_(false), middle_button_(false),
    enemy_paths_(false), path_turns_(0), end_turn_(false)
{
}

void turn_info::turn_slice()
{
	events::pump();

	int mousex, mousey;
	const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex,mousey,mouse_flags & SDL_BUTTON_LMASK);

	const int scroll_threshold = 5;

	const theme::menu* const m = gui_.menu_pressed(mousex,mousey,mouse_flags&SDL_BUTTON_LMASK);
	if(m != NULL) {
		show_menu(m->items());
	}

	if(key_[SDLK_UP] || mousey < scroll_threshold)
		gui_.scroll(0.0,-preferences::scroll_speed());

	if(key_[SDLK_DOWN] || mousey > gui_.y()-scroll_threshold)
		gui_.scroll(0.0,preferences::scroll_speed());

	if(key_[SDLK_LEFT] || mousex < scroll_threshold)
		gui_.scroll(-preferences::scroll_speed(),0.0);

	if(key_[SDLK_RIGHT] || mousex > gui_.x()-scroll_threshold)
		gui_.scroll(preferences::scroll_speed(),0.0);
}

bool turn_info::turn_over() const { return end_turn_; }

int turn_info::send_data(int first_command)
{
	if(network::nconnections() > 0 && (undo_stack_.empty() || end_turn_) &&
	   first_command < recorder.ncommands()) {
		config cfg;
		cfg.add_child("turn",recorder.get_data_range(first_command,recorder.ncommands()));
		network::send_data(cfg);
		return recorder.ncommands();
	} else {
		return first_command;
	}
}

void turn_info::handle_event(const SDL_Event& event)
{
	if(gui::in_dialog() || commands_disabled)
		return;

	switch(event.type) {
	case SDL_KEYDOWN:
		hotkey::key_event(gui_,event.key,this);

		//intentionally fall-through
	case SDL_KEYUP:

		//if the user has pressed 1 through 9, we want to show how far
		//the unit can move in that many turns
		if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '7') {
			const int new_path_turns = (event.type == SDL_KEYDOWN) ?
			                           event.key.keysym.sym - '1' : 0;

			if(new_path_turns != path_turns_) {
				path_turns_ = new_path_turns;

				unit_map::iterator u = units_.find(selected_hex_);
				if(u == units_.end() || gui_.fogged(u->first.x,u->first.y)) {
					u = units_.find(last_hex_);
					if(u != units_.end() && (u->second.side() == team_num_ || gui_.fogged(u->first.x,u->first.y))) {
						u = units_.end();
					}
				} else if(u->second.side() != team_num_ || gui_.fogged(u->first.x,u->first.y)) {
					u = units_.end();
				}

				if(u != units_.end()) {
					const bool ignore_zocs = u->second.type().is_skirmisher();
					const bool teleport = u->second.type().teleports();
					current_paths_ = paths(map_,gameinfo_,units_,u->first,
					                       teams_,ignore_zocs,teleport,
					                       path_turns_);
					gui_.set_paths(&current_paths_);
				}
			}
		}

		break;
	case SDL_MOUSEMOTION:
		mouse_motion(event.motion);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		mouse_press(event.button);
		break;
	default:
		break;
	}
}

void turn_info::mouse_motion(const SDL_MouseMotionEvent& event)
{
	if(commands_disabled)
		return;

	const team& current_team = teams_[team_num_-1];
	const gamemap::location new_hex = gui_.hex_clicked_on(event.x,event.y);

	if(new_hex != last_hex_) {
		if(new_hex.valid() == false) {
			current_route_.steps.clear();
			gui_.set_route(NULL);
		}

		gui_.highlight_hex(new_hex);

		if(enemy_paths_) {
			enemy_paths_ = false;
			current_paths_ = paths();
			gui_.set_paths(NULL);
		}
		
		if(new_hex == selected_hex_) {
			current_route_.steps.clear();
			gui_.set_route(NULL);
		} else if(!enemy_paths_ && new_hex != last_hex_ &&
		   !current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			const unit_map::const_iterator un = units_.find(selected_hex_);
			
			if(un != units_.end()) {
				const shortest_path_calculator calc(un->second,current_team,
				                                    units_,map_);
				const bool can_teleport = un->second.type().teleports();

				const std::set<gamemap::location>* const teleports =
				     can_teleport ? &current_team.towers() : NULL;

				current_route_ = a_star_search(selected_hex_,new_hex,
				                               10000.0,calc,teleports);

				current_route_.move_left = route_turns_to_complete(un->second,map_,current_route_);

				if(!browse_)
					gui_.set_route(&current_route_);
			}
		}

		const unit_map::iterator un = units_.find(new_hex);

		if(un != units_.end() && un->second.side() != team_num_ &&
		   current_paths_.routes.empty() && !gui_.fogged(un->first.x,un->first.y)) {
			unit_movement_resetter move_reset(un->second);

			const bool ignore_zocs = un->second.type().is_skirmisher();
			const bool teleport = un->second.type().teleports();
			current_paths_ = paths(map_,gameinfo_,units_,new_hex,teams_,
			                   ignore_zocs,teleport,path_turns_);
			gui_.set_paths(&current_paths_);
			enemy_paths_ = true;
		}
	}

	last_hex_ = new_hex;
}

void turn_info::mouse_press(const SDL_MouseButtonEvent& event)
{
	if(commands_disabled)
		return;
	
	if(event.button == SDL_BUTTON_LEFT && event.state == SDL_PRESSED) {
		left_click(event);
	} else if(event.button == SDL_BUTTON_RIGHT && event.state == SDL_PRESSED) {
		if(!current_paths_.routes.empty()) {
			selected_hex_ = gamemap::location();
			gui_.select_hex(gamemap::location());
			gui_.set_paths(NULL);
			current_paths_ = paths();
			current_route_.steps.clear();
			gui_.set_route(NULL);
		} else {
			const theme::menu* const m = gui_.get_theme().context_menu();
			if(m != NULL) {
				std::cerr << "found context menu\n";
				show_menu(m->items());
			} else {
				std::cerr << "no context menu found...\n";
			}
		}
	} else if(event.button == SDL_BUTTON_MIDDLE && event.state == SDL_PRESSED) {
		const SDL_Rect& rect = gui_.map_area();
		const int centerx = (rect.x + rect.w)/2;
		const int centery = (rect.y + rect.h)/2;

		const int xdisp = event.x - centerx;
		const int ydisp = event.y - centery;

		gui_.scroll(xdisp,ydisp);
	} else if(event.button == SDL_BUTTON_WHEELUP ||
	          event.button == SDL_BUTTON_WHEELDOWN) {
		const double speed = preferences::scroll_speed() *
			(event.button == SDL_BUTTON_WHEELUP ? -1.0:1.0);

		const int centerx = gui_.mapx()/2;
		const int centery = gui_.y()/2;

		const int xdisp = abs(centerx - event.x);
		const int ydisp = abs(centery - event.y);

		if(xdisp > ydisp)
			gui_.scroll(speed,0.0);
		else
			gui_.scroll(0.0,speed);
	}
}

void turn_info::left_click(const SDL_MouseButtonEvent& event)
{
	const team& current_team = teams_[team_num_-1];

	const gamemap::location& loc = gui_.minimap_location_on(event.x,event.y);
	if(loc.valid()) {
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP);
	}

	gamemap::location hex = gui_.hex_clicked_on(event.x,event.y);

	unit_map::iterator u = units_.find(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command is cancelled
	if(u != units_.end() && !browse_ &&
	   selected_hex_ == hex && u->second.side() == team_num_) {
		u->second.set_goto(gamemap::location());
	}

	//if we can move to that tile
	std::map<gamemap::location,paths::route>::const_iterator
			route = enemy_paths_ ? current_paths_.routes.end() :
	                               current_paths_.routes.find(hex);
	unit_map::iterator enemy = units_.find(hex);

	//see if we're trying to attack an enemy
	if(route != current_paths_.routes.end() && enemy != units_.end() &&
	   hex != selected_hex_ && !browse_ &&
	   enemy->second.side() != u->second.side() &&
	   current_team.is_enemy(enemy->second.side())) {

		const std::vector<attack_type>& attacks = u->second.attacks();
		std::vector<std::string> items;

		std::vector<unit> units_list;

		const int range = distance_between(u->first,enemy->first);
		std::vector<int> attacks_in_range;

		for(size_t a = 0; a != attacks.size(); ++a) {
			if(attacks[a].hexes() < range)
				continue;

			attacks_in_range.push_back(a);

			const battle_stats stats = evaluate_battle_stats(
			                               map_,selected_hex_,hex,
										   a,units_,status_,gameinfo_);

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
			att << "&" << stats.attack_icon << "," << attack_name
			    << " " << stats.damage_defender_takes << "-"
				<< stats.nattacks << " " << range << " "
				<< stats.chance_to_hit_defender
			    << "%";

			att << "," << string_table["versus"] << ",";
			att << defend_name << " " << stats.damage_attacker_takes << "-"
				<< stats.ndefends << " "
				<< stats.chance_to_hit_attacker
			    << "%,&" << stats.defend_icon;

			items.push_back(att.str());
			units_list.push_back(enemy->second);
		}

		//make it so that when we attack an enemy, the attacking unit
		//is again shown in the status bar, so that we can easily
		//compare between the attacking and defending unit
		gui_.highlight_hex(gamemap::location());
		gui_.draw(true,true);

		int res = gui::show_dialog(gui_,NULL,"",
		                           string_table["choose_weapon"]+":\n",
		                           gui::OK_CANCEL,&items,&units_list);

		if(size_t(res) < attacks_in_range.size()) {
			res = attacks_in_range[res];

			u->second.set_goto(gamemap::location());
			undo_stack_.clear();
			redo_stack_.clear();

			current_paths_ = paths();
			gui_.set_paths(NULL);

			game_events::fire("attack",selected_hex_,hex);

			//the event could have killed either the attacker or
			//defender, so we have to make sure they still exist
			u = units_.find(selected_hex_);
			enemy = units_.find(hex);

			if(u == units_.end() || enemy == units_.end() ||
			   size_t(res) >= attacks.size()) {
				return;
			}

			gui_.invalidate_all();
			gui_.draw();

			const bool defender_human = teams_[enemy->second.side()-1].is_human();

			recorder.add_attack(selected_hex_,hex,res);

			attack(gui_,map_,selected_hex_,hex,res,units_,
			       status_,gameinfo_,true);

			dialogs::advance_unit(gameinfo_,units_,selected_hex_,gui_);
			dialogs::advance_unit(gameinfo_,units_,hex,gui_,!defender_human);

			selected_hex_ = gamemap::location();
			current_route_.steps.clear();
			gui_.set_route(NULL);

			check_victory(units_,teams_);

			gui_.invalidate_all();
			gui_.draw(); //clear the screen
		}
	}

	//otherwise we're trying to move to a hex
	else if(!browse_ && selected_hex_.valid() && selected_hex_ != hex &&
		     units_.count(selected_hex_) && !enemy_paths_ &&
		     enemy == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {


		const size_t moves = move_unit(&gui_,gameinfo_,map_,units_,teams_,
		                   current_route_.steps,&recorder,&undo_stack_);

		gui_.invalidate_game_status();

		selected_hex_ = gamemap::location();
		gui_.set_route(NULL);
		gui_.select_hex(gamemap::location());
		gui_.set_paths(NULL);
		current_paths_ = paths();

		if(moves == 0)
			return;

		redo_stack_.clear();

		assert(moves <= current_route_.steps.size());
		const gamemap::location& dst = current_route_.steps[moves-1];
		const unit_map::const_iterator u = units_.find(dst);

		//u may be equal to units_.end() in the case of e.g. a [teleport]
		if(u != units_.end()) {
			assert(u != units_.end());

			const int range = u->second.longest_range();

			current_route_.steps.clear();

			show_attack_options(u);

			if(current_paths_.routes.empty() == false) {
				current_paths_.routes[dst] = paths::route();
				selected_hex_ = dst;
				gui_.select_hex(dst);
				gui_.set_paths(&current_paths_);
			}

			if(clear_shroud(gui_,map_,gameinfo_,units_,teams_,team_num_-1)) {
				undo_stack_.clear();
			}
		}
	} else {
		gui_.set_paths(NULL);
		current_paths_ = paths();

		selected_hex_ = hex;
		gui_.select_hex(hex);
		current_route_.steps.clear();
		gui_.set_route(NULL);

		const unit_map::iterator it = units_.find(hex);
		if(it != units_.end() && it->second.side() == team_num_ && !gui_.fogged(it->first.x,it->first.y)) {
			const bool ignore_zocs = it->second.type().is_skirmisher();
			const bool teleport = it->second.type().teleports();
			current_paths_ = paths(map_,gameinfo_,units_,hex,teams_,
			                   ignore_zocs,teleport,path_turns_);

			show_attack_options(it);

			gui_.set_paths(&current_paths_);

			unit u = it->second;
			const gamemap::location go_to = u.get_goto();
			if(map_.on_board(go_to)) {
				const shortest_path_calculator calc(u,current_team,
				                                    units_,map_);

				const std::set<gamemap::location>* const teleports =
				        teleport ? &current_team.towers() : NULL;

				paths::route route = a_star_search(it->first,go_to,
				                               10000.0,calc,teleports);
				route.move_left = route_turns_to_complete(it->second,map_,route);
				gui_.set_route(&route);
			}
		}
	}
}

void turn_info::show_attack_options(unit_map::const_iterator u)
{
	team& current_team = teams_[team_num_-1];

	if(u == units_.end() || u->second.can_attack() == false)
		return;

	const int range = u->second.longest_range();
	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) <= range) {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

bool turn_info::can_execute_command(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_LEADER:
	case hotkey::HOTKEY_CYCLE_UNITS:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_ACCELERATED:
	case hotkey::HOTKEY_SAVE_GAME:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
		return true;

	case hotkey::HOTKEY_SPEAK:
		return network::nconnections() > 0;

	case hotkey::HOTKEY_REDO:
		return !browse_ && !redo_stack_.empty();
	case hotkey::HOTKEY_UNDO:
		return !browse_ && !undo_stack_.empty();

	//commands we can only do if we are actually playing, not just viewing
	case hotkey::HOTKEY_END_UNIT_TURN:
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL:
	case hotkey::HOTKEY_ENDTURN:
		return !browse_;

	//commands we can only do if there is an active unit
	case hotkey::HOTKEY_TERRAIN_TABLE:
	case hotkey::HOTKEY_ATTACK_RESISTANCE:
    case hotkey::HOTKEY_UNIT_DESCRIPTION:
	case hotkey::HOTKEY_RENAME_UNIT:
		return current_unit() != units_.end();

	//commands we can only do if in debug mode
	case hotkey::HOTKEY_CREATE_UNIT:
		return game_config::debug;

	default:
		return false;
	}
}

namespace {

	struct cannot_execute {
		cannot_execute(const turn_info& info) : info_(info) {}
		bool operator()(const std::string& str) const {
			return !info_.can_execute_command(hotkey::string_to_command(str));
		}
	private:
		const turn_info& info_;
	};
}

void turn_info::show_menu(const std::vector<std::string>& items_arg)
{
	std::vector<std::string> items = items_arg;
	items.erase(std::remove_if(items.begin(),items.end(),cannot_execute(*this)),items.end());
	if(items.empty())
		return;

	//if just one item is passed in, that means we should execute that item
	if(items.size() == 1 && items_arg.size() == 1) {
		hotkey::execute_command(gui_,hotkey::string_to_command(items.front()),this);
		return;
	}

	std::vector<std::string> menu;
	for(std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i) {
		menu.push_back(translate_string("action_" + *i));
	}

	const int res = gui::show_dialog(gui_,NULL,"",string_table["options"]+":\n",
	                                 gui::MESSAGE,&menu);
	if(res == -1)
		return;

	const hotkey::HOTKEY_COMMAND cmd = hotkey::string_to_command(items[res]);
	hotkey::execute_command(gui_,cmd,this);
}

void turn_info::cycle_units()
{
	unit_map::const_iterator it = units_.find(next_unit_);
	if(it != units_.end()) {
		for(++it; it != units_.end(); ++it) {
			if(it->second.side() == team_num_ &&
			   unit_can_move(it->first,units_,map_,teams_) &&
			   !gui_.fogged(it->first.x,it->first.y)) {
				break;
			}
		}
	}

	if(it == units_.end()) {
		for(it = units_.begin(); it != units_.end(); ++it) {
			if(it->second.side() == team_num_ &&
			   unit_can_move(it->first,units_,map_,teams_) &&
			   !gui_.fogged(it->first.x,it->first.y)) {
				break;
			}
		}
	}

	if(it != units_.end() && !gui_.fogged(it->first.x,it->first.y)) {
		const bool ignore_zocs = it->second.type().is_skirmisher();
		const bool teleport = it->second.type().teleports();
		current_paths_ = paths(map_,gameinfo_,units_,
		               it->first,teams_,ignore_zocs,teleport,path_turns_);
		gui_.set_paths(&current_paths_);

		gui_.scroll_to_tile(it->first.x,it->first.y,display::WARP);
	}

	if(it != units_.end()) {
		next_unit_ = it->first;
		selected_hex_ = next_unit_;
		gui_.select_hex(selected_hex_);
		current_route_.steps.clear();
		gui_.set_route(NULL);
	} else
		next_unit_ = gamemap::location();
}

void turn_info::end_turn()
{
	if(browse_)
		return;

	end_turn_ = true;

	//auto-save
	config start_pos;
	write_game_snapshot(start_pos);
	recorder.save_game(gameinfo_,string_table["auto_save"],start_pos);
	recorder.end_turn();
}

void turn_info::goto_leader()
{
	const unit_map::const_iterator i = team_leader(team_num_,units_);
	if(i != units_.end()) {
		gui_.scroll_to_tile(i->first.x,i->first.y,display::WARP);
	}
}

void turn_info::end_unit_turn()
{
	if(browse_)
		return;

	const unit_map::iterator un = units_.find(selected_hex_);
	if(un != units_.end() && un->second.side() == team_num_ &&
	   un->second.movement_left() > 0) {
		std::vector<gamemap::location> steps;
		steps.push_back(selected_hex_);
		undo_stack_.push_back(undo_action(steps,un->second.movement_left(),-1));
		redo_stack_.clear();
		un->second.set_movement(0);
		gui_.draw_tile(selected_hex_.x,selected_hex_.y);

		gui_.set_paths(NULL);
		current_paths_ = paths();
		recorder.add_movement(selected_hex_,selected_hex_);

		cycle_units();
	}
}

void turn_info::undo()
{
	if(undo_stack_.empty())
		return;

	const command_disabler disable_commands;

	const int starting_moves = undo_stack_.back().starting_moves;
	std::vector<gamemap::location> route = undo_stack_.back().route;
	std::reverse(route.begin(),route.end());
	const unit_map::iterator u = units_.find(route.front());
	if(u == units_.end()) {
		assert(false);
		return;
	}

	if(map_.underlying_terrain(map_[route.front().x][route.front().y]) == gamemap::TOWER) {
		get_tower(route.front(),teams_,
		          undo_stack_.back().original_village_owner,units_);
	}

	undo_stack_.back().starting_moves = u->second.movement_left();

	unit un = u->second;
	un.set_goto(gamemap::location());
	units_.erase(u);
	gui_.move_unit(route,un);
	un.set_movement(starting_moves);
	units_.insert(std::pair<gamemap::location,unit>(route.back(),un));
	gui_.invalidate_unit();
	gui_.invalidate_game_status();
	gui_.draw_tile(route.back().x,route.back().y);

	redo_stack_.push_back(undo_stack_.back());
	undo_stack_.pop_back();

	gui_.set_paths(NULL);
	current_paths_ = paths();
	selected_hex_ = gamemap::location();
	current_route_.steps.clear();
	gui_.set_route(NULL);

	recorder.undo();
}

void turn_info::redo()
{
	if(redo_stack_.empty())
		return;

	const command_disabler disable_commands;

	//clear routes, selected hex, etc
	gui_.set_paths(NULL);
	current_paths_ = paths();
	selected_hex_ = gamemap::location();
	current_route_.steps.clear();
	gui_.set_route(NULL);

	const int starting_moves = redo_stack_.back().starting_moves;
	std::vector<gamemap::location> route = redo_stack_.back().route;
	const unit_map::iterator u = units_.find(route.front());
	if(u == units_.end()) {
		assert(false);
		return;
	}

	redo_stack_.back().starting_moves = u->second.movement_left();

	unit un = u->second;
	un.set_goto(gamemap::location());
	units_.erase(u);
	gui_.move_unit(route,un);
	un.set_movement(starting_moves);
	units_.insert(std::pair<gamemap::location,unit>(route.back(),un));
	gui_.invalidate_unit();
	gui_.invalidate_game_status();

	recorder.add_movement(route.front(),route.back());

	if(map_.underlying_terrain(map_[route.back().x][route.back().y]) == gamemap::TOWER) {
		get_tower(route.back(),teams_,un.side()-1,units_);
	}

	gui_.draw_tile(route.back().x,route.back().y);

	undo_stack_.push_back(redo_stack_.back());
	redo_stack_.pop_back();
}

void turn_info::terrain_table()
{
	unit_map::const_iterator un = current_unit();

	if(un == units_.end()) {
		return;
	}

	gui_.draw();

	std::vector<std::string> items;
	items.push_back(string_table["terrain"] + "," +
	                string_table["movement"] + "," +
					string_table["defense"]);

	const unit_type& type = un->second.type();
	const unit_movement_type& move_type = type.movement_type();
	const std::vector<gamemap::TERRAIN>& terrains =
	                    map_.get_terrain_precedence();
	for(std::vector<gamemap::TERRAIN>::const_iterator t =
	    terrains.begin(); t != terrains.end(); ++t) {
		const terrain_type& info = map_.get_terrain_info(*t);
		if(!info.is_alias()) {
			const std::string& name = map_.terrain_name(*t);
			const std::string& lang_name = string_table[name];
			const int moves = move_type.movement_cost(map_,*t);

			std::stringstream str;
			str << lang_name << ",";
			if(moves < 10)
				str << moves;
			else
				str << "--";

			const int defense = 100 - move_type.defense_modifier(map_,*t);
			str << "," << defense << "%";

			items.push_back(str.str());
		}
	}

	const std::vector<unit> units_list(items.size(),un->second);
	const scoped_sdl_surface unit_image(image::get_image(un->second.type().image_profile(),image::UNSCALED));
	gui::show_dialog(gui_,unit_image,un->second.type().language_name(),
					 string_table["terrain_info"],
					 gui::MESSAGE,&items,&units_list);
}

void turn_info::attack_resistance()
{
	const unit_map::const_iterator un = current_unit();
	if(un == units_.end())
		return;

	gui_.draw();

	std::vector<std::string> items;
	items.push_back(string_table["attack_type"] + "," +
	                string_table["attack_resistance"]);
	const std::map<std::string,std::string>& table =
	     un->second.type().movement_type().damage_table();
	for(std::map<std::string,std::string>::const_iterator i
	    = table.begin(); i != table.end(); ++i) {
		int resistance = 100 - atoi(i->second.c_str());

		//if resistance is less than 0, display in red
		std::string prefix = "";
		if(resistance < 0) {
			prefix = "#";
		}

		const std::string& lang_weapon =
		               string_table["weapon_type_" + i->first];
		const std::string& weap = lang_weapon.empty() ? i->first : lang_weapon;

		std::stringstream str;
		str << weap << "," << prefix << resistance << "%";
		items.push_back(str.str());
	}

	const std::vector<unit> units_list(items.size(), un->second);
	const scoped_sdl_surface unit_image(image::get_image(un->second.type().image_profile(),image::UNSCALED));
	gui::show_dialog(gui_,unit_image,
	                 un->second.type().language_name(),
					 string_table["unit_resistance_table"],
					 gui::MESSAGE,&items,&units_list);
}

void turn_info::unit_description()
{
	const unit_map::const_iterator un = current_unit();
	if(un == units_.end()) {
		return;
	}

	const std::string description = un->second.type().unit_description()
	                                + "\n\n" + string_table["see_also"];

	std::vector<std::string> options;

	options.push_back(string_table["terrain_info"]);
	options.push_back(string_table["attack_resistance"]);
	options.push_back(string_table["close_window"]);

	const scoped_sdl_surface unit_image(image::get_image(un->second.type().image_profile(), image::UNSCALED));

	const int res = gui::show_dialog(gui_,unit_image,
                                     un->second.type().language_name(),
	                                 description,gui::MESSAGE, &options);
	if(res == 0) {
		terrain_table();
	} else if(res == 1) {
		attack_resistance();
	}
}

void turn_info::rename_unit()
{
	const unit_map::iterator un = current_unit();
	if(un == units_.end() || gui_.viewing_team()+1 != un->second.side())
		return;

	std::string name = un->second.description();
	const int res = gui::show_dialog(gui_,NULL,"",string_table["rename_unit"], gui::OK_CANCEL,NULL,NULL,"",&name);
	if(res == 0) {
		un->second.rename(name);
		gui_.invalidate_unit();
	}
}

void turn_info::save_game()
{
	std::stringstream stream;
	stream << state_of_game_.label << " " << string_table["turn"]
	       << " " << status_.turn();
	std::string label = stream.str();

	const int res = dialogs::get_save_name(gui_,"",string_table["save_game_label"],&label,gui::OK_CANCEL);

	if(res == 0) {
		config start;
		write_game_snapshot(start);
		recorder.save_game(gameinfo_,label,start);
		gui::show_dialog(gui_,NULL,"",string_table["save_confirm_message"], gui::OK_ONLY);
	}
}

void turn_info::write_game_snapshot(config& start) const
{
	start["snapshot"] = "yes";

	char buf[50];
	sprintf(buf,"%d",gui_.playing_team());
	start["playing_team"] = buf;

	for(std::vector<team>::const_iterator t = teams_.begin(); t != teams_.end(); ++t) {
		const int side_num = t - teams_.begin() + 1;

		config& side = start.add_child("side");
		t->write(side);
		side["no_leader"] = "yes";
		sprintf(buf,"%d",side_num);
		side["side"] = buf;

		for(std::map<gamemap::location,unit>::const_iterator i = units_.begin();
		    i != units_.end(); ++i) {
			if(i->second.side() == side_num) {
				config& u = side.add_child("unit");
				i->first.write(u);
				i->second.write(u);
			}
		}
	}

	status_.write(start);
	game_events::write_events(start);

	write_game(state_of_game_,start);
	start["gold"] = "-1000000"; //just make sure gold is read in from the teams

	//copy over important scenario stats
	start["id"] = (*level_)["id"];
	start["name"] = (*level_)["name"];
	start["objectives"] = (*level_)["objectives"];
	start["next_scenario"] = (*level_)["next_scenario"];
	start["music"] = (*level_)["music"];

	//write out the current state of the map
	start["map_data"] = map_.write();
}

void turn_info::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
	gui_.invalidate_all();
}

void turn_info::status_table()
{
	std::vector<std::string> items;
	std::stringstream heading;
	heading << string_table["leader"] << "," << string_table["villages"] << ","
	        << string_table["units"] << "," << string_table["upkeep"] << ","
	        << string_table["income"];

	if(game_config::debug)
		heading << "," << string_table["gold"];

	items.push_back(heading.str());

	const team& viewing_team = teams_[gui_.viewing_team()];

	//if the player is under shroud or fog, they don't get to see
	//details about the other sides, only their own side, allied sides and a ??? is
	//shown to demonstrate lack of information about the other sides
	const bool fog = viewing_team.uses_fog() || viewing_team.uses_shroud();

	for(size_t n = 0; n != teams_.size(); ++n) {
		if(fog && viewing_team.is_enemy(n+1))
			continue;

		const team_data data = calculate_team_data(teams_[n],n+1,units_);

		std::stringstream str;

		//output the number of the side first, and this will
		//cause it to be displayed in the correct colour
		str << char(n+1) << team_name(n+1,units_) << ","
		    << data.villages << ","
		    << data.units << "," << data.upkeep << ","
		    << (data.net_income < 0 ? "#":"") << data.net_income;

		if(game_config::debug)
			str << "," << teams_[n].gold();

		items.push_back(str.str());
	}

	if(fog)
		items.push_back("???\n");

	gui::show_dialog(gui_,NULL,"","",gui::MESSAGE,&items);
}

void turn_info::recruit()
{
	if(browse_)
		return;

	team& current_team = teams_[team_num_-1];

	std::vector<unit> sample_units;

	gui_.draw(); //clear the old menu
	std::vector<std::string> item_keys;
	std::vector<std::string> items;
	const std::set<std::string>& recruits = current_team.recruits();
	for(std::set<std::string>::const_iterator it =
	    recruits.begin(); it != recruits.end(); ++it) {
		item_keys.push_back(*it);
		const std::map<std::string,unit_type>::const_iterator
				u_type = gameinfo_.unit_types.find(*it);
		if(u_type == gameinfo_.unit_types.end()) {
			std::cerr << "could not find " << *it << std::endl;
			assert(false);
			return;
		}

		const unit_type& type = u_type->second;

		//display units that we can't afford to recruit in red
		const std::string prefix = (type.cost() > current_team.gold() ? "#" : "");

		std::stringstream description;

		description << prefix << type.language_name() << ","
		            << prefix << type.cost() << " gold";
		items.push_back(description.str());
		sample_units.push_back(unit(&type,team_num_));
	}

	if(sample_units.empty()) {
		gui::show_dialog(gui_,NULL,"",string_table["no_units_to_recruit"],gui::MESSAGE);
		return;
	}

	const int recruit_res = gui::show_dialog(gui_,NULL,"",
	                                 string_table["recruit_unit"] + ":\n",
	                                 gui::OK_CANCEL,&items,&sample_units);
	if(recruit_res != -1) {
		do_recruit(item_keys[recruit_res]);
	}
}

void turn_info::do_recruit(const std::string& name)
{
	team& current_team = teams_[team_num_-1];

	int recruit_num = 0;
	const std::set<std::string>& recruits = current_team.recruits();
	for(std::set<std::string>::const_iterator r = recruits.begin(); r != recruits.end(); ++r) {
		if(name == *r)
			break;

		++recruit_num;
	}

	const std::map<std::string,unit_type>::const_iterator
			u_type = gameinfo_.unit_types.find(name);
	assert(u_type != gameinfo_.unit_types.end());

	if(u_type->second.cost() > current_team.gold()) {
		gui::show_dialog(gui_,NULL,"",
		     string_table["not_enough_gold_to_recruit"],gui::OK_ONLY);
	} else {
		last_recruit_ = name;

		//create a unit with traits
		recorder.add_recruit(recruit_num,last_hex_);
		unit new_unit(&(u_type->second),team_num_,true);
		const std::string& msg =
		   recruit_unit(map_,team_num_,units_,new_unit,last_hex_,&gui_);
		if(msg.empty()) {
			current_team.spend_gold(u_type->second.cost());
		} else {
			recorder.undo();
			gui::show_dialog(gui_,NULL,"",msg,gui::OK_ONLY);
		}

		undo_stack_.clear();
		redo_stack_.clear();

		gui_.invalidate_game_status();
		gui_.invalidate_all();
	}
}

void turn_info::repeat_recruit()
{
	if(last_recruit_.empty() == false)
		do_recruit(last_recruit_);
}

void turn_info::recall()
{
	if(browse_)
		return;

	team& current_team = teams_[team_num_-1];

	//sort the available units into order by value
	//so that the most valuable units are shown first
	std::sort(state_of_game_.available_units.begin(),
	          state_of_game_.available_units.end(),
			  compare_unit_values());

	gui_.draw(); //clear the old menu

	if((*level_)["disallow_recall"] == "yes") {
		gui::show_dialog(gui_,NULL,"",string_table["recall_disallowed"]);
	} else if(state_of_game_.available_units.empty()) {
		gui::show_dialog(gui_,NULL,"",string_table["no_recall"]);
	} else if(current_team.gold() < game_config::recall_cost) {
		std::stringstream msg;
		msg << string_table["not_enough_gold_to_recall_1"]
		    << " " << game_config::recall_cost << " "
			<< string_table["not_enough_gold_to_recall_2"];
		gui::show_dialog(gui_,NULL,"",msg.str());
	} else {
		std::vector<std::string> options;
		for(std::vector<unit>::const_iterator unit =
		  state_of_game_.available_units.begin();
		  unit != state_of_game_.available_units.end(); ++unit) {
			std::stringstream option;
			const std::string& description = unit->description().empty() ? "-" : unit->description();
			option << unit->type().language_name() << "," << description << ","
			       << string_table["level"] << ": "
			       << unit->type().level() << ","
			       << string_table["xp"] << ": "
			       << unit->experience() << "/";

			if(unit->type().advances_to().empty())
				option << "-";
			else
				option << unit->max_experience();
			options.push_back(option.str());
		}

		const int res = gui::show_dialog(gui_,NULL,"",
		                                 string_table["select_unit"] + ":\n",
		                                 gui::OK_CANCEL,&options,
		                                 &state_of_game_.available_units);
		if(res >= 0) {
			const std::string err = recruit_unit(map_,team_num_,
			                       units_,state_of_game_.available_units[res],
			                       last_hex_,&gui_);
			if(!err.empty()) {
				gui::show_dialog(gui_,NULL,"",err,gui::OK_ONLY);
			} else {
				current_team.spend_gold(game_config::recall_cost);
				state_of_game_.available_units.erase(
				   state_of_game_.available_units.begin()+res);

				recorder.add_recall(res,last_hex_);

				undo_stack_.clear();
				redo_stack_.clear();

				gui_.invalidate_game_status();
			}
		}
	}
}


void turn_info::speak()
{
	const unit_map::const_iterator leader = find_leader(units_,gui_.viewing_team()+1);
	if(leader == units_.end())
		return;

	std::string message;
	const int res = gui::show_dialog(gui_,NULL,"",string_table["speak"],gui::OK_CANCEL,NULL,NULL,
		                             string_table["message"] + ":", &message);
	if(res == 0) {
		config cfg;
		cfg["description"] = leader->second.description();
		cfg["message"] = message;
		char buf[50];
		sprintf(buf,"%d",leader->second.side());
		cfg["side"] = buf;

		recorder.speak(cfg);
		dialogs::unit_speak(cfg,gui_,units_);

		//speaking is an unretractable operation
		undo_stack_.clear();
		redo_stack_.clear();
	}
}

void turn_info::create_unit()
{
	std::vector<std::string> options;
	std::vector<unit> unit_choices;
	for(game_data::unit_type_map::const_iterator i = gameinfo_.unit_types.begin();
	    i != gameinfo_.unit_types.end(); ++i) {
		options.push_back(i->first);
		unit_choices.push_back(unit(&i->second,1,false));
	}

	const int choice = gui::show_dialog(gui_,NULL,"","Create unit (debug):",
		                                gui::OK_CANCEL,&options,&unit_choices);
	if(choice >= 0 && choice < unit_choices.size()) {
		units_.erase(last_hex_);
		units_.insert(std::pair<gamemap::location,unit>(last_hex_,unit_choices[choice]));
		gui_.invalidate(last_hex_);
		gui_.invalidate_unit();
	}
}

void turn_info::preferences()
{
	preferences::show_preferences_dialog(gui_);
	gui_.redraw_everything();
}

void turn_info::objectives()
{
	dialogs::show_objectives(gui_,*level_);
}

void turn_info::unit_list()
{
	const std::string heading = string_table["type"] + "," +
	                            string_table["name"] + "," +
	                            string_table["hp"] + "," +
	                            string_table["xp"] + "," +
	                            string_table["moves"] + "," +
	                            string_table["location"];

	std::vector<std::string> items;
	items.push_back(heading);

	std::vector<gamemap::location> locations_list;
	std::vector<unit> units_list;
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() != team_num_)
			continue;

		std::stringstream row;
		row << i->second.type().language_name() << ","
			<< i->second.description() << ","
			<< i->second.hitpoints()
		    << "/" << i->second.max_hitpoints() << ","
			<< i->second.experience() << "/";

		if(i->second.type().advances_to().empty())
			row << "-";
		else
			row << i->second.max_experience();

		row << ","
			<< i->second.movement_left() << "/"
			<< i->second.total_movement() << ","
			<< (i->first.x+1) << "-" << (i->first.y+1);

		items.push_back(row.str());

		//extra unit for the first row to make up the heading
		if(units_list.empty()) {
			locations_list.push_back(i->first);
			units_list.push_back(i->second);
		}

		locations_list.push_back(i->first);
		units_list.push_back(i->second);
	}

	const int selected = gui::show_dialog(gui_,NULL,string_table["unit_list"],"",
	                                      gui::OK_ONLY,&items,&units_list);
	if(selected > 0 && selected < int(locations_list.size())) {
		const gamemap::location& loc = locations_list[selected];
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP);
	}
}

unit_map::iterator turn_info::current_unit()
{
	unit_map::iterator i = units_.find(last_hex_);
	if(i == units_.end()) {
		i = units_.find(selected_hex_);
	}

	return i;
}

unit_map::const_iterator turn_info::current_unit() const
{
	unit_map::const_iterator i = units_.find(last_hex_);
	if(i == units_.end()) {
		i = units_.find(selected_hex_);
	}

	return i;
}
