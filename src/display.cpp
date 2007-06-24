/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "actions.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "minimap.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "sound.hpp"
#include "team.hpp"
#include "theme.hpp"
#include "tooltips.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "wassert.hpp"

#include "SDL_image.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#define ERR_DP LOG_STREAM(err, display)
#define INFO_DP LOG_STREAM(info, display)

// Methods for subclass aware only of terrain go here

namespace {
#ifdef USE_TINY_GUI
	const int DefaultZoom = 36;
#else
	const int DefaultZoom = 72;
#endif

	const int MinZoom = 4;
	const int MaxZoom = 200;
	size_t sunset_delay = 0;
	size_t sunset_timer = 0;
}

map_display::map_display(CVideo& video, const gamemap& map, const config& theme_cfg, const config& cfg, const config& level) : 
	screen_(video), map_(map), xpos_(0), ypos_(0),
	theme_(theme_cfg,screen_area()), zoom_(DefaultZoom),
	builder_(cfg, level, map),
	minimap_(NULL), redrawMinimap_(false), redraw_background_(true),
	invalidateAll_(true), 
	fps_handle_(0)
{
	if(non_interactive()) {
		screen_.lock_updates(true);
	}

	image::set_zoom(zoom_);
}

map_display::~map_display()
{
}

const SDL_Rect& map_display::map_area() const 
{
	static SDL_Rect res = {0, 0, 0, 0};
	res = map_outside_area();

	// hex_size() is always a multiple of 4 and hex_width() a multiple of
	// 3 so there shouldn't be off by one errors due to rounding
	// To display a hex fully on screen a little bit extra space is needed
	// Also added 2 hexes for the border.
	const int width = lexical_cast<int>((map_.x() + (7.0/3.0)) * hex_width()); 
	const int height = lexical_cast<int>((map_.y() + 2.5) * hex_size());

	if(width < res.w) {
		// map is smaller, center
		res.x += (res.w - width)/2;
		res.w = width;
	}

	if(height < res.h) {
		// map is smaller, center
		res.y += (res.h - height)/2;
		res.h = height;
	}

	return res;
}

bool map_display::outside_area(const SDL_Rect& area, const int x, const int y) const
{
	const int x_thresh = hex_width();
	const int y_thresh = hex_size();
	return (x < area.x || x > area.x + area.w - x_thresh ||
		y < area.y || y > area.y + area.h - y_thresh);
}

// This function use the screen as reference
const gamemap::location map_display::hex_clicked_on(int xclick, int yclick, 
		gamemap::location::DIRECTION* nearest_hex, 
		gamemap::location::DIRECTION* second_nearest_hex) const
{
	const SDL_Rect& rect = map_area();
	if(point_in_rect(xclick,yclick,rect) == false) {
		return gamemap::location();
	}

	xclick -= rect.x;
	yclick -= rect.y;

	return pixel_position_to_hex(xpos_ + xclick, ypos_ + yclick, nearest_hex, second_nearest_hex);
}


// This function use the rect of map_area as reference
const gamemap::location map_display::pixel_position_to_hex(int x, int y, 
		gamemap::location::DIRECTION* nearest_hex, 
		gamemap::location::DIRECTION* second_nearest_hex) const
{
	// adjust for the 1 hex border
	x -= hex_width() ;
	y -= hex_size();
	const int s = hex_size();
	const int tesselation_x_size = hex_width() * 2;
	const int tesselation_y_size = s;
	const int x_base = x / tesselation_x_size * 2;
	const int x_mod = x % tesselation_x_size;
	const int y_base = y / tesselation_y_size;
	const int y_mod = y % tesselation_y_size;

	int x_modifier = 0;
	int y_modifier = 0;

	if (y_mod < tesselation_y_size / 2) {
		if ((x_mod * 2 + y_mod) < (s / 2)) {
			x_modifier = -1;
			y_modifier = -1;
		} else if ((x_mod * 2 - y_mod) < (s * 3 / 2)) {
			x_modifier = 0;
			y_modifier = 0;
		} else {
			x_modifier = 1;
			y_modifier = -1;
		}

	} else {
		if ((x_mod * 2 - (y_mod - s / 2)) < 0) {
			x_modifier = -1;
			y_modifier = 0;
		} else if ((x_mod * 2 + (y_mod - s / 2)) < s * 2) {
			x_modifier = 0;
			y_modifier = 0;
		} else {
			x_modifier = 1;
			y_modifier = 0;
		}
	}

	const gamemap::location res(x_base + x_modifier, y_base + y_modifier);

	if(nearest_hex != NULL) {
		// our x and y use the map_area as reference
		// and the coorfinates given by get_location use the screen as reference
		// so we need to convert it
		const int centerx = (get_location_x(res) - map_area().x + xpos_) + hex_size()/2 - hex_width();
		const int centery = (get_location_y(res) - map_area().y + ypos_) + hex_size()/2 - hex_size();
		const int x_offset = x - centerx;
		const int y_offset = y - centery;
		if(y_offset > 0) {
			if(x_offset > y_offset/2) {
				*nearest_hex = gamemap::location::SOUTH_EAST;
				if(second_nearest_hex != NULL) {
					if(x_offset/2 > y_offset) {
						*second_nearest_hex = gamemap::location::NORTH_EAST;
					} else {
						*second_nearest_hex = gamemap::location::SOUTH;
					}
				}
			} else if(-x_offset > y_offset/2) {
				*nearest_hex = gamemap::location::SOUTH_WEST;
				if(second_nearest_hex != NULL) {
					if(-x_offset/2 > y_offset) {
						*second_nearest_hex = gamemap::location::NORTH_WEST;
					} else {
						*second_nearest_hex = gamemap::location::SOUTH;
					}
				}
			} else {
				*nearest_hex = gamemap::location::SOUTH;
				if(second_nearest_hex != NULL) {
					if(x_offset > 0) {
						*second_nearest_hex = gamemap::location::SOUTH_EAST;
					} else {
						*second_nearest_hex = gamemap::location::SOUTH_WEST;
					}
				}
			}
		} else { // y_offset <= 0
			if(x_offset > -y_offset/2) {
				*nearest_hex = gamemap::location::NORTH_EAST;
				if(second_nearest_hex != NULL) {
					if(x_offset/2 > -y_offset) {
						*second_nearest_hex = gamemap::location::SOUTH_EAST;
					} else {
						*second_nearest_hex = gamemap::location::NORTH;
					}
				}
			} else if(-x_offset > -y_offset/2) {
				*nearest_hex = gamemap::location::NORTH_WEST;
				if(second_nearest_hex != NULL) {
					if(-x_offset/2 > -y_offset) {
						*second_nearest_hex = gamemap::location::SOUTH_WEST;
					} else {
						*second_nearest_hex = gamemap::location::NORTH;
					}
				}
			} else {
				*nearest_hex = gamemap::location::NORTH;
				if(second_nearest_hex != NULL) {
					if(x_offset > 0) {
						*second_nearest_hex = gamemap::location::NORTH_EAST;
					} else {
						*second_nearest_hex = gamemap::location::NORTH_WEST;
					}
				}
			}
		}
	}

	return res;
}

void map_display::get_rect_hex_bounds(SDL_Rect rect, gamemap::location &topleft, gamemap::location &bottomright) const
{
	// change the coordinates of the rect send to be relative 
	// to the map area instead of the screen area
	const SDL_Rect& map_rect = map_area();
	rect.x -= map_rect.x;
	rect.y -= map_rect.y;
	rect.w += map_rect.x;
	rect.h += map_rect.y;

	const int tile_width = hex_width();

	// adjust for the 1 hex border
	topleft.x  = -1 + (xpos_ + rect.x) / tile_width;
	topleft.y  = -1 + (ypos_ + rect.y - (is_odd(topleft.x) ? zoom_/2 : 0)) / zoom_;

	bottomright.x  = -1 + (xpos_ + rect.x + rect.w) / tile_width;
	bottomright.y  = -1 + ((ypos_ + rect.y + rect.h) - (is_odd(bottomright.x) ? zoom_/2 : 0)) / zoom_;

	// This routine does a rough approximation so might be off by one
	// to be sure enough tiles are incuded the boundries are increased
	// by one if the terrain is "on the map" due to the extra border
	// this uses a bit larger area. 
	// FIXME this routine should properly determine what to update and
	// not increase by one just to be sure.
	if(topleft.x >= -1) {
		topleft.x--;
	}
	if(topleft.y >= -1) {
		topleft.y--;
	}
	if(bottomright.x <= map_.x()) {
		bottomright.x++;
	}
	if(bottomright.y <= map_.y()) {
		bottomright.y++;
	}
}

gamemap::location map_display::minimap_location_on(int x, int y)
{
	const SDL_Rect rect = minimap_area();

	if(x < rect.x || y < rect.y ||
	   x >= rect.x + rect.w || y >= rect.y + rect.h) {
		return gamemap::location();
	}

	const double xdiv = double(rect.w) / double(map_.x());
	const double ydiv = double(rect.h) / double(map_.y());

	return gamemap::location(int((x - rect.x)/xdiv),int((y-rect.y)/ydiv));
}

void map_display::get_visible_hex_bounds(gamemap::location &topleft, gamemap::location &bottomright) const
{
	SDL_Rect r = map_area();
	r.x=0;
	r.y=0;
	get_rect_hex_bounds(r, topleft, bottomright);
}

void map_display::screenshot()
{
	std::string datadir = get_screenshot_dir();
	static unsigned int counter = 0;
	std::string name;

	do {
		std::stringstream filename;

		filename << datadir << "/" << _("Screenshot") << "_";
		filename.width(5);
		filename.fill('0');
		filename.setf(std::ios_base::right);
		filename << counter << ".bmp";

		counter++;
		name = filename.str();

	} while(file_exists(name));

	SDL_SaveBMP(screen_.getSurface().get(), name.c_str());
}

gui::button* map_display::find_button(const std::string& id)
{
	for (size_t i = 0; i < buttons_.size(); ++i) {
		if(buttons_[i].id() == id) {
			return &buttons_[i];
		}
	}
	return NULL;
}

void map_display::create_buttons()
{
	std::vector<gui::button> work;

	const std::vector<theme::menu>& buttons = theme_.menus();
	for(std::vector<theme::menu>::const_iterator i = buttons.begin(); i != buttons.end(); ++i) {
		gui::button b(screen_,i->title(),string_to_button_type(i->type()),i->image());
		b.set_id(i->get_id());
		const SDL_Rect& loc = i->location(screen_area());
		b.set_location(loc.x,loc.y);
		if (!i->tooltip().empty()){
			tooltips::add_tooltip(loc, i->tooltip());
		}
		if(rects_overlap(b.location(),map_area())) {
			b.set_volatile(true);
		}

		gui::button* b_prev = find_button(b.id());
		if(b_prev) b.enable(b_prev->enabled());

		work.push_back(b);
	}

	buttons_.swap(work);
}

