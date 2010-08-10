/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "global.hpp"
#include "unit_display.hpp"

#include "foreach.hpp"
#include "game_preferences.hpp"
#include "game_events.hpp"
#include "log.hpp"
#include "mouse_events.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"


#define LOG_DP LOG_STREAM(info, display)

static void teleport_unit_between( const map_location& a, const map_location& b, unit& temp_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->video().faked() || (disp->fogged(a) && disp->fogged(b))) {
		return;
	}
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.0,false);

	temp_unit.set_location(a);
	if (!disp->fogged(a)) { // teleport
		disp->invalidate(temp_unit.get_location());
		temp_unit.set_facing(a.get_relative_dir(b));
		unit_animator animator;
		animator.add_animation(&temp_unit,"pre_teleport",a);
		animator.start_animations();
		animator.wait_for_end();
	}

	temp_unit.set_location(b);
	if (!disp->fogged(b)) { // teleport
		disp->invalidate(temp_unit.get_location());
		temp_unit.set_facing(a.get_relative_dir(b));
		disp->scroll_to_tiles(b,a,game_display::ONSCREEN,true,0.0,false);
		unit_animator animator;
		animator.add_animation(&temp_unit,"post_teleport",b);
		animator.start_animations();
		animator.wait_for_end();
	}

	temp_unit.set_standing();
	disp->update_display();
	events::pump();
}

static void move_unit_between(const map_location& a, const map_location& b, unit& temp_unit,unsigned int step_num,unsigned int step_left)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->video().faked() || (disp->fogged(a) && disp->fogged(b))) {
		return;
	}

	temp_unit.set_location(a);
	disp->invalidate(temp_unit.get_location());
	temp_unit.set_facing(a.get_relative_dir(b));
	unit_animator animator;
	animator.replace_anim_if_invalid(&temp_unit,"movement",a,b,step_num,
			false,false,"",0,unit_animation::INVALID,NULL,NULL,step_left);
	animator.start_animations();
        animator.pause_animation();
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.0,false);
        animator.restart_animation();

	// useless now, previous short draw() just did one
	// new_animation_frame();

	int target_time = animator.get_animation_time_potential();

		// target_time must be short to avoid jumpy move
		// std::cout << "target time: " << target_time << "\n";
	// we round it to the next multile of 200
	target_time += 200;
	target_time -= target_time%200;

	// This code causes backwards teleport because the time > 200 causes offset > 1.0
	// which will not match with the following -1.0
	// if(  target_time - animator.get_animation_time_potential() < 100 ) target_time +=200;

	animator.wait_until(target_time);
		// debug code, see unit_frame::redraw()
		// std::cout << "   end\n";
	map_location arr[6];
	get_adjacent_tiles(a, arr);
	unsigned int i;
	for (i = 0; i < 6; ++i) {
		disp->invalidate(arr[i]);
	}
	get_adjacent_tiles(b, arr);
	for (i = 0; i < 6; ++i) {
		disp->invalidate(arr[i]);
	}
}

