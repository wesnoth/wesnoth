/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "actions.hpp"
#include "display.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "halo.hpp"
#include "image.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "wassert.hpp"

#define LOG_DP LOG_STREAM(info, display)

namespace
{

void teleport_unit_between(display& disp, const gamemap::location& a, const gamemap::location& b, unit& temp_unit)
{
	if(disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)) {
		return;
	}

	temp_unit.set_teleporting(disp,a);
	if (!disp.fogged(a.x, a.y)) { // teleport
		disp.scroll_to_tile(a.x,a.y,display::ONSCREEN);
		while(!temp_unit.get_animation()->animation_finished()  && temp_unit.get_animation()->get_animation_time() < 0) {
			disp.invalidate(a);
			disp.place_temporary_unit(temp_unit, a);
			disp.draw();
			events::pump();
			disp.delay(10);
		}
	}
	if (!disp.fogged(b.x, b.y)) { // teleport
		temp_unit.restart_animation(disp,0);
		disp.scroll_to_tile(b.x,b.y,display::ONSCREEN);
		while(!temp_unit.get_animation()->animation_finished()) {
			disp.invalidate(b);
			disp.place_temporary_unit(temp_unit, b);
			disp.draw();
			events::pump();
			disp.delay(10);
		}
	}
	temp_unit.set_standing(disp,b);
	disp.update_display();
	events::pump();
}

void move_unit_between(display& disp, const gamemap& map, const gamemap::location& a, const gamemap::location& b, unit& temp_unit)
{
	if(disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)) {
		return;
	}

	const gamemap::TERRAIN dst_terrain = map.get_terrain(b);

	const int acceleration = disp.turbo() ? 5:1;

	gamemap::location src_adjacent[6];
	get_adjacent_tiles(a, src_adjacent);

	gamemap::location dst_adjacent[6];
	get_adjacent_tiles(b, dst_adjacent);

	const int total_mvt_time = 150 * temp_unit.movement_cost(dst_terrain)/acceleration;
	const unsigned int start_time = SDL_GetTicks();
	int mvt_time = 1;
	disp.scroll_to_tiles(a.x,a.y,b.x,b.y,display::ONSCREEN);

	while(mvt_time < total_mvt_time-1) {
		disp.delay(10);
		mvt_time = SDL_GetTicks() -start_time;
		if(mvt_time >=total_mvt_time) mvt_time = total_mvt_time -1;
		double pos =double(mvt_time)/total_mvt_time;
		const gamemap::location& ref_loc =pos<0.5?a:b;
		if(pos >= 0.5) pos = pos -1;
		temp_unit.set_walking(disp,ref_loc);
		temp_unit.set_offset(pos);
		disp.place_temporary_unit(temp_unit,ref_loc);
		disp.invalidate(ref_loc);
		disp.draw();
		events::pump();

	}
}

}