gui::button::TYPE map_display::string_to_button_type(std::string type)
{
	gui::button::TYPE res = gui::button::TYPE_PRESS;
	if (type == "checkbox") { res = gui::button::TYPE_CHECK; }
	else if (type == "image") { res = gui::button::TYPE_IMAGE; }
	return res;
}

static const std::string& get_direction(size_t n)
{
	static std::string const dirs[6] = { "-n", "-ne", "-se", "-s", "-sw", "-nw" };
	return dirs[n >= sizeof(dirs)/sizeof(*dirs) ? 0 : n];
}

std::vector<std::string> map_display::get_fog_shroud_graphics(const gamemap::location& loc)
{
	std::vector<std::string> res;

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	t_translation::t_letter tiles[6];
	
	static const t_translation::t_letter terrain_types[] = 
		{ t_translation::FOGGED, t_translation::VOID_TERRAIN, t_translation::NONE_TERRAIN };

	for(int i = 0; i != 6; ++i) {
		if(shrouded(adjacent[i])) {
			tiles[i] = t_translation::VOID_TERRAIN;
		} else if(!fogged(loc) && fogged(adjacent[i])) {
			tiles[i] = t_translation::FOGGED;
		} else {
			tiles[i] = t_translation::NONE_TERRAIN; 
		}
	}


	for(const t_translation::t_letter *terrain = terrain_types; 
			*terrain != t_translation::NONE_TERRAIN; terrain ++) {

		//find somewhere that doesn't have overlap to use as a starting point
		int start;
		for(start = 0; start != 6; ++start) {
			if(tiles[start] != *terrain) {
				break;
			}
		}

		if(start == 6) {
			start = 0;
		}

		//find all the directions overlap occurs from
		for(int i = (start+1)%6, n = 0; i != start && n != 6; ++n) {
			if(tiles[i] == *terrain) {
				std::ostringstream stream;
				std::string name;
				// if(*terrain == terrain_type::VOID_TERRAIN)
				//	stream << "void";
				//else
				//	stream << "fog";
				stream << "terrain/" << map_.get_terrain_info(*terrain).symbol_image();

				for(int n = 0; *terrain == tiles[i] && n != 6; i = (i+1)%6, ++n) {
					stream << get_direction(i);

					if(!image::exists(stream.str() + ".png")) {
						//if we don't have any surface at all,
						//then move onto the next overlapped area
						if(name.empty()) {
							i = (i+1)%6;
						}
						break;
					} else {
						name = stream.str();
					}
				}

				if(!name.empty()) {
					res.push_back(name + ".png");
				}
			} else {
				i = (i+1)%6;
			}
		}
	}

	return res;
}

std::vector<surface> map_display::get_terrain_images(const gamemap::location &loc, 
		const time_of_day& tod,
		image::TYPE image_type, 
		ADJACENT_TERRAIN_TYPE terrain_type)
{
	std::vector<surface> res;

	if(terrain_type == ADJACENT_FOGSHROUD) {
		const std::vector<std::string> fog_shroud = get_fog_shroud_graphics(loc);

		if(!fog_shroud.empty()) {
			for(std::vector<std::string>::const_iterator it = fog_shroud.begin(); it != fog_shroud.end(); ++it) {
				image::locator image(*it);
				// image.filename = "terrain/" + *it;

				const surface surface(image::get_image(image, image_type));
				if (!surface.null()) {
					res.push_back(surface);
				}
			}

		}

		return res;
	}

	terrain_builder::ADJACENT_TERRAIN_TYPE builder_terrain_type =
	      (terrain_type == ADJACENT_FOREGROUND ?
		  terrain_builder::ADJACENT_FOREGROUND : terrain_builder::ADJACENT_BACKGROUND);
	const terrain_builder::imagelist* const terrains = builder_.get_terrain_at(loc,
			tod.id, builder_terrain_type);

	if(terrains != NULL) {
		for(std::vector<animated<image::locator> >::const_iterator it = terrains->begin(); it != terrains->end(); ++it) {
			// it->update_current_frame();
			image::locator image = it->get_current_frame();
			// image.filename = "terrain/" + image.filename;

			const surface surface(image::get_image(image, image_type));
			if (!surface.null()) {
				res.push_back(surface);
			}
		}
	} else if(terrain_type == ADJACENT_BACKGROUND){
		// this should only happen with the off map tiles and now 
		// return the void image. NOTE this is a temp hack
		const surface surface(image::get_image("terrain/off-map/alpha.png"));
		wassert(!surface.null());
		if (!surface.null()) {
			res.push_back(surface);
		}
	}

	return res;
}

void map_display::draw_terrain_on_tile(const gamemap::location& loc, 
				       const time_of_day& tod,
				       image::TYPE image_type, 
				       ADJACENT_TERRAIN_TYPE type)
{
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	SDL_Rect clip_rect = map_area();

	if(xpos > clip_rect.x + clip_rect.w || ypos > clip_rect.y + clip_rect.h ||
	   xpos + zoom_ < clip_rect.x || ypos + zoom_ < clip_rect.y) {
		return;
	}

	surface const dst(screen_.getSurface());

	clip_rect_setter set_clip_rect(dst,clip_rect);

	const std::vector<surface>& images = get_terrain_images(loc,tod,image_type,type);

	std::vector<surface>::const_iterator itor;
	for(itor = images.begin(); itor != images.end(); ++itor) {
		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(*itor,NULL,dst,&dstrect);
	}
}

void map_display::sunset(const size_t delay) {
	// this allow both parametric and toggle use
	sunset_delay = (sunset_delay == 0 && delay == 0) ? 5 : delay;
}

void map_display::flip()
{
	if(video().faked()) {
		return;
	}

	const surface frameBuffer = get_video_surface();

	// this is just the debug function "sunset" to progressively darken the map area
	if (sunset_delay && ++sunset_timer > sunset_delay) {
		sunset_timer = 0;
		SDL_Rect r = map_area(); //use frameBuffer to also test the UI
		const Uint32 color =  SDL_MapRGBA(video().getSurface()->format,0,0,0,255);
		// adjust the alpha if you want to balance cpu-cost / smooth sunset
		fill_rect_alpha(r, color, 1, frameBuffer);
		update_rect(r);
	}

	font::draw_floating_labels(frameBuffer);
	events::raise_volatile_draw_event();
	cursor::draw(frameBuffer);

	video().flip();

	cursor::undraw(frameBuffer);
	events::raise_volatile_undraw_event();
	font::undraw_floating_labels(frameBuffer);
}

void map_display::update_display()
{
	if(screen_.update_locked()) {
		return;
	}

	if(preferences::show_fps()) {
		static int last_sample = SDL_GetTicks();
		static int frames = 0;
		++frames;

		if(frames == 10) {
			const int this_sample = SDL_GetTicks();

			const int fps = (frames*1000)/(this_sample - last_sample);
			last_sample = this_sample;
			frames = 0;

			if(fps_handle_ != 0) {
				font::remove_floating_label(fps_handle_);
				fps_handle_ = 0;
			}
			std::ostringstream stream;
			stream << fps << "fps";
			fps_handle_ = font::add_floating_label(stream.str(),12,font::NORMAL_COLOUR,10,100,0,0,-1,screen_area(),font::LEFT_ALIGN);
		}
	} else if(fps_handle_ != 0) {
		font::remove_floating_label(fps_handle_);
		fps_handle_ = 0;
	}

	flip();
}

static void draw_panel(CVideo& video, const theme::panel& panel, std::vector<gui::button>& buttons)
{
	//log_scope("draw panel");
	surface surf(image::get_image(panel.image(),image::UNSCALED));

	const SDL_Rect screen = screen_area();
	SDL_Rect& loc = panel.location(screen);
	if(!surf.null()) {
		if(surf->w != loc.w || surf->h != loc.h) {
			surf.assign(scale_surface(surf,loc.w,loc.h));
		}

		video.blit_surface(loc.x,loc.y,surf);
		update_rect(loc);
	}

	static bool first_time = true;
	for(std::vector<gui::button>::iterator b = buttons.begin(); b != buttons.end(); ++b) {
		if(rects_overlap(b->location(),loc)) {
			b->set_dirty(true);
			if (first_time){
				//FixMe
				//YogiHH: This is only made to have the buttons store their background information,
				//otherwise the background will appear completely black. It would more
				//straightforward to call bg_update, but that is not public and there seems to be
				//no other way atm to call it. I will check if bg_update can be made public.
				b->hide(true);
				b->hide(false);
			}
		}
	}
}

static void draw_label(CVideo& video, surface target, const theme::label& label)
{
	//log_scope("draw label");

        std::stringstream temp;
	Uint32 RGB=label.font_rgb();
        int red = (RGB & 0x00FF0000)>>16;
        int green = (RGB & 0x0000FF00)>>8;
        int blue = (RGB & 0x000000FF);

        std::string c_start="<";
        std::string c_sep=",";
        std::string c_end=">";
        std::stringstream color;
        color<< c_start << red << c_sep << green << c_sep << blue << c_end;
        std::string text = label.text();

        if(label.font_rgb_set()) {
		color<<text;
		text = color.str();
        }
	const std::string& icon = label.icon();
	SDL_Rect& loc = label.location(screen_area());

	if(icon.empty() == false) {
		surface surf(image::get_image(icon,image::UNSCALED));
		if(!surf.null()) {
			if(surf->w > loc.w || surf->h > loc.h) {
				surf.assign(scale_surface(surf,loc.w,loc.h));
			}

			SDL_BlitSurface(surf,NULL,target,&loc);
		}

		if(text.empty() == false) {
			tooltips::add_tooltip(loc,text);
		}
	} else if(text.empty() == false) {
		font::draw_text(&video,loc,label.font_size(),font::NORMAL_COLOUR,text,loc.x,loc.y);
	}

	update_rect(loc);
}

void map_display::draw_all_panels()
{
	surface const screen(screen_.getSurface());

	const std::vector<theme::panel>& panels = theme_.panels();
	for(std::vector<theme::panel>::const_iterator p = panels.begin(); p != panels.end(); ++p) {
		draw_panel(video(),*p,buttons_);
	}

	const std::vector<theme::label>& labels = theme_.labels();
	for(std::vector<theme::label>::const_iterator i = labels.begin(); i != labels.end(); ++i) {
		draw_label(video(),screen,*i);
	}

	create_buttons();
}

/**
 * Proof-of-concept of the new background still has some flaws
 * * upon scrolling the background static (so maps scrolls over wood)
 * * _off^usr has redraw glitches since background is only updated every now and then
 * * the alpha at the border tends to "build up"
 * * we impose a huge performance hit
 *
 * needs quite some work to become fully working, but first evaluate whether
 * the new way is really wanted.
 */
