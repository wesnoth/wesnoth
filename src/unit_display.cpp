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

// ensure the map position is sufficiently centered on the acting rectangle
void adjust_map_position(display& disp, int x, int y, int w, int h)
{
	const int side_threshold = 80;
	int border;
	SDL_Rect area = disp.map_area();

	border = area.x + side_threshold;
	if (x < border) disp.scroll(x - border, 0);

	border = area.y + side_threshold;
	if (y < border) disp.scroll(0, y - border);

	x += w;
	border = area.x + area.w - side_threshold;
	if (x > border) disp.scroll(x - border, 0);

	y += h;
	border = area.y + area.h - side_threshold;
	if (y > border) disp.scroll(0, y - border);
}

void move_unit_between(display& disp, const gamemap& map, const gamemap::location& a, const gamemap::location& b, const unit& u)
{
	if(disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)) {
		return;
	}

	const bool face_left = u.facing_left();

	int xsrc = disp.get_location_x(a);
	int ysrc = disp.get_location_y(a);
	int xdst = disp.get_location_x(b);
	int ydst = disp.get_location_y(b);

	const gamemap::TERRAIN src_terrain = map.get_terrain(a);
	const gamemap::TERRAIN dst_terrain = map.get_terrain(b);

	const int src_height_adjust = u.is_flying() ? 0 : int(map.get_terrain_info(src_terrain).unit_height_adjust() * disp.zoom());
	const int dst_height_adjust = u.is_flying() ? 0 : int(map.get_terrain_info(dst_terrain).unit_height_adjust() * disp.zoom());

	const double src_submerge = u.is_flying() ? 0.0 : map.get_terrain_info(src_terrain).unit_submerge();
	const double dst_submerge = u.is_flying() ? 0.0 : map.get_terrain_info(dst_terrain).unit_submerge();

	LOG_DP << "submerge: " << src_submerge << " -> " << dst_submerge << "\n";

	const gamemap::TERRAIN terrain = map.get_terrain(b);
	const int nsteps = disp.turbo() ? 3 : 7*u.movement_cost(map,terrain);
	const double xstep = double(xdst - xsrc) / nsteps;
	const double ystep = double(ydst - ysrc) / nsteps;

	const int time_between_frames = disp.turbo() ? 10 : 20;
	int ticks = SDL_GetTicks();

	int skips = 0;

	const int acceleration = disp.turbo() ? 5:1;

	gamemap::location src_adjacent[6];
	get_adjacent_tiles(a, src_adjacent);

	const std::string& halo = u.type().image_halo();
	util::scoped_resource<int,halo::remover> halo_effect(0);
	if(halo.empty() == false && !disp.fogged(b.x,b.y)) {
		halo_effect.assign(halo::add(0,0,halo));
	}
	
	const unit_animation *teleport_animation_p = u.type().teleport_animation();
	bool teleport_unit = teleport_animation_p && !tiles_adjacent(a, b);
	if (teleport_unit && !disp.fogged(a.x, a.y)) { // teleport
		unit_animation teleport_animation =  *teleport_animation_p;
		int animation_time;
		const int begin_at = teleport_animation.get_first_frame_time(unit_animation::UNIT_FRAME);
		teleport_animation.start_animation(begin_at, unit_animation::UNIT_FRAME, acceleration);
		animation_time = teleport_animation.get_animation_time();
		adjust_map_position(disp, xsrc, ysrc, disp.hex_size(), disp.hex_size());
		while(animation_time < 0) {
			const std::string* unit_image = &teleport_animation.get_current_frame(unit_animation::UNIT_FRAME).image;
			if (unit_image->empty()) {
				unit_image = &u.type().image();
			}
			surface image(image::get_image(*unit_image));
			if (!face_left) {
				image.assign(image::reverse_image(image));
			}
			disp.draw_tile(a.x,a.y);
			for(int tile = 0; tile != 6; ++tile) {
				disp.draw_tile(src_adjacent[tile].x, src_adjacent[tile].y);
			}
			disp.draw_unit(xsrc,ysrc,image,false, ftofxp(1.0), 0, 0.0, src_submerge);
			disp.update_display();
			events::pump();
			teleport_animation.update_current_frames();
			animation_time = teleport_animation.get_animation_time();
		}
	}

	for(int i = 0; i < nsteps; ++i) {
		events::pump();

		surface image(image::get_image(u.type().image_moving()));
		if(!face_left) {
			image.assign(image::reverse_image(image));
		}

		if(image == NULL) {
			LOG_STREAM(err, display) << "failed to get image " << u.type().image_moving() << "\n";
			return;
		}

		xsrc = disp.get_location_x(a);
		ysrc = disp.get_location_y(a);
		int xloc = xsrc + int(xstep * i);
		int yloc = ysrc + int(ystep * i);

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		adjust_map_position(disp, xloc, yloc, image->w, image->h);

		if(xsrc != disp.get_location_x(a) || ysrc != disp.get_location_y(a)) {
			disp.scroll_to_tile(b.x,b.y,display::WARP);
			xsrc = disp.get_location_x(a);
			ysrc = disp.get_location_y(a);
			xloc = xsrc + int(xstep * i);
			yloc = ysrc + int(ystep * i);
		}

		//invalidate the source tile and all adjacent tiles,
		//since the unit can partially overlap adjacent tiles
		disp.draw_tile(a.x, a.y);
		for(int tile = 0; tile != 6; ++tile)
			disp.draw_tile(src_adjacent[tile].x, src_adjacent[tile].y);

		if (!teleport_unit) {
			const int height_adjust = src_height_adjust + (dst_height_adjust - src_height_adjust) * i / nsteps;
			const double submerge = src_submerge + (dst_submerge - src_submerge) * i / nsteps;
			const int xpos = xloc;
			const int ypos = yloc - height_adjust;
			disp.draw_unit(xpos, ypos, image, false, ftofxp(1.0), 0, 0.0, submerge);

			if (halo_effect != 0) {
				int d = disp.hex_size() / 2;
				halo::set_location(halo_effect, xpos + d, ypos + d);
			}
		}

		const int new_ticks = SDL_GetTicks();
		const int wait_time = time_between_frames - (new_ticks - ticks);
		SDL_Delay(maximum<int>(wait_time,1));

		ticks = SDL_GetTicks();

		if(wait_time >= 0 || skips == 4 || (i+1.0) >= nsteps) {
			skips = 0;
			disp.update_display();
		} else {
			++skips;
		}
	}

	gamemap::location dst_adjacent[6];
	get_adjacent_tiles(b, dst_adjacent);

	if (teleport_unit && !disp.fogged(b.x, b.y)) { // teleport
		unit_animation teleport_animation =  *teleport_animation_p;
		int animation_time;
		const int end_at = teleport_animation.get_last_frame_time();
		teleport_animation.start_animation(0, unit_animation::UNIT_FRAME, acceleration);
		animation_time = teleport_animation.get_animation_time();
		adjust_map_position(disp, xdst, ydst, disp.hex_size(), disp.hex_size());
		while(animation_time < end_at) {
			const std::string* unit_image = &teleport_animation.get_current_frame(unit_animation::UNIT_FRAME).image;
			if (unit_image->empty()) {
				unit_image = &u.type().image();
			}
			surface image(image::get_image(*unit_image));
			if (!face_left) {
				image.assign(image::reverse_image(image));
			}
			disp.draw_tile(b.x,b.y);
			for(int tile = 0; tile != 6; ++tile) {
				disp.draw_tile(dst_adjacent[tile].x,dst_adjacent[tile].y);
			}
			disp.draw_unit(xdst, ydst, image, false, ftofxp(1.0), 0, 0.0, dst_submerge);
			disp.update_display();
			events::pump();
			teleport_animation.update_current_frames();
			animation_time = teleport_animation.get_animation_time();
		}
	}
}

}

