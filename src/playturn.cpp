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

#include "actions.hpp"
#include "events.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "mouse.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"
#include "unit.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "widgets/menu.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {
	int commands_disabled = 0;
}

command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

void play_turn(game_data& gameinfo, game_state& state_of_game,
               gamestatus& status, const config& terrain_config,
               config* level, CKey& key, display& gui, gamemap& map,
               std::vector<team>& teams, int team_num,
               std::map<gamemap::location,unit>& units,
               turn_info::floating_textbox& textbox,
               replay_network_sender& network_sender)
{
	log_scope("player turn");

	gui.set_team(team_num-1);
	gui.recalculate_minimap();
	gui.invalidate_all();
	gui.draw();
	gui.update_display();

	const paths_wiper wiper(gui);

	if(preferences::turn_bell()) {
		sound::play_sound(game_config::sounds::turn_bell);
	}

	if(preferences::turn_dialog()) {
		gui::show_dialog(gui,NULL,"",_("It is now your turn"),gui::MESSAGE);
	}

	const std::string& turn_cmd = preferences::turn_cmd();
	if(turn_cmd.empty() == false) {
		system(turn_cmd.c_str());
	}

	turn_info turn_data(gameinfo,state_of_game,status,terrain_config,level,
	                    key,gui,map,teams,team_num,units,turn_info::PLAY_TURN,textbox,network_sender);

	//execute gotos - first collect gotos in a list
	std::vector<gamemap::location> gotos;

	for(unit_map::iterator ui = units.begin(); ui != units.end(); ++ui) {
		if(ui->second.get_goto() == ui->first)
			ui->second.set_goto(gamemap::location());

		if(ui->second.side() == team_num && map.on_board(ui->second.get_goto()))
			gotos.push_back(ui->first);
	}

	for(std::vector<gamemap::location>::const_iterator g = gotos.begin(); g != gotos.end(); ++g) {
		unit_map::const_iterator ui = units.find(*g);
		turn_data.move_unit_to_loc(ui,ui->second.get_goto(),false);
	}
	
	turn_data.start_interactive_turn();

	while(!turn_data.turn_over()) {

		try {
			config cfg;
			const network::connection res = network::receive_data(cfg);
			std::deque<config> backlog;

			if(res != network::null_connection) {
				turn_data.process_network_data(cfg,res,backlog);
			}

			turn_data.turn_slice();
		} catch(end_level_exception& e) {
			turn_data.send_data();
			throw e;
		}

		// gui.invalidate_animations();
		gui.draw();

		turn_data.send_data();
	}

	//send one more time to make sure network is up-to-date.
	turn_data.send_data();
}

turn_info::turn_info(game_data& gameinfo, game_state& state_of_game,
                     gamestatus& status, const config& terrain_config, config* level,
                     CKey& key, display& gui, gamemap& map,
                     std::vector<team>& teams, int team_num,
                     unit_map& units, TURN_MODE mode, floating_textbox& textbox, replay_network_sender& replay_sender)
  : paths_wiper(gui),
    gameinfo_(gameinfo), state_of_game_(state_of_game), status_(status),
    terrain_config_(terrain_config), level_(level),
    key_(key), gui_(gui), map_(map), teams_(teams), team_num_(team_num),
    units_(units), browse_(mode != PLAY_TURN), allow_network_commands_(mode == BROWSE_NETWORKED),
    left_button_(false), right_button_(false), middle_button_(false),
	minimap_scrolling_(false), enemy_paths_(false), last_nearest_(gamemap::location::NORTH),
    path_turns_(0), end_turn_(false), start_ncmd_(-1), textbox_(textbox), replay_sender_(replay_sender)
{
	enemies_visible_ = enemies_visible();
}

void turn_info::turn_slice()
{
	events::pump();
	events::raise_process_event();
	events::raise_draw_event();

	const theme::menu* const m = gui_.menu_pressed();
	if(m != NULL) {
		const SDL_Rect& menu_loc = m->location(gui_.screen_area());
		show_menu(m->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false);
		return;
	}

	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	const int scroll_threshold = 5;

	if(key_[SDLK_UP] || mousey < scroll_threshold)
		gui_.scroll(0,-preferences::scroll_speed());

	if(key_[SDLK_DOWN] || mousey > gui_.y()-scroll_threshold)
		gui_.scroll(0,preferences::scroll_speed());

	const bool use_left_right = (textbox_.active() == false);
	if(use_left_right && key_[SDLK_LEFT] || mousex < scroll_threshold)
		gui_.scroll(-preferences::scroll_speed(),0);
	
	if(use_left_right && key_[SDLK_RIGHT] || mousex > gui_.x()-scroll_threshold)
		gui_.scroll(preferences::scroll_speed(),0);
}

bool turn_info::turn_over() const { return end_turn_; }

void turn_info::send_data()
{
	if(undo_stack_.empty() || end_turn_) {
		replay_sender_.commit_and_sync();
	} else {
		replay_sender_.sync_non_undoable();
	}
}

void turn_info::handle_event(const SDL_Event& event)
{
	if(gui::in_dialog()) {
		return;
	}

	switch(event.type) {
	case SDL_KEYDOWN:
		//detect key press events, unless there is a textbox present on-screen
		//in which case the key press events should go only to it.
		if(textbox_.active() == false) {
			hotkey::key_event(gui_,event.key,this);
		} else if(event.key.keysym.sym == SDLK_ESCAPE) {
			close_textbox();
		} else if(event.key.keysym.sym == SDLK_TAB) {
			tab_textbox();
		} else if(event.key.keysym.sym == SDLK_RETURN) {
			enter_textbox();
		}

		//intentionally fall-through
	case SDL_KEYUP:

		//if the user has pressed 1 through 9, we want to show how far
		//the unit can move in that many turns
		if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '7') {
			const int new_path_turns = (event.type == SDL_KEYDOWN) ?
			                           event.key.keysym.sym - '1' : 0;

			if(new_path_turns != path_turns_) {
				path_turns_ = new_path_turns;

				const unit_map::iterator u = selected_unit();

				if(u != units_.end() && u->second.side() == team_num_) {
					const bool ignore_zocs = u->second.type().is_skirmisher();
					const bool teleport = u->second.type().teleports();
					current_paths_ = paths(map_,status_,gameinfo_,units_,u->first,
					                       teams_,ignore_zocs,teleport,
					                       path_turns_);
					gui_.set_paths(&current_paths_);
				}
			}
		}

		break;
	case SDL_MOUSEMOTION:
		// ignore old mouse motion events in the event queue
		SDL_Event new_event;

		if(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
					SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) {
			while(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
						SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0);
			mouse_motion(new_event.motion);
		} else {
			mouse_motion(event.motion);
		}
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
	mouse_motion(event.x,event.y);
}

void turn_info::mouse_motion(int x, int y)
{
	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(NULL,NULL) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const gamemap::location& loc = gui_.minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}
		if(minimap_scrolling_) return;
	}

	gamemap::location::DIRECTION nearest_hex;
	const team& current_team = teams_[team_num_-1];
	const gamemap::location new_hex = gui_.hex_clicked_on(x,y,&nearest_hex);

	if(new_hex != last_hex_ || nearest_hex != last_nearest_) {
		if(new_hex.valid() == false) {
			current_route_.steps.clear();
			gui_.set_route(NULL);
		}

		gui_.highlight_hex(new_hex);


		//see if we should show the normal cursor, the movement cursor, or
		//the attack cursor

		const unit_map::const_iterator selected_unit = find_unit(selected_hex_);
		const unit_map::const_iterator mouseover_unit = find_unit(new_hex);

		gamemap::location attack_from;
		if(selected_unit != units_.end() && mouseover_unit != units_.end()) {
			const gamemap::location preferred = new_hex.get_direction(nearest_hex);
			attack_from = current_unit_attacks_from(new_hex,&preferred);
		}

		if(selected_unit != units_.end() && (current_paths_.routes.count(new_hex) ||
		                                     attack_from.valid())) {
			if(mouseover_unit == units_.end()) {
				cursor::set(cursor::MOVE);
			} else if(current_team.is_enemy(mouseover_unit->second.side()) && !mouseover_unit->second.stone()) {
				cursor::set(cursor::ATTACK);
			} else {
				cursor::set(cursor::NORMAL);
			}
		} else {
			cursor::set(cursor::NORMAL);
		}

		if(enemy_paths_) {
			enemy_paths_ = false;
			current_paths_ = paths();
			gui_.set_paths(NULL);
		}
		
		if(new_hex == selected_hex_) {
			current_route_.steps.clear();
			gui_.set_route(NULL);
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			const gamemap::location& dest = attack_from.valid() ? attack_from : new_hex;

			unit_map::const_iterator un = find_unit(selected_hex_);
			const unit_map::const_iterator dest_un = find_unit(dest);

			if((new_hex != last_hex_ || attack_from.valid()) && un != units_.end() && dest_un == units_.end() && !un->second.stone()) {
				const shortest_path_calculator calc(un->second,current_team,
				                                    visible_units(),teams_,map_,status_);
				const bool can_teleport = un->second.type().teleports();

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(can_teleport) {
					allowed_teleports = vacant_villages(current_team.villages(),units_);
					teleports = &allowed_teleports;
					if(current_team.villages().count(un->first))
						allowed_teleports.insert(un->first);
				}

				current_route_ = a_star_search(selected_hex_, dest, 10000.0, &calc, teleports);

				current_route_.move_left = route_turns_to_complete(un->second,map_,current_route_);

				if(!browse_) {
					gui_.set_route(&current_route_);
				}
			}
		}

		unit_map::iterator un = find_unit(new_hex);

		if(un != units_.end() && un->second.side() != team_num_ &&
		   current_paths_.routes.empty() && !gui_.fogged(un->first.x,un->first.y)) {
			unit_movement_resetter move_reset(un->second);

			const bool ignore_zocs = un->second.type().is_skirmisher();
			const bool teleport = un->second.type().teleports();
			current_paths_ = paths(map_,status_,gameinfo_,units_,new_hex,teams_,
			                   ignore_zocs,teleport,path_turns_);
			gui_.set_paths(&current_paths_);
			enemy_paths_ = true;
		}
	}

	last_hex_ = new_hex;
	last_nearest_ = nearest_hex;
}