static void draw_background(surface screen, const SDL_Rect& area)
{
	static const surface wood(image::get_image("terrain/off-map/wood.png",image::UNSCALED));
	static const unsigned int width = wood->w;
	static const unsigned int height = wood->h;
	wassert(!wood.null());

	const unsigned int w_count = static_cast<int>(ceil(static_cast<double>(area.w) / static_cast<double>(width)));
	const unsigned int h_count = static_cast<int>(ceil(static_cast<double>(area.h) / static_cast<double>(height)));

	for(unsigned int w = 0, w_off = area.x; w < w_count; ++w, w_off += width) {
		for(unsigned int h = 0, h_off = area.y; h < h_count; ++h, h_off += height) {
			SDL_Rect clip = {w_off, h_off, 0, 0};
			SDL_BlitSurface(wood, NULL, screen, &clip);
		}
	}
}

// Methods for superclass aware of units go here

std::map<gamemap::location,fixed_t> display::debugHighlights_;

display::display(unit_map& units, CVideo& video, const gamemap& map,
		const gamestatus& status, const std::vector<team>& t,
		const config& theme_cfg, const config& cfg, const config& level) :
	map_display(video, map, theme_cfg, cfg, level),
	_scroll_event("scrolled"),
	units_(units),
	temp_unit_(NULL),
	status_(status),
	teams_(t), nextDraw_(0),
	invalidateUnit_(true),
	invalidateGameStatus_(true), panelsDrawn_(false),
	currentTeam_(0), activeTeam_(0),
	turbo_speed_(2), turbo_(false), grid_(false), sidebarScaling_(1.0),
	first_turn_(true), in_game_(false), map_labels_(*this,map, 0),
	tod_hex_mask1(NULL), tod_hex_mask2(NULL), reach_map_changed_(true),
	diagnostic_label_(0)
{
	singleton_ = this;
	std::fill(reportRects_,reportRects_+reports::NUM_REPORTS,empty_rect);

	//inits the flag list
	flags_.reserve(teams_.size());
	for(size_t i = 0; i != teams_.size(); ++i) {
		std::string flag = teams_[i].flag();
		std::string old_rgb = game_config::flag_rgb;
		std::string new_rgb = team::get_side_colour_index(i+1);

		if(flag.empty()) {
			flag = game_config::flag_image;
		}

		LOG_STREAM(info, display) << "Adding flag for team " << i << " from animation " << flag << "\n";

		//must recolor flag image
		animated<image::locator> temp_anim;

		std::vector<std::string> items = utils::split(flag);
		std::vector<std::string>::const_iterator itor = items.begin();
		for(; itor != items.end(); ++itor) {
			const std::vector<std::string>& items = utils::split(*itor, ':');
			std::string str;
			int time;

			if(items.size() > 1) {
				str = items.front();
				time = atoi(items.back().c_str());
			} else {
				str = *itor;
				time = 100;
			}
			std::stringstream temp;
			temp << str << "~RC(" << old_rgb << ">"<< new_rgb << ")";
			image::locator flag_image(temp.str());
			temp_anim.add_frame(time, flag_image);
		}
		flags_.push_back(temp_anim);

		flags_.back().start_animation(rand()%flags_.back().get_end_time(), true);
	}

	//clear the screen contents
	surface const disp(screen_.getSurface());
	SDL_Rect area = screen_area();
	SDL_FillRect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
}

display::~display()
{
	// SDL_FreeSurface(minimap_);
	prune_chat_messages(true);
	singleton_ = NULL;
}

void display::new_turn()
{
	const time_of_day& tod = status_.get_time_of_day();

	if( !first_turn_) {
		image::set_image_mask("");

		const time_of_day& old_tod = status_.get_previous_time_of_day();

		if(old_tod.image_mask != tod.image_mask) {
			const surface old_mask(image::get_image(old_tod.image_mask,image::UNMASKED));
			const surface new_mask(image::get_image(tod.image_mask,image::UNMASKED));

			const int niterations = (int)(10/turbo_speed());
			const int frame_time = 30;
			const int starting_ticks = SDL_GetTicks();
			for(int i = 0; i != niterations; ++i) {

				if(old_mask != NULL) {
					const fixed_t proportion = ftofxp(1.0) - fxpdiv(i,niterations);
					tod_hex_mask1.assign(adjust_surface_alpha(old_mask,proportion));
				}

				if(new_mask != NULL) {
					const fixed_t proportion = fxpdiv(i,niterations);
					tod_hex_mask2.assign(adjust_surface_alpha(new_mask,proportion));
				}

				invalidate_all();
				draw();

				const int cur_ticks = SDL_GetTicks();
				const int wanted_ticks = starting_ticks + i*frame_time;
				if(cur_ticks < wanted_ticks) {
					SDL_Delay(wanted_ticks - cur_ticks);
				}
			}
		}

		tod_hex_mask1.assign(NULL);
		tod_hex_mask2.assign(NULL);
	}

	first_turn_ = false;

	image::set_colour_adjustment(tod.red,tod.green,tod.blue);
	image::set_image_mask(tod.image_mask);

	invalidate_all();
	draw();
}

void display::adjust_colours(int r, int g, int b)
{
	const time_of_day& tod = status_.get_time_of_day();
	image::set_colour_adjustment(tod.red+r,tod.green+g,tod.blue+b);
}

void display::select_hex(gamemap::location hex)
{
	if(fogged(hex)) {
		return;
	}

	invalidate(selectedHex_);
	selectedHex_ = hex;
	invalidate(selectedHex_);
	invalidate_unit();
}

void display::highlight_hex(gamemap::location hex)
{
	const int has_unit = units_.count(mouseoverHex_) + units_.count(hex);

	invalidate(mouseoverHex_);
	mouseoverHex_ = hex;
	invalidate(mouseoverHex_);
	invalidate_game_status();

	if(has_unit) {
		invalidate_unit();
	}
}

void display::scroll(int xmove, int ymove)
{
	const int orig_x = xpos_;
	const int orig_y = ypos_;
	xpos_ += xmove;
	ypos_ += ymove;
	bounds_check_position();
	const int dx = orig_x - xpos_; // dx = -xmove
	const int dy = orig_y - ypos_; // dy = -ymove

	//only invalidate if we've actually moved
	if(dx == 0 && dy == 0)
		return;

	map_labels_.scroll(dx, dy);
	font::scroll_floating_labels(dx, dy);
	
	surface screen(screen_.getSurface());
	
	SDL_Rect dstrect = map_area();
	dstrect.x += dx;
	dstrect.y += dy;
	dstrect = intersect_rects(dstrect, map_area());

	SDL_Rect srcrect = dstrect;
	srcrect.x -= dx;
	srcrect.y -= dy;
	
	SDL_BlitSurface(screen,&srcrect,screen,&dstrect);
	
	//invalidate locations in the newly visible rects

	if (dy != 0) {
		SDL_Rect r = map_area();
		r.x = 0;
		r.y = dy < 0 ? r.h+dy : 0;
		r.h = abs(dy);
		invalidate_locations_in_rect(r);
	}
	if (dx != 0) {
		SDL_Rect r = map_area();
		r.x = dx < 0 ? r.w+dx : 0;
		r.y = 0;
		r.w = abs(dx);
		invalidate_locations_in_rect(r);
	}

	_scroll_event.notify_observers();
	update_rect(map_area());

	redraw_background_ = true;
	redrawMinimap_ = true;
}

void display::invalidate_locations_in_rect(SDL_Rect r)
{
	gamemap::location topleft, bottomright;
	get_rect_hex_bounds(r, topleft, bottomright);
	for (int x = topleft.x; x <= bottomright.x; ++x) {
		for (int y = topleft.y; y <= bottomright.y; ++y) {
			gamemap::location loc(x, y);
			invalidate(loc);
		}
	}
}

void display::set_zoom(int amount)
{
	int new_zoom = zoom_ + amount;
	if (new_zoom < MinZoom) {
		new_zoom = MinZoom;
	}
	if (new_zoom > MaxZoom) {
		new_zoom = MaxZoom;
	}
	if (new_zoom != zoom_) {
		SDL_Rect const &area = map_area();
		xpos_ += (xpos_ + area.w / 2) * amount / zoom_;
		ypos_ += (ypos_ + area.h / 2) * amount / zoom_;
		zoom_ = new_zoom;
		bounds_check_position();

		energy_bar_rects_.clear();
		image::set_zoom(zoom_);
		map_labels_.recalculate_labels();
		redraw_background_ = true;
		invalidate_all();

		// Forces a redraw after zooming. This prevents some graphic glitches from occurring.
		draw();
	}
}

void display::set_default_zoom()
{ 
	set_zoom(DefaultZoom - zoom_); 
}

void display::scroll_to_tile(const gamemap::location& loc, SCROLL_TYPE scroll_type, bool check_fogged)
{
	if(screen_.update_locked() || (check_fogged && fogged(loc))) {
		return;
	}

	if(map_.on_board(loc) == false) {
		return;
	}

	// current position of target (upper left tile corner) in screen coordinates
	const int screenxpos = get_location_x(loc);
	const int screenypos = get_location_y(loc);

	if (scroll_type == ONSCREEN) {
		// the tile must be fully visible
		SDL_Rect r = map_area();
		r.w -= hex_width();
		r.h -= zoom_;

		if (!outside_area(r,screenxpos,screenypos)) {
			return;
		}
	}

	const SDL_Rect area = map_area();
	const int xmove_expected = (screenxpos + hex_width()/2) - (area.x + area.w/2 - zoom_/2);
	const int ymove_expected = (screenypos + zoom_/2)       - (area.y + area.h/2 - zoom_/2);

	int xpos = xpos_ + xmove_expected;
	int ypos = ypos_ + ymove_expected;
	bounds_check_position(xpos, ypos);
	int xmove = xpos - xpos_;
	int ymove = ypos - ypos_;

	if(scroll_type == WARP ) {
		scroll(xmove,ymove);
		draw();
		return;
	}

	// doing an animated scroll, with acceleration etc.

	int x_old = 0;
	int y_old = 0;

	const double dist_total = hypot(xmove, ymove);
	double dist_moved = 0.0;

	int t_prev = SDL_GetTicks();
	
	double velocity = 0.0;
	while (dist_moved < dist_total) {
		events::pump();
		
		int t = SDL_GetTicks();
		double dt = (t - t_prev) / 1000.0;
		if (dt > 0.200) {
			// do not skip too many frames on slow pcs
			dt = 0.200;
		}
		t_prev = t;

		//std::cout << t << " " << hypot(x_old, y_old) << "\n";

		// those values might need some fine-tuning:
		const double accel_time = 0.3 / turbo_speed(); // seconds until full speed is reached 
		const double decel_time = 0.4 / turbo_speed(); // seconds from full speed to stop

		double velocity_max = preferences::scroll_speed() * 60.0;
		velocity_max *= turbo_speed();
		double accel = velocity_max / accel_time;
		double decel = velocity_max / decel_time;

		// If we started to decelerate now, where would we stop?
		double stop_time = velocity / decel;
		double dist_stop = dist_moved + velocity*stop_time - 0.5*decel*stop_time*stop_time;
		if (dist_stop > dist_total || velocity > velocity_max) {
			velocity -= decel * dt;
			if (velocity < 1.0) velocity = 1.0;
		} else {
			velocity += accel * dt;
			if (velocity > velocity_max) velocity = velocity_max;
		}

		dist_moved += velocity * dt;
		if (dist_moved > dist_total) dist_moved = dist_total;

		int x_new = round_double(xmove * dist_moved / dist_total);
		int y_new = round_double(ymove * dist_moved / dist_total);

		int dx = x_new - x_old;
		int dy = y_new - y_old;

		scroll(dx,dy);
		x_old += dx;
		y_old += dy;
		draw();
	}
}

