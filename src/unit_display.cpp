#include "actions.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "halo.hpp"
#include "image.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "unit_display.hpp"
#include "util.hpp"

namespace
{

void move_unit_between(display& disp, const gamemap& map, const gamemap::location& a, const gamemap::location& b, const unit& u)
{
	if(disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)) {
		return;
	}

	const bool face_left = u.facing_left();

	const int side_threshhold = 80;

	int xsrc = disp.get_location_x(a);
	int ysrc = disp.get_location_y(a);
	int xdst = disp.get_location_x(b);
	int ydst = disp.get_location_y(b);

	const gamemap::TERRAIN src_terrain = map.get_terrain(a);
	const gamemap::TERRAIN dst_terrain = map.get_terrain(b);

	const int src_height_adjust = u.is_flying() ? 0 : int(map.get_terrain_info(src_terrain).unit_height_adjust() * disp.zoom());
	const int dst_height_adjust = u.is_flying() ? 0 : int(map.get_terrain_info(dst_terrain).unit_height_adjust() * disp.zoom());

	const double src_submerge = u.is_flying() ? 0 : int(map.get_terrain_info(src_terrain).unit_submerge());
	const double dst_submerge = u.is_flying() ? 0 : int(map.get_terrain_info(dst_terrain).unit_submerge());

	const int nsteps = disp.turbo() ? 3 : 10;
	const double xstep = double(xdst - xsrc)/double(nsteps);
	const double ystep = double(ydst - ysrc)/double(nsteps);

	const int time_between_frames = disp.turbo() ? 2 : 10;
	int ticks = SDL_GetTicks();

	int skips = 0;

	for(int i = 0; i < nsteps; ++i) {
		events::pump();

		scoped_sdl_surface image(image::get_image(u.type().image_moving()));
		if(!face_left) {
			image.assign(image::reverse_image(image));
		}

		if(image == NULL) {
			std::cerr << "failed to get image " << u.type().image_moving() << "\n";
			return;
		}

		xsrc = disp.get_location_x(a);
		ysrc = disp.get_location_y(a);
		xdst = disp.get_location_x(b);
		ydst = disp.get_location_y(b);

		int xloc = xsrc + int(xstep*double(i));
		int yloc = ysrc + int(ystep*double(i));

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		if(xloc < side_threshhold) {
			disp.scroll(xloc - side_threshhold,0);
		}

		if(yloc < side_threshhold) {
			disp.scroll(0,yloc - side_threshhold);
		}

		if(xloc + double(image->w) > disp.mapx() - side_threshhold) {
			disp.scroll(((xloc + image->w) - (disp.mapx() - side_threshhold)),0);
		}

		if(yloc + double(image->h) > disp.y() - side_threshhold) {
			disp.scroll(0,((yloc + image->h) - (disp.y() - side_threshhold)));
		}

		if(xsrc != disp.get_location_x(a) || ysrc != disp.get_location_y(a)) {
			disp.scroll_to_tile(b.x,b.y,display::WARP);
			xsrc = disp.get_location_x(a);
			ysrc = disp.get_location_y(a);
			xdst = disp.get_location_x(b);
			ydst = disp.get_location_y(b);
			xloc = xsrc + int(xstep*i);
			yloc = ysrc + int(ystep*i);
		}

		//invalidate the source tile and all adjacent tiles,
		//since the unit can partially overlap adjacent tiles
		gamemap::location adjacent[6];
		get_adjacent_tiles(a,adjacent);
		disp.draw_tile(a.x,a.y);
		for(int tile = 0; tile != 6; ++tile) {
			disp.draw_tile(adjacent[tile].x,adjacent[tile].y);
		}

		const int height_adjust = src_height_adjust + (dst_height_adjust-src_height_adjust)*(i/nsteps);
		const double submerge = src_submerge + (dst_submerge-src_submerge)*(i/nsteps);

		disp.draw(false);
		disp.draw_unit((int)xloc,(int)yloc - height_adjust,image,false,1.0,0,submerge);

		const int new_ticks = SDL_GetTicks();
		const int wait_time = time_between_frames - (new_ticks - ticks);
		if(wait_time > 0) {
			SDL_Delay(wait_time);
		}

		ticks = SDL_GetTicks();

		if(wait_time >= 0 || skips == 4 || (i+1.0) >= nsteps) {
			skips = 0;
			disp.update_display();
		} else {
			++skips;
		}
	}
}

}