namespace unit_display
{

bool unit_visible_on_path(display& disp, const gamemap& map, const std::vector<gamemap::location>& path, unit& u, const time_of_day& tod, const unit_map& units, const std::vector<team>& teams)
{
	for(size_t i = 0; i+1 < path.size(); ++i) {
		const bool invisible = teams[u.side()-1].is_enemy(int(disp.viewing_team()+1)) &&
	             u.invisible(map.underlying_terrain(path[i]),tod.lawful_bonus,path[i],units,teams) &&
		         u.invisible(map.underlying_terrain(path[i+1]),tod.lawful_bonus,path[i+1],units,teams);
		if(!invisible) {
			return true;
		}
	}

	return false;
}

void move_unit(display& disp, const gamemap& map, const std::vector<gamemap::location>& path, unit& u, const time_of_day& tod, const unit_map& units, const std::vector<team>& teams)
{
	bool previous_visible = false;
	for(size_t i = 0; i+1 < path.size(); ++i) {
		if(path[i+1].x > path[i].x) {
			u.set_facing_left(true);
		} else if(path[i+1].x < path[i].x) {
			u.set_facing_left(false);
		}

		disp.remove_footstep(path[i]);

		bool invisible;

		if(u.side() == 0) {
			invisible = false;
		} else {
			invisible = teams[u.side()-1].is_enemy(int(disp.viewing_team()+1)) &&
				u.invisible(map.underlying_terrain(path[i]),tod.lawful_bonus,path[i],units,teams) &&
				u.invisible(map.underlying_terrain(path[i+1]),tod.lawful_bonus,path[i+1],units,teams);
		}

		if(!invisible) {
			move_unit_between(disp,map,path[i],path[i+1],u);
			previous_visible = true;
		} else if(previous_visible) {
			disp.draw_tile(path[i].x,path[i].y);
		}
	}

	//make sure the entire path is cleaned properly
	for(std::vector<gamemap::location>::const_iterator it = path.begin(); it != path.end(); ++it) {
		disp.draw_tile(it->x,it->y);
	}
}

void unit_die(display& disp, const gamemap::location& loc, const unit& u, const attack_type* attack)
{
	if(disp.update_locked() || disp.fogged(loc.x,loc.y) || preferences::show_combat() == false) {
		return;
	}

	const std::string& die_sound = u.type().die_sound();
	if(die_sound != "" && die_sound != "null") {
		sound::play_sound(die_sound);
	}

	surface unit_image(NULL);

	const unit_animation* const anim_ptr = u.type().die_animation(attack);
	if(anim_ptr != NULL) {
		unit_animation anim(*anim_ptr);

		anim.start_animation(anim.get_first_frame_time(),unit_animation::UNIT_FRAME,disp.turbo() ? 5:1);
		anim.update_current_frames();

		while(!anim.animation_finished()) {

			const unit_animation::frame& frame = anim.get_current_frame();
			
			const surface surf(image::get_image(frame.image));
			if(surf.get() != NULL) {
				unit_image = surf;
			}
			disp.draw_tile(loc.x,loc.y,unit_image);
			disp.update_display();

			SDL_Delay(10);

			anim.update_current_frames();
		}
	}

	const int frame_time = 30;
	int ticks = SDL_GetTicks();

	for(fixed_t alpha = ftofxp(1.0); alpha > ftofxp(0.0); alpha -= ftofxp(0.05)) {
		disp.draw_tile(loc.x,loc.y,unit_image,alpha);

		const int wait_time = ticks + frame_time - SDL_GetTicks();

		if(wait_time > 0 && !disp.turbo())
			SDL_Delay(wait_time);

		ticks = SDL_GetTicks();

		disp.update_display();
	}

	disp.draw_tile(loc.x,loc.y,unit_image,ftofxp(0.0));
	disp.update_display();
}

namespace {
  
bool unit_attack_ranged(display& disp, unit_map& units,
                        const gamemap::location& a, const gamemap::location& b,
                        int damage, const attack_type& attack)
{
	const bool hide = disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)
	                  || preferences::show_combat() == false;

