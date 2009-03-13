/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file unit_display.cpp */

#include "global.hpp"
#include "unit_display.hpp"

#include "game_preferences.hpp"
#include "game_events.hpp"
#include "log.hpp"
#include "mouse_events.hpp"
#include "terrain_filter.hpp"


#define LOG_DP LOG_STREAM(info, display)

static void teleport_unit_between( const map_location& a, const map_location& b, unit& temp_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || (disp->fogged(a) && disp->fogged(b))) {
		return;
	}
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.0,false);

	if (!disp->fogged(a)) { // teleport
		disp->place_temporary_unit(temp_unit,a);
		temp_unit.set_facing(a.get_relative_dir(b));
		unit_animator animator;
		animator.add_animation(&temp_unit,"pre_teleport",a);
		animator.start_animations();
		animator.wait_for_end();
	}
	if (!disp->fogged(b)) { // teleport
		disp->place_temporary_unit(temp_unit,b);
		temp_unit.set_facing(a.get_relative_dir(b));
		disp->scroll_to_tiles(b,a,game_display::ONSCREEN,true,0.0,false);
		unit_animator animator;
		animator.add_animation(&temp_unit,"post_teleport",b);
		animator.start_animations();
		animator.wait_for_end();
	}
	temp_unit.set_standing(b);
	disp->update_display();
	events::pump();
}

static void move_unit_between(const map_location& a, const map_location& b, unit& temp_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || (disp->fogged(a) && disp->fogged(b))) {
		return;
	}


	disp->place_temporary_unit(temp_unit,a);
	temp_unit.set_facing(a.get_relative_dir(b));
	unit_animator animator;
	animator.replace_anim_if_invalid(&temp_unit,"movement",a);
	animator.start_animations();
        animator.pause_animation();
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.0,false);
        animator.restart_animation();
	new_animation_frame(); //fix bug #13179: Unit's move have sometimes a jumpy start
	int target_time = animator.get_animation_time_potential();
	target_time += 150;
	target_time -= target_time%150;
	if(  target_time - animator.get_animation_time_potential() < 100 ) target_time +=150;
	animator.wait_until(target_time);
	map_location arr[6];
	get_adjacent_tiles(a, arr);
	unsigned int i;
	for (i = 0; i < 6; i++) {
		disp->invalidate(arr[i]);
	}
	get_adjacent_tiles(b, arr);
	for (i = 0; i < 6; i++) {
		disp->invalidate(arr[i]);
	}
}