namespace unit_display
{

void move_unit(display& disp, const gamemap& map, const std::vector<gamemap::location>& path, unit& u)
{
	for(size_t i = 0; i+1 < path.size(); ++i) {
		if(path[i+1].x > path[i].x) {
			u.set_facing_left(true);
		} else if(path[i+1].x < path[i].x) {
			u.set_facing_left(false);
		}

		disp.remove_footstep(path[i]);

		move_unit_between(disp,map,path[i],path[i+1],u);
	}

	//make sure the entire path is cleaned properly
	for(std::vector<gamemap::location>::const_iterator it = path.begin(); it != path.end(); ++it) {
		disp.draw_tile(it->x,it->y);
	}
}

void unit_die(display& disp, const gamemap::location& loc, const unit& u)
{
	if(disp.update_locked() || disp.fogged(loc.x,loc.y) || preferences::show_combat() == false) {
		return;
	}

	const std::string& die_sound = u.type().die_sound();
	if(die_sound != "" && die_sound != "null") {
		sound::play_sound(die_sound);
	}

	const int frame_time = 30;
	int ticks = SDL_GetTicks();

	for(double alpha = 1.0; alpha > 0.0; alpha -= 0.05) {
		disp.draw_tile(loc.x,loc.y,NULL,alpha);

		const int wait_time = ticks + frame_time - SDL_GetTicks();

		if(wait_time > 0 && !disp.turbo())
			SDL_Delay(wait_time);

		ticks = SDL_GetTicks();

		disp.update_display();
	}

	disp.draw(true,true);
}

namespace {
  
bool unit_attack_ranged(display& disp, unit_map& units, const gamemap& map,
                        const gamemap::location& a, const gamemap::location& b, int damage,
                        const attack_type& attack)
{
	const bool hide = disp.update_locked() || disp.fogged(a.x,a.y) && disp.fogged(b.x,b.y)
	                  || preferences::show_combat() == false;

	const unit_map::iterator att = units.find(a);
	const unit_map::iterator def = units.find(b);

	def->second.set_defending(true,attack_type::LONG_RANGE);

	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()) {
		leader = units.find(leader_loc);
		assert(leader != units.end());
		leader->second.set_leading(true);
	}

	//the missile frames are based around the time when the missile impacts.
	//the 'real' frames are based around the time when the missile launches.
	const int first_missile = minimum<int>(-100,attack.get_first_frame(attack_type::MISSILE_FRAME));
	const int last_missile = attack.get_last_frame(attack_type::MISSILE_FRAME);

	const int real_last_missile = last_missile - first_missile;
	const int missile_impact = -first_missile;

	const int time_resolution = 20;
	const int acceleration = disp.turbo() ? 5:1;

	const std::vector<attack_type::sfx>& sounds = attack.sound_effects();
	std::vector<attack_type::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const bool hits = damage > 0;
	const int begin_at = attack.get_first_frame();
	const int end_at   = maximum((damage+1)*time_resolution+missile_impact,
					       maximum(attack.get_last_frame(),real_last_missile));

	const double xsrc = disp.get_location_x(a);
	const double ysrc = disp.get_location_y(a);
	const double xdst = disp.get_location_x(b);
	const double ydst = disp.get_location_y(b);

	gamemap::location update_tiles[6];
	get_adjacent_tiles(a,update_tiles);

	const bool vflip = b.y > a.y || b.y == a.y && is_even(a.x);
	const bool hflip = b.x < a.x;
	const attack_type::FRAME_DIRECTION dir =
	         (a.x == b.x) ? attack_type::VERTICAL:attack_type::DIAGONAL;

	bool dead = false;
	const int drain_speed = 1*acceleration;

	int flash_num = 0;

	int ticks = SDL_GetTicks();