namespace unit_display
{

bool unit_visible_on_path(display& disp, const std::vector<gamemap::location>& path, const unit& u, const unit_map& units, const std::vector<team>& teams)
{
	for(size_t i = 0; i+1 < path.size(); ++i) {
		const bool invisible = teams[u.side()-1].is_enemy(int(disp.viewing_team()+1)) &&
	             u.invisible(path[i],units,teams) &&
		         u.invisible(path[i+1],units,teams);
		if(!invisible) {
			return true;
		}
	}

	return false;
}

void move_unit(display& disp, const gamemap& map, const std::vector<gamemap::location>& path, unit& u, const unit_map& units, const std::vector<team>& teams)
{
	wassert(!path.empty());

	bool previous_visible = false;
	bool was_hidden = u.get_hidden();
	// Original unit is usually hidden (but still on map, so count is correct)
	unit temp_unit = u;
	u.set_hidden(true);
	temp_unit.set_hidden(false);

	for(size_t i = 0; i+1 < path.size(); ++i) {
		temp_unit.set_facing(path[i].get_relative_dir(path[i+1]));

		disp.remove_footstep(path[i]);

		bool invisible;

		if(temp_unit.side() == 0) {
			invisible = false;
		} else {
			invisible = teams[temp_unit.side()-1].is_enemy(int(disp.viewing_team()+1)) &&
				temp_unit.invisible(path[i],units,teams) &&
				temp_unit.invisible(path[i+1],units,teams);
		}

		if(!invisible) {
			if( !tiles_adjacent(path[i], path[i+1])) {
				teleport_unit_between(disp,path[i],path[i+1],temp_unit);
			} else {
				move_unit_between(disp,map,path[i],path[i+1],temp_unit);
			}
			previous_visible = true;
		} else if(previous_visible) {
			disp.invalidate(path[i]);
			disp.draw();
		}
	}
	disp.remove_temporary_unit();
	u.set_facing(path[path.size()-2].get_relative_dir(path[path.size()-1]));

	//make sure the entire path is cleaned properly
	for(std::vector<gamemap::location>::const_iterator it = path.begin(); it != path.end(); ++it) {
		disp.invalidate(*it);
	}
	u.set_hidden(was_hidden);
}

void unit_die(display& disp,const gamemap::location& loc, unit& u, const attack_type* attack)
{
	if(disp.update_locked() || disp.fogged(loc.x,loc.y) || preferences::show_combat() == false) {
		return;
	}

	const std::string& die_sound = u.die_sound();
	if(die_sound != "" && die_sound != "null") {
		sound::play_sound(die_sound);
	}
	u.set_dying(disp,loc,attack);


	while(!u.get_animation()->animation_finished()) {

		disp.invalidate(loc);
		disp.draw();
		events::pump();
		disp.delay(10);
	}
	u.set_standing(disp,loc);
	disp.update_display();
	events::pump();
}

namespace {

bool unit_attack_ranged(display& disp, unit_map& units,
                        const gamemap::location& a, const gamemap::location& b,
			int damage, const attack_type& attack, bool update_display)

{
	const bool hide = disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)
		|| preferences::show_combat() == false || (!update_display);

	log_scope("unit_attack_range");

	const unit_map::iterator att = units.find(a);
	wassert(att != units.end());
	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	wassert(def != units.end());
	unit& defender = def->second;

	const bool hits = damage > 0;
	const int acceleration = disp.turbo() ? 5 : 1;


	// more damage shown for longer, but 1s at most for this factor
	const double xsrc = disp.get_location_x(a);
	const double ysrc = disp.get_location_y(a);
	const double xdst = disp.get_location_x(b);
	const double ydst = disp.get_location_y(b);

	gamemap::location update_tiles[6];
	get_adjacent_tiles(b,update_tiles);

	bool dead = false;



	// start leader and attacker animation, wait for attacker animation to end
	unit_animation missile_animation = attacker.set_attacking(disp,a,hits,attack);
	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()){
		LOG_DP << "found leader at " << leader_loc << '\n';
		leader = units.find(leader_loc);
		wassert(leader != units.end());
		leader->second.set_facing(leader_loc.get_relative_dir(a));
		leader->second.set_leading(disp,leader_loc);
	}
	int animation_time;


	halo::ORIENTATION orientation;
	const gamemap::location::DIRECTION attack_ori = a.get_relative_dir(b);
	switch(attack_ori)
	{
		case gamemap::location::NORTH:
		case gamemap::location::NORTH_EAST:
			orientation = halo::NORMAL;
			break;
		case gamemap::location::SOUTH_EAST:
		case gamemap::location::SOUTH:
			orientation = halo::VREVERSE;
			break;
		case gamemap::location::SOUTH_WEST:
			orientation = halo::HVREVERSE;
			break;
		case gamemap::location::NORTH_WEST:
			orientation = halo::HREVERSE;
			break;
		case gamemap::location::NDIRECTIONS:
		default:
			orientation = halo::NORMAL;
			break;
	}
	const unit_animation::FRAME_DIRECTION dir = (a.x == b.x) ? unit_animation::VERTICAL:unit_animation::DIAGONAL;