namespace unit_display
{

bool unit_visible_on_path( const std::vector<map_location>& path, const unit& u, const unit_map& units, const std::vector<team>& teams)
{
	game_display* disp = game_display::get_singleton();
	assert(disp);
	for(size_t i = 0; i+1 < path.size(); ++i) {
		const bool invisible = teams[u.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
	             u.invisible(path[i],units,teams) &&
		         u.invisible(path[i+1],units,teams);
		if(!invisible) {
			return true;
		}
	}

	return false;
}

void move_unit(const std::vector<map_location>& path, unit& u, const std::vector<team>& teams)
{
	game_display* disp = game_display::get_singleton();
	assert(!path.empty());
	assert(disp);
	// One hex path (strange), nothing to do
	if (path.size()==1) return;

	const unit_map& units = disp->get_units();

	bool invisible = teams[u.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
		u.invisible(path[0],units,teams);

	if(!invisible) {
		// Scroll to the path, but only if it fully fits on screen.
		// If it does not fit we might be able to do a better scroll later.
		disp->scroll_to_tiles(path, game_display::ONSCREEN, true, true,0.0,false);
	}

	bool was_hidden = u.get_hidden();
	// Original unit is usually hidden (but still on map, so count is correct)
	unit temp_unit = u;
	u.set_hidden(true);
	temp_unit.set_standing(path[0],false);
	temp_unit.set_hidden(false);
	disp->place_temporary_unit(temp_unit,path[0]);
        disp->draw();
	for(size_t i = 0; i+1 < path.size(); ++i) {

		invisible = teams[temp_unit.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
				temp_unit.invisible(path[i],units,teams) &&
				temp_unit.invisible(path[i+1],units,teams);

		if(!invisible) {
			if (!disp->tile_on_screen(path[i]) || !disp->tile_on_screen(path[i+1])) {
				// prevent the unit from dissappearing if we scroll here with i == 0
				disp->place_temporary_unit(temp_unit,path[i]);
				// scroll in as much of the remaining path as possible
				std::vector<map_location> remaining_path;
				for(size_t j = i; j < path.size(); j++) {
					remaining_path.push_back(path[j]);
				}
				disp->scroll_to_tiles(remaining_path, game_display::ONSCREEN, true,false,0.0,false);
			}

			if( !tiles_adjacent(path[i], path[i+1])) {
				teleport_unit_between(path[i],path[i+1],temp_unit);
			} else {
				move_unit_between(path[i],path[i+1],temp_unit);
			}
		}
	}
	disp->remove_temporary_unit();
	u.set_facing(path[path.size()-2].get_relative_dir(path[path.size()-1]));
	u.set_standing(path[path.size()-1]);

	u.set_hidden(was_hidden);
	disp->invalidate_unit_after_move(path[0], path[path.size()-1]);

	events::mouse_handler* mousehandler = events::mouse_handler::get_singleton();
	if (mousehandler) {
		mousehandler->invalidate_reachmap();
	}
}

void unit_die(const map_location& loc, unit& loser,
		const attack_type* attack,const attack_type* secondary_attack, unit* winner)
{
	game_display* disp = game_display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	// hide the hp/xp bars of the loser (useless and prevent bars around an erased unit)
	animator.add_animation(&loser,"death",loc,0,false,false,"",0,unit_animation::KILL,attack,secondary_attack,0);
	// but show the bars of the winner (avoid blinking and show its xp gain)
	animator.add_animation(winner,"victory",loc.get_direction(loser.facing()),0,true,false,"",0,
			unit_animation::KILL,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

	events::mouse_handler* mousehandler = events::mouse_handler::get_singleton();
	if (mousehandler) {
		mousehandler->invalidate_reachmap();
	}
}


void unit_attack(
                 const map_location& a, const map_location& b, int damage,
                 const attack_type& attack, const attack_type* secondary_attack,
		  int swing,std::string hit_text,bool drain,std::string att_text)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || preferences::show_combat() == false) return;
	unit_map& units = disp->get_units();
	disp->select_hex(map_location::null_location);
	const bool hide = disp->video().update_locked() || (disp->fogged(a) && disp->fogged(b));

	if(!hide) {
		// scroll such that there is at least half a hex spacing around fighters
		disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.5,false);
	}

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	assert(att != units.end());
	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	assert(def != units.end());
	// do a copy so we can change the caracteristics
	unit defender = def->second;
	bool was_hidden = defender.get_hidden();
	def->second.set_hidden(true);
	disp->place_temporary_unit(defender,def->first);


	att->second.set_facing(a.get_relative_dir(b));
	def->second.set_facing(b.get_relative_dir(a));
	defender.set_facing(b.get_relative_dir(a));


	unit_animator animator;
	unit_ability_list leaders = attacker.get_abilities("leadership",a);
	unit_ability_list helpers = defender.get_abilities("resistance",b);

	{
		std::string text ;
		if(damage) text = lexical_cast<std::string>(damage);
		if(!hit_text.empty()) {
			text.insert(text.begin(),hit_text.size()/2,' ');
			text = text + "\n" + hit_text;
		}

		std::string text_2 ;
		if(drain && damage) text_2 = lexical_cast<std::string>(std::min<int>(damage,defender.hitpoints())/2);
		if(!att_text.empty()) {
			text_2.insert(text_2.begin(),att_text.size()/2,' ');
			text_2 = text_2 + "\n" + att_text;
		}

		unit_animation::hit_type hit_type;
		if(damage >= defender.hitpoints()) {
			hit_type = unit_animation::KILL;
		} else if(damage > 0) {
			hit_type = unit_animation::HIT;
		}else {
			hit_type = unit_animation::MISS;
		}
		animator.add_animation(&attacker,"attack",att->first,damage,true,false,text_2,display::rgb(0,255,0),hit_type,&attack,secondary_attack,swing);
		animator.add_animation(&defender,"defend",def->first,damage,true,false,text  ,display::rgb(255,0,0),hit_type,&attack,secondary_attack,swing);

		for(std::vector<std::pair<config*,map_location> >::iterator itor = leaders.cfgs.begin(); itor != leaders.cfgs.end(); itor++) {
			if(itor->second == a) continue;
			if(itor->second == b) continue;
			unit_map::iterator leader = units.find(itor->second);
			assert(leader != units.end());
			leader->second.set_facing(itor->second.get_relative_dir(a));
			animator.add_animation(&leader->second,"leading",itor->second,damage,true,false,"",0,hit_type,&attack,secondary_attack,swing);
		}
		for(std::vector<std::pair<config*,map_location> >::iterator itor = helpers.cfgs.begin(); itor != helpers.cfgs.end(); itor++) {
			if(itor->second == a) continue;
			if(itor->second == b) continue;
			unit_map::iterator helper = units.find(itor->second);
			assert(helper != units.end());
			helper->second.set_facing(itor->second.get_relative_dir(b));
			animator.add_animation(&helper->second,"resistance",itor->second,damage,true,false,"",0,hit_type,&attack,secondary_attack,swing);
		}

	}

	animator.start_animations();
	animator.wait_until(0);
	int damage_left = damage;
	while(damage_left > 0 && !animator.would_end()) {
		int step_left = (animator.get_end_time() - animator.get_animation_time() )/50;
		if(step_left < 1) step_left = 1;
		int removed_hp =  damage_left/step_left ;
		if(removed_hp < 1) removed_hp = 1;
		defender.take_hit(removed_hp);
		damage_left -= removed_hp;
		animator.wait_until(animator.get_animation_time_potential() +50);
	}
	animator.wait_for_end();
	animator.set_all_standing();
	disp->remove_temporary_unit();
	def->second.set_hidden(was_hidden);
	def->second.set_standing(def->first);

}


void unit_recruited(map_location& loc)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() ||disp->fogged(loc)) return;
	unit_map::iterator u = disp->get_units().find(loc);
	if(u == disp->get_units().end()) return;