void display::scroll_to_tiles(const gamemap::location& loc1, const gamemap::location& loc2,
                              SCROLL_TYPE scroll_type, bool check_fogged)
{
	const int xpos1 = get_location_x(loc1);
	const int ypos1 = get_location_y(loc1);
	const int xpos2 = get_location_x(loc2);;
	const int ypos2 = get_location_y(loc2);;

	const int minx = minimum<int>(xpos1,xpos2);
	const int maxx = maximum<int>(xpos1,xpos2);
	const int miny = minimum<int>(ypos1,ypos2);
	const int maxy = maximum<int>(ypos1,ypos2);
	const int diffx = maxx - minx;
	const int diffy = maxy - miny;

	// if rectangle formed by corners loc1 and loc2 is larger
	// than map area then just scroll to loc1
	if(diffx > map_area().w || diffy > map_area().h) {
		scroll_to_tile(loc1,scroll_type,check_fogged);
	} else {
		// only scroll if rectangle is not completely inside map area
		// assume most paths are within rectangle, sometimes
		// with rugged terrain this is not true -- but use common
		// cases to determine behaviour instead of exceptions
		if (outside_area(map_area(),minx,miny) ||
		    outside_area(map_area(),maxx,maxy)) {
			// scroll to middle point of rectangle
			scroll_to_tile(gamemap::location((loc1.x+loc2.x)/2,(loc1.y+loc2.y)/2),scroll_type,check_fogged);
		} // else don't scroll, rectangle is already on screen
	}
}

void display::scroll_to_leader(unit_map& units, int side)
{
	const unit_map::iterator leader = find_leader(units,side);

	if(leader != units_.end()) {
		/* YogiHH: I can't see why we need another key_handler here, therefore
		i will comment it out
		const hotkey::basic_handler key_events_handler(gui_);
		*/
		scroll_to_tile(leader->first, ONSCREEN);
	}
}

void display::bounds_check_position()
{
	const int orig_zoom = zoom_;

	if(zoom_ < MinZoom) {
		zoom_ = MinZoom;
	}

	if(zoom_ > MaxZoom) {
		zoom_ = MaxZoom;
	}

	bounds_check_position(xpos_, ypos_);

	if(zoom_ != orig_zoom) {
		image::set_zoom(zoom_);
	}
}

void display::bounds_check_position(int& xpos, int& ypos)
{
	const int tile_width = hex_width();

	// adjust for the border 2 times 1 hex
	const int xend = tile_width * (map_.x() + 2) + tile_width/3;
	const int yend = zoom_ * (map_.y() + 2) + zoom_/2;

	if(xpos > xend - map_area().w) {
		xpos = xend - map_area().w;
	}

	if(ypos > yend - map_area().h) {
		ypos = yend - map_area().h;
	}

	if(xpos < 0) {
		xpos = 0;
	}

	if(ypos < 0) {
		ypos = 0;
	}
}

void display::redraw_everything()
{
	if(screen_.update_locked() || teams_.empty())
		return;

	bounds_check_position();

	invalidateGameStatus_ = true;

	for(size_t n = 0; n != reports::NUM_REPORTS; ++n) {
		reportRects_[n] = empty_rect;
		reportSurfaces_[n].assign(NULL);
		reports_[n] = reports::report();
	}

	tooltips::clear_tooltips();

	theme_.set_resolution(screen_area());

	if(buttons_.empty() == false) {
		create_buttons();
	}

	panelsDrawn_ = false;

	map_labels_.recalculate_labels();

	redraw_background_ = true;

	invalidate_all();
	draw(true,true);
}

void display::draw(bool update,bool force)
{
	bool changed = false;

	//log_scope("Drawing");
	invalidate_animations();

	process_reachmap_changes();

	if(!panelsDrawn_) {
		draw_all_panels();
		panelsDrawn_ = true;
		changed = true;
	}

	if(redraw_background_) {
		// full redraw of the background
		const SDL_Rect clip_rect = map_outside_area();
		const surface outside_surf(screen_.getSurface());
		clip_rect_setter set_clip_rect(outside_surf, clip_rect);
		draw_background(outside_surf, clip_rect);

		redraw_background_ = false;

		// force a full map redraw
		invalidateAll_ = true;
	}

	if(invalidateAll_ && !map_.empty()) {
		INFO_DP << "draw() with invalidateAll\n";
		gamemap::location topleft;
		gamemap::location bottomright;
		get_visible_hex_bounds(topleft, bottomright);
		for(int x = topleft.x; x <= bottomright.x; ++x)
			for(int y = topleft.y; y <= bottomright.y; ++y)
				invalidated_.insert(gamemap::location(x,y));
		invalidateAll_ = false;

		redrawMinimap_ = true;
	}

	int simulate_delay = 0;
	if(!map_.empty() && !invalidated_.empty()) {
		changed = true;
		
		halo::unrender(invalidated_);

		// Units can overlap multiple hexes, so we need to (1)
		// redraw them last, and (2) redraw them if they are
		// adjacent existing hexes.
		std::set<gamemap::location> unit_invals;

		SDL_Rect clip_rect = map_area();
		surface const dst(screen_.getSurface());
		clip_rect_setter set_clip_rect(dst, clip_rect);

		std::set<gamemap::location>::const_iterator it;
		for(it = invalidated_.begin(); it != invalidated_.end(); ++it) {
			if (units_.find(*it) != units_.end()) {
				unit_invals.insert(*it);
			}
			const time_of_day& tod = status_.get_time_of_day();
			const time_of_day& tod_at = timeofday_at(status_,units_,*it,map_);
			image::TYPE image_type = image::SCALED_TO_HEX;


			unit_map::iterator un = find_visible_unit(units_, *it, map_, 
							teams_,teams_[currentTeam_]);

			if(*it == mouseoverHex_ && map_.on_board(mouseoverHex_) ||
			   *it == selectedHex_ && (un != units_.end())) {
				image_type = image::BRIGHTENED;
			}
			else if (highlighted_locations_.find(*it) != highlighted_locations_.end()) {
				image_type = image::SEMI_BRIGHTENED;
			}

			draw_tile(*it, tod, tod_at, image_type, clip_rect);
			simulate_delay += 1;
		}

		for(it = unit_invals.begin(); it != unit_invals.end(); ++it) {
			unit &u = units_.find(*it)->second;
			u.redraw_unit(*this, *it);
			simulate_delay += 1;
		}
		if (temp_unit_ && invalidated_.find(temp_unit_loc_) != invalidated_.end()) {
			temp_unit_->redraw_unit(*this, temp_unit_loc_);
		}

		halo::render();
		
		invalidated_.clear();
	} else if (!map_.empty()) {
		// if no hexes are invalidated we still need to update the haloes since
		// there might be animated or expired haloes
		wassert(invalidated_.empty());
		halo::unrender(invalidated_);
		halo::render();
	}

	if(redrawMinimap_) {
		redrawMinimap_ = false;
		const SDL_Rect area = minimap_area();
		draw_minimap(area.x,area.y,area.w,area.h);
		changed = true;
	}


	if(!map_.empty()) {
		draw_sidebar();
		changed = true;
	}

	prune_chat_messages();

	static const int time_between_draws = preferences::draw_delay();
	const int current_time = SDL_GetTicks();
	const int wait_time = nextDraw_ - current_time;

	// simulate slow pc
	//SDL_Delay(2*simulate_delay + rand() % 20);

	if(update) {
		if(force || changed) {
			if(!force && wait_time > 0) {
				// if it's not time yet to draw delay until it is
				SDL_Delay(wait_time);
			}
			update_display();
		}
		
		// set the theortical next draw time
		nextDraw_ += time_between_draws;
		
		// if the next draw already should have been finished we'll enter an
		// update frenzy, so make sure that the too late value doesn't keep
		// growing. Note if force is used too often we can also get the 
		// opposite effect.
		nextDraw_ = maximum<int>(nextDraw_, SDL_GetTicks());
	}
}

void display::draw_sidebar()
{
	draw_report(reports::REPORT_CLOCK);
	draw_report(reports::REPORT_COUNTDOWN);

	if(teams_.empty()) {
		return;
	}

	if(invalidateUnit_) {
		//we display the unit the mouse is over if it is over a unit
		//otherwise we display the unit that is selected
		unit_map::const_iterator i =
			find_visible_unit(units_,mouseoverHex_,
					map_,
					teams_,teams_[viewing_team()]);

		if(i == units_.end()) {
			i = find_visible_unit(units_,selectedHex_,
					map_,
					teams_,teams_[viewing_team()]);
		}

		if(i != units_.end()) {
			for(size_t r = reports::UNIT_REPORTS_BEGIN; r != reports::UNIT_REPORTS_END; ++r) {
				draw_report(reports::TYPE(r));
			}
		}

		invalidateUnit_ = false;
	}

	if(invalidateGameStatus_) {
		draw_game_status();
		invalidateGameStatus_ = false;
	}
}

void display::draw_game_status()
{
	if(teams_.empty()) {
		return;
	}

	for(size_t r = reports::STATUS_REPORTS_BEGIN; r != reports::STATUS_REPORTS_END; ++r) {
		draw_report(reports::TYPE(r));
	}

	// the mouse-over needs to update the visibility icon
	draw_report(reports::UNIT_STATUS);
}