namespace {

bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_LMETA) != 0;
#else
	return false;
#endif
}

bool is_left_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool is_middle_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool is_right_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_RIGHT || event.button == SDL_BUTTON_LEFT && command_active();
}

//which attack is the better one to select for the player by default
//(the player can change the selected weapon if desired)
class simple_attack_rating
{
public:
	simple_attack_rating() : attacker_weapon_rating_(0), defender_weapon_rating_(0) {}

	simple_attack_rating(const battle_stats& stats) : 
		attacker_weapon_rating_(stats.chance_to_hit_defender *
				stats.damage_defender_takes * stats.nattacks),
		defender_weapon_rating_(stats.chance_to_hit_attacker *
				stats.damage_attacker_takes * stats.ndefends) {}

	bool operator<(const simple_attack_rating& a) const
	{
		//if our weapon does less damage, it's worse
		if(attacker_weapon_rating_ < a.attacker_weapon_rating_)
			return true;

		//if both weapons are the same but 
		//ours makes the enemy retaliate for more damage, it's worse
		else if(attacker_weapon_rating_ == a.attacker_weapon_rating_ &&
		   defender_weapon_rating_ > a.defender_weapon_rating_)
			return true;

		//otherwise, ours is at least as good a default weapon
		return false;
	}
private:
	int attacker_weapon_rating_, defender_weapon_rating_;
};

} //end anonymous namespace

void turn_info::mouse_press(const SDL_MouseButtonEvent& event)
{
	mouse_motion(event.x,event.y);

	if(is_left_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_middle_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_left_click(event) && event.state == SDL_PRESSED) {
		left_click(event);
	} else if(is_right_click(event) && event.state == SDL_PRESSED) {
		if(!current_paths_.routes.empty()) {
			selected_hex_ = gamemap::location();
			gui_.select_hex(gamemap::location());
			gui_.set_paths(NULL);
			current_paths_ = paths();
			current_route_.steps.clear();
			gui_.set_route(NULL);

			cursor::set(cursor::NORMAL);
		} else {
			gui_.draw(); // redraw highlight (and maybe some more)
			const theme::menu* const m = gui_.get_theme().context_menu();
			if (m != NULL)
				show_menu(m->items(),event.x,event.y,true);
			else
				lg::warn(lg::display) << "no context menu found...\n";
		}
	} else if(is_middle_click(event) && event.state == SDL_PRESSED) {
		// clicked on a hex on the minimap? then initiate minimap scrolling
		const gamemap::location& loc = gui_.minimap_location_on(event.x,event.y);
		minimap_scrolling_ = false;
		if(loc.valid()) {
			minimap_scrolling_ = true;
			last_hex_ = loc;
			gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
			return;
		} else {
		const SDL_Rect& rect = gui_.map_area();
		const int centerx = (rect.x + rect.w)/2;
		const int centery = (rect.y + rect.h)/2;

		const int xdisp = event.x - centerx;
		const int ydisp = event.y - centery;

		gui_.scroll(xdisp,ydisp);
		}
	} else if(event.button == SDL_BUTTON_WHEELUP ||
	          event.button == SDL_BUTTON_WHEELDOWN) {
		const int speed = preferences::scroll_speed() *
			(event.button == SDL_BUTTON_WHEELUP ? -1:1);

		const int centerx = gui_.mapx()/2;
		const int centery = gui_.y()/2;

		const int xdisp = abs(centerx - event.x);
		const int ydisp = abs(centery - event.y);

		if(xdisp > ydisp)
			gui_.scroll(speed,0);
		else
			gui_.scroll(0,speed);
	}
}

//a class which, when a button is pressed, will display
//how an attack was calculated
namespace {
class attack_calculations_displayer : public gui::dialog_button_action
{
public:
	typedef std::vector< battle_stats_strings > stats_vector;
	attack_calculations_displayer(display &disp, stats_vector const &stats)
		: disp_(disp), stats_(stats)
	{}

	RESULT button_pressed(int selection);
private:
	display &disp_;
	stats_vector const &stats_;
};

gui::dialog_button_action::RESULT attack_calculations_displayer::button_pressed(int selection)
{
	const size_t index = size_t(selection);
	if(index < stats_.size()) {
		battle_stats_strings const &sts = stats_[index];
		std::vector< std::string > sts_att = sts.attack_calculations,
		                           sts_def = sts.defend_calculations,
		                           calcs;
		unsigned sts_att_sz = sts_att.size(),
		         sts_def_sz = sts_def.size(),
		         sts_sz = maximum< unsigned >(sts_att_sz, sts_def_sz);

		std::stringstream str;
		str << _("Attacker") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR;
		if (sts_def_sz > 0)
			str << _("Defender");
		calcs.push_back(str.str());

		for(unsigned i = 0; i < sts_sz; ++i) {
			std::stringstream str;
			if (i < sts_att_sz)
				str << sts_att[i];
			else
				str << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ';

			str << COLUMN_SEPARATOR;

			if (i < sts_def_sz)
				str << sts_def[i];
			else
				str << ' ' << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR << ' ';

			calcs.push_back(str.str());
		}

		gui::show_dialog(disp_, NULL, "", _("Damage Calculations"), gui::OK_ONLY, &calcs);
	}

	return NO_EFFECT;
}

}
bool turn_info::attack_enemy(unit_map::iterator attacker, unit_map::iterator defender)
{
	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const gamemap::location attacker_loc = attacker->first;
	const gamemap::location defender_loc = defender->first;

	const std::vector<attack_type>& attacks = attacker->second.attacks();
	std::vector<std::string> items;

	const int range = distance_between(attacker->first,defender->first);

	int best_weapon_index = -1;
	simple_attack_rating best_weapon_rating;

	attack_calculations_displayer::stats_vector stats;

	for(size_t a = 0; a != attacks.size(); ++a) {

		battle_stats_strings sts;
		battle_stats st = evaluate_battle_stats(map_, attacker_loc, defender_loc,
		                                        a, units_, status_, 0, &sts);
		stats.push_back(sts);

		simple_attack_rating weapon_rating(st);

		if (best_weapon_index < 0 || best_weapon_rating < weapon_rating) {
			best_weapon_index = items.size();
			best_weapon_rating = weapon_rating;
		}

		//if there is an attack special or defend special, we output a single space for the other unit, to make sure
		//that the attacks line up nicely.
		std::string special_pad = (sts.attack_special.empty() && sts.defend_special.empty()) ? "" : " ";

		std::stringstream att;
		att << IMAGE_PREFIX << sts.attack_icon << COLUMN_SEPARATOR
		    << font::BOLD_TEXT << sts.attack_name << "\n" << st.damage_defender_takes << "-"
		    << st.nattacks << " " << sts.range << " (" << st.chance_to_hit_defender << "%)\n"
		    << sts.attack_special << special_pad
		    << COLUMN_SEPARATOR << _("vs") << COLUMN_SEPARATOR
		    << font::BOLD_TEXT << sts.defend_name << "\n" << st.damage_attacker_takes << "-"
		    << st.ndefends << " " << sts.range << " (" << st.chance_to_hit_attacker << "%)\n"
		    << sts.defend_special << special_pad << COLUMN_SEPARATOR
		    << IMAGE_PREFIX << sts.defend_icon;

		items.push_back(att.str());
	}

	if (best_weapon_index >= 0) {
		items[best_weapon_index] = DEFAULT_ITEM + items[best_weapon_index];
	}

	//make it so that when we attack an enemy, the attacking unit
	//is again shown in the status bar, so that we can easily
	//compare between the attacking and defending unit
	gui_.highlight_hex(gamemap::location());
	gui_.draw(true,true);

	attack_calculations_displayer calc_displayer(gui_,stats);
	std::vector<gui::dialog_button> buttons;
	buttons.push_back(gui::dialog_button(&calc_displayer,_("Damage Calculations")));

	int res = 0;

	{
		const events::event_context dialog_events_context;
		dialogs::unit_preview_pane attacker_preview(gui_,&map_,attacker->second,dialogs::unit_preview_pane::SHOW_BASIC,true);
		dialogs::unit_preview_pane defender_preview(gui_,&map_,defender->second,dialogs::unit_preview_pane::SHOW_BASIC,false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		res = gui::show_dialog(gui_,NULL,_("Attack Enemy"),
				_("Choose weapon")+std::string(":\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,NULL,-1,-1,
				NULL,&buttons);
	}

	cursor::set(cursor::NORMAL);

	if(size_t(res) < attacks.size()) {

		attacker->second.set_goto(gamemap::location());
		clear_undo_stack();
		redo_stack_.clear();

		current_paths_ = paths();
		gui_.set_paths(NULL);

		game_events::fire("attack",attacker_loc,defender_loc);

		//the event could have killed either the attacker or
		//defender, so we have to make sure they still exist
		attacker = units_.find(attacker_loc);
		defender = units_.find(defender_loc);

		if(attacker == units_.end() || defender == units_.end() || size_t(res) >= attacks.size()) {
			return true;
		}

		gui_.invalidate_all();
		gui_.draw();

		const bool defender_human = teams_[defender->second.side()-1].is_human();

		recorder.add_attack(attacker_loc,defender_loc,res);

		try {
			attack(gui_,map_,teams_,attacker_loc,defender_loc,res,units_,status_,gameinfo_);
		} catch(end_level_exception&) {
			//if the level ends due to a unit being killed, still see if
			//either the attacker or defender should advance
			dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,gui_);
			dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,gui_,!defender_human);
			throw;
		}

		dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,gui_);
		dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,gui_,!defender_human);

		selected_hex_ = gamemap::location();
		current_route_.steps.clear();
		gui_.set_route(NULL);

		check_victory(units_,teams_);

		gui_.invalidate_all();
		gui_.draw(); //clear the screen

		return true;
	} else {
		return false;
	}
}