	bool shown_label = false;
	
	for(int i = begin_at; i < end_at; i += time_resolution*acceleration) {
		events::pump();

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && i >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && i >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		const std::string* unit_image = attack.get_frame(i);

		if(unit_image == NULL) {
			unit_image = &att->second.type().image_fighting(attack_type::LONG_RANGE);
		}

		if(!hide) {
			const scoped_sdl_surface image((unit_image == NULL) ? NULL : image::get_image(*unit_image));
			disp.draw_tile(a.x,a.y,image);
		}

		if(damage > 0 && i >= missile_impact && shown_label == false) {
			shown_label = true;
			disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		}

		Uint32 defensive_colour = 0;
		double defensive_alpha = 1.0;

		if(damage > 0 && i >= missile_impact) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defensive_alpha = 0.0;
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

		util::scoped_resource<int,halo::remover> halo_effect(0);

		if(i >= 0 && i < real_last_missile && !hide) {
			const int missile_frame = i + first_missile;

			const std::string* halo_image = NULL;
			const std::string* missile_image = attack.get_frame(missile_frame,NULL,
			                                                    attack_type::MISSILE_FRAME,dir,&halo_image);

			static const std::string default_missile(game_config::missile_n_image);
			static const std::string default_diag_missile(game_config::missile_ne_image);
			if(missile_image == NULL) {
				if(dir == attack_type::VERTICAL)
					missile_image = &default_missile;
				else
					missile_image = &default_diag_missile;
			}

			scoped_sdl_surface img(image::get_image(*missile_image));

			if(hflip) {
				img.assign(image::reverse_image(img));
			}

			double pos = double(missile_impact - i)/double(missile_impact);
			if(pos < 0.0) {
				pos = 0.0;
			}

			const int xpos = int(xsrc*pos + xdst*(1.0-pos));
			const int ypos = int(ysrc*pos + ydst*(1.0-pos));

			if(img != NULL) {
				disp.draw_unit(xpos,ypos,img,vflip);
			}

			if(halo_image != NULL) {
				halo_effect.assign(halo::add(xpos+disp.hex_width()/2,ypos+disp.hex_size()/2,*halo_image));
			}
		}

		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide) {
			SDL_Delay(wait_time);
		} else if(wait_time < 0) {
			//if we're not keeping up, then skip frames
			i += minimum<int>(time_resolution*4,-wait_time);
		}

		ticks = SDL_GetTicks();

		disp.update_display();
	}

	if(damage > 0 && def->second.gets_hit(damage)) {
		dead = true;
		damage = 0;
	}

	def->second.set_defending(false);

	if(leader_loc.valid()){
		leader->second.set_leading(false);
	}

	disp.draw_tile(a.x,a.y);
	disp.draw_tile(b.x,b.y);

	if(leader_loc.valid()){
		disp.draw_tile(leader_loc.x,leader_loc.y);
	}

	if(dead) {
		unit_die(disp,def->first,def->second);
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
		const int side_threshhold = 80;

		int xloc = disp.get_location_x(a);
		int yloc = disp.get_location_y(a);

		SDL_Rect area = disp.map_area();

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		if(xloc < area.x + side_threshhold) {
			disp.scroll(xloc - side_threshhold - disp.map_area().x,0);
		}

		if(yloc < area.y + side_threshhold) {
			disp.scroll(0,yloc - side_threshhold - area.y);
		}

		if(xloc + disp.hex_size() > area.x + area.w - side_threshhold) {
			disp.scroll(((xloc + disp.hex_size()) - (area.x + area.w - side_threshhold)),0);
		}

		if(yloc + disp.hex_size() > area.y + area.h - side_threshhold) {
			disp.scroll(0,((yloc + disp.hex_size()) - (area.y + area.h - side_threshhold)));
		}

		if(xloc != disp.get_location_x(a) || yloc != disp.get_location_y(a)) {
			disp.scroll_to_tile(a.x,a.y,display::WARP);
		}
	}

	log_scope("unit_attack");
	disp.invalidate_all();
	disp.draw(true,true);