	const unit_map::iterator att = units.find(a);
	const unit_map::iterator def = units.find(b);

	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()) {
		leader = units.find(leader_loc);
		wassert(leader != units.end());
		leader->second.set_leading(true);
	}

	unit_animation attack_anim = attack.animation(get_adjacent_direction(a,b));

	//the missile frames are based around the time when the missile impacts.
	//the 'real' frames are based around the time when the missile launches.
	const int first_missile = minimum<int>(-100,attack_anim.get_first_frame_time(unit_animation::MISSILE_FRAME));
	const int last_missile = attack_anim.get_last_frame_time(unit_animation::MISSILE_FRAME);

	const int real_last_missile = last_missile - first_missile;
	const int missile_impact = -first_missile;

	const int time_resolution = 20;
	const int acceleration = disp.turbo() ? 5:1;

	const std::vector<unit_animation::sfx>& sounds = attack_anim.sound_effects();
	std::vector<unit_animation::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const bool hits = damage > 0;
	const int begin_at = attack_anim.get_first_frame_time(unit_animation::UNIT_FRAME);
	const int end_at   = maximum((damage+1)*time_resolution+missile_impact,
				       maximum(attack_anim.get_last_frame_time(),real_last_missile));

	const double xsrc = disp.get_location_x(a);
	const double ysrc = disp.get_location_y(a);
	const double xdst = disp.get_location_x(b);
	const double ydst = disp.get_location_y(b);