bool turn_info::move_unit_along_current_route(bool check_shroud)
{
	const std::vector<gamemap::location> steps = current_route_.steps;
	if(steps.empty()) {
		return false;
	}

	const size_t moves = ::move_unit(&gui_,gameinfo_,status_,map_,units_,teams_,
	                   steps,&recorder,&undo_stack_,&next_unit_,false,check_shroud);

	cursor::set(cursor::NORMAL);

	gui_.invalidate_game_status();

	selected_hex_ = gamemap::location();
	gui_.select_hex(gamemap::location());
	
	gui_.set_route(NULL);
	gui_.set_paths(NULL);
	current_paths_ = paths();

	if(moves == 0)
		return false;

	redo_stack_.clear();

	wassert(moves <= steps.size());
	const gamemap::location& dst = steps[moves-1];
	const unit_map::const_iterator u = units_.find(dst);

	//u may be equal to units_.end() in the case of e.g. a [teleport]
	if(u != units_.end()) {
		//Reselect the unit if the move was interrupted
		if(dst != steps.back()) {
			selected_hex_ = dst;
			gui_.select_hex(dst);
		}
		
		current_route_.steps.clear();
		show_attack_options(u);
		
		if(current_paths_.routes.empty() == false) {
			current_paths_.routes[dst] = paths::route();
			selected_hex_ = dst;
			gui_.select_hex(dst);
			gui_.set_paths(&current_paths_);
		}
	}

	return moves == steps.size();
}