namespace unit_display
{

void move_unit(const std::vector<map_location>& path, unit& u,
		const std::vector<team>& teams, bool animate,
		map_location::DIRECTION dir)
{
	game_display* disp = game_display::get_singleton();
	assert(!path.empty());
	assert(disp);
	if(!disp || disp->video().update_locked() || disp->video().faked())
		return;
	// One hex path (strange), nothing to do
	if(path.size() == 1)
		return;
	if(dir == map_location::NDIRECTIONS)
		dir = path[path.size()-2].get_relative_dir(path.back());
	// Don't animate, only set facing and redraw path ends
	if(!animate) {
		u.set_facing(dir);
		disp->invalidate(path.front());
		disp->invalidate(path.back());
		return;
	}

	bool invisible = teams[u.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
		u.invisible(path[0]);

	bool was_hidden = u.get_hidden();
	// Original unit is usually hidden (but still on map, so count is correct)
	unit temp_unit = u;
	u.set_hidden(true);
	temp_unit.set_standing(false);
	temp_unit.set_hidden(false);
	disp->place_temporary_unit(&temp_unit);
	if(!invisible) {
		// Scroll to the path, but only if it fully fits on screen.
		// If it does not fit we might be able to do a better scroll later.
		disp->scroll_to_tiles(path, game_display::ONSCREEN, true, true,0.0,false);
	}
	// We need to clear big invalidation before the move and have a smooth animation
	// (mainly black stripes and invalidation after canceling atatck dialog)
	// Two draw calls are needed to also redraw the previously invalidated hexes
	// We use update=false because we don't need delay here (no time wasted)
	// and no screen refresh (will be done by last 3rd draw() and it optimizes
	// the double blitting done by these invalidations)
	disp->draw(false);
	disp->draw(false);

	// The last draw() was still slow, and its inital new_animation_frame() call
	// is now old, so we do another draw() to get a fresh one
	// TODO: replace that by a new_animation_frame() before starting anims
	//       don't forget to change the previous draw(false) to true
	disp->draw(true);

	// extra immobile mvt anim for take-off
	temp_unit.set_location(path[0]);
	disp->invalidate(temp_unit.get_location());
	temp_unit.set_facing(path[0].get_relative_dir(path[1]));
	unit_animator animator;
	animator.add_animation(&temp_unit,"pre_movement",path[0],path[1]);
	animator.start_animations();
	animator.wait_for_end();

	for(size_t i = 0; i+1 < path.size(); ++i) {

		invisible = teams[temp_unit.side()-1].is_enemy(int(disp->viewing_team()+1)) &&
				temp_unit.invisible(path[i]) &&
				temp_unit.invisible(path[i+1]);

		if(!invisible) {
			if (!disp->tile_fully_on_screen(path[i]) || !disp->tile_fully_on_screen(path[i+1])) {
				// prevent the unit from dissappearing if we scroll here with i == 0
				temp_unit.set_location(path[i]);
				disp->invalidate(temp_unit.get_location());
				// scroll in as much of the remaining path as possible
				std::vector<map_location> remaining_path;
				for(size_t j = i; j < path.size(); ++j) {
					remaining_path.push_back(path[j]);
				}
				temp_unit.get_animation()->pause_animation();
				disp->scroll_to_tiles(remaining_path,
							game_display::ONSCREEN, true,false,0.0,false);
				temp_unit.get_animation()->restart_animation();
			}

			if(tiles_adjacent(path[i], path[i+1])) {
				move_unit_between(path[i],path[i+1],temp_unit,i,path.size()-2-i);
			} else if (path[i] != path[i+1]) {
				teleport_unit_between(path[i],path[i+1],temp_unit);
			} else {
				continue; // no move needed
			}
		}
	}
	temp_unit.set_location(path[path.size() - 1]);
	temp_unit.set_facing(path[path.size()-2].get_relative_dir(path[path.size()-1]));
	animator.clear();
	animator.add_animation(&temp_unit,"post_movement",path[path.size()-1],map_location::null_location);
	animator.start_animations();
	animator.wait_for_end();
	disp->remove_temporary_unit(&temp_unit);

	u.set_facing(dir);
	u.set_hidden(was_hidden);

	events::mouse_handler* mousehandler = events::mouse_handler::get_singleton();
	if (mousehandler) {
		mousehandler->invalidate_reachmap();
	}

	disp->invalidate(path.front());
	disp->invalidate(path.back());
}

void reset_helpers(const unit *attacker,const unit *defender);

void unit_draw_weapon(const map_location& loc, unit& attacker,
		const attack_type* attack,const attack_type* secondary_attack, const map_location& defender_loc,unit* defender)
{
	game_display* disp = game_display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	animator.add_animation(&attacker,"draw_weapon",loc,defender_loc,0,false,false,"",0,unit_animation::HIT,attack,secondary_attack,0);
	animator.add_animation(defender,"draw_weapon",defender_loc,loc,0,false,false,"",0,unit_animation::MISS,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

}


void unit_sheath_weapon(const map_location& primary_loc, unit* primary_unit,
		const attack_type* primary_attack,const attack_type* secondary_attack, const map_location& secondary_loc,unit* secondary_unit)
{
	game_display* disp = game_display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(primary_loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	if(primary_unit) {
		animator.add_animation(primary_unit,"sheath_weapon",primary_loc,secondary_loc,0,false,false,"",0,unit_animation::INVALID,primary_attack,secondary_attack,0);
	}
	if(secondary_unit) {
		animator.add_animation(secondary_unit,"sheath_weapon",secondary_loc,primary_loc,0,false,false,"",0,unit_animation::INVALID,secondary_attack,primary_attack,0);
	}

	if(primary_unit || secondary_unit) {
		animator.start_animations();
		animator.wait_for_end();
	}
	if(primary_unit) {
		primary_unit->set_standing();
	}
	if(secondary_unit) {
		secondary_unit->set_standing();
	}
	reset_helpers(primary_unit,secondary_unit);

}


void unit_die(const map_location& loc, unit& loser,
		const attack_type* attack,const attack_type* secondary_attack, const map_location& winner_loc,unit* winner)
{
	game_display* disp = game_display::get_singleton();
	if(!disp ||disp->video().update_locked() || disp->video().faked() || disp->fogged(loc) || preferences::show_combat() == false) {
		return;
	}
	unit_animator animator;
	// hide the hp/xp bars of the loser (useless and prevent bars around an erased unit)
	animator.add_animation(&loser,"death",loc,winner_loc,0,false,false,"",0,unit_animation::KILL,attack,secondary_attack,0);
	// but show the bars of the winner (avoid blinking and show its xp gain)
	animator.add_animation(winner,"victory",winner_loc,loc,0,true,false,"",0,
			unit_animation::KILL,secondary_attack,attack,0);
	animator.start_animations();
	animator.wait_for_end();

	reset_helpers(winner,&loser);
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
	if(!disp ||disp->video().update_locked() || disp->video().faked() ||
			(disp->fogged(a) && disp->fogged(b)) || preferences::show_combat() == false) {
		return;
	}
	unit_map& units = disp->get_units();
	disp->select_hex(map_location::null_location);

	// scroll such that there is at least half a hex spacing around fighters
	disp->scroll_to_tiles(a,b,game_display::ONSCREEN,true,0.5,false);

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	assert(att != units.end());
	unit& attacker = *att;

	const unit_map::iterator def = units.find(b);
	assert(def != units.end());
	unit &defender = *def;
	int def_hitpoints = defender.hitpoints();

	att->set_facing(a.get_relative_dir(b));
	def->set_facing(b.get_relative_dir(a));
	defender.set_facing(b.get_relative_dir(a));


	unit_animator animator;
	unit_ability_list leaders = attacker.get_abilities("leadership");
	unit_ability_list helpers = defender.get_abilities("resistance");

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
	animator.add_animation(&attacker, "attack", att->get_location(),
		def->get_location(), damage, true, false, text_2,
		display::rgb(0, 255, 0), hit_type, &attack, secondary_attack,
		swing);

	// note that we take an anim from the real unit, we'll use it later
	const unit_animation *defender_anim = def->choose_animation(*disp,
		def->get_location(), "defend", att->get_location(), damage,
		hit_type, &attack, secondary_attack, swing);
	animator.add_animation(&defender, defender_anim, def->get_location(),
		true, false, text , display::rgb(255, 0, 0));

	for (std::vector<std::pair<const config *, map_location> >::iterator itor = leaders.cfgs.begin(); itor != leaders.cfgs.end(); ++itor) {
		if(itor->second == a) continue;
		if(itor->second == b) continue;
		unit_map::iterator leader = units.find(itor->second);
		assert(leader != units.end());
		leader->set_facing(itor->second.get_relative_dir(a));
		animator.add_animation(&*leader, "leading", itor->second,
			att->get_location(), damage, true, false, "", 0,
			hit_type, &attack, secondary_attack, swing);
	}
	for (std::vector<std::pair<const config *, map_location> >::iterator itor = helpers.cfgs.begin(); itor != helpers.cfgs.end(); ++itor) {
		if(itor->second == a) continue;
		if(itor->second == b) continue;
		unit_map::iterator helper = units.find(itor->second);
		assert(helper != units.end());
		helper->set_facing(itor->second.get_relative_dir(b));
		animator.add_animation(&*helper, "resistance", itor->second,
			def->get_location(), damage, true, false, "", 0,
			hit_type, &attack, secondary_attack, swing);
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
	// pass the animation back to the real unit
	def->start_animation(animator.get_end_time(), defender_anim, true);
	reset_helpers(&*att, &*def);
	def->set_hitpoints(def_hitpoints);
}

// private helper function, set all helpers to default position
void reset_helpers(const unit *attacker,const unit *defender)
{
	game_display* disp = game_display::get_singleton();
	unit_map& units = disp->get_units();
	if(attacker) {
		unit_ability_list leaders = attacker->get_abilities("leadership");
		for (std::vector<std::pair<const config *, map_location> >::iterator itor = leaders.cfgs.begin(); itor != leaders.cfgs.end(); ++itor) {
			unit_map::iterator leader = units.find(itor->second);
			assert(leader != units.end());
			leader->set_standing();
		}
	}

	if(defender) {
		unit_ability_list helpers = defender->get_abilities("resistance");
		for (std::vector<std::pair<const config *, map_location> >::iterator itor = helpers.cfgs.begin(); itor != helpers.cfgs.end(); ++itor) {
			unit_map::iterator helper = units.find(itor->second);
			assert(helper != units.end());
			helper->set_standing();
		}
	}
}

void unit_recruited(const map_location& loc,const map_location& leader_loc)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->video().faked() ||disp->fogged(loc)) return;
	unit_map::iterator u = disp->get_units().find(loc);
	if(u == disp->get_units().end()) return;
	u->set_hidden(true);

	unit_animator animator;
	if(leader_loc != map_location::null_location) {
		unit_map::iterator leader = disp->get_units().find(leader_loc);
		if(leader == disp->get_units().end()) return;
		disp->scroll_to_tiles(loc,leader_loc,game_display::ONSCREEN,true,0.0,false);
		leader->set_facing(leader_loc.get_relative_dir(loc));
		animator.add_animation(&*leader, "recruiting", leader_loc, loc, 0, true);
	} else {
		disp->scroll_to_tile(loc,game_display::ONSCREEN,true,false);
	}

	disp->draw();
	u->set_hidden(false);
	u->set_facing(static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS));
	animator.add_animation(&*u, "recruited", loc, leader_loc);
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
	if (loc==disp->mouseover_hex()) disp->invalidate_unit();
}

void unit_healing(unit &healed, const map_location &healed_loc,
	const std::vector<unit *> &healers, int healing)
{
	game_display* disp = game_display::get_singleton();
	if(!disp || disp->video().update_locked() || disp->video().faked() || disp->fogged(healed_loc)) return;
	if(healing==0) return;
	// This is all the pretty stuff.
	disp->scroll_to_tile(healed_loc, game_display::ONSCREEN,true,false);
	disp->display_unit_hex(healed_loc);
	unit_animator animator;

	foreach (unit *h, healers) {
		h->set_facing(h->get_location().get_relative_dir(healed_loc));
		animator.add_animation(h, "healing", h->get_location(),
			healed_loc, healing);
	}
	if (healing < 0) {
		animator.add_animation(&healed,"poisoned",healed_loc,map_location::null_location,-healing,false,false,lexical_cast<std::string>(-healing), display::rgb(255,0,0));
	} else {
		animator.add_animation(&healed,"healed",healed_loc,map_location::null_location,healing,false,false,lexical_cast<std::string>(healing), display::rgb(0,255,0));
	}
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();

}

void wml_animation_internal(unit_animator &animator, const vconfig &cfg, const map_location &default_location = map_location::null_location);

void wml_animation(const vconfig &cfg, const map_location &default_location)
{
	game_display &disp = *resources::screen;
	if (disp.video().update_locked() || disp.video().faked()) return;
	unit_animator animator;
	wml_animation_internal(animator, cfg, default_location);
	animator.start_animations();
	animator.wait_for_end();
	animator.set_all_standing();
}

void wml_animation_internal(unit_animator &animator, const vconfig &cfg, const map_location &default_location)
{
	unit_map::iterator u = resources::units->find(default_location);

	// Search for a valid unit filter,
	// and if we have one, look for the matching unit
	vconfig filter = cfg.child("filter");
	if(!filter.null()) {
		for (u = resources::units->begin(); u != resources::units->end(); ++u) {
			if (game_events::unit_matches_filter(*u, filter))
				break;
		}
	}

	// We have found a unit that matches the filter
	if (u.valid() && !resources::screen->fogged(u->get_location()))
	{
		attack_type *primary = NULL;
		attack_type *secondary = NULL;
		Uint32 text_color;
		unit_animation::hit_type hits=  unit_animation::INVALID;
		std::vector<attack_type> attacks = u->attacks();
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
		resources::screen->scroll_to_tile(u->get_location(), game_display::ONSCREEN, true, false);
		vconfig t_filter = cfg.child("facing");
		map_location secondary_loc = map_location::null_location;
		if(!t_filter.empty()) {
			terrain_filter filter(t_filter, *resources::units);
			std::set<map_location> locs;
			filter.get_locations(locs);
			if (!locs.empty() && u->get_location() != *locs.begin()) {
				map_location::DIRECTION dir =u->get_location().get_relative_dir(*locs.begin());
				u->set_facing(dir);
				secondary_loc = u->get_location().get_direction(dir);
			}
		}
		animator.add_animation(&*u, cfg["flag"], u->get_location(),
			secondary_loc, lexical_cast_default<int>(cfg["value"]),
			utils::string_bool(cfg["with_bars"]), false,
			cfg["text"], text_color, hits, primary, secondary,
			lexical_cast_default<int>(cfg["value_second"]));
	}
	const vconfig::child_list sub_anims = cfg.get_children("animate");
	vconfig::child_list::const_iterator anim_itor;
	for(anim_itor = sub_anims.begin(); anim_itor != sub_anims.end();++anim_itor) {
		wml_animation_internal(animator, *anim_itor);
	}

}
} // end unit_display namespace