	gamemap::location update_tiles[6];
	get_adjacent_tiles(a,update_tiles);

	const bool vflip = b.y > a.y || b.y == a.y && is_even(a.x);
	const bool hflip = b.x < a.x;
	const unit_animation::FRAME_DIRECTION dir = (a.x == b.x) ? unit_animation::VERTICAL:unit_animation::DIAGONAL;

	bool dead = false;
	const int drain_speed = 1*acceleration;

	int flash_num = 0;

	bool shown_label = false;

	util::scoped_resource<int,halo::remover> missile_halo_effect(0), unit_halo_effect(0);
	const std::string* missile_halo_image = NULL;
	const std::string* unit_halo_image = NULL;
	int missile_halo_x = -1, missile_halo_y = -1, unit_halo_x = -1, unit_halo_y = -1;
	
	attack_anim.start_animation(begin_at, unit_animation::UNIT_FRAME, acceleration);
	attack_anim.start_animation(begin_at + first_missile, unit_animation::MISSILE_FRAME, acceleration);

	attack_anim.update_current_frames();
	int animation_time = attack_anim.get_animation_time();

	def->second.set_defending(hits, attack_type::LONG_RANGE, animation_time, acceleration);

	while(animation_time < end_at) {

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && animation_time >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && animation_time >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		const unit_animation::frame& unit_frame = attack_anim.get_current_frame(unit_animation::UNIT_FRAME);

		LOG_DP << "Animation time :" << animation_time << ", image " << unit_frame.image << "\n";
		int new_halo_x = unit_frame.halo_x;
		int new_halo_y = unit_frame.halo_y;
		const std::string* unit_image = &unit_frame.image;

		if(att->second.facing_left() == false) {
			new_halo_x *= -1;
		}

		if(unit_halo_image != &unit_frame.halo || 
				unit_halo_x != new_halo_x ||
				unit_halo_y != new_halo_y) {
			
			unit_halo_image = &unit_frame.halo;
			unit_halo_x = new_halo_x;
			unit_halo_y = new_halo_y;

			if(!unit_frame.halo.empty() && !disp.fogged(a.x,a.y)) {
				const int halo_xpos = int(disp.get_location_x(a) +
						disp.hex_size()/2.0 + unit_halo_x*disp.zoom());
				const int halo_ypos = int(disp.get_location_y(a) +
						disp.hex_size()/2.0 + unit_halo_y*disp.zoom());

				unit_halo_effect.assign(halo::add(halo_xpos,halo_ypos,*unit_halo_image));
			} else {
				unit_halo_effect.assign(0);
			}
		}

		if(unit_image->empty()) {
			unit_image = &att->second.type().image_fighting(attack_type::LONG_RANGE);
		}

		if(!hide) {
			const surface image((unit_image == NULL) ? surface(NULL) : image::get_image(*unit_image));
			disp.draw_tile(a.x,a.y,image);
		}

		if(damage > 0 && animation_time >= missile_impact && shown_label == false) {
			shown_label = true;
			disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		}

		Uint32 defensive_colour = 0;
		fixed_t defensive_alpha = ftofxp(1.0);

		LOG_DP << "Waiting for missile impact at " << missile_impact << "\n";
		if(damage > 0 && animation_time >= missile_impact) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defensive_alpha = ftofxp(0.0);
				defensive_colour = disp.rgb(200,0,0);
			}