void turn_info::left_click(const SDL_MouseButtonEvent& event)
{
	if(commands_disabled) {
		return;
	}

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const gamemap::location& loc = gui_.minimap_location_on(event.x,event.y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
		return;
	}

	gamemap::location::DIRECTION nearest_hex;
	gamemap::location hex = gui_.hex_clicked_on(event.x,event.y,&nearest_hex);

	unit_map::iterator u = find_unit(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command is cancelled
	if(u != units_.end() && !browse_ && selected_hex_ == hex && u->second.side() == team_num_) {
		u->second.set_goto(gamemap::location());
	}

	//if we can move to that tile
	std::map<gamemap::location,paths::route>::const_iterator
			route = enemy_paths_ ? current_paths_.routes.end() :
	                               current_paths_.routes.find(hex);

	unit_map::iterator enemy = find_unit(hex);

	const gamemap::location src = selected_hex_;
	paths orig_paths = current_paths_;

	//see if we're trying to do a move-and-attack
	if(!browse_ && u != units_.end() && enemy != units_.end() && !current_route_.steps.empty()) {
		const gamemap::location preferred = hex.get_direction(nearest_hex);
		const gamemap::location& attack_from = current_unit_attacks_from(hex,&preferred);
		if(attack_from.valid()) {
			if(move_unit_along_current_route(false)) { //move the unit without updating shroud
				u = find_unit(attack_from);
				enemy = find_unit(hex);
				if(u != units_.end() && u->second.side() == team_num_ &&
				   enemy != units_.end() && current_team().is_enemy(enemy->second.side()) && !enemy->second.stone()) {
					if(attack_enemy(u,enemy) == false) {
						undo();
						selected_hex_ = src;
						gui_.select_hex(src);
						current_paths_ = orig_paths;
						gui_.set_paths(&current_paths_);
						return;
					}
				}
			}

			if(clear_shroud()) {
				clear_undo_stack();
			}

			return;
		}
	}

	//see if we're trying to attack an enemy
	if(u != units_.end() && route != current_paths_.routes.end() && enemy != units_.end() &&
	   hex != selected_hex_ && !browse_ &&
	   enemy->second.side() != u->second.side() &&
	   current_team().is_enemy(enemy->second.side())) {
		attack_enemy(u,enemy);
	}

	//otherwise we're trying to move to a hex
	else if(!browse_ && selected_hex_.valid() && selected_hex_ != hex &&
		     units_.count(selected_hex_) && !enemy_paths_ &&
		     enemy == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {
		move_unit_along_current_route();
		if(clear_shroud()) {
			clear_undo_stack();
		}
	} else {
		gui_.set_paths(NULL);
		current_paths_ = paths();

		selected_hex_ = hex;
		gui_.select_hex(hex);
		current_route_.steps.clear();
		gui_.set_route(NULL);

		const unit_map::iterator it = find_unit(hex);

		if(it != units_.end() && it->second.side() == team_num_ && !gui_.fogged(it->first.x,it->first.y)) {
			const bool ignore_zocs = it->second.type().is_skirmisher();
			const bool teleport = it->second.type().teleports();
			current_paths_ = paths(map_,status_,gameinfo_,units_,hex,teams_,
			                   ignore_zocs,teleport,path_turns_);

			next_unit_ = it->first;

			show_attack_options(it);

			gui_.set_paths(&current_paths_);

			unit u = it->second;
			const gamemap::location go_to = u.get_goto();
			if(map_.on_board(go_to)) {
				const shortest_path_calculator calc(u,current_team(),
				                                    visible_units(),teams_,map_,status_);

				const std::set<gamemap::location>* teleports = NULL;

				std::set<gamemap::location> allowed_teleports;
				if(u.type().teleports()) {
					allowed_teleports = vacant_villages(current_team().villages(),units_);
					teleports = &allowed_teleports;
					if(current_team().villages().count(it->first))
						allowed_teleports.insert(it->first);

				}

				paths::route route = a_star_search(it->first, go_to, 10000.0, &calc, teleports);
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

	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) == 1 && !target->second.stone()) {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

gamemap::location turn_info::current_unit_attacks_from(const gamemap::location& loc, const gamemap::location* preferred) const
{
	const unit_map::const_iterator current = find_unit(selected_hex_);
	if(current == units_.end() || current->second.side() != team_num_) {
		return gamemap::location();
	}

	const unit_map::const_iterator enemy = find_unit(loc);
	if(enemy == units_.end() || current_team().is_enemy(enemy->second.side()) == false) {
		return gamemap::location();
	}

	int best_defense = 100;
	gamemap::location res;
	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(map_.on_board(adj[n]) == false) {
			continue;
		}

		if(adj[n] == selected_hex_) {
			return gamemap::location();
		}

		if(find_unit(adj[n]) != units_.end()) {
			continue;
		}

		if(current_paths_.routes.count(adj[n])) {
			const int defense = preferred != NULL && *preferred == adj[n] ? 0 : current->second.defense_modifier(map_,map_.get_terrain(loc));
			if(defense < best_defense || res.valid() == false) {
				best_defense = defense;
				res = adj[n];
			}
		}
	}

	return res;
}

void turn_info::move_unit_to_loc(const unit_map::const_iterator& ui, const gamemap::location& target, bool continue_move)
{
	wassert(ui != units_.end());

	unit u = ui->second;
	const shortest_path_calculator calc(u,current_team(),visible_units(),teams_,map_,status_);

	const std::set<gamemap::location>* teleports = NULL;

	std::set<gamemap::location> allowed_teleports;
	if(u.type().teleports()) {
		allowed_teleports = vacant_villages(current_team().villages(),units_);
		teleports = &allowed_teleports;
		if(current_team().villages().count(ui->first))
			allowed_teleports.insert(ui->first);
	}

	paths::route route = a_star_search(ui->first, target, 10000.0, &calc, teleports);
	if(route.steps.empty())
		return;

	wassert(route.steps.front() == ui->first);

	route.move_left = route_turns_to_complete(ui->second,map_,route);
	gui_.set_route(&route);
	move_unit(&gui_,gameinfo_,status_,map_,units_,teams_,route.steps,&recorder,&undos(),NULL,continue_move);
	gui_.invalidate_game_status();
}

void turn_info::start_interactive_turn() {
	lg::info(lg::engine) << "done gotos\n";
	start_ncmd_ = recorder.ncommands();
}

hotkey::ACTION_STATE turn_info::get_action_state(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {
	case hotkey::HOTKEY_DELAY_SHROUD:
		return current_team().auto_shroud_updates() ? hotkey::ACTION_OFF : hotkey::ACTION_ON;
	default:
		return hotkey::ACTION_STATELESS;
	}
}

// Indicates whether the command should be in the context menu or not.
// Independant of whether or not we can actually execute the command.
bool turn_info::in_context_menu(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {
	//Only display these if the mouse is over a castle or keep tile
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL:
		// last_hex_ is set by turn_info::mouse_motion
		// Enable recruit/recall on castle/keep tiles
		return map_.is_castle(last_hex_);
	default:
		return true;
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
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
	case hotkey::HOTKEY_STATISTICS:
	case hotkey::HOTKEY_QUIT_GAME:
	case hotkey::HOTKEY_SEARCH:
	case hotkey::HOTKEY_HELP:
	case hotkey::HOTKEY_USER_CMD:
		return true;

	case hotkey::HOTKEY_SAVE_GAME:
		return !commands_disabled;

	case hotkey::HOTKEY_LOAD_GAME:
		return network::nconnections() == 0; //can only load games if not in a network game

	case hotkey::HOTKEY_SHOW_ENEMY_MOVES:
	case hotkey::HOTKEY_BEST_ENEMY_MOVES:
		return enemies_visible_;

	case hotkey::HOTKEY_SPEAK:
	case hotkey::HOTKEY_SPEAK_ALLY:
	case hotkey::HOTKEY_SPEAK_ALL:
	case hotkey::HOTKEY_CHAT_LOG:
		return network::nconnections() > 0;

	case hotkey::HOTKEY_REDO:
		return !browse_ && !redo_stack_.empty() && !commands_disabled;
	case hotkey::HOTKEY_UNDO:
		return !browse_ && !undo_stack_.empty() && !commands_disabled;

	case hotkey::HOTKEY_CONTINUE_MOVE: {
		if(browse_ || commands_disabled)
			return false;

		if(current_unit() != units_.end() && current_unit()->second.move_interrupted())
			return true;
		const unit_map::const_iterator i = units_.find(selected_hex_);
		if (i == units_.end()) return false;
		return i->second.move_interrupted();
	}
	
	case hotkey::HOTKEY_DELAY_SHROUD:
		return !browse_ && (current_team().uses_fog() || current_team().uses_shroud());
	case hotkey::HOTKEY_UPDATE_SHROUD:
		return !browse_ && !commands_disabled && current_team().auto_shroud_updates() == false;

	//commands we can only do if we are actually playing, not just viewing
	case hotkey::HOTKEY_END_UNIT_TURN:
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL:
	case hotkey::HOTKEY_ENDTURN:
		return !browse_ && !commands_disabled;

	//commands we can only do if there is an active unit
	case hotkey::HOTKEY_UNIT_DESCRIPTION:
		return current_unit() != units_.end();

	case hotkey::HOTKEY_RENAME_UNIT:
		return !commands_disabled &&
			current_unit() != units_.end() &&
			!(current_unit()->second.unrenamable()) &&
			current_unit()->second.side() == gui_.viewing_team()+1;

	case hotkey::HOTKEY_LABEL_TERRAIN:
		return !commands_disabled && map_.on_board(last_hex_) && !gui_.shrouded(last_hex_.x,last_hex_.y) && !is_observer();

	//commands we can only do if in debug mode
	case hotkey::HOTKEY_CREATE_UNIT:
	case hotkey::HOTKEY_CHANGE_UNIT_SIDE:
		return !commands_disabled && game_config::debug && map_.on_board(last_hex_);

	default:
		return false;
	}
}

namespace {

	struct cannot_execute {
		cannot_execute(const turn_info& info) : info_(info) {}
		bool operator()(const std::string& str) const {
			return !info_.can_execute_command(hotkey::get_hotkey(str).get_id());
		}
	private:
		const turn_info& info_;
	};
	
	struct not_in_context_menu {
		not_in_context_menu(const turn_info& info) : info_(info) {}
		bool operator()(const std::string& str) const {
			return !info_.in_context_menu(hotkey::get_hotkey(str).get_id());
		}
	private:
		const turn_info& info_;
	};
}

void turn_info::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	std::vector<std::string> items = items_arg;
	items.erase(std::remove_if(items.begin(),items.end(),cannot_execute(*this)),items.end());
	if(context_menu)
		items.erase(std::remove_if(items.begin(),items.end(),not_in_context_menu(*this)),items.end());
	if(items.empty())
		return;

	//if just one item is passed in, that means we should execute that item
	if(items.size() == 1 && items_arg.size() == 1) {
		hotkey::execute_command(gui_,hotkey::get_hotkey(items.front()).get_id(),this);
		return;
	}

	bool has_image = false;
	std::vector<std::string> menu;
	for(std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i) {
		const hotkey::hotkey_item hk = hotkey::get_hotkey(*i);

		std::stringstream str;
		//see if this menu item has an associated image
		std::string img(get_menu_image(hk.get_id()));
		if(img.empty() == false) {
			has_image = true;
			str << IMAGE_PREFIX << img << COLUMN_SEPARATOR;
		}
		
		str << hk.get_description() << COLUMN_SEPARATOR << hk.get_name();

		menu.push_back(str.str());
	}
	//If any of the menu items have an image, create an image column
	if(has_image)
		for(std::vector<std::string>::iterator i = menu.begin(); i != menu.end(); ++i)
			if(*(i->begin()) != IMAGE_PREFIX)
				i->insert(i->begin(), COLUMN_SEPARATOR);

	static const std::string style = "menu2";
	const int res = gui::show_dialog(gui_,NULL,"","",
			gui::MESSAGE,&menu,NULL,"",NULL,-1,NULL,NULL,xloc,yloc,&style);
	if(res < 0 || res >= items.size())
		return;

	const hotkey::HOTKEY_COMMAND cmd = hotkey::get_hotkey(items[res]).get_id();
	hotkey::execute_command(gui_,cmd,this);
}

bool turn_info::unit_in_cycle(unit_map::const_iterator it) const
{
	if(it->second.side() == team_num_ && unit_can_move(it->first,units_,map_,teams_) && it->second.user_end_turn() == false && !gui_.fogged(it->first.x,it->first.y)) {
		const bool is_enemy = current_team().is_enemy(int(gui_.viewing_team()+1));
		return is_enemy == false || it->second.invisible(map_.underlying_terrain(it->first),status_.get_time_of_day().lawful_bonus,it->first,units_,teams_) == false;
	}

	return false;
	
}

void turn_info::cycle_units()
{

	unit_map::const_iterator it = units_.find(next_unit_);
	unit_map::const_iterator yellow_it = units_.end();
	if(it != units_.end()) {
		for(++it; it != units_.end(); ++it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it == units_.end()) {
		for(it = units_.begin(); it != units_.end(); ++it) {
			if(unit_in_cycle(it)) {
				break;
			}
		}
	}

	if(it != units_.end() && !gui_.fogged(it->first.x,it->first.y)) {
		const bool ignore_zocs = it->second.type().is_skirmisher();
		const bool teleport = it->second.type().teleports();
		current_paths_ = paths(map_,status_,gameinfo_,units_,it->first,teams_,ignore_zocs,teleport,path_turns_);
		gui_.set_paths(&current_paths_);

		gui_.scroll_to_tile(it->first.x,it->first.y,display::WARP);
	}

	if(it != units_.end()) {
		next_unit_ = it->first;
		selected_hex_ = next_unit_;
		gui_.select_hex(selected_hex_);
		gui_.highlight_hex(selected_hex_);
		current_route_.steps.clear();
		gui_.set_route(NULL);
		show_attack_options(it);
	} else {
		next_unit_ = gamemap::location();
	}
}

void turn_info::end_turn()
{
	if(browse_)
		return;

	bool unmoved_units = false, partmoved_units = false;
	for(unit_map::const_iterator un = units_.begin(); un != units_.end(); ++un) {
		if(un->second.side() == team_num_) {
			if(unit_can_move(un->first,units_,map_,teams_)) {
				if(un->second.movement_left() == un->second.total_movement()) {
					unmoved_units = true;
				}

				partmoved_units = true;
			}
		}
	}

	//Ask for confirmation if the player hasn't made any moves (other than gotos).
	if(preferences::confirm_no_moves() && unmoved_units) {
		if (recorder.ncommands() == start_ncmd_) {
			const int res = gui::show_dialog(gui_,NULL,"",_("You have not started your turn yet.  Do you really want to end your turn?"), gui::YES_NO);
			if(res != 0) {
				return;
			}
		}
	}
	
	// Ask for confirmation if units still have movement left
	if(preferences::yellow_confirm() && partmoved_units) {
		const int res = gui::show_dialog(gui_,NULL,"",_("Some units have movement left. Do you really want to end your turn?"),gui::YES_NO);
		if (res != 0) {
			return;
		}
	} else if (preferences::green_confirm() && unmoved_units) {
		const int res = gui::show_dialog(gui_,NULL,"",_("Some units have movement left. Do you really want to end your turn?"),gui::YES_NO);
		if (res != 0) {
			return;
		}
	}

	//force any pending fog updates
	clear_undo_stack();
	end_turn_ = true;
	gui_.set_route(NULL);

	//auto-save
	config snapshot;
	write_game_snapshot(snapshot);
	try {
		recorder.save_game(_("Auto-Save"), snapshot, state_of_game_.starting_pos);
	} catch(gamestatus::save_game_failed&) {
		gui::show_dialog(gui_,NULL,"",_("Could not auto save the game. Please save the game manually."),gui::MESSAGE);
		//do not bother retrying, since the user can just save the game
	}

	recorder.end_turn();
}

void turn_info::goto_leader()
{
	const unit_map::const_iterator i = team_leader(team_num_,units_);
	if(i != units_.end()) {
		clear_shroud();
		gui_.scroll_to_tile(i->first.x,i->first.y,display::WARP);
	}
}

void turn_info::end_unit_turn()
{
	if(browse_)
		return;

	const unit_map::iterator un = units_.find(selected_hex_);
	if(un != units_.end() && un->second.side() == team_num_ && un->second.movement_left() >= 0) {
		un->second.set_user_end_turn(!un->second.user_end_turn());
		gui_.draw_tile(selected_hex_.x,selected_hex_.y);

		gui_.set_paths(NULL);
		current_paths_ = paths();

		if(un->second.user_end_turn()) {
			cycle_units();
		}
	}
}

void turn_info::undo()
{
	if(undo_stack_.empty())
		return;

	const command_disabler disable_commands;

	undo_action& action = undo_stack_.back();
	if(action.is_recall()) {
		player_info* const player = state_of_game_.get_player(teams_[team_num_-1].save_id());

		if(player == NULL) {
			lg::err(lg::engine) << "trying to undo a recall for side " << team_num_
				<< ", which has no recall list!\n";
		} else {
			// Undo a recall action
			team& current_team = teams_[team_num_-1];
			if(units_.count(action.recall_loc) == 0) {
				return;
			}

			const unit& un = units_.find(action.recall_loc)->second;
			statistics::un_recall_unit(un);
			current_team.spend_gold(-game_config::recall_cost);

			std::vector<unit>& recall_list = player->available_units;
			recall_list.insert(recall_list.begin()+action.recall_pos,un);
			units_.erase(action.recall_loc);
			gui_.draw_tile(action.recall_loc.x,action.recall_loc.y);
		}
	} else {
		// Undo a move action
		const int starting_moves = action.starting_moves;
		std::vector<gamemap::location> route = action.route;
		std::reverse(route.begin(),route.end());
		const unit_map::iterator u = units_.find(route.front());
		if(u == units_.end()) {
			//this can actually happen if the scenario designer has abused the [allow_undo] command
			lg::err(lg::engine) << "Illegal 'undo' found. Possible abuse of [allow_undo]?\n";
			return;
		}
	
		if(map_.is_village(route.front())) {
			get_village(route.front(),teams_,action.original_village_owner,units_);
		}
	
		action.starting_moves = u->second.movement_left();
	
		unit un = u->second;
		un.set_goto(gamemap::location());
		units_.erase(u);
		unit_display::move_unit(gui_,map_,route,un,status_.get_time_of_day(),units_,teams_);

		un.set_movement(starting_moves);
		units_.insert(std::pair<gamemap::location,unit>(route.back(),un));
		gui_.draw_tile(route.back().x,route.back().y);
		gui_.update_display();
	}

	gui_.invalidate_unit();
	gui_.invalidate_game_status();

	redo_stack_.push_back(action);
	undo_stack_.pop_back();

	gui_.set_paths(NULL);
	current_paths_ = paths();
	selected_hex_ = gamemap::location();
	current_route_.steps.clear();
	gui_.set_route(NULL);

	recorder.undo();

	const bool shroud_cleared = clear_shroud();

	if(shroud_cleared) {
		gui_.recalculate_minimap();
	} else {
		gui_.redraw_minimap();
	}
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

	undo_action& action = redo_stack_.back();
	if(action.is_recall()) {
		player_info *player=state_of_game_.get_player(teams_[team_num_-1].save_id());
		if(!player) {
			lg::err(lg::engine) << "trying to redo a recall for side " << team_num_
				<< ", which has no recall list!\n";
		} else {
			// Redo recall
			std::vector<unit>& recall_list = player->available_units;
			unit& un = recall_list[action.recall_pos];
			recruit_unit(map_,team_num_,units_,un,action.recall_loc,&gui_);
			statistics::recall_unit(un);
			team& current_team = teams_[team_num_-1];
			current_team.spend_gold(game_config::recall_cost);
			recall_list.erase(recall_list.begin()+action.recall_pos);
			recorder.add_recall(action.recall_pos,action.recall_loc);

			gui_.draw_tile(action.recall_loc.x,action.recall_loc.y);
		}
	} else {
		// Redo movement action
		const int starting_moves = action.starting_moves;
		std::vector<gamemap::location> route = action.route;
		const unit_map::iterator u = units_.find(route.front());
		if(u == units_.end()) {
			wassert(false);
			return;
		}
	
		action.starting_moves = u->second.movement_left();
	
		unit un = u->second;
		un.set_goto(gamemap::location());
		units_.erase(u);
		unit_display::move_unit(gui_,map_,route,un,status_.get_time_of_day(),units_,teams_);
		un.set_movement(starting_moves);
		units_.insert(std::pair<gamemap::location,unit>(route.back(),un));
	
		if(map_.is_village(route.back())) {
			get_village(route.back(),teams_,un.side()-1,units_);
		}
	
		gui_.draw_tile(route.back().x,route.back().y);
	
		recorder.add_movement(route.front(),route.back());
	}
	gui_.invalidate_unit();
	gui_.invalidate_game_status();

	undo_stack_.push_back(action);
	redo_stack_.pop_back();
}

void turn_info::unit_description()
{
	const unit_map::const_iterator un = current_unit();
	if(un != units_.end()) {
		dialogs::show_unit_description(gui_, un->second);
	}
}

void turn_info::rename_unit()
{
	const unit_map::iterator un = current_unit();
	if(un == units_.end() || gui_.viewing_team()+1 != un->second.side())
		return;

	std::string name = un->second.description();
	const int res = gui::show_dialog(gui_,NULL,_("Rename Unit"),"", gui::OK_CANCEL,NULL,NULL,"",&name);
	if(res == 0) {
		un->second.rename(name);
		gui_.invalidate_unit();
	}
}

void turn_info::save_game()
{
	save_game("",gui::OK_CANCEL);
}

void turn_info::load_game()
{
	bool show_replay = false;
	const std::string game = dialogs::load_game_dialog(gui_,terrain_config_,gameinfo_,&show_replay);
	if(game != "") {
		throw gamestatus::load_game_exception(game,show_replay);
	}
}

namespace {

bool is_illegal_file_char(char c)
{
	return c == '/' || c == '\\' || c == ':';
}

}

void turn_info::save_game(const std::string& message, gui::DIALOG_TYPE dialog_type)
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(state_of_game_.label, 
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("Turn")
	       << " " << status_.turn();
	std::string label = stream.str();
	if(dialog_type == gui::NULL_DIALOG && message != "") {
		label = message;
	}

	label.erase(std::remove_if(label.begin(),label.end(),is_illegal_file_char),label.end());

	const int res = dialog_type == gui::NULL_DIALOG ? 0 : dialogs::get_save_name(gui_,message,_("Name:"),&label,dialog_type);

	if(std::count_if(label.begin(),label.end(),is_illegal_file_char)) {
		gui::show_dialog(gui_,NULL,_("Error"),_("Save names may not contain colons, slashes, or backslashes. Please choose a different name."),gui::OK_ONLY);
		save_game(message,dialog_type);
		return;
	}

	if(res == 0) {
		config snapshot;
		write_game_snapshot(snapshot);
		try {
			recorder.save_game(label, snapshot, state_of_game_.starting_pos);
			if(dialog_type != gui::NULL_DIALOG) {
				gui::show_dialog(gui_,NULL,_("Saved"),_("The game has been saved"), gui::OK_ONLY);
			}
		} catch(gamestatus::save_game_failed&) {
			gui::show_dialog(gui_,NULL,_("Error"),_("The game could not be saved"),gui::MESSAGE);
			//do not bother retrying, since the user can just try to save the game again
		};
	}
}

unit_map::const_iterator turn_info::find_unit(const gamemap::location& hex) const
{
	if(gui_.fogged(hex.x,hex.y)) {
		return units_.end();
	}

	return find_visible_unit(units_,hex,map_,status_.get_time_of_day().lawful_bonus,teams_,viewing_team());
}

unit_map::iterator turn_info::find_unit(const gamemap::location& hex)
{
	if(gui_.fogged(hex.x,hex.y)) {
		return units_.end();
	}

	return find_visible_unit(units_,hex,map_,status_.get_time_of_day().lawful_bonus,teams_,viewing_team());
}

unit_map::const_iterator turn_info::selected_unit() const
{
	unit_map::const_iterator res = find_unit(selected_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(last_hex_);
	}
}

unit_map::iterator turn_info::selected_unit()
{
	unit_map::iterator res = find_unit(selected_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(last_hex_);
	}
}

unit_map::const_iterator turn_info::current_unit() const
{
	unit_map::const_iterator res = find_unit(last_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(selected_hex_);
	}
}

unit_map::iterator turn_info::current_unit()
{
	unit_map::iterator res = find_unit(last_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(selected_hex_);
	}
}

void turn_info::write_game_snapshot(config& start) const
{
	start.values = level_->values;

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

		for(std::map<gamemap::location,unit>::const_iterator i = units_.begin(); i != units_.end(); ++i) {
			if(i->second.side() == side_num) {
				config& u = side.add_child("unit");
				i->first.write(u);
				i->second.write(u);
			}
		}
	}

	status_.write(start);
	game_events::write_events(start);

	write_game(state_of_game_,start,WRITE_SNAPSHOT_ONLY);

	// Clobber gold values to make sure the snapshot uses the values
	// in [side] instead.
	const config::child_list& players=start.get_children("player");
	for(config::child_list::const_iterator pi=players.begin();
	    pi!=players.end(); ++pi) {
		(**pi)["gold"] = "-1000000";
	}

	//write out the current state of the map
	start["map_data"] = map_.write();

	gui_.labels().write(start);
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
	heading << _("Leader") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR
	        << _("Gold") << COLUMN_SEPARATOR
	        << _("Villages") << COLUMN_SEPARATOR
	        << _("Units") << COLUMN_SEPARATOR
	        << _("Upkeep") << COLUMN_SEPARATOR
	        << _("Income");

	if(game_config::debug)
		heading << COLUMN_SEPARATOR << _("Gold");

	items.push_back(heading.str());

	const team& viewing_team = teams_[gui_.viewing_team()];

	//if the player is under shroud or fog, they don't get to see
	//details about the other sides, only their own side, allied sides and a ??? is
	//shown to demonstrate lack of information about the other sides
	bool fog = false;
	for(size_t n = 0; n != teams_.size(); ++n) {
		if(teams_[n].is_empty()) {
			continue;
		}

		const bool known = viewing_team.knows_about_team(n);
		const bool enemy = viewing_team.is_enemy(n+1);
		if(!known) {
			fog = true;
			continue;
		}

		const team_data data = calculate_team_data(teams_[n],n+1,units_);

		std::stringstream str;

		const unit_map::const_iterator leader = team_leader(n+1,units_);
		if(leader != units_.end()) {
			str << IMAGE_PREFIX << leader->second.type().image() << COLUMN_SEPARATOR
			    << char(n+1) << leader->second.description() << COLUMN_SEPARATOR;
		} else {
			str << char(n+1) << '-' << COLUMN_SEPARATOR << char(n+1) << '-' << COLUMN_SEPARATOR;
		}

		if(enemy) {
			str << ' ' << COLUMN_SEPARATOR;
		} else {
			str << data.gold << COLUMN_SEPARATOR;
		}
		//output the number of the side first, and this will
		//cause it to be displayed in the correct colour
		str << data.villages << COLUMN_SEPARATOR
		    << data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
			<< (data.net_income < 0 ? font::BAD_TEXT:font::NULL_MARKUP) << data.net_income;

		if(game_config::debug)
			str << COLUMN_SEPARATOR << teams_[n].gold();

		items.push_back(str.str());
	}

	if(fog)
		items.push_back(IMAGE_PREFIX + std::string("random-enemy.png") + COLUMN_SEPARATOR +
		                IMAGE_PREFIX + "random-enemy.png");

	gui::show_dialog(gui_,NULL,"","",gui::CLOSE_ONLY,&items);
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
	for(std::set<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it) {
		const std::map<std::string,unit_type>::const_iterator
				u_type = gameinfo_.unit_types.find(*it);
		if(u_type == gameinfo_.unit_types.end()) {
			lg::err(lg::engine) << "could not find unit '" << *it << "'";
			return;
		}

		item_keys.push_back(*it);

		const unit_type& type = u_type->second;

		//display units that we can't afford to recruit in red
		const char prefix = (type.cost() > current_team.gold() ? font::BAD_TEXT : font::NULL_MARKUP);

		std::stringstream description;

		description << font::IMAGE << type.image() << COLUMN_SEPARATOR << font::LARGE_TEXT
		            << prefix << type.language_name() << "\n"
		            << prefix << type.cost() << " " << sgettext("unit^Gold");
		items.push_back(description.str());
		sample_units.push_back(unit(&type,team_num_));
	}

	if(sample_units.empty()) {
		gui::show_dialog(gui_,NULL,"",_("You have no units available to recruit."),gui::MESSAGE);
		return;
	}

	int recruit_res = 0;

	{
		const events::event_context dialog_events_context;
		dialogs::unit_preview_pane unit_preview(gui_,&map_,sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		recruit_res = gui::show_dialog(gui_,NULL,_("Recruit"),
				_("Select unit") + std::string(":\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,NULL,-1,-1,
				NULL,NULL,"recruit_and_recall");
	}

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
	wassert(u_type != gameinfo_.unit_types.end());

	if(u_type->second.cost() > current_team.gold()) {
		gui::show_dialog(gui_,NULL,"",
		     _("You don't have enough gold to recruit that unit"),gui::OK_ONLY);
	} else {
		last_recruit_ = name;

		//create a unit with traits
		recorder.add_recruit(recruit_num,last_hex_);
		unit new_unit(&(u_type->second),team_num_,true);
		gamemap::location loc = last_hex_;
		const std::string& msg = recruit_unit(map_,team_num_,units_,new_unit,loc,&gui_);
		if(msg.empty()) {
			current_team.spend_gold(u_type->second.cost());
			statistics::recruit_unit(new_unit);
		} else {
			recorder.undo();
			gui::show_dialog(gui_,NULL,"",msg,gui::OK_ONLY);
		}

		clear_undo_stack();
		redo_stack_.clear();

		clear_shroud();

		gui_.recalculate_minimap();
		gui_.invalidate_game_status();
		gui_.invalidate_all();
	}
}

void turn_info::repeat_recruit()
{
	if(last_recruit_.empty() == false)
		do_recruit(last_recruit_);
}

//a class to handle deleting an item from the recall list
namespace {

class delete_recall_unit : public gui::dialog_button_action
{
public:
	delete_recall_unit(display& disp, std::vector<unit>& units) : disp_(disp), units_(units) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<unit>& units_;
};

gui::dialog_button_action::RESULT delete_recall_unit::button_pressed(int menu_selection)
{
	const size_t index = size_t(menu_selection);
	if(index < units_.size()) {
		const unit& u = units_[index];

		//if the unit is of level > 1, or is close to advancing, we warn the player
		//about it
		std::string message = "";
		if(u.type().level() > 1) {
			message = _("My lord, this unit is an experienced one, having advanced levels! Do you really want to dismiss $noun?");
		} else if(u.experience() > u.max_experience()/2) {
			message = _("My lord, this unit is close to advancing a level! Do you really want to dismiss $noun?");
		}

		if(message != "") {
			string_map symbols;
			symbols["noun"] = (u.gender() == unit_race::MALE ? _("him") : _("her"));
			message = config::interpolate_variables_into_string(message,&symbols);

			const int res = gui::show_dialog(disp_,NULL,"",message,gui::YES_NO);
			if(res != 0) {
				return gui::dialog_button_action::NO_EFFECT;
			}
		}

		units_.erase(units_.begin() + index);
		recorder.add_disband(index);
		return gui::dialog_button_action::DELETE_ITEM;
	} else {
		return gui::dialog_button_action::NO_EFFECT;
	}
}

} //end anon namespace

void turn_info::recall()
{
	if(browse_)
		return;

	player_info *player = state_of_game_.get_player(teams_[team_num_-1].save_id());
	if(!player) {
		lg::err(lg::engine) << "cannot recall a unit for side " << team_num_
			<< ", which has no recall list!\n";
		return;
	}

	team& current_team = teams_[team_num_-1];
	std::vector<unit>& recall_list = player->available_units;

	//sort the available units into order by value
	//so that the most valuable units are shown first
	std::sort(recall_list.begin(),recall_list.end(),compare_unit_values());

	gui_.draw(); //clear the old menu

	if((*level_)["disallow_recall"] == "yes") {
		gui::show_dialog(gui_,NULL,"",_("You are separated from your soldiers and may not recall them"));
	} else if(recall_list.empty()) {
		gui::show_dialog(gui_,NULL,"",_("There are no troops available to recall\n\
(You must have veteran survivors from a previous scenario)"));
	} else if(current_team.gold() < game_config::recall_cost) {
		std::stringstream msg;
		string_map i18n_symbols;
		i18n_symbols["cost"] = lexical_cast<std::string>(game_config::recall_cost);
		msg << vgettext("You must have at least $cost gold pieces to recall a unit", i18n_symbols);
		gui::show_dialog(gui_,NULL,"",msg.str());
	} else {
		std::vector<std::string> options;
		for(std::vector<unit>::const_iterator u = recall_list.begin(); u != recall_list.end(); ++u) {
			std::stringstream option;
			const std::string& description = u->description().empty() ? "-" : u->description();
			option << IMAGE_PREFIX << u->type().image() << COLUMN_SEPARATOR
			       << u->type().language_name() << COLUMN_SEPARATOR
			       << description << COLUMN_SEPARATOR
			       << _("level") << ": " << u->type().level() << COLUMN_SEPARATOR
			       << _("XP") << ": " << u->experience() << "/";

			if(u->can_advance() == false) {
				option << "-";
			} else {
				option << u->max_experience();
			}

			options.push_back(option.str());
		}

		delete_recall_unit recall_deleter(gui_,recall_list);
		gui::dialog_button delete_button(&recall_deleter,_("Dismiss Unit"));
		std::vector<gui::dialog_button> buttons;
		buttons.push_back(delete_button);

		int res = 0;

		{
			const events::event_context dialog_events_context;
			dialogs::unit_preview_pane unit_preview(gui_,&map_,recall_list);
			std::vector<gui::preview_pane*> preview_panes;
			preview_panes.push_back(&unit_preview);

			res = gui::show_dialog(gui_,NULL,_("Recall"),
					_("Select unit") + std::string(":\n"),
					gui::OK_CANCEL,&options,
					&preview_panes,"",NULL,-1,
					NULL,NULL,-1,-1,NULL,&buttons);
		}

		if(res >= 0) {
			unit& un = recall_list[res];
			gamemap::location loc = last_hex_;
			const std::string err = recruit_unit(map_,team_num_,units_,un,loc,&gui_);
			if(!err.empty()) {
				gui::show_dialog(gui_,NULL,"",err,gui::OK_ONLY);
			} else {
				statistics::recall_unit(un);
				current_team.spend_gold(game_config::recall_cost);
				recorder.add_recall(res,loc);

				undo_stack_.push_back(undo_action(un,loc,res));
				redo_stack_.clear();
				
				recall_list.erase(recall_list.begin()+res);
				gui_.invalidate_game_status();
			}
		}
	}
}

bool turn_info::has_friends() const
{
	if(is_observer()) {
		return false;
	}

	for(size_t n = 0; n != teams_.size(); ++n) {
		if(n != gui_.viewing_team() && teams_[gui_.viewing_team()].team_name() == teams_[n].team_name() && teams_[n].is_network()) {
			return true;
		}
	}

	return false;
}

void turn_info::speak()
{
	create_textbox(floating_textbox::TEXTBOX_MESSAGE,_("Message") + std::string(":"), has_friends() ? _("Send to allies only") : "", preferences::message_private());
}

void turn_info::do_speak(const std::string& message, bool allies_only)
{
	if(message == "") {
		return;
	}

	config cfg;
	cfg["description"] = preferences::login();
	cfg["message"] = message;

	const int side = is_observer() ? 0 : gui_.viewing_team()+1;
	if(!is_observer()) {
		cfg["side"] = lexical_cast<std::string>(side);
	}

	bool private_message = has_friends() && allies_only;

	if(private_message) {
		cfg["team_name"] = teams_[gui_.viewing_team()].team_name();
	}

	lg::info(lg::config) << "logging speech: '" << cfg.write() << "'\n";
	recorder.speak(cfg);
	gui_.add_chat_message(cfg["description"],side,message,
		                  private_message ? display::MESSAGE_PRIVATE : display::MESSAGE_PUBLIC);
}

void turn_info::create_unit()
{
	std::vector<std::string> options;
	std::vector<unit> unit_choices;
	for(game_data::unit_type_map::const_iterator i = gameinfo_.unit_types.begin(); i != gameinfo_.unit_types.end(); ++i) {
		options.push_back(i->first);
		unit_choices.push_back(unit(&i->second,1,false));
		unit_choices.back().new_turn();
	}

	int choice = 0;

	{
		const events::event_context dialog_events_context;
		dialogs::unit_preview_pane unit_preview(gui_,&map_,unit_choices);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		choice = gui::show_dialog(gui_,NULL,"","Create unit (debug):",
		                          gui::OK_CANCEL,&options,&preview_panes);
	}

	if(choice >= 0 && choice < unit_choices.size()) {
		units_.erase(last_hex_);
		units_.insert(std::pair<gamemap::location,unit>(last_hex_,unit_choices[choice]));
		gui_.invalidate(last_hex_);
		gui_.invalidate_unit();
	}
}

void turn_info::change_unit_side()
{
	const unit_map::iterator i = units_.find(last_hex_);
	if(i == units_.end()) {
		return;
	}

	int side = i->second.side();
	++side;
	if(side > team::nteams()) {
		side = 1;
	}

	i->second.set_side(side);
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
	const std::string heading = _("Type") + std::string(1, COLUMN_SEPARATOR) +
	                            _("Name") + COLUMN_SEPARATOR +
	                            _("HP") + COLUMN_SEPARATOR +
	                            _("XP") + COLUMN_SEPARATOR +
	                            _("Traits") + COLUMN_SEPARATOR +
	                            _("Moves") + COLUMN_SEPARATOR +
	                            _("Location");

	std::vector<std::string> items;
	items.push_back(heading);

	std::vector<gamemap::location> locations_list;
	std::vector<unit> units_list;
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
 		if(i->second.side() != (gui_.viewing_team()+1))
			continue;

		std::stringstream row;
		row << i->second.type().language_name() << COLUMN_SEPARATOR
		    << i->second.description() << COLUMN_SEPARATOR
		    << i->second.hitpoints() << "/" << i->second.max_hitpoints() << COLUMN_SEPARATOR
		    << i->second.experience() << "/";

		if(i->second.can_advance() == false)
			row << "-";
		else
			row << i->second.max_experience();

		row << COLUMN_SEPARATOR
		    << i->second.traits_description() << COLUMN_SEPARATOR
		    << i->second.movement_left() << "/"
		    << i->second.total_movement() << COLUMN_SEPARATOR
		    << (i->first.x + 1) << "-" << (i->first.y + 1);

		items.push_back(row.str());

		//extra unit for the first row to make up the heading
		if(units_list.empty()) {
			locations_list.push_back(i->first);
			units_list.push_back(i->second);
		}

		locations_list.push_back(i->first);
		units_list.push_back(i->second);
	}

	int selected = 0;

	{
		const events::event_context dialog_events_context;
		dialogs::unit_preview_pane unit_preview(gui_,&map_,units_list);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		selected = gui::show_dialog(gui_,NULL,_("Unit List"),"",
		                            gui::OK_ONLY,&items,&preview_panes);
	}

	if(selected > 0 && selected < int(locations_list.size())) {
		const gamemap::location& loc = locations_list[selected];
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP);
	}
}

std::vector<std::string> turn_info::create_unit_table(const statistics::stats::str_int_map& m)
{
	std::vector<std::string> table;
	for(statistics::stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		const game_data::unit_type_map::const_iterator type = gameinfo_.unit_types.find(i->first);
		if(type == gameinfo_.unit_types.end()) {
			continue;
		}

		std::stringstream str;
		str << IMAGE_PREFIX << type->second.image() << COLUMN_SEPARATOR
		    << type->second.language_name() << COLUMN_SEPARATOR << i->second << "\n";
		table.push_back(str.str());
	}

	return table;
}

void turn_info::show_statistics()
{
	const statistics::stats& stats = statistics::calculate_stats(0,gui_.viewing_team()+1);
	std::vector<std::string> items;

	{
		std::stringstream str;
		str << _("Recruits") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.recruits);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Recalls") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.recalls);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Advancements") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.advanced_to);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Losses") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.deaths);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Kills") << COLUMN_SEPARATOR << statistics::sum_str_int_map(stats.killed);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Inflicted") << COLUMN_SEPARATOR << stats.damage_inflicted;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Taken") << COLUMN_SEPARATOR << stats.damage_taken;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Inflicted (EV)") << COLUMN_SEPARATOR
		    << (stats.expected_damage_inflicted / 100.0);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Taken (EV)") <<  COLUMN_SEPARATOR
		    << (stats.expected_damage_taken / 100.0);
		items.push_back(str.str());
	}

	for(;;) {
		const int res = gui::show_dialog(gui_, NULL, _("Statistics"), "", gui::OK_CANCEL, &items);
		std::string title;
		std::vector<std::string> items_sub;

		switch(res) {
		case 0:
			items_sub = create_unit_table(stats.recruits);
			title = _("Recruits");
			break;
		case 1:
			items_sub = create_unit_table(stats.recalls);
			title = _("Recalls");
			break;
		case 2:
			items_sub = create_unit_table(stats.advanced_to);
			title = _("Advancements");
			break;
		case 3:
			items_sub = create_unit_table(stats.deaths);
			title = _("Losses");
			break;
		case 4:
			items_sub = create_unit_table(stats.killed);
			title = _("Kills");
			break;
		default:
			return;
		}

		if (items_sub.empty() == false)
			gui::show_dialog(gui_, NULL, "", title, gui::OK_ONLY, &items_sub);
	}
}