	defender.set_defending(disp,b,damage,&attack);
	// min of attacker, defender, missile and -200
	const int start_time = minimum<int>(
			minimum<int>(
				minimum<int>(defender.get_animation()->get_first_frame_time(),
				missile_animation.get_first_frame_time()),
				attacker.get_animation()->get_first_frame_time()),
			-200);
	missile_animation.start_animation(start_time,1,acceleration);
	defender.restart_animation(disp,start_time);
	attacker.restart_animation(disp,start_time);
	animation_time = defender.get_animation()->get_animation_time();
	bool sound_played = false ;
	int missile_frame_halo =0;
	int missile_halo =0;
	while(!hide && (
		attacker.state() != unit::STATE_STANDING ||
		defender.state() != unit::STATE_STANDING ||
		!missile_animation.animation_finished()  ||
		(leader_loc.valid() && leader->second.state() != unit::STATE_STANDING))
		){
		const double pos = animation_time < missile_animation.get_first_frame_time()?1.0:
			double(animation_time)/double(missile_animation.get_first_frame_time());
		const int posx = int(pos*xsrc + (1.0-pos)*xdst);
		const int posy = int(pos*ysrc + (1.0-pos)*ydst);
		disp.invalidate(b);
		disp.invalidate(a);
		if(leader_loc.valid()) disp.invalidate(leader_loc);
		halo::remove(missile_halo);
		halo::remove(missile_frame_halo);
		missile_halo = 0;
		missile_frame_halo = 0;
		if(animation_time < missile_animation.get_last_frame_time() && pos < 1.0 && (!disp.fogged(b.x,b.y) || !disp.fogged(a.x,a.y))) {

			missile_animation.update_current_frame();
			const unit_frame& missile_frame = missile_animation.get_current_frame();
			std::string missile_image= missile_frame.image;
			const int d = disp.hex_size() / 2;
			if(dir == unit_animation::VERTICAL) {
				missile_image = missile_frame.image;
			} else {
				missile_image = missile_frame.image_diagonal;
			}
			if(!missile_frame.halo.empty()) {
				int time = missile_frame.begin_time;
				unsigned int sub_halo = 0;
				while(time < animation_time && sub_halo < missile_frame.halo.size()) {
					time += missile_frame.halo[sub_halo].second;
					sub_halo++;

				}
				sub_halo--; //correct frame is the previous one

				if(sub_halo >= missile_frame.halo.size()) sub_halo = missile_frame.halo.size() -1;
				if(attack_ori != gamemap::location::SOUTH_WEST && attack_ori != gamemap::location::NORTH_WEST) {
					missile_halo = halo::add(posx+d+missile_frame.halo_x,
							posy+d+missile_frame.halo_y,
							missile_frame.halo[sub_halo].first,
							gamemap::location(-1, -1),
							orientation);
				} else {
					missile_halo = halo::add(posx+d-missile_frame.halo_x,
							posy+d+missile_frame.halo_y,
							missile_frame.halo[sub_halo].first,
							gamemap::location(-1, -1),
							orientation);
				}
			}
			missile_frame_halo = halo::add(posx+d,
					posy+d,
					missile_image,
					gamemap::location(-1, -1),
					orientation);

		}
		if(damage > 0 && animation_time > 0 && !sound_played) {
			sound_played = true;
			sound::play_sound(def->second.get_hit_sound());
			disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
			disp.invalidate_unit();
		}
		disp.draw();
		events::pump();
		if(attacker.get_animation()->animation_finished()) {
		   attacker.set_standing(disp,a,false);
		}
		if(defender.get_animation()->animation_finished()) {
		   defender.set_standing(disp,b,false);
		}
		if(leader_loc.valid() && leader->second.get_animation()->animation_finished() ) {
		   leader->second.set_standing(disp,leader_loc,true);
		}
		disp.delay(10);
		// we use missile animation because it's the only one not reseted in the middle to go to standing
		animation_time = missile_animation.get_animation_time();
		missile_animation.update_current_frame();
	}
	// make sure get hit sound is always played and labels always displayed
	if(damage > 0 && !hide  && !sound_played) {
		sound_played = true;
		sound::play_sound(def->second.get_hit_sound());
		disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		disp.invalidate_unit();
	}
	halo::remove(missile_halo);
	missile_halo = 0;
	halo::remove(missile_frame_halo);
	missile_frame_halo = 0;
	if(def->second.take_hit(damage)) {
		dead = true;
	}


	if(dead) {
		unit_display::unit_die(disp,def->first,def->second,&attack);
		if(leader_loc.valid()) leader->second.set_standing(disp,leader_loc); 	 
                 att->second.set_standing(disp,a); 	 
	}
	disp.update_display();
	events::pump();