			++flash_num;
		}

		for(int j = 0; j != 6; ++j) {
			if(update_tiles[j] != b) {
				disp.draw_tile(update_tiles[j].x,update_tiles[j].y);
			}
		}

		disp.draw_tile(b.x,b.y,NULL,defensive_alpha,defensive_colour);
		if(leader_loc.valid()) {
			disp.draw_tile(leader_loc.x,leader_loc.y);
		}

		if(animation_time >= 0 && animation_time < real_last_missile && !hide) {
			const unit_animation::frame& missile_frame = attack_anim.get_current_frame(unit_animation::MISSILE_FRAME);
			LOG_DP << "Missile: animation time :" << animation_time << ", image "
				<< missile_frame.image << ", halo: " << missile_frame.halo << "\n";

			new_halo_x = missile_frame.halo_x;
			new_halo_y = missile_frame.halo_y;

			if(att->second.facing_left() == false) {
				new_halo_x *= -1;
			}

			new_halo_x = int(new_halo_x*disp.zoom());
			new_halo_y = int(new_halo_y*disp.zoom());

			const std::string *missile_image = NULL;
			if(dir == unit_animation::VERTICAL) {
				missile_image = &missile_frame.image;
			} else {
				missile_image = &missile_frame.image_diagonal;
			}

			static const std::string default_missile(game_config::missile_n_image);
			static const std::string default_diag_missile(game_config::missile_ne_image);
			if(missile_image->empty()) {
				if(dir == unit_animation::VERTICAL)
					missile_image = &default_missile;
				else
					missile_image = &default_diag_missile;
			}

			surface img(image::get_image(*missile_image));

			if(hflip) {
				img.assign(image::reverse_image(img));
			}

			double pos = double(missile_impact - animation_time)/double(missile_impact);
			if(pos < 0.0) {
				pos = 0.0;
			}

			const int xpos = int((xsrc+new_halo_x)*pos + xdst*(1.0-pos));
			const int ypos = int((ysrc+new_halo_y)*pos + ydst*(1.0-pos));

			if(img != NULL) {
				disp.draw_unit(xpos,ypos,img,vflip);
			}

			const int halo_xpos = xpos+disp.hex_size()/2;
			const int halo_ypos = ypos+disp.hex_size()/2;

			if(missile_halo_image != &missile_frame.halo || missile_halo_x != new_halo_x || missile_halo_y != new_halo_y) {
				missile_halo_image = &missile_frame.halo;
				missile_halo_x = new_halo_x;
				missile_halo_y = new_halo_y;

				if(missile_halo_image != NULL &&
						!missile_halo_image->empty() &&
						!disp.fogged(b.x,b.y)) {
					missile_halo_effect.assign(halo::add(halo_xpos,halo_ypos,*missile_halo_image));
				} else {
					missile_halo_effect.assign(0);
				}
			}

			else if(missile_halo_effect != 0) {
				halo::set_location(missile_halo_effect,halo_xpos,halo_ypos);
			}
		} else {
			//the missile halo should disappear now, since the missile has stopped being shown
			missile_halo_effect.assign(0);
		}

		//TODO: fix this
		SDL_Delay(20);
#if 0
		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide) {
			SDL_Delay(wait_time);
		} else if(wait_time < 0) {
			//if we're not keeping up, then skip frames
			i += minimum<int>(time_resolution*4,-wait_time);
		}