void turn_info::search()
{
	std::ostringstream msg;
	msg << _("Search");
	if(last_search_hit_.valid()) {
		msg << " [" << last_search_ << "]";
	}
	msg << ':';
	create_textbox(floating_textbox::TEXTBOX_SEARCH,msg.str());
}

void turn_info::user_command()
{
	create_textbox(floating_textbox::TEXTBOX_COMMAND,sgettext("prompt^Command:"));
}

void turn_info::show_help()
{
	help::show_help(gui_);
}

void turn_info::show_chat_log()
{
	std::string text = recorder.build_chat_log(teams_[gui_.viewing_team()].team_name());
	gui::show_dialog(gui_,NULL,_("Chat Log"),"",gui::CLOSE_ONLY,NULL,NULL,"",&text);
}

void turn_info::do_search(const std::string& new_search)
{
	if(new_search.empty() == false && new_search != last_search_)
		last_search_ = new_search;
	
	if(last_search_.empty()) return;
	
	bool found = false;
	gamemap::location loc = last_search_hit_;
	//If this is a location search, just center on that location.
	std::vector<std::string> args = config::split(last_search_,',');
	if(args.size() == 2) {
		int x, y;
		x = lexical_cast_default<int>(args[0], 0)-1;
		y = lexical_cast_default<int>(args[1], 0)-1;
		if(x >= 0 && x < map_.x() && y >= 0 && y < map_.y()) {
			loc = gamemap::location(x,y);
			found = true;
		}
	}
	//Start scanning the game map
	if(loc.valid() == false)
		loc = gamemap::location(map_.x()-1,map_.y()-1);
	gamemap::location start = loc;
	while (!found) {
		//Move to the next location
		loc.x = (loc.x + 1) % map_.x();
		if(loc.x == 0)
			loc.y = (loc.y + 1) % map_.y();
		
		//Search label
		const std::string label = gui_.labels().get_label(loc);
		if(label.empty() == false) {
			if(std::search(label.begin(), label.end(),
					last_search_.begin(), last_search_.end(),
					chars_equal_insensitive) != label.end()) {
				found = !gui_.shrouded(loc.x, loc.y);
			}
		}
		//Search unit name
		unit_map::const_iterator ui = units_.find(loc);
		if(ui != units_.end()) {
			const std::string name = ui->second.description();
			if(std::search(name.begin(), name.end(),
					last_search_.begin(), last_search_.end(),
					chars_equal_insensitive) != name.end()) {
				found = !gui_.fogged(loc.x, loc.y);
			}
		}
		if(loc == start)
			break;
	}
	if(found) {
		last_search_hit_ = loc;
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
		gui_.highlight_hex(loc);
	} else {
		last_search_hit_ = gamemap::location();
		//Not found, inform the player
		string_map symbols;
		symbols["search"] = last_search_;
		const std::string msg = config::interpolate_variables_into_string(
			_("Couldn't find label or unit containing the string '$search'."),&symbols);
		gui::show_dialog(gui_,NULL,"",msg);
	}
}