void display::draw_image_for_report(surface& img, SDL_Rect& rect)
{
	SDL_Rect visible_area = get_non_transparent_portion(img);
	SDL_Rect target = rect;
	if(visible_area.x != 0 || visible_area.y != 0 || visible_area.w != img->w || visible_area.h != img->h) {
		if(visible_area.w == 0 || visible_area.h == 0) {
			return;
		}

		if(visible_area.w > rect.w || visible_area.h > rect.h) {
			img.assign(get_surface_portion(img,visible_area));
			img.assign(scale_surface(img,rect.w,rect.h));
			visible_area.x = 0;
			visible_area.y = 0;
			visible_area.w = img->w;
			visible_area.h = img->h;
		} else {
			target.x = rect.x + (rect.w - visible_area.w)/2;
			target.y = rect.y + (rect.h - visible_area.h)/2;
			target.w = visible_area.w;
			target.h = visible_area.h;
		}

		SDL_BlitSurface(img,&visible_area,screen_.getSurface(),&target);
	} else {
		if(img->w != rect.w || img->h != rect.h) {
			img.assign(scale_surface(img,rect.w,rect.h));
		}

		SDL_BlitSurface(img,NULL,screen_.getSurface(),&target);
	}
}

void display::draw_report(reports::TYPE report_num)
{
	if(!team_valid()) {
		return;
	}

	const theme::status_item* const item = theme_.get_status_item(reports::report_name(report_num));
	if(item != NULL) {

		reports::report report = reports::generate_report(report_num,map_,
				units_, teams_,
		      teams_[viewing_team()],
				size_t(currentTeam_+1),size_t(activeTeam_+1),
				selectedHex_,mouseoverHex_,status_,observers_);

		SDL_Rect& rect = reportRects_[report_num];
		const SDL_Rect& new_rect = item->location(screen_area());

		//report and its location is unchanged since last time. Do nothing.
		if(rect == new_rect && reports_[report_num] == report) {
			return;
		}

		reports_[report_num] = report;

		surface& surf = reportSurfaces_[report_num];

		if(surf != NULL) {
			SDL_BlitSurface(surf,NULL,screen_.getSurface(),&rect);
			update_rect(rect);
		}
		//if the rectangle has just changed, assign the surface to it
		if(new_rect != rect || surf == NULL) {
			surf.assign(NULL);
			rect = new_rect;

			//if the rectangle is present, and we are blitting text, then
			//we need to backup the surface. (Images generally won't need backing
			//up unless they are transperant, but that is done later)
			if(rect.w > 0 && rect.h > 0) {
				surf.assign(get_surface_portion(screen_.getSurface(),rect));
				if(reportSurfaces_[report_num] == NULL) {
					ERR_DP << "Could not backup background for report!\n";
				}
			}

			update_rect(rect);
		}

		tooltips::clear_tooltips(rect);

		SDL_Rect area = rect;

		int x = rect.x, y = rect.y;

		if(!report.empty()) {
			// Add prefix, postfix elements. Make sure that they get the same tooltip as the guys
			// around them.
			std::stringstream temp;
			Uint32 RGB = item->font_rgb();
			int red = (RGB & 0x00FF0000)>>16;
			int green = (RGB & 0x0000FF00)>>8;
			int blue = (RGB & 0x000000FF);

			std::string c_start="<";
			std::string c_sep=",";
			std::string c_end=">";
			std::stringstream color;
			color<< c_start << red << c_sep << green << c_sep << blue << c_end;
			std::string str;

			str = item->prefix();
			if(str.empty() == false) {
			  report.insert(report.begin(), reports::element(str,"",report.begin()->tooltip));
			}
			str = item->postfix();
			if(str.empty() == false) {
			  report.push_back(reports::element(str,"",report.end()->tooltip));
			}
			// Loop through and display each report element
			size_t tallest = 0;
			int image_count = 0;
			bool used_ellipsis=false;
			std::stringstream ellipsis_tooltip;
			SDL_Rect ellipsis_area =rect;
			for(reports::report::iterator i = report.begin(); i != report.end(); ++i) {
			  temp.str("");
				if(i->text.empty() == false) {
				  if(used_ellipsis == false) {
					// Draw a text element
				        if(item->font_rgb_set()) {
						temp <<color.str();
					}
					temp << i->text;
					str = temp.str();
					area = font::draw_text(&screen_,rect,item->font_size(),font::NORMAL_COLOUR,str,x,y);
					if(area.h > tallest) {
						tallest = area.h;
					}
					if(i->text[i->text.size() - 1] == '\n') {
						x = rect.x;
						y += tallest;
						tallest = 0;
					} else {
						x += area.w;
					}
				  }
				} else if(i->image.get_filename().empty() == false) {
				  if(used_ellipsis == false) {
					// Draw an image element
					surface img(image::get_image(i->image,image::UNSCALED));

					if(report_num == reports::TIME_OF_DAY && img != NULL && preferences::flip_time()) {
						img = flip_surface(img);
					}

					if(img == NULL) {
						ERR_DP << "could not find image for report: '" << i->image.get_filename() << "'\n";
						continue;
					}

					if(rect.w + rect.x - x < img->w && image_count) {
					  //we have more than one image, and this one doesn't fit.
					  img=surface(image::get_image(game_config::ellipsis_image,image::UNSCALED));
					  used_ellipsis=true;
					}

					area.x = x;
					area.y = y;
					area.w = minimum<int>(rect.w + rect.x - x, img->w);
					area.h = minimum<int>(rect.h + rect.y - y, img->h);
					draw_image_for_report(img, area);

					// draw illuminated time
					if(report_num == reports::TIME_OF_DAY && img != NULL) {
						time_of_day tod = timeofday_at(status_,units_,mouseoverHex_,map_);
						// don't show illuminated time on fogged/shrouded tiles
						if (teams_[viewing_team()].fogged(mouseoverHex_.x, mouseoverHex_.y) || 
								teams_[viewing_team()].shrouded(mouseoverHex_.x, mouseoverHex_.y)) {

							tod = status_.get_time_of_day(false,mouseoverHex_);
						}
						if(tod.bonus_modified > 0) {
							surface tod_bright(image::get_image(game_config:: tod_bright_image,image::UNSCALED));
							if(tod_bright != NULL) {
								draw_image_for_report(tod_bright,area);
//								std::stringstream mod_str;
//								mod_str << "+" << tod.bonus_modified << "%";
//								font::draw_text(&screen_,rect,item->font_size(),font::DARK_COLOUR,mod_str.str(),area.x+2,area.y+2);
							}
						}
					}

					image_count++;
					if(area.h > tallest) {
						tallest = area.h;
					}

					if(! used_ellipsis) {
						x += area.w;
					} else {
						ellipsis_area = area;
					}
				  }
				} else {
					// No text or image, skip this element
					continue;
				}
				if(i->tooltip.empty() == false) {
					if(! used_ellipsis) {
						tooltips::add_tooltip(area,i->tooltip);
					} else { //collect all tooltips for the ellipsis
						ellipsis_tooltip<<i->tooltip<<"\n";
					}
				}
			}
			if(used_ellipsis) {
				tooltips::add_tooltip(ellipsis_area,ellipsis_tooltip.str());
			}
		}
	} else {
		reportSurfaces_[report_num].assign(NULL);
	}
}

void display::draw_minimap(int x, int y, int w, int h)
{
	const surface surf(get_minimap(w,h));
	if(surf == NULL) {
		return;
	}

	SDL_Rect minimap_location = {x,y,w,h};

	clip_rect_setter clip_setter(video().getSurface(),minimap_location);

	SDL_Rect loc = minimap_location;
	SDL_BlitSurface(surf,NULL,video().getSurface(),&loc);

	int map_w = map_.x(), map_h = map_.y();

	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
		if(fogged(u->first) ||
				(teams_[currentTeam_].is_enemy(u->second.side()) &&
				u->second.invisible(u->first,units_,teams_))) {
			continue;
		}

		const int side = u->second.side();
		const SDL_Color col = team::get_side_colour(side);
		const Uint32 mapped_col = SDL_MapRGB(video().getSurface()->format,col.r,col.g,col.b);
		SDL_Rect rect = { x + (u->first.x * w) / map_w,
		                  y + (u->first.y * h + (is_odd(u->first.x) ? h / 2 : 0)) / map_h,
		                  w / map_w, h / map_h };
		SDL_FillRect(video().getSurface(),&rect,mapped_col);
	}

	const double xscaling = double(surf->w) / map_w;
	const double yscaling = double(surf->h) / map_h;

	const int tile_width = hex_width();

	const int xbox = static_cast<int>(xscaling*(xpos_-tile_width)/tile_width);
	const int ybox = static_cast<int>(yscaling*(ypos_-zoom_)/zoom_);

	// The magic numbers experimental determined like in display::map_area()
	const int wbox = static_cast<int>(xscaling*(map_area().w+(4.0/3.0)*tile_width)/tile_width - xscaling);
	const int hbox = static_cast<int>(yscaling*(map_area().h+1.5*zoom_)/zoom_ - yscaling); 

	const Uint32 boxcolour = SDL_MapRGB(surf->format,0xFF,0xFF,0xFF);
	const surface screen(screen_.getSurface());

	draw_rectangle(x+xbox,y+ybox,wbox,hbox,boxcolour,screen);

	update_rect(minimap_location);
}

void display::draw_bar(const std::string& image, int xpos, int ypos, size_t height, double filled, const SDL_Color& col, fixed_t alpha)
{
	filled = minimum<double>(maximum<double>(filled,0.0),1.0);
	height = static_cast<size_t>(height*get_zoom_factor());
#ifdef USE_TINY_GUI
	height /= 2;
#endif

	surface surf(image::get_image(image,image::SCALED_TO_HEX,image::NO_ADJUST_COLOUR));

	// we use UNSCALED because scaling (and bilinear interpolaion )
	// is bad for calculate_energy_bar. But we will do a geometric scaling later
	surface bar_surf(image::get_image(image,image::UNSCALED,image::NO_ADJUST_COLOUR));
	if(surf == NULL || bar_surf == NULL) {
		return;
	}

	// calculate_energy_bar returns incorrect results if the surface colors
	// have changed (for example, due to bilinear interpolaion)
	const SDL_Rect& unscaled_bar_loc = calculate_energy_bar(bar_surf);

	SDL_Rect bar_loc;
	if (surf->w == bar_surf->w && surf->h == bar_surf->h)
	  bar_loc = unscaled_bar_loc;
	else {
	  const fixed_t xratio = fxpdiv(surf->w,bar_surf->w);
	  const fixed_t yratio = fxpdiv(surf->h,bar_surf->h);
	  const SDL_Rect scaled_bar_loc = {fxptoi(unscaled_bar_loc. x * xratio),
					   fxptoi(unscaled_bar_loc. y * yratio + 127),
					   fxptoi(unscaled_bar_loc. w * xratio + 255),
					   fxptoi(unscaled_bar_loc. h * yratio + 255)};
	  bar_loc = scaled_bar_loc;
	}

	if(height > bar_loc.h) {
		height = bar_loc.h;
	}

	//if(alpha != ftofxp(1.0)) {
	//	surf.assign(adjust_surface_alpha(surf,alpha));
	//	if(surf == NULL) {
	//		return;
	//	}
	//}

	const size_t skip_rows = bar_loc.h - height;

	SDL_Rect top = {0,0,surf->w,bar_loc.y};
	SDL_Rect bot = {0,bar_loc.y+skip_rows,surf->w,0};
	bot.h = surf->w - bot.y;

	video().blit_surface(xpos,ypos,surf,&top);
	video().blit_surface(xpos,ypos+top.h,surf,&bot);

	const size_t unfilled = (const size_t)(height*(1.0 - filled));

	if(unfilled < height && alpha >= ftofxp(0.3)) {
		SDL_Rect filled_area = {xpos+bar_loc.x,ypos+bar_loc.y+unfilled,bar_loc.w,height-unfilled};
		const Uint32 colour = SDL_MapRGB(video().getSurface()->format,col.r,col.g,col.b);
		const Uint8 r_alpha = minimum<unsigned>(unsigned(fxpmult(alpha,255)),255);
		fill_rect_alpha(filled_area,colour,r_alpha,video().getSurface());
	}
}