	u->second.set_hidden(true);
	disp->scroll_to_tile(loc,game_display::ONSCREEN,true,false);
	disp->draw();
	u->second.set_hidden(false);
	u->second.set_facing(static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS));
	unit_animator animator;
	animator.add_animation(&u->second,"recruited",loc);
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
	if (loc==disp->mouseover_hex()) disp->invalidate_unit();
}

void unit_healing(unit& healed,map_location& healed_loc, std::vector<unit_map::iterator> healers, int healing)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->fogged(healed_loc)) return;
	if(healing==0) return;
	// This is all the pretty stuff.
	disp->scroll_to_tile(healed_loc, game_display::ONSCREEN,true,false);
	disp->display_unit_hex(healed_loc);
	unit_animator animator;

	for(std::vector<unit_map::iterator>::iterator heal_anim_it = healers.begin(); heal_anim_it != healers.end(); ++heal_anim_it) {
		(*heal_anim_it)->second.set_facing((*heal_anim_it)->first.get_relative_dir(healed_loc));
		animator.add_animation(&(*heal_anim_it)->second,"healing",(*heal_anim_it)->first,healing);
	}
	if (healing < 0) {
		animator.add_animation(&healed,"poisoned",healed_loc,-healing,false,false,lexical_cast<std::string>(-healing), display::rgb(255,0,0));
	} else {
		animator.add_animation(&healed,"healed",healed_loc,healing,false,false,lexical_cast<std::string>(healing), display::rgb(0,255,0));
	}
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();

}