void turn_info::do_command(const std::string& str)
{
	const std::string::const_iterator i = std::find(str.begin(),str.end(),' ');
	const std::string cmd(str.begin(),i);
	const std::string data(i == str.end() ? str.end() : i+1,str.end());

	if(cmd == "refresh") {
		image::flush_cache();
		gui_.redraw_everything();
	} else if(cmd == "ban") {
		config cfg;
		config& ban = cfg.add_child("ban");
		ban["username"] = data;

		network::send_data(cfg);
	} else if(cmd == "control") {
		const std::string::const_iterator j = std::find(data.begin(),data.end(),' ');
		if(j != data.end()) {
			const std::string side(data.begin(),j);
			const std::string player(j+1,data.end());

			change_side_controller(side,player);
		}
	} else if(cmd == "clear") {
		gui_.clear_chat_messages();
	} else if(cmd == "w") {
		save_game(data,gui::NULL_DIALOG);
	} else if(cmd == "wq") {
		save_game(data,gui::NULL_DIALOG);
		throw end_level_exception(QUIT);
	} else if(cmd == "q!" || cmd == "q") {
		throw end_level_exception(QUIT);
	} else if(cmd == "debug") {
		game_config::debug = (data != "off") ? true : false;
	} else if(cmd == "n" && game_config::debug) {
		throw end_level_exception(VICTORY);
	} else if(game_config::debug && cmd == "unit") {
		const unit_map::iterator i = current_unit();
		if(i != units_.end()) {
			const std::string::const_iterator j = std::find(data.begin(),data.end(),'=');
			if(j != data.end()) {
				const std::string name(data.begin(),j);
				const std::string value(j+1,data.end());
				config cfg;
				i->second.write(cfg);
				cfg[name] = value;
				i->second = unit(gameinfo_,cfg);

				gui_.invalidate(i->first);
				gui_.invalidate_unit();
			}
		}
	}
}