void display::draw_tile(const gamemap::location &loc, const time_of_day& tod, const time_of_day & tod_at, image::TYPE image_type, const SDL_Rect &clip_rect)
{
	if(screen_.update_locked()) {
		return;
	}

	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	if(xpos >= clip_rect.x + clip_rect.w || ypos >= clip_rect.y + clip_rect.h ||
	   xpos + zoom_ < clip_rect.x || ypos + zoom_ < clip_rect.y) {
		return;
	}

	surface const dst(screen_.getSurface());

	// If the terrain is off the map it shouldn't be included for 
	// reachmap, fog, shroud and the grid.
	// In the future it may not depend on whether the location is on the map
	// but whether it's an _off^* terrain.
	// (atm not too happy with how the grid looks)
	// (the shroud has some glitches due to commented out code
	// but enabling it looks worse)
	const bool on_map = map_.on_board(loc);
	const bool is_shrouded = shrouded(loc);
	t_translation::t_letter terrain = t_translation::VOID_TERRAIN;

	if(!is_shrouded || !on_map) {
		terrain = map_.get_terrain(loc);
	}

	std::string mask = tod_at.image_mask;

	if(!is_shrouded /*|| !on_map*/) {
	  draw_terrain_on_tile(loc, tod, image_type, ADJACENT_BACKGROUND);

		surface flag(get_flag(terrain,loc));
		if(flag != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(flag,NULL,dst,&dstrect);
		}

		typedef overlay_map::const_iterator Itor;

		for(std::pair<Itor,Itor> overlays = overlays_.equal_range(loc);
			overlays.first != overlays.second; ++overlays.first) {

			surface overlay_surface(image::get_image(overlays.first->second.image,image_type));

			//note that dstrect can be changed by SDL_BlitSurface and so a
			//new instance should be initialized to pass to each call to
			//SDL_BlitSurface
			if(overlay_surface != NULL) {
				SDL_Rect dstrect = { xpos, ypos, 0, 0 };
				SDL_BlitSurface(overlay_surface,NULL,dst,&dstrect);
			}
		}
	} else if(on_map) {
		surface surface(image::get_image(game_config::void_image));


		if(surface == NULL) {
			ERR_DP << "Could not get void surface!\n";
			return;
		}

		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(surface,NULL,dst,&dstrect);
	}

	draw_footstep(loc,xpos,ypos);

	if(!is_shrouded /*|| !on_map*/) {
		draw_terrain_on_tile(loc,tod,image_type,ADJACENT_FOREGROUND);
	}

	if(fogged(loc) && on_map && !is_shrouded) {
		const surface fog_surface(image::get_image(game_config::fog_image));
		if(fog_surface != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(fog_surface,NULL,dst,&dstrect);
		}
	}

	if(!is_shrouded && on_map) {
		draw_terrain_on_tile(loc,tod,image_type,ADJACENT_FOGSHROUD);
	}

	//draw the time-of-day mask on top of the hex
	if(tod_hex_mask1 != NULL || tod_hex_mask2 != NULL) {
		if(tod_hex_mask1 != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(tod_hex_mask1,NULL,dst,&dstrect);
		}

		if(tod_hex_mask2 != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(tod_hex_mask2,NULL,dst,&dstrect);
		}
	} else if(mask != "") {
		const surface img(image::get_image(mask,image::UNMASKED,image::NO_ADJUST_COLOUR));
		if(img != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(img,NULL,dst,&dstrect);
		}
	}

	if (!reach_map_.empty() && on_map) {
		reach_map::iterator reach = reach_map_.find(loc);
		if (reach == reach_map_.end()) {
			const surface img(image::get_image(game_config::unreachable_image,image::UNMASKED,image::NO_ADJUST_COLOUR));
			if(img != NULL) {
				SDL_Rect dstrect = { xpos, ypos, 0, 0 };
				SDL_BlitSurface(img,NULL,dst,&dstrect);
			}
		} else if (reach->second > 1) {
			const std::string num = lexical_cast<std::string>(reach->second);
			draw_text_in_hex(loc, num, font::SIZE_PLUS, font::YELLOW_COLOUR);
		}
	}

	if(!is_shrouded && on_map) {
		draw_movement_info(loc);
	}

	if(grid_ && on_map) {
		surface grid_surface(image::get_image(game_config::grid_image));
		if(grid_surface != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(grid_surface,NULL,dst,&dstrect);
		}
	}

	if(game_config::debug && debugHighlights_.count(loc)) {
		const surface cross(image::get_image(game_config::cross_image));
		if(cross != NULL) {
			draw_unit(xpos,ypos,cross,false,debugHighlights_[loc],0);
		}
	}

	// Add the top layer overlay surfaces
	if(! hex_overlay_.empty()) {
		std::map<gamemap::location, surface>::const_iterator itor = hex_overlay_.find(loc);
		if(itor != hex_overlay_.end()) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(itor->second,NULL,dst,&dstrect);
		}
	}

    if(loc == selectedHex_ && map_.on_board(selectedHex_) && selected_hex_overlay_ != NULL) {
		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(selected_hex_overlay_,NULL,dst,&dstrect);
	}

    if(loc == mouseoverHex_ && map_.on_board(mouseoverHex_) && mouseover_hex_overlay_ != NULL) {
		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(mouseover_hex_overlay_,NULL,dst,&dstrect);
	}

	update_rect(xpos,ypos,zoom_,zoom_);
}

void display::draw_footstep(const gamemap::location& loc, int xloc, int yloc)
{
	std::vector<gamemap::location>::const_iterator i =
	         std::find(route_.steps.begin(),route_.steps.end(),loc);

	if(i == route_.steps.begin() || i == route_.steps.end()) {
		return;
	}

	const bool left_foot = is_even(i - route_.steps.begin());

	//generally we want the footsteps facing toward the direction they're going
	//to go next.
	//if we're on the last step, then we want them facing according to where
	//they came from, so we move i back by one
	if(i+1 == route_.steps.end() && i != route_.steps.begin()) {
		--i;
	}

	gamemap::location::DIRECTION direction = gamemap::location::NORTH;

	if(i+1 != route_.steps.end()) {
		for(int n = 0; n != 6; ++n) {
			direction = gamemap::location::DIRECTION(n);
			if(i->get_direction(direction) == *(i+1)) {
				break;
			}
		}
	}

	const std::vector<std::string>* image_category = NULL;


	if(left_foot) {
		if(direction == gamemap::location::NORTH ||
		   direction == gamemap::location::SOUTH) {
			image_category = &game_config::foot_left_n;
		} else {
			image_category = &game_config::foot_left_nw;
		}
	} else {
		if(direction == gamemap::location::NORTH ||
		   direction == gamemap::location::SOUTH) {
			image_category = &game_config::foot_right_n;
		} else {
			image_category = &game_config::foot_right_nw;
		}
	}

	if(image_category == NULL || image_category->empty()) {
		return;
	}

	const std::string* image_str = &image_category->front();
	const unit_map::const_iterator un = units_.find(route_.steps.front());
	if(un != units_.end()) {
		const int move_cost = un->second.movement_cost(map_.get_terrain(loc)) - 1;
		if(move_cost >= int(image_category->size())) {
			image_str = &image_category->back();
		} else if(move_cost > 0) {
			image_str = &(*image_category)[move_cost];
		}
	}

	surface image(image::get_image(*image_str, image::UNMASKED, image::NO_ADJUST_COLOUR));
	if(image == NULL) {
		ERR_DP << "Could not find image: " << *image_str << "\n";
		return;
	}

	const bool hflip = !(direction > gamemap::location::NORTH &&
	                     direction <= gamemap::location::SOUTH);
	const bool vflip = (direction >= gamemap::location::SOUTH_EAST &&
	                    direction <= gamemap::location::SOUTH_WEST);

	if(!hflip) {
		image.assign(image::reverse_image(image));
	}

	draw_unit(xloc,yloc,image,vflip);
}

void display::draw_text_in_hex(const gamemap::location& loc, const std::string& text,
		size_t font_size, SDL_Color color, double x_in_hex, double y_in_hex)
{
	if (text.empty()) return;
	
#ifdef USE_TINY_GUI
	// FIXME: This 16/9 must be defined elsewhere,
	// It's the ratio of the font size bewteen the two gui.
	// It makes the text fill the hex like it does in the normal gui.
	const size_t font_sz = static_cast<size_t>(font_size * get_zoom_factor() * 16/9);
#else
	const size_t font_sz = static_cast<size_t>(font_size * get_zoom_factor());
#endif
	
	const SDL_Rect& text_area = font::text_area(text,font_sz);
	const int x = get_location_x(loc) - text_area.w/2
	              + static_cast<int>(x_in_hex* hex_size());
	const int y = get_location_y(loc) - text_area.h/2
	              + static_cast<int>(y_in_hex* hex_size());

	const SDL_Rect& rect = map_area();
	for (int dy=-1; dy <= 1; dy++) {
		for (int dx=-1; dx <= 1; dx++) {
			if (dx!=0 || dy!=0)
				font::draw_text(&screen_, rect, font_sz, font::DARK_COLOUR, text, x+dx, y+dy);
		}
	}
	font::draw_text(&screen_, rect,font_sz, color, text, x, y);
}