	return dead;

}

} //end anon namespace

bool unit_attack(display& disp, unit_map& units,
                 const gamemap::location& a, const gamemap::location& b, int damage,
                 const attack_type& attack, bool update_display)
{
	const bool hide = disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)
	                  || preferences::show_combat() == false || (!update_display);

	if(!hide) {
		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		disp.scroll_to_tile(a.x,a.y,display::ONSCREEN);
	}

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	wassert(att != units.end());
	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	wassert(def != units.end());
	unit& defender = def->second;

	att->second.set_facing(a.get_relative_dir(b));
	def->second.set_facing(b.get_relative_dir(a));
	if(attack.range_type() == attack_type::LONG_RANGE) {
		return unit_attack_ranged(disp, units, a, b, damage, attack, update_display);
	}

	const bool hits = damage > 0;
	int start_time = 500;
	int end_time = 0;


	attacker.set_attacking(disp,a,hits,attack);
	start_time=minimum<int>(start_time,attacker.get_animation()->get_first_frame_time());
	end_time=attacker.get_animation()->get_last_frame_time();

	defender.set_defending(disp,b,damage,&attack);
	start_time=minimum<int>(start_time,defender.get_animation()->get_first_frame_time());


	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()){
		LOG_DP << "found leader at " << leader_loc << '\n';
		leader = units.find(leader_loc);
		wassert(leader != units.end());
		leader->second.set_facing(leader_loc.get_relative_dir(a));
		leader->second.set_leading(disp,leader_loc);
		start_time=minimum<int>(start_time,leader->second.get_animation()->get_first_frame_time());
	}


	gamemap::location update_tiles[6];
	get_adjacent_tiles(b,update_tiles);

	bool dead = false;





	attacker.restart_animation(disp,start_time);
	defender.restart_animation(disp,start_time);
	if(leader_loc.valid()) leader->second.restart_animation(disp,start_time);

	int animation_time = start_time;
	while(animation_time < 0 && !hide) {
		double pos = 0.0;
	        if(animation_time < attacker.get_animation()->get_first_frame_time()) {
			pos = 0.0;
		} else if( animation_time > 0) {
			pos = 1.0;
		} else {
			pos = 1.0 - double(animation_time)/double(attacker.get_animation()->get_first_frame_time());
		}
		attacker.set_offset(pos*0.6);
		disp.invalidate(b);
		disp.invalidate(a);
		if(leader_loc.valid()) disp.invalidate(leader_loc);
		disp.draw();
		events::pump();
		disp.delay(10);

		animation_time = attacker.get_animation()->get_animation_time();
	}
	if(damage > 0 && !hide) {
		sound::play_sound(def->second.get_hit_sound());
		disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		disp.invalidate_unit();
	}
	if(def->second.take_hit(damage)) {
		dead = true;
	}
	while(!hide && (
		attacker.state() != unit::STATE_STANDING ||
		defender.state() != unit::STATE_STANDING ||
		(leader_loc.valid() && leader->second.state() != unit::STATE_STANDING))
	     ){

		const double pos = (1.0-double(animation_time)/double(end_time));
		if(attacker.state() != unit::STATE_STANDING && pos > 0.0) {
			attacker.set_offset(pos*0.6);
		}
		disp.invalidate(b);
		disp.invalidate(a);
		if(leader_loc.valid()) disp.invalidate(leader_loc);
		disp.draw();
		events::pump();
		if(attacker.get_animation()->animation_finished()) {
		   attacker.set_standing(disp,a,false);
		   attacker.set_offset(0.0);
		}
		if(defender.get_animation()->animation_finished()) {
		   defender.set_standing(disp,b,false);
		}
		if(leader_loc.valid() && leader->second.get_animation()->animation_finished() ) {
		   leader->second.set_standing(disp,leader_loc,true);
		}
		disp.delay(10);
		animation_time = attacker.get_animation()->get_animation_time();
	}


	if(dead) {
		unit_display::unit_die(disp,def->first,def->second,&attack);
		if(leader_loc.valid()) leader->second.set_standing(disp,leader_loc); 	 
                 att->second.set_standing(disp,a); 	 
	}
	disp.update_display();
	events::pump();

	return dead;

}
} // end unit display namespace