void turn_info::change_side_controller(const std::string& side, const std::string& player, bool orphan_side)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;

	if(orphan_side) {
		change["orphan_side"] = "yes";
	}

	network::send_data(cfg);
}

void turn_info::label_terrain()
{
	if(map_.on_board(last_hex_) == false) {
		return;
	}

	std::string label = gui_.labels().get_label(last_hex_);
	const int res = gui::show_dialog(gui_,NULL,_("Place Label"),"",gui::OK_CANCEL,
	                                 NULL,NULL,_("Label") + std::string(":"),&label,
					 map_labels::get_max_chars());
	if(res == 0) {
		gui_.labels().set_label(last_hex_,label);
		recorder.add_label(label,last_hex_);
	}
}

// Returns true if any enemy units are visible.
bool turn_info::enemies_visible() const
{
	// If we aren't using fog/shroud, this is easy :)
	if(current_team().uses_fog() == false && current_team().uses_shroud() == false)
		return true;
	
	//See if any enemies are visible
	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u)
		if(current_team().is_enemy(u->second.side()) && !gui_.fogged(u->first.x,u->first.y))
			return true;
		
	return false;
}

// Highlights squares that an enemy could move to on their turn
void turn_info::show_enemy_moves(bool ignore_units)
{
	all_paths_ = paths();
	
	// Compute enemy movement positions
	for(unit_map::iterator u = units_.begin(); u != units_.end(); ++u) {
		if(current_team().is_enemy(u->second.side()) && !gui_.fogged(u->first.x,u->first.y) && !u->second.stone()) {
			const unit_movement_resetter move_reset(u->second);
			const bool is_skirmisher = u->second.type().is_skirmisher();
			const bool teleports = u->second.type().teleports();
			unit_map units;
			units.insert(*u);
			const paths& path = paths(map_,status_,gameinfo_,ignore_units?units:units_,
									  u->first,teams_,is_skirmisher,teleports);
			
			for (paths::routes_map::const_iterator route = path.routes.begin(); route != path.routes.end(); ++route) {
				// map<...>::operator[](const key_type& key) inserts key into
				// the map with a default instance of value_type
				all_paths_.routes[route->first];
			}
		}
	}
	
	gui_.set_paths(&all_paths_);
}