void wml_animation_internal(unit_animator & animator,const vconfig &cfg, const gamemap& map, const gamestatus& game_status,unit_map & units,const map_location& default_location = map_location::null_location);
void wml_animation(const vconfig &cfg,unit_map & units, const gamemap& map, const gamestatus& game_status,const map_location& default_location)
{
		unit_animator animator;
		wml_animation_internal(animator,cfg,map,game_status,units,default_location);
		animator.start_animations();
		animator.wait_for_end();
		animator.set_all_standing();
}

void wml_animation_internal(unit_animator & animator,const vconfig &cfg, const gamemap& map, const gamestatus& game_status,unit_map & units,const map_location& default_location)
{
	unit_map::iterator u = units.find(default_location);

	// Search for a valid unit filter,
	// and if we have one, look for the matching unit
	vconfig filter = cfg.child("filter");
	if(!filter.null()) {
		for(u = units.begin(); u != units.end(); ++u){
			if(game_events::unit_matches_filter(u, filter))
				break;
		}
	}

	// We have found a unit that matches the filter
	game_display* disp = game_display::get_singleton();
	if(u != units.end() && ! disp->fogged(u->first)) {
		attack_type *primary = NULL;
		attack_type *secondary = NULL;
		Uint32 text_color;
		unit_animation::hit_type hits=  unit_animation::INVALID;
		std::vector<attack_type> attacks = u->second.attacks();
		std::vector<attack_type>::iterator itor;

		filter = cfg.child("primary_attack");
		if(!filter.null()) {
			for(itor = attacks.begin(); itor != attacks.end(); ++itor){
				if(itor->matches_filter(filter.get_parsed_config())) {
					primary = &*itor;
					break;
				}
			}
		}

		filter = cfg.child("secondary_attack");
		if(!filter.null()) {
			for(itor = attacks.begin(); itor != attacks.end(); ++itor){
				if(itor->matches_filter(filter.get_parsed_config())) {
					secondary = &*itor;
					break;
				}
			}
		}

		if(cfg["hits"] == "yes" || cfg["hits"] == "hit") {
			hits = unit_animation::HIT;
		}
		if(cfg["hits"] == "no" || cfg["hits"] == "miss") {
			hits = unit_animation::MISS;
		}
		if( cfg["hits"] == "kill" ) {
			hits = unit_animation::KILL;
		}
		if(cfg["red"].empty() && cfg["green"].empty() && cfg["blue"].empty()) {
			text_color = display::rgb(0xff,0xff,0xff);
		} else {
			text_color = display::rgb(atoi(cfg["red"].c_str()),atoi(cfg["green"].c_str()),atoi(cfg["blue"].c_str()));
		}
		disp->scroll_to_tile(u->first, game_display::ONSCREEN,true,false);
		vconfig t_filter = cfg.child("facing");
		if(!t_filter.empty()) {
			terrain_filter filter(t_filter,map,game_status,units);
			std::set<map_location> locs;
			filter.get_locations(locs);
			if(!locs.empty()) {
				u->second.set_facing(u->first.get_relative_dir(*locs.begin()));
			}
		}
		animator.add_animation(&u->second,cfg["flag"],u->first,lexical_cast_default<int>(cfg["value"]),utils::string_bool(cfg["with_bars"]),
				false,cfg["text"],text_color, hits,primary,secondary,0);
	}
	const vconfig::child_list sub_anims = cfg.get_children("animate");
	vconfig::child_list::const_iterator anim_itor;
	for(anim_itor = sub_anims.begin(); anim_itor != sub_anims.end();anim_itor++) {
		wml_animation_internal(animator,*anim_itor,map,game_status,units);
	}

}
} // end unit_display namespace