	const unit_map::iterator att = units.find(a);
	assert(att != units.end());

	unit& attacker = att->second;

	const unit_map::iterator def = units.find(b);
	assert(def != units.end());

	if(b.x > a.x) {
		att->second.set_facing_left(true);
		def->second.set_facing_left(false);
	} else if(b.x < a.x) {
		att->second.set_facing_left(false);
		def->second.set_facing_left(true);
	}

	if(attack.range() == attack_type::LONG_RANGE) {
		return unit_attack_ranged(disp,units,map,a,b,damage,attack);
	}

	const bool hits = damage > 0;
	const std::vector<attack_type::sfx>& sounds = attack.sound_effects();
	std::vector<attack_type::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const int time_resolution = 20;
	const int acceleration = disp.turbo() ? 5 : 1;

	def->second.set_defending(true,attack_type::SHORT_RANGE);

	const gamemap::location leader_loc = under_leadership(units,a);
	unit_map::iterator leader = units.end();
	if(leader_loc.valid()){
		std::cerr << "found leader at " << (leader_loc.x+1) << "," << (leader_loc.y+1) << "\n";
		leader = units.find(leader_loc);
		assert(leader != units.end());
		leader->second.set_leading(true);
	}

	const int begin_at = minimum<int>(-200,attack.get_first_frame());
	const int end_at = maximum<int>((damage+1)*time_resolution,
	                                       maximum<int>(200,attack.get_last_frame()));

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

	for(int i = begin_at; i < end_at; i += time_resolution*acceleration) {
		events::pump();

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && i >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && i >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		for(int j = 0; j != 6; ++j) {
			disp.draw_tile(update_tiles[j].x,update_tiles[j].y);
		}

		Uint32 defender_colour = 0;
		double defender_alpha = 1.0;

		if(damage > 0 && i >= 0 && shown_label == false) {
			shown_label = true;
			disp.float_label(b,lexical_cast<std::string>(damage),255,0,0);
		}

		if(damage > 0 && i >= 0) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defender_alpha = 0.0;
				defender_colour = disp.rgb(200,0,0);
			}

			++flash_num;
		}


		disp.draw_tile(b.x,b.y,NULL,defender_alpha,defender_colour);
		if(leader_loc.valid()) {
			disp.draw_tile(leader_loc.x,leader_loc.y);
		}

		int xoffset = 0;
		const std::string* unit_image = attack.get_frame(i,&xoffset);
		if(!attacker.facing_left())
			xoffset *= -1;

		xoffset = int(double(xoffset)*disp.zoom());

		if(unit_image == NULL) {
			unit_image = &attacker.image();
		}

		scoped_sdl_surface image((unit_image == NULL) ? NULL : image::get_image(*unit_image));
		if(attacker.facing_left() == false) {
			image.assign(image::reverse_image(image));
		}

		const double pos = double(i)/double(i < 0 ? begin_at : end_at);
		const int posx = int(pos*xsrc + (1.0-pos)*xdst) + xoffset;
		const int posy = int(pos*ysrc + (1.0-pos)*ydst);

		const int height_adjust = int(src_height_adjust*pos + dst_height_adjust*(1.0-pos));
		const double submerge = src_submerge*pos + dst_submerge*(1.0-pos);

		if(image != NULL && !hide) {
			disp.draw_unit(posx,posy-height_adjust,image,false,1.0,0,submerge);
		}

		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide) {
			SDL_Delay(wait_time);
		}

		ticks = SDL_GetTicks();

		disp.update_display();
	}

	disp.hide_unit(gamemap::location());

	if(damage > 0 && def->second.gets_hit(damage)) {
		dead = true;
		damage = 0;
	}

	def->second.set_defending(false);

	if(leader_loc.valid()){
		leader->second.set_leading(false);
	}

	disp.draw_tile(a.x,a.y);
	disp.draw_tile(b.x,b.y);
	if(leader_loc.valid()) {
		disp.draw_tile(leader_loc.x,leader_loc.y);
	}

	if(dead) {
		unit_display::unit_die(disp,def->first,def->second);
	}

	return dead;
}

}