void turn_info::toggle_shroud_updates() {
	bool auto_shroud = current_team().auto_shroud_updates();
	// If we're turning automatic shroud updates on, then commit all moves
	if(auto_shroud == false) update_shroud_now();
	current_team().set_auto_shroud_updates(!auto_shroud);
}

bool turn_info::clear_shroud()
{
	bool cleared = current_team().auto_shroud_updates() && 
		::clear_shroud(gui_,status_,map_,gameinfo_,units_,teams_,team_num_-1);
	enemies_visible_ = enemies_visible();
	return cleared;
}

void turn_info::clear_undo_stack()
{
	if(current_team().auto_shroud_updates() == false)
		apply_shroud_changes(undo_stack_,&gui_,status_,map_,gameinfo_,units_,teams_,team_num_-1);
	undo_stack_.clear();
}

void turn_info::update_shroud_now()
{
	clear_undo_stack();
}

void turn_info::continue_move()
{
	unit_map::iterator i = current_unit();
	if(i == units_.end() || i->second.move_interrupted() == false) {
		i = units_.find(selected_hex_);
		if (i == units_.end() || i->second.move_interrupted() == false) return;
	}
	move_unit_to_loc(i,i->second.get_interrupted_move(),true);
}

turn_info::PROCESS_DATA_RESULT turn_info::process_network_data(const config& cfg, network::connection from, std::deque<config>& backlog)
{
	if(cfg.child("observer") != NULL) {
		const config::child_list& observers = cfg.get_children("observer");
		for(config::child_list::const_iterator ob = observers.begin(); ob != observers.end(); ++ob) {
			gui_.add_observer((**ob)["name"]);
		}
	}

	if(cfg.child("observer_quit") != NULL) {
		const config::child_list& observers = cfg.get_children("observer_quit");
		for(config::child_list::const_iterator ob = observers.begin(); ob != observers.end(); ++ob) {
			gui_.remove_observer((**ob)["name"]);
		}
	}

	if(cfg.child("leave_game") != NULL) {
		throw network::error("");
	}

	bool turn_end = false;

	const config::child_list& turns = cfg.get_children("turn");
	if(turns.empty() == false && from != network::null_connection) {
		//forward the data to other peers
		network::send_data_all_except(cfg,from);
	}

	for(config::child_list::const_iterator t = turns.begin(); t != turns.end(); ++t) {

		if(turn_end == false) {
			replay replay_obj(**t);
			replay_obj.start_replay();

			try {
				turn_end = do_replay(gui_,map_,gameinfo_,units_,teams_,
				team_num_,status_,state_of_game_,&replay_obj);
			} catch(replay::error&) {
				//notify remote hosts of out of sync error
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "out of sync";
				network::send_data(cfg);

				save_game(_("The games are out of sync and will have to exit. Do you want to save an error log of your game?"),gui::YES_NO);

				//throw e;
			}

			recorder.add_config(**t,replay::MARK_AS_SENT);
		} else {

			//this turn has finished, so push the remaining moves
			//into the backlog
			backlog.push_back(config());
			backlog.back().add_child("turn",**t);
		}
	}

	if(const config* change= cfg.child("change_controller")) {
		const int side = lexical_cast_default<int>((*change)["side"],1);
		const size_t index = static_cast<size_t>(side-1);

		const std::string& controller = (*change)["controller"];

		if(index < teams_.size()) {
			if(controller == "human") {
				teams_[index].make_human();
			} else if(controller == "network") {
				teams_[index].make_network();
			} else if(controller == "ai") {
				teams_[index].make_ai();
			}

			return PROCESS_RESTART_TURN;
		}
	}

	//if a side has dropped out of the game.
	if(cfg["side_drop"] != "") {
		const size_t side = atoi(cfg["side_drop"].c_str())-1;
		if(side >= teams_.size()) {
			lg::err(lg::network) << "unknown side " << side << " is dropping game\n";
			throw network::error("");
		}

		int action = 0;

		std::vector<std::string> observers;

		//see if the side still has a leader alive. If they have
		//no leader, we assume they just want to be replaced by
		//the AI.
		const unit_map::const_iterator leader = find_leader(units_,side+1);
		if(leader != units_.end()) {
			std::vector<std::string> options;
			options.push_back(_("Replace with AI"));
			options.push_back(_("Replace with local player"));
			options.push_back(_("Abort game"));

			for(std::set<std::string>::const_iterator ob = gui_.observers().begin(); ob != gui_.observers().end(); ++ob) {
				options.push_back(_("Replace with ") + *ob);
				observers.push_back(*ob);
			}

			const std::string msg = leader->second.description() + " " + _("has left the game. What do you want to do?");
			action = gui::show_dialog(gui_,NULL,"",msg,gui::OK_ONLY,&options);
		}

		//make the player an AI, and redo this turn, in case
		//it was the current player's team who has just changed into
		//an AI.
		if(action == 0) {
			teams_[side].make_ai();
			return PROCESS_RESTART_TURN;
		} else if(action == 1) {
			teams_[side].make_human();
			return PROCESS_RESTART_TURN;
		} else if(action > 2) {
			const size_t index = size_t(action - 3);
			if(index < observers.size()) {
				change_side_controller(cfg["side_drop"],observers[index],true);
			}

			teams_[side].make_human();
			return PROCESS_RESTART_TURN;
		}

		throw network::error("");
	}

	return turn_end ? PROCESS_END_TURN : PROCESS_CONTINUE;
}

void turn_info::create_textbox(floating_textbox::MODE mode, const std::string& label, const std::string& check_label, bool checked)
{
	close_textbox();

	textbox_.mode = mode;

	if(check_label != "") {
		textbox_.check.assign(new gui::button(gui_,check_label,gui::button::TYPE_CHECK));
		textbox_.check->set_check(checked);
	}

	const SDL_Rect& area = gui_.map_area();

	const int border_size = 10;

	const int ypos = area.y+area.h-30 - (textbox_.check != NULL ? textbox_.check->height() + border_size : 0);
	textbox_.label = font::add_floating_label(label,font::SIZE_NORMAL,font::YELLOW_COLOUR,area.x+border_size,ypos,0,0,-1,
		area,font::LEFT_ALIGN);
	if(textbox_.label == 0) {
		return;
	}

	const SDL_Rect& label_area = font::get_floating_label_rect(textbox_.label);
	const int textbox_width = area.w - label_area.w - border_size*3;

	if(textbox_width <= 0) {
		font::remove_floating_label(textbox_.label);
		return;
	}

	textbox_.box.assign(new gui::textbox(gui_,textbox_width,"",true,256,0.8,0.6));
	textbox_.box->set_volatile(true);
	textbox_.box->set_location(area.x + label_area.w + border_size*2,ypos);

	if(textbox_.check != NULL) {
		textbox_.check->set_volatile(true);
		textbox_.check->set_location(textbox_.box->location().x,textbox_.box->location().y + textbox_.box->location().h + border_size);
	}
}

void turn_info::close_textbox()
{
	if(textbox_.active() == false) {
		return;
	}
	if(textbox_.check != NULL) {
		if(textbox_.mode == floating_textbox::TEXTBOX_MESSAGE) {
			preferences::set_message_private(textbox_.check->checked());
		}
	}
	textbox_.box.assign(NULL);
	textbox_.check.assign(NULL);
	font::remove_floating_label(textbox_.label);
	textbox_.mode = floating_textbox::TEXTBOX_NONE;
	gui_.invalidate_all();
}

void turn_info::enter_textbox()
{
	if(textbox_.active() == false) {
		return;
	}

	switch(textbox_.mode) {
	case floating_textbox::TEXTBOX_SEARCH:
		do_search(textbox_.box->text());
		break;
	case floating_textbox::TEXTBOX_MESSAGE:
		do_speak(textbox_.box->text(),textbox_.check != NULL ? textbox_.check->checked() : false);
		break;
	case floating_textbox::TEXTBOX_COMMAND:
		do_command(textbox_.box->text());
		break;
	default:
		lg::err(lg::display) << "unknown textbox mode\n";
	}

	close_textbox();
}

void turn_info::tab_textbox()
{
	if(textbox_.active() == false) {
		return;
	}

	switch(textbox_.mode) {
	case floating_textbox::TEXTBOX_MESSAGE:
	{
		std::string text = textbox_.box->text();
		std::string semiword;
		bool beginning;

		const size_t last_space = text.rfind(" ");

		//if last character is a space return
		if(last_space == text.size() -1) {
			return;
		}

		if(last_space == -1) {
			beginning = true;
			semiword = text;	
		}else{
			beginning = false;
			semiword.assign(text,last_space+1,text.size());
		}

		std::string guess;

		for(size_t n = 0; n != teams_.size(); ++n) {
			if(teams_[n].is_empty()) {
				continue;
			}
			const unit_map::const_iterator leader = team_leader(n+1,units_);
			if(leader != units_.end()) {
				const std::string name = leader->second.description();
				if(name.find(semiword) == 0) {
					if(guess.size() == 0) {
						guess = name;
					}else{
						int i;
						for(i=0; (i < guess.size()) || (i < name.size()); i++) {
							if(guess[i] != name[i]) {
								break;
							}
						}
						guess.assign(guess,0,i);
					}
				}
			}
		}

		if(guess.size() != 0) {
			std::string add = beginning ? ": " : " ";
			text.replace(last_space+1, semiword.size(), guess + add);
			textbox_.box->set_text(text);
		}
		break;
	}
	default:
		lg::err(lg::display) << "unknown textbox mode\n";
	}
}

const unit_map& turn_info::visible_units() const
{
	if(viewing_team().uses_shroud() == false && viewing_team().uses_fog() == false) {
		lg::info(lg::engine) << "all units are visible...\n";
		return units_;
	}

	visible_units_.clear();
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if(gui_.fogged(i->first.x,i->first.y) == false) {
			visible_units_.insert(*i);
		}
	}

	lg::info(lg::engine) << "number of visible units: " << visible_units_.size() << "\n";

	return visible_units_;
}