#endif

		// ticks = SDL_GetTicks();

		attack_anim.update_current_frames();
		def->second.update_defending_frame();
		animation_time = attack_anim.get_animation_time();
		events::pump();
		disp.update_display();
	}

	unit_halo_effect.assign(0);
	missile_halo_effect.assign(0);

	if(damage > 0 && shown_label == false) {
		shown_label = true;
		disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
	}

	if(damage > 0 && def->second.gets_hit(damage)) {
		dead = true;
		damage = 0;
	}

	if(leader_loc.valid()){
		leader->second.set_leading(false);
	}

	disp.invalidate(a);
	disp.invalidate(b);

	def->second.set_standing();

	if(leader_loc.valid()){
		disp.draw_tile(leader_loc.x,leader_loc.y);
	}

	if(dead) {
		unit_die(disp,def->first,def->second,&attack);
	}

	return dead;
}

} //end anon namespace

bool unit_attack(display& disp, unit_map& units, const gamemap& map,
                 const gamemap::location& a, const gamemap::location& b, int damage,
                 const attack_type& attack)
{
	const bool hide = disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)
	                  || preferences::show_combat() == false;

	if(!hide) {
		int xloc = disp.get_location_x(a);
		int yloc = disp.get_location_y(a);

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		adjust_map_position(disp, xloc, yloc, disp.hex_size(), disp.hex_size());

		if(xloc != disp.get_location_x(a) || yloc != disp.get_location_y(a)) {
			disp.scroll_to_tile(a.x,a.y,display::WARP);
		}
	}

	log_scope("unit_attack");

	const unit_map::iterator att = units.find(a);
	wassert(att != units.end());

	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	wassert(def != units.end());

	if(b.x > a.x) {
		att->second.set_facing_left(true);
		def->second.set_facing_left(false);
	} else if(b.x < a.x) {
		att->second.set_facing_left(false);
		def->second.set_facing_left(true);
	}

	if(attack.range() == attack_type::LONG_RANGE) {
		return unit_attack_ranged(disp, units, a, b, damage, attack);
	}

	unit_animation attack_anim = attack.animation(get_adjacent_direction(a,b));

	const bool hits = damage > 0;
	const std::vector<unit_animation::sfx>& sounds = attack_anim.sound_effects();
	std::vector<unit_animation::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const int time_resolution = 20;
	const int acceleration = disp.turbo() ? 5 : 1;

	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()){
		LOG_DP << "found leader at " << leader_loc << '\n';
		leader = units.find(leader_loc);
		wassert(leader != units.end());
		leader->second.set_leading(true);
	}

	const int begin_at = minimum<int>(-200,attack_anim.get_first_frame_time());
	const int end_at = maximum<int>((damage+1)*time_resolution,
	                                       maximum<int>(200,attack_anim.get_last_frame_time()));

	const double xsrc = disp.get_location_x(a);
	const double ysrc = disp.get_location_y(a);
	const double xdst = disp.get_location_x(b)*0.6 + xsrc*0.4;
	const double ydst = disp.get_location_y(b)*0.6 + ysrc*0.4;

	gamemap::location update_tiles[6];
	get_adjacent_tiles(b,update_tiles);

	bool dead = false;
	const int drain_speed = 1*acceleration;

	int flash_num = 0;

	int ticks = SDL_GetTicks();

	disp.hide_unit(a);

	const gamemap::TERRAIN src_terrain = map.get_terrain(a);
	const gamemap::TERRAIN dst_terrain = map.get_terrain(b);

	const double src_height_adjust = attacker.is_flying() ? 0 : map.get_terrain_info(src_terrain).unit_height_adjust() * disp.zoom();
	const double dst_height_adjust = attacker.is_flying() ? 0 : map.get_terrain_info(dst_terrain).unit_height_adjust() * disp.zoom();

	const double src_submerge = attacker.is_flying() ? 0 : map.get_terrain_info(src_terrain).unit_submerge();
	const double dst_submerge = attacker.is_flying() ? 0 : map.get_terrain_info(dst_terrain).unit_submerge();

	bool shown_label = false;

	util::scoped_resource<int,halo::remover> halo_effect(0);
	const std::string* halo_image = NULL;
	int halo_x = -1, halo_y = -1;

	attack_anim.start_animation(begin_at, unit_animation::UNIT_FRAME, acceleration);

	int animation_time = attack_anim.get_animation_time();

	def->second.set_defending(hits, attack_type::SHORT_RANGE, animation_time, acceleration);

	while(animation_time < end_at) {

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && animation_time >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && animation_time >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		for(int j = 0; j != 6; ++j) {
			disp.draw_tile(update_tiles[j].x,update_tiles[j].y);
		}

		Uint32 defender_colour = 0;
		fixed_t defender_alpha = ftofxp(1.0);

		if(damage > 0 && animation_time >= 0 && shown_label == false) {
			shown_label = true;
			disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		}

		if(damage > 0 && animation_time >= 0) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defender_alpha = ftofxp(0.0);
				defender_colour = disp.rgb(200,0,0);
			}

			++flash_num;
		}

		disp.draw_tile(b.x,b.y,NULL,defender_alpha,defender_colour);
		if(leader_loc.valid()) {
			disp.draw_tile(leader_loc.x,leader_loc.y);
		}


		int xoffset = 0;

		const unit_animation::frame& unit_frame = attack_anim.get_current_frame(unit_animation::UNIT_FRAME);
		int new_halo_x = unit_frame.halo_x;
		int new_halo_y = unit_frame.halo_y;
		const std::string& unit_image = unit_frame.image.empty() ? attacker.image() : unit_frame.image;

		if(!attacker.facing_left()) {
			xoffset *= -1;
			new_halo_x *= -1;
		}

		new_halo_x = int(new_halo_x*disp.zoom());

		xoffset = int(double(xoffset)*disp.zoom());

		surface image(image::get_image(unit_image));
		if(attacker.facing_left() == false) {
			image.assign(image::reverse_image(image));
		}

		const double pos = double(animation_time)/double(animation_time < 0 ? begin_at : end_at);
		const int posx = int(pos*xsrc + (1.0-pos)*xdst) + xoffset;
		const int posy = int(pos*ysrc + (1.0-pos)*ydst);

		const int halo_xpos = posx+disp.hex_size()/2;
		const int halo_ypos = posy+disp.hex_size()/2;

		if(&unit_frame.halo != halo_image ||
				new_halo_x != halo_x ||
				new_halo_y != halo_y) {
			halo_image = &unit_frame.halo;
			halo_x = new_halo_x;
			halo_y = new_halo_y;

			if(!unit_frame.halo.empty() && 
					(!disp.fogged(b.x,b.y) || !disp.fogged(a.x,a.y))) {
				halo_effect.assign(halo::add(halo_xpos,halo_ypos,*halo_image));
			} else {
				halo_effect.assign(0);
			}
		}

		else if(halo_effect != 0) {
			halo::set_location(halo_effect,halo_xpos,halo_ypos);
		}

		const int height_adjust = int(src_height_adjust*pos + dst_height_adjust*(1.0-pos));
		const double submerge = src_submerge*pos + dst_submerge*(1.0-pos);

		if(image != NULL && !hide) {
			disp.draw_unit(posx, posy - height_adjust, image, false, ftofxp(1.0), 0, 0.0, submerge);
		}

		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide) {
			SDL_Delay(wait_time);
		}

		ticks = SDL_GetTicks();

		attack_anim.update_current_frames();
		def->second.update_defending_frame();
		animation_time = attack_anim.get_animation_time();
		events::pump();
		disp.update_display();
	}

	halo_effect.assign(0);

	if(damage > 0 && shown_label == false) {
		shown_label = true;
		disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
	}

	disp.hide_unit(gamemap::location());

	if(damage > 0 && def->second.gets_hit(damage)) {
		dead = true;
		damage = 0;
	}

	if(leader_loc.valid()){
		leader->second.set_leading(false);
	}

	disp.invalidate(a);
	disp.invalidate(b);
	if(leader_loc.valid()) {
		disp.draw_tile(leader_loc.x,leader_loc.y);
	}

	def->second.set_standing();

	if(dead) {
		unit_display::unit_die(disp,def->first,def->second,&attack);
	}

	return dead;
}

}