void display::draw_movement_info(const gamemap::location& loc)
{
	//check if there is a path and if we are not at its start because
	//we don't want to display movement info on the unit's hex (useless and unreadable)
	if (route_.steps.empty() || route_.steps.front() == loc) return;

	//search if there is a turn waypoint here
	std::map<gamemap::location, int>::iterator turn_waypoint_iter = route_.turn_waypoints.find(loc);
	if(turn_waypoint_iter == route_.turn_waypoints.end()) return;

	//display the def% of this terrain
#ifndef USE_TINY_GUI
	const unit_map::const_iterator un = units_.find(route_.steps.front());
	if(un != units_.end()) {
		const int def =  100 - un->second.defense_modifier(map_.get_terrain(loc));
		std::stringstream def_text;
		def_text << def << "%";

		// with 11 colors, the last one will be used only for def=100
		int val = (game_config::defense_color_scale.size()-1) * def/100;
		SDL_Color color = int_to_color(game_config::defense_color_scale[val]);

		draw_text_in_hex(loc, def_text.str(), font::SIZE_LARGE, color, 0.5, 0.5);
	}
#endif

	//display the number of turn to reach if > 0
	int turns_to_reach = turn_waypoint_iter->second;
	if(turns_to_reach > 0 && turns_to_reach < 10) {
		std::stringstream turns_text;
		turns_text << "(" << turns_to_reach << ")";
		const SDL_Color turns_color = font::NORMAL_COLOUR;
#ifndef USE_TINY_GUI
		draw_text_in_hex(loc, turns_text.str(), font::SIZE_PLUS, turns_color, 0.5, 0.8);
#else
		draw_text_in_hex(loc, turns_text.str(), font::SIZE_PLUS, turns_color, 0.5, 0.5);
#endif
	}
}

surface display::get_flag(const t_translation::t_letter& terrain, const gamemap::location& loc)
{
	if(!map_.is_village(terrain)) {
		return surface(NULL);
	}

	for(size_t i = 0; i != teams_.size(); ++i) {
		if(teams_[i].owns_village(loc) &&
		  (!fogged(loc) || !teams_[currentTeam_].is_enemy(i+1)))
		{
			flags_[i].update_last_draw_time();
			return image::get_image(flags_[i].get_current_frame());
		}
	}

	return surface(NULL);
}

surface display::get_minimap(int w, int h)
{
	if(minimap_ != NULL && (minimap_->w != w || minimap_->h != h)) {
		minimap_ = NULL;
	}

	if(minimap_ == NULL) {
		minimap_ = image::getMinimap(w, h, map_,
		                             team_valid() ? &teams_[currentTeam_] : NULL);
	}

	return minimap_;
}

void display::highlight_reach(const paths &paths_list)
{
	unhighlight_reach();
	highlight_another_reach(paths_list);
}

void display::highlight_another_reach(const paths &paths_list)
{
	paths::routes_map::const_iterator r;

	// Fold endpoints of routes into reachability map.
	for (r = paths_list.routes.begin(); r != paths_list.routes.end(); ++r) {
		reach_map_[r->first]++;
	}
	reach_map_changed_ = true;
}

void display::unhighlight_reach()
{
	reach_map_ = reach_map();
	reach_map_changed_ = true;
}

void display::process_reachmap_changes()
{
	if (!reach_map_changed_) return;
	if (reach_map_.empty() != reach_map_old_.empty()) {
		// invalidate everything except the non-darkened tiles
		reach_map &full = reach_map_.empty() ? reach_map_old_ : reach_map_;
		gamemap::location topleft;
		gamemap::location bottomright;
		get_visible_hex_bounds(topleft, bottomright);
		for(int x = topleft.x; x <= bottomright.x; ++x) {
			for(int y = topleft.y; y <= bottomright.y; ++y) {
				gamemap::location loc(x, y);
				reach_map::iterator reach = full.find(loc);
				if (reach == full.end()) {
					// location needs to be darkened or brightened
					invalidate(loc);
				} else if (reach->second != 1) {
					// number needs to be displayed or cleared
					invalidate(loc);
				}
			}
		}
	} else if (!reach_map_.empty()) {
		// invalidate only changes
		reach_map::iterator reach, reach_old;
		for (reach = reach_map_.begin(); reach != reach_map_.end(); ++reach) {
			reach_old = reach_map_old_.find(reach->first);
			if (reach_old == reach_map_old_.end()) {
				invalidate(reach->first);
			} else {
				if (reach_old->second != reach->second) {
					invalidate(reach->first);
				}
				reach_map_old_.erase(reach_old);
			}
		}
		for (reach_old = reach_map_old_.begin(); reach_old != reach_map_old_.end(); ++reach_old) {
			invalidate(reach_old->first);
		}
	}
	reach_map_old_ = reach_map_;
	reach_map_changed_ = false;
}

void display::invalidate_route()
{
	for(std::vector<gamemap::location>::const_iterator i = route_.steps.begin();
	    i != route_.steps.end(); ++i) {
		invalidate(*i);
	}
}

void display::set_route(const paths::route* route)
{
	invalidate_route();

	if(route != NULL) {
		route_ = *route;
	} else {
		route_.steps.clear();
		route_.turn_waypoints.clear();
	}

	invalidate_route();
}

void display::remove_footstep(const gamemap::location& loc)
{
	const std::vector<gamemap::location>::iterator it = std::find(route_.steps.begin(),route_.steps.end(),loc);
	if(it != route_.steps.end()) {
		route_.steps.erase(it);
	}
}

void display::float_label(const gamemap::location& loc, const std::string& text,
						  int red, int green, int blue)
{
	if(preferences::show_floating_labels() == false || fogged(loc)) {
		return;
	}

	const SDL_Color color = {red,green,blue,255};
	int lifetime = static_cast<int>(60/turbo_speed());
	font::add_floating_label(text,font::SIZE_XLARGE,color,get_location_x(loc)+zoom_/2,get_location_y(loc),
	                         0,-2*turbo_speed(),lifetime,screen_area(),font::CENTER_ALIGN,NULL,0,font::ANCHOR_LABEL_MAP);
}

void display::clear_hex_overlay(const gamemap::location& loc)
{
	if(! hex_overlay_.empty()) {
		std::map<gamemap::location, surface>::iterator itor = hex_overlay_.find(loc);
		if(itor != hex_overlay_.end()) {
			hex_overlay_.erase(itor);
		}
	}
}

void display::draw_unit(int x, int y, surface image,
		bool upside_down, fixed_t alpha, Uint32 blendto,
		double blend_ratio, double submerged)
{

	surface surf(image);

	if(upside_down) {
		surf.assign(flop_surface(surf));
	}

	if(blend_ratio != 0) {
		surf = blend_surface(surf, blend_ratio, blendto);
	}
	if(alpha > ftofxp(1.0)) {
		surf = brighten_image(surf,alpha);
	//} else if(alpha != 1.0 && blendto != 0) {
	//	surf.assign(blend_surface(surf,1.0-alpha,blendto));
	} else if(alpha != ftofxp(1.0)) {
		surf = adjust_surface_alpha(surf,alpha,false);
	}

	if(surf == NULL) {
		ERR_DP << "surface lost...\n";
		return;
	}

	const int submerge_height = minimum<int>(surf->h,maximum<int>(0,int(surf->h*(1.0-submerged))));

	SDL_Rect clip_rect = map_area();
	SDL_Rect srcrect = {0,0,surf->w,submerge_height};
	video().blit_surface(x,y,surf,&srcrect,&clip_rect);

	if(submerge_height != surf->h) {
		surf.assign(adjust_surface_alpha(surf,ftofxp(0.2),false));

		srcrect.y = submerge_height;
		srcrect.h = surf->h-submerge_height;
		y += submerge_height;

		video().blit_surface(x,y,surf,&srcrect,&clip_rect);
	}

}

struct is_energy_colour {
	bool operator()(Uint32 colour) const { return (colour&0xFF000000) < 0x50000000 &&
	                                              (colour&0x00FF0000) > 0x00990000 &&
												  (colour&0x0000FF00) > 0x00009900 &&
												  (colour&0x000000FF) > 0x00000099; }
};

const SDL_Rect& display::calculate_energy_bar(surface surf)
{
	const std::map<surface,SDL_Rect>::const_iterator i = energy_bar_rects_.find(surf);
	if(i != energy_bar_rects_.end()) {
		return i->second;
	}

	int first_row = -1, last_row = -1, first_col = -1, last_col = -1;

	surface image(make_neutral_surface(surf));

	surface_lock image_lock(image);
	const Uint32* const begin = image_lock.pixels();

	for(int y = 0; y != image->h; ++y) {
		const Uint32* const i1 = begin + image->w*y;
		const Uint32* const i2 = i1 + image->w;
		const Uint32* const itor = std::find_if(i1,i2,is_energy_colour());
		const int count = std::count_if(itor,i2,is_energy_colour());

		if(itor != i2) {
			if(first_row == -1) {
				first_row = y;
			}

			first_col = itor - i1;
			last_col = first_col + count;
			last_row = y;
		}
	}

	const SDL_Rect res = {first_col,first_row,last_col-first_col,last_row+1-first_row};
	energy_bar_rects_.insert(std::pair<surface,SDL_Rect>(surf,res));
	return calculate_energy_bar(surf);
}

void display::invalidate(const gamemap::location& loc)
{
	if(!invalidateAll_) {
		if (invalidated_.insert(loc).second) {
			// Units can overlap adjacent tiles.
			unit_map::iterator u = units_.find(loc);

			if (u != units_.end()) {
				std::set<gamemap::location> overlaps = u->second.overlaps(*this, u->first);
				for (std::set<gamemap::location>::iterator i = overlaps.begin(); i != overlaps.end(); i++) {
					invalidate(*i);
				}
			}
			if (temp_unit_  && temp_unit_loc_ == loc ) {
				std::set<gamemap::location> overlaps = temp_unit_->overlaps(*this, temp_unit_loc_);
				for (std::set<gamemap::location>::iterator i = overlaps.begin(); i != overlaps.end(); i++) {
					invalidate(*i);
				}
			}
			// if neighbour has a unit which overlaps us invalidate him
			gamemap::location adjacent[6];
			get_adjacent_tiles(loc, adjacent);
			for (unsigned int i = 0; i < 6; i++) {
				u = units_.find(adjacent[i]);
				if (u != units_.end()) {
					std::set<gamemap::location> overlaps = u->second.overlaps(*this, u->first);
					if (overlaps.find(loc) != overlaps.end()) {
						invalidate(u->first);
					}
				}
				if (temp_unit_  && temp_unit_loc_ == adjacent[i] ) {
					std::set<gamemap::location> overlaps = temp_unit_->overlaps(*this, temp_unit_loc_);
					if (overlaps.find(loc) != overlaps.end()) {
						invalidate(temp_unit_loc_);
					}
				}
			}
		}
	}
}

void display::invalidate_all()
{
	INFO_DP << "invalidate_all()";
	invalidateAll_ = true;
	invalidated_.clear();
	update_rect(map_area());
}

