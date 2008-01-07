/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file unit_display.cpp
//!

#include "global.hpp"

#include "actions.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "halo.hpp"
#include "image.hpp"
#include "log.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "unit_display.hpp"
#include "util.hpp"

#include <cassert>
#include <climits>

#define LOG_DP LOG_STREAM(info, display)

static void teleport_unit_between( const gamemap::location& a, const gamemap::location& b, unit& temp_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->fogged(a) && disp->fogged(b)) {
		return;
	}
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true);

	if (!disp->fogged(a)) { // teleport
		disp->place_temporary_unit(temp_unit,a);
		unit_animator animator;
		animator.add_animation(&temp_unit,"pre_teleport",a);
		animator.start_animations();
		animator.wait_for_end();
	}
	if (!disp->fogged(b)) { // teleport
		disp->place_temporary_unit(temp_unit,b);
		disp->scroll_to_tiles(b,a,game_display::ONSCREEN,true);
		unit_animator animator;
		animator.add_animation(&temp_unit,"post_teleport",b);
		animator.start_animations();
		animator.wait_for_end();
	}
	temp_unit.set_standing(b);
	disp->update_display();
	events::pump();
}

static void move_unit_between(const gamemap::location& a, const gamemap::location& b, unit& temp_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->fogged(a) && disp->fogged(b)) {
		return;
	}


	disp->place_temporary_unit(temp_unit,a);
	temp_unit.set_facing(a.get_relative_dir(b));
	unit_animator animator;
	animator.replace_anim_if_invalid(&temp_unit,"movement",a);
	animator.start_animations();
	int target_time = animator.get_animation_time();
	target_time += 150;
	target_time -= target_time%150;
	if(  target_time - animator.get_animation_time() < 100 ) target_time +=150;
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN);
	animator.wait_until(target_time);
}

namespace unit_display
{

bool unit_visible_on_path( const std::vector<gamemap::location>& path, const unit& u, const unit_map& units, const std::vector<team>& teams)
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

void move_unit(const std::vector<gamemap::location>& path, unit& u, const std::vector<team>& teams)
{
	game_display* disp = game_display::get_singleton();
	int begin =0;
	int end = 0;
	assert(!path.empty());
	assert(disp);
	// One hex path (strange), nothing to do
	if (path.size()==1) return;

	const unit_map& units = disp->get_units();

	// find the index of first visible tile
	while(begin < path.size() &&  teams[u.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
			u.invisible(path[begin],units,teams)) {
		begin++;
	}
	// find the last visible tile
	end = begin;
	while(end < path.size() &&  !(teams[u.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
			u.invisible(path[end],units,teams))) {
		end++;
	}
	if (begin != path.size()) { //found a visible tile
		if(end == path.size()) end--;
		disp->scroll_to_tiles(path[begin],path[end],game_display::ONSCREEN);
	}

	bool was_hidden = u.get_hidden();
	// Original unit is usually hidden (but still on map, so count is correct)
	unit temp_unit = u;
	u.set_hidden(true);
	temp_unit.set_hidden(false);

	for(size_t i = 0; i+1 < path.size(); ++i) {

		bool invisible = teams[temp_unit.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
				temp_unit.invisible(path[i],units,teams) &&
				temp_unit.invisible(path[i+1],units,teams);

		if(!invisible) {
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

	// Clean the footsteps path, its hexes will be invalidated if needed
	disp->set_route(NULL);

	u.set_hidden(was_hidden);
	disp->invalidate_unit();
}

void unit_die(const gamemap::location& loc, unit& loser,
		const attack_type* attack,const attack_type* secondary_attack, unit* winner)
{
	game_display* disp = game_display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	animator.add_animation(&loser,"death",loc,0,false,false,"",0,unit_animation::KILL,attack,secondary_attack,0);
	animator.add_animation(winner,"victory",loc.get_direction(loser.facing()),0,false,false,"",0,
			unit_animation::KILL,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

}


void unit_attack(
                 const gamemap::location& a, const gamemap::location& b, int damage,
                 const attack_type& attack, const attack_type* secondary_attack,
		  int swing,std::string hit_text)
{
	game_display* disp = game_display::get_singleton();
	if(!disp) return;
	unit_map& units = disp->get_units();
	disp->select_hex(gamemap::location::null_location);
	const bool hide = disp->video().update_locked() || disp->fogged(a) && disp->fogged(b)
	                  || preferences::show_combat() == false;

	if(!hide) {
		disp->scroll_to_tiles(a,b,game_display::ONSCREEN);
	}

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	assert(att != units.end());
	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	assert(def != units.end());
	unit& defender = def->second;

	att->second.set_facing(a.get_relative_dir(b));
	def->second.set_facing(b.get_relative_dir(a));


	unit_animator animator;
	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();

	{
		std::string text ;
		if(damage) text = lexical_cast<std::string>(damage);
		if(!hit_text.empty()) {
			text.insert(text.begin(),hit_text.size()/2,' ');
			text = text + "\n" + hit_text;
		}

		unit_animation::hit_type hit_type;
		if(damage >= defender.hitpoints()) {
			hit_type = unit_animation::KILL;
		} else if(damage > 0) {
			hit_type = unit_animation::HIT;
		}else {
			hit_type = unit_animation::MISS;
		}
		animator.add_animation(&attacker,"attack",att->first,damage,true,false,"",0,hit_type,&attack,secondary_attack,swing);
		animator.add_animation(&defender,"defend",def->first,damage,true,false,text,display::rgb(255,0,0),hit_type,&attack,secondary_attack,swing);

		if(leader_loc.valid()){
			leader = units.find(leader_loc);
			leader->second.set_facing(leader_loc.get_relative_dir(a));
			assert(leader != units.end());
			animator.add_animation(&leader->second,"leading",leader_loc,damage,true,false,"",0,hit_type,&attack,secondary_attack,swing);
		}
	}





	animator.start_animations();
	animator.wait_for_end();

	if(leader_loc.valid()) leader->second.set_standing(leader_loc);
	att->second.set_standing(a);
	def->second.set_standing(b);
}


void unit_recruited(gamemap::location& loc)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() ||disp->fogged(loc)) return;
	unit_map::iterator u = disp->get_units().find(loc);
	if(u == disp->get_units().end()) return;

	u->second.set_hidden(true);
	disp->scroll_to_tile(loc,game_display::ONSCREEN);
	disp->draw();
	u->second.set_hidden(false);
	unit_animator animator;
	animator.add_animation(&u->second,"recruited",loc);
	animator.start_animations();
	animator.wait_for_end();
	u->second.set_standing(loc);
	if (loc==disp->mouseover_hex()) disp->invalidate_unit();
}

void unit_healing(unit& healed,gamemap::location& healed_loc, std::vector<unit_map::iterator> healers, int healing)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->fogged(healed_loc)) return;
	if(healing==0) return;
	// This is all the pretty stuff.
	disp->scroll_to_tile(healed_loc, game_display::ONSCREEN);
	disp->select_hex(healed_loc);
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

	healed.set_standing(healed_loc);
	for(std::vector<unit_map::iterator>::iterator heal_sanim_it = healers.begin(); heal_sanim_it != healers.end(); ++heal_sanim_it) {
		(*heal_sanim_it)->second.set_standing((*heal_sanim_it)->first);
	}

}

} // end unit_display namespace