void display::invalidate_animations()
{
	new_animation_frame();
	gamemap::location topleft;
	gamemap::location bottomright;
	get_visible_hex_bounds(topleft, bottomright);
	
	for(int x = topleft.x; x <= bottomright.x; ++x) {
		for(int y = topleft.y; y <= bottomright.y; ++y) {
			const gamemap::location loc(x,y);
			if (!shrouded(loc)) {
				if (builder_.update_animation(loc)) {
					invalidate(loc);
				} else if (map_.is_village(loc)) {
					const int owner = player_teams::village_owner(loc);
					if (owner >= 0 && flags_[owner].need_update() && (!fogged(loc) || !teams_[currentTeam_].is_enemy(owner+1)))
						invalidate(loc);
				}
			}
		}
	}
	unit_map::iterator unit;
	for(unit=units_.begin() ; unit != units_.end() ; unit++) {
		if (unit->second.get_animation() && unit->second.get_animation()->need_update())
			invalidate(unit->first);
		unit->second.refresh(*this,unit->first);
	}
	if (temp_unit_ ) {
		if (temp_unit_->get_animation() && temp_unit_->get_animation()->need_update())
			invalidate(temp_unit_loc_);
		temp_unit_->refresh(*this, temp_unit_loc_);
	}


}

void display::recalculate_minimap()
{
	if(minimap_ != NULL) {
		minimap_.assign(NULL);
	}

	redraw_minimap();
	// remove unit after invalidating...
}
void display::place_temporary_unit(unit &u, const gamemap::location& loc)
{
	temp_unit_ = &u;
	temp_unit_loc_ = loc;
	invalidate(loc);
}

void display::remove_temporary_unit()
{
	if(!temp_unit_) return;

	invalidate(temp_unit_loc_);
	// redraw with no location to get rid of haloes
	temp_unit_->redraw_unit(*this,gamemap::location());
	temp_unit_ = NULL;
}
void display::add_overlay(const gamemap::location& loc, const std::string& img, const std::string& halo)
{
	const int halo_handle = halo::add(get_location_x(loc) + hex_size() / 2, 
			get_location_y(loc) + hex_size() / 2, halo, loc);

	const overlay item(img,halo,halo_handle);
	overlays_.insert(overlay_map::value_type(loc,item));
}

void display::remove_overlay(const gamemap::location& loc)
{
	typedef overlay_map::const_iterator Itor;
	std::pair<Itor,Itor> itors = overlays_.equal_range(loc);
	while(itors.first != itors.second) {
		halo::remove(itors.first->second.halo_handle);
		++itors.first;
	}

	overlays_.erase(loc);
}

void display::write_overlays(config& cfg) const
{
	for(overlay_map::const_iterator i = overlays_.begin(); i != overlays_.end(); ++i) {
		config& item = cfg.add_child("item");
		i->first.write(item);
		item["image"] = i->second.image;
		item["halo"] = i->second.halo;
	}
}

const std::string display::current_team_name() const
{
	if (team_valid())
	{
		return teams_[currentTeam_].team_name();
	}
	return std::string();
}

void display::set_team(size_t team)
{
	wassert(team < teams_.size());
	currentTeam_ = team;
	if (!is_observer())
	{
		labels().set_team(&teams_[team]);
	}
	else
	{
		labels().set_team(0);
	}
	labels().recalculate_labels();
}

void display::set_playing_team(size_t team)
{
	wassert(team < teams_.size());
	activeTeam_ = team;
	invalidate_game_status();
}


double display::turbo_speed() const
{
	bool res = turbo_;
	if(keys_[SDLK_LSHIFT] || keys_[SDLK_RSHIFT]) {
		res = !res;
	}

	res |= screen_.faked();
	if (res)
		return turbo_speed_;
	else
		return 1.0;
}

//Delay routines: use these not SDL_Delay (for --nogui).

void display::delay(unsigned int milliseconds) const
{
	if (!game_config::no_delay)
		SDL_Delay(milliseconds);
}

// timestring() returns the current date as a string.
// Uses preferences::clock_format() for formatting.
static std::string timestring ()
{
    time_t now = time (NULL);
    struct tm *lt = localtime(&now);
    char buf[10];
    strftime(buf,sizeof(buf),preferences::clock_format().c_str(),lt);

    return buf;
}

void display::debug_highlight(const gamemap::location& loc, fixed_t amount)
{
	wassert(game_config::debug);
	debugHighlights_[loc] += amount;
}

const theme::menu* display::menu_pressed()
{

	for(std::vector<gui::button>::iterator i = buttons_.begin(); i != buttons_.end(); ++i) {
		if(i->pressed()) {
			const size_t index = i - buttons_.begin();
			wassert(index < theme_.menus().size());
			return &theme_.menus()[index];
		}
	}

	return NULL;
}

void display::enable_menu(const std::string& item, bool enable)
{
	for(std::vector<theme::menu>::const_iterator menu = theme_.menus().begin();
			menu != theme_.menus().end(); ++menu) {

		std::vector<std::string>::const_iterator hasitem =
			std::find(menu->items().begin(), menu->items().end(), item);

		if(hasitem != menu->items().end()) {
			const size_t index = menu - theme_.menus().begin();
			wassert(index < buttons_.size());
			buttons_[index].enable(enable);
		}
	}
}

void display::begin_game()
{
	in_game_ = true;
	create_buttons();
	invalidate_all();
}

namespace {
	const int chat_message_border = 5;
	const int chat_message_x = 10;
	const int chat_message_y = 10;
	const SDL_Color chat_message_colour = {255,255,255,255};
	const SDL_Color chat_message_bg     = {0,0,0,140};
}

void display::add_chat_message(const std::string& speaker, int side, const std::string& message, display::MESSAGE_TYPE type, bool bell)
{
	config* cignore;
	bool ignored = false;
	if ((cignore = preferences::get_prefs()->child("relationship"))){
		for(std::map<std::string,t_string>::const_iterator i = cignore->values.begin();
		i != cignore->values.end(); ++i){
			 if(speaker == i->first || speaker == "whisper: " + i->first){
				if (i->second == "ignored"){
					ignored = true;
				}
			}
		}
	}

	if (!ignored){
		bool action;
		std::string msg;

		if (bell) {
			sound::play_UI_sound(game_config::sounds::receive_message);
		}

		if(message.find("/me ") == 0) {
			msg.assign(message,4,message.size());
			action = true;
		} else {
			msg = message;
			action = false;
		}
		msg = font::word_wrap_text(msg,font::SIZE_SMALL,map_area().w*3/4);

		int ypos = chat_message_x;
		for(std::vector<chat_message>::const_iterator m = chat_messages_.begin(); m != chat_messages_.end(); ++m) {
			ypos += font::get_floating_label_rect(m->handle).h;
		}
		SDL_Color speaker_colour = {255,255,255,255};
		if(side >= 1) {
			speaker_colour = int_to_color(team::get_side_color_range(side).mid());
		}

		SDL_Color message_colour = chat_message_colour;
		std::stringstream str;
		std::stringstream message_str;
		if(type == MESSAGE_PUBLIC) {
			if(action) {
				str << "<" << speaker << " " << msg << ">";
				message_colour = speaker_colour;
				message_str << " ";
			} else {
				str << "<" << speaker << ">";
				message_str << msg;
			}
		} else {
			if(action) {
				str << "*" << speaker << " " << msg << "*";
				message_colour = speaker_colour;
				message_str << " ";
			} else {
				str << "*" << speaker << "*";
				message_str << msg;
			}
		}

		// prepend message with timestamp
		std::stringstream message_complete;
		if (preferences::chat_timestamp()) {
			message_complete << timestring() << " ";
		}
		message_complete << str.str();

		const SDL_Rect rect = map_area();
		const int speaker_handle = font::add_floating_label(message_complete.str(),font::SIZE_SMALL,speaker_colour,
			rect.x+chat_message_x,rect.y+ypos,
			0,0,-1,rect,font::LEFT_ALIGN,&chat_message_bg,chat_message_border);

		const int message_handle = font::add_floating_label(message_str.str(),font::SIZE_SMALL,message_colour,
			rect.x + chat_message_x + font::get_floating_label_rect(speaker_handle).w,rect.y+ypos,
			0,0,-1,rect,font::LEFT_ALIGN,&chat_message_bg,chat_message_border);

		chat_messages_.push_back(chat_message(speaker_handle,message_handle));

		prune_chat_messages();
	}
}

void display::prune_chat_messages(bool remove_all)
{
	const unsigned int message_ttl = remove_all ? 0 : 1200000;
	const unsigned int max_chat_messages = preferences::chat_lines();
	if(chat_messages_.empty() == false && (chat_messages_.front().created_at+message_ttl < SDL_GetTicks() || chat_messages_.size() > max_chat_messages)) {
		const int movement = font::get_floating_label_rect(chat_messages_.front().handle).h;

		font::remove_floating_label(chat_messages_.front().speaker_handle);
		font::remove_floating_label(chat_messages_.front().handle);
		chat_messages_.erase(chat_messages_.begin());

		for(std::vector<chat_message>::const_iterator i = chat_messages_.begin(); i != chat_messages_.end(); ++i) {
			font::move_floating_label(i->speaker_handle,0,-movement);
			font::move_floating_label(i->handle,0,-movement);
		}

		prune_chat_messages(remove_all);
	}
}

void display::set_diagnostic(const std::string& msg)
{
	if(diagnostic_label_ != 0) {
		font::remove_floating_label(diagnostic_label_);
		diagnostic_label_ = 0;
	}

	if(msg != "") {
		diagnostic_label_ = font::add_floating_label(msg,font::SIZE_PLUS,font::YELLOW_COLOUR,300.0,50.0,0.0,0.0,-1,map_area());
	}
}

void display::add_highlighted_loc(const gamemap::location &hex) 
{
	// Only invalidate and insert if this is a new addition, for
	// efficiency.
	if (highlighted_locations_.find(hex) == highlighted_locations_.end()) {
		highlighted_locations_.insert(hex);
		invalidate(hex);
	}
}

void display::clear_highlighted_locs() 
{
	for (std::set<gamemap::location>::const_iterator it = highlighted_locations_.begin();
		 it != highlighted_locations_.end(); it++) {
		invalidate(*it);
	}
	highlighted_locations_.clear();
}

void display::remove_highlighted_loc(const gamemap::location &hex) 
{
	std::set<gamemap::location>::iterator it = highlighted_locations_.find(hex);
	// Only invalidate and remove if the hex was found, for efficiency.
	if (it != highlighted_locations_.end()) {
		highlighted_locations_.erase(it);
		invalidate(hex);
	}
}

void display::announce(const std::string message, const SDL_Color& colour)
{
	font::add_floating_label(message,
				 font::SIZE_XLARGE,
				 colour,
				 map_area().w/2,
				 map_area().h/3,
				 0.0,0.0,100,
				 map_area(),
				 font::CENTER_ALIGN);
}

display *display::singleton_ = NULL;
