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

//! @file display.cpp
//! Routines to set up the display, scroll and zoom the map.

#include "global.hpp"

#include "actions.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "minimap.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "theme.hpp"
#include "tooltips.hpp"
#include "util.hpp"
#include "wassert.hpp"

#include "SDL_image.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#define ERR_DP LOG_STREAM(err, display)
#define INFO_DP LOG_STREAM(info, display)

namespace {
#ifdef USE_TINY_GUI
	const int DefaultZoom = 36;
	const int SmallZoom   = 16;
#else
	const int DefaultZoom = 72;
	const int SmallZoom   = 36;
#endif

	const int MinZoom = 4;
	const int MaxZoom = 200;
	size_t sunset_delay = 0;
	size_t sunset_timer = 0;

	bool benchmark = false;
}

display::display(CVideo& video, const gamemap& map, const config& theme_cfg, const config& cfg, const config& level) :
	screen_(video), map_(map), viewpoint_(NULL), xpos_(0), ypos_(0),
	theme_(theme_cfg,screen_area()),
	zoom_(DefaultZoom), last_zoom_(SmallZoom),
	builder_(cfg, level, map, theme_.border().tile_image),
	minimap_(NULL), minimap_location_(empty_rect),
	redrawMinimap_(false), redraw_background_(true),
	invalidateAll_(true), grid_(false),
	diagnostic_label_(0), panelsDrawn_(false),
	turbo_speed_(2), turbo_(false),
	invalidateGameStatus_(true),
	map_labels_(*this,map, 0),
	_scroll_event("scrolled"),
	nextDraw_(0), fps_handle_(0),
	idle_anim_(preferences::idle_anim()),
	idle_anim_rate_(1.0)
{
	if(non_interactive()) {
		screen_.lock_updates(true);
	}

	set_idle_anim_rate(preferences::idle_anim_rate());

	std::fill(reportRects_,reportRects_+reports::NUM_REPORTS,empty_rect);

	image::set_zoom(zoom_);
}

display::~display()
{
}

const SDL_Rect& display::map_area() const
{
	static SDL_Rect res = {0, 0, 0, 0};
	res = map_outside_area();

	// hex_size() is always a multiple of 4
	// and hex_width() a multiple of 3,
	// so there shouldn't be off-by-one-errors
	// due to rounding.
	// To display a hex fully on screen,
	// a little bit extra space is needed.
	// Also added the border two times.
	const int width  = static_cast<int>((map_.w() + 2 * theme_.border().size + 1.0/3.0) * hex_width());
	const int height = static_cast<int>((map_.h() + 2 * theme_.border().size + 0.5) * hex_size());

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

bool display::outside_area(const SDL_Rect& area, const int x, const int y) const
{
	const int x_thresh = hex_width();
	const int y_thresh = hex_size();
	return (x < area.x || x > area.x + area.w - x_thresh ||
		y < area.y || y > area.y + area.h - y_thresh);
}

// This function use the screen as reference
const gamemap::location display::hex_clicked_on(int xclick, int yclick,
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
const gamemap::location display::pixel_position_to_hex(int x, int y,
		gamemap::location::DIRECTION* nearest_hex,
		gamemap::location::DIRECTION* second_nearest_hex) const
{
	// adjust for the border
	x -= static_cast<int>(theme_.border().size * hex_width());
	y -= static_cast<int>(theme_.border().size * hex_size());
	const int s = hex_size();
	const int tesselation_x_size = hex_width() * 2;
	const int tesselation_y_size = s;
	const int x_base = x / tesselation_x_size * 2;
	const int x_mod  = x % tesselation_x_size;
	const int y_base = y / tesselation_y_size;
	const int y_mod  = y % tesselation_y_size;

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
		// Our x and y use the map_area as reference.
		// The coordinates given by get_location use the screen as reference,
		// so we need to convert it.
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

void display::get_rect_hex_bounds(SDL_Rect rect, gamemap::location &topleft, gamemap::location &bottomright) const
{
	// Change the coordinates of the rect send
	// to be relative to the map area, instead of the screen area.
	const SDL_Rect& map_rect = map_area();
	rect.x -= map_rect.x;
	rect.y -= map_rect.y;
	// Only move the left side.
	// The right side should remain
	// at the same coordinates, so fix that
	rect.w += map_rect.x;
	rect.h += map_rect.y;

	const int tile_width = hex_width();

	// Adjust for the border
	topleft.x = static_cast<int>(-theme_.border().size + (xpos_ + rect.x) / tile_width);
	topleft.y = static_cast<int>(-theme_.border().size + (ypos_ + rect.y - (is_odd(topleft.x) ? zoom_/2 : 0)) / zoom_);

	bottomright.x = static_cast<int>(-theme_.border().size + (xpos_ + rect.x + rect.w) / tile_width);
	bottomright.y = static_cast<int>(-theme_.border().size + ((ypos_ + rect.y + rect.h) - (is_odd(bottomright.x) ? zoom_/2 : 0)) / zoom_);

	// This routine does a rough approximation, so might be off by one.
	// To be sure enough tiles are included, the boundaries are increased
	// by one if the terrain is "on the map" due to the extra border.
	// This uses a bit larger area.
	//! @todo FIXME This routine should properly determine what to update,
	//! and not increase by one just to be sure.
	if(topleft.x >= -1) {
		topleft.x--;
	}
	if(topleft.y >= -1) {
		topleft.y--;
	}
	if(bottomright.x <= map_.w()) {
		bottomright.x++;
	}
	if(bottomright.y <= map_.h()) {
		bottomright.y++;
	}
}

int display::get_location_x(const gamemap::location& loc) const
{
	return static_cast<int>(map_area().x + (loc.x + theme_.border().size) * hex_width() - xpos_);
}

int display::get_location_y(const gamemap::location& loc) const
{
	return static_cast<int>(map_area().y + (loc.y + theme_.border().size) * zoom_ - ypos_ + (is_odd(loc.x) ? zoom_/2 : 0));
}

gamemap::location display::minimap_location_on(int x, int y)
{
	//TODO: don't return location for this,
	// instead directly scroll to the clicked pixel position

	if (!point_in_rect(x, y, minimap_location_)) {
		return gamemap::location();
	}

	// we transfom the coordinates from minimap to the full map image
	// probably more adjustements to do (border, minimap shift...)
	// but the mouse and human capacity to evaluate the rectangle center
	// is not pixel precise.
	int px = (x - minimap_location_.x) * map_.w()*hex_width() / minimap_location_.w;
	int py = (y - minimap_location_.y) * map_.h()*hex_size() / minimap_location_.h;

	return pixel_position_to_hex(px, py);
}

void display::get_visible_hex_bounds(gamemap::location &topleft, gamemap::location &bottomright) const
{
	SDL_Rect r = map_area();
	get_rect_hex_bounds(r, topleft, bottomright);
}

void display::screenshot()
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

gui::button* display::find_button(const std::string& id)
{
	for (size_t i = 0; i < buttons_.size(); ++i) {
		if(buttons_[i].id() == id) {
			return &buttons_[i];
		}
	}
	return NULL;
}

void display::create_buttons()
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
		if(rects_overlap(b.location(),map_outside_area())) {
			b.set_volatile(true);
		}

		gui::button* b_prev = find_button(b.id());
		if(b_prev) b.enable(b_prev->enabled());

		work.push_back(b);
	}

	buttons_.swap(work);
}

gui::button::TYPE display::string_to_button_type(std::string type)
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

std::vector<std::string> display::get_fog_shroud_graphics(const gamemap::location& loc)
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

		// Find somewhere that doesn't have overlap to use as a starting point
		int start;
		for(start = 0; start != 6; ++start) {
			if(tiles[start] != *terrain) {
				break;
			}
		}

		if(start == 6) {
			start = 0;
		}

		// Find all the directions overlap occurs from
		for(int i = (start+1)%6, n = 0; i != start && n != 6; ++n) {
			if(tiles[i] == *terrain) {
				std::ostringstream stream;
				std::string name;
				// if(*terrain == terrain_type::VOID_TERRAIN)
				//	stream << "void";
				//else
				//	stream << "fog";
				stream << "terrain/" << map_.get_terrain_info(*terrain).minimap_image();

				for(int n = 0; *terrain == tiles[i] && n != 6; i = (i+1)%6, ++n) {
					stream << get_direction(i);

					if(!image::exists(stream.str() + ".png")) {
						// If we don't have any surface at all,
						// then move onto the next overlapped area
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

std::vector<surface> display::get_terrain_images(const gamemap::location &loc,
						     const std::string timeid,
		image::TYPE image_type,
		ADJACENT_TERRAIN_TYPE terrain_type)
{
	std::vector<surface> res;

	if(terrain_type == ADJACENT_FOGSHROUD) {
		const std::vector<std::string> fog_shroud = get_fog_shroud_graphics(loc);

		if(!fog_shroud.empty()) {
			for(std::vector<std::string>::const_iterator it = fog_shroud.begin(); it != fog_shroud.end(); ++it) {
				image::locator image(*it);

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
			timeid, builder_terrain_type);

	if(terrains != NULL) {
		// Cache the offmap name.
		// Since it is themabel it can change,
		// so don't make it static.
		const std::string off_map_name = "terrain/" + theme_.border().tile_image + ".png";
		for(std::vector<animated<image::locator> >::const_iterator it =
				terrains->begin(); it != terrains->end(); ++it) {

			image::locator image = preferences::animate_map() ?
				it->get_current_frame() : it->get_first_frame();

			// We prevent ToD colouring and brightening of off-map tiles,
			// except if we are not in_game and so in the editor.
			// We need to test for the tile to be rendered and
			// not the location, since the transitions are rendered
			// over the offmap-terrain and these need a ToD colouring.
			const bool off_map = (image.get_filename() == off_map_name);
			const surface surface(image::get_image(image,
				off_map ? image::UNMASKED : image_type));

			if (!surface.null()) {
				res.push_back(surface);
			}
		}
	}

	return res;
}

void display::tile_stack_append(const surface surf)
{
	if (surf)
		tile_stack_.push_back(surf);
};

void display::tile_stack_append(const std::vector<surface>& surfaces)
{
	std::vector<surface>::const_iterator itor;
	for(itor = surfaces.begin(); itor != surfaces.end(); ++itor) {
		tile_stack_append(*itor);
	}
};

//! Render a stack of tile surfaces at the specified location.
void display::tile_stack_render(int x, int y)
{
	surface const dst(screen_.getSurface());

	std::vector<surface>::const_iterator itor;
	for(itor=tile_stack_.begin(); itor!=tile_stack_.end(); ++itor) {
		// Note that dstrect can be changed by SDL_BlitSurface
		// and so a new instance should be initialized
		// to pass to each call to SDL_BlitSurface.
		SDL_Rect dstrect = { x, y, 0, 0 };
		SDL_BlitSurface(*itor, NULL, dst, &dstrect);
	}
	tile_stack_.clear();

	update_rect(x, y, zoom_, zoom_);
}

void display::sunset(const size_t delay)
{
	// This allow both parametric and toggle use
	sunset_delay = (sunset_delay == 0 && delay == 0) ? 5 : delay;
}

void display::toggle_benchmark()
{
	benchmark = !benchmark;
}

void display::flip()
{
	if(video().faked()) {
		return;
	}

	const surface frameBuffer = get_video_surface();

	// This is just the debug function "sunset" to progressively darken the map area
	if (sunset_delay && ++sunset_timer > sunset_delay) {
		sunset_timer = 0;
		SDL_Rect r = map_outside_area(); // Use frameBuffer to also test the UI
		const Uint32 color =  SDL_MapRGBA(video().getSurface()->format,0,0,0,255);
		// Adjust the alpha if you want to balance cpu-cost / smooth sunset
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

void display::update_display()
{
	if(screen_.update_locked()) {
		return;
	}

	if(preferences::show_fps() || benchmark) {
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
			fps_handle_ = font::add_floating_label(stream.str(),12,
				benchmark ? font::BAD_COLOUR : font::NORMAL_COLOUR,
				10,100,0,0,-1,screen_area(),font::LEFT_ALIGN);
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
	surface surf(image::get_image(panel.image()));

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
				//! @todo FixMe YogiHH:
				// This is only made to have the buttons store
				// their background information, otherwise
				// the background will appear completely black.
				// It would more straightforward to call bg_update,
				// but that is not public and there seems to be
				// no other way atm to call it.
				// I will check if bg_update can be made public.
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
		surface surf(image::get_image(icon));
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

void display::draw_all_panels()
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

static void draw_background(surface screen, const SDL_Rect& area, const std::string& image)
{
	const surface background(image::get_image(image));
	if(background.null()) {
		return;
	}
	const unsigned int width = background->w;
	const unsigned int height = background->h;

	const unsigned int w_count = static_cast<int>(ceil(static_cast<double>(area.w) / static_cast<double>(width)));
	const unsigned int h_count = static_cast<int>(ceil(static_cast<double>(area.h) / static_cast<double>(height)));

	for(unsigned int w = 0, w_off = area.x; w < w_count; ++w, w_off += width) {
		for(unsigned int h = 0, h_off = area.y; h < h_count; ++h, h_off += height) {
			SDL_Rect clip = {w_off, h_off, 0, 0};
			SDL_BlitSurface(background, NULL, screen, &clip);
		}
	}
}

void display::draw_text_in_hex(const gamemap::location& loc, const std::string& text,
		size_t font_size, SDL_Color color, double x_in_hex, double y_in_hex)
{
	if (text.empty()) return;

	const size_t font_sz = static_cast<size_t>(font_size * get_zoom_factor()
#ifdef USE_TINY_GUI
		/ 2	// the hex is only half size
#endif
	);

	surface text_surf = font::get_rendered_text(text, font_sz, color);
	surface back_surf = font::get_rendered_text(text, font_sz, font::DARK_COLOUR);
	const int x = get_location_x(loc) - text_surf->w/2
	              + static_cast<int>(x_in_hex* hex_size());
	const int y = get_location_y(loc) - text_surf->h/2
	              + static_cast<int>(y_in_hex* hex_size());

	SDL_Rect clip_rect = map_area();
	for (int dy=-1; dy <= 1; dy++) {
		for (int dx=-1; dx <= 1; dx++) {
			if (dx!=0 || dy!=0)
				video().blit_surface(x+dx, y+dy, back_surf, NULL, &clip_rect);
		}
	}
	video().blit_surface(x, y, text_surf, NULL, &clip_rect);
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

void display::render_unit_image(int x, int y, surface image,
		bool reverse, bool greyscale, fixed_t alpha,
		Uint32 blendto, double blend_ratio, double submerged)
{

	if (image==NULL)
		return;

	SDL_Rect image_rect = {x, y, image->w, image->h};
	SDL_Rect clip_rect = map_area();
	if (!rects_overlap(image_rect, clip_rect))
		return;

	surface surf(image);

	if(reverse) {
		surf = image::reverse_image(surf);
	}

	if(greyscale) {
		surf = greyscale_image(surf);
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

void display::select_hex(gamemap::location hex)
{
	invalidate(selectedHex_);
	selectedHex_ = hex;
	invalidate(selectedHex_);
}

void display::highlight_hex(gamemap::location hex)
{
	invalidate(mouseoverHex_);
	mouseoverHex_ = hex;
	invalidate(mouseoverHex_);
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

void display::set_diagnostic(const std::string& msg)
{
	if(diagnostic_label_ != 0) {
		font::remove_floating_label(diagnostic_label_);
		diagnostic_label_ = 0;
	}

	if(msg != "") {
		diagnostic_label_ = font::add_floating_label(msg,font::SIZE_PLUS,font::YELLOW_COLOUR,300.0,50.0,0.0,0.0,-1,map_outside_area());
	}
}

//! Initiate a redraw.
//! May require redrawing panels and background.
bool display::draw_init()
{
	bool changed = false;

	if(benchmark) {
		redraw_background_ = true;
		invalidateAll_ = true;
	}

	if(!panelsDrawn_) {
		draw_all_panels();
		panelsDrawn_ = true;
		changed = true;
	}

	if(redraw_background_) {
		// Full redraw of the background
		const SDL_Rect clip_rect = map_outside_area();
		const surface outside_surf(screen_.getSurface());
		clip_rect_setter set_clip_rect(outside_surf, clip_rect);
		draw_background(outside_surf, clip_rect, theme_.border().background_image);
		update_rect(clip_rect);

		redraw_background_ = false;

		// Force a full map redraw
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

	return changed;
}

void display::draw_wrap(bool update,bool force,bool changed)
{
	static const int time_between_draws = preferences::draw_delay();
	const int current_time = SDL_GetTicks();
	const int wait_time = nextDraw_ - current_time;

	if(redrawMinimap_) {
		redrawMinimap_ = false;
		draw_minimap();
		changed = true;
	}

	if(update) {
		if(force || changed) {
			if(!force && wait_time > 0) {
				// If it's not time yet to draw, delay until it is
				SDL_Delay(wait_time);
			}
			update_display();
		}

		// Set the theortical next draw time
		nextDraw_ += time_between_draws;


		// If the next draw already should have been finished,
		// we'll enter an update frenzy, so make sure that the
		// too late value doesn't keep growing.
		// Note: if force is used too often,
		// we can also get the opposite effect.
		nextDraw_ = maximum<int>(nextDraw_, SDL_GetTicks());
	}
}

//! Delay routines: use these instead of SDL_Delay (for --nogui).
void display::delay(unsigned int milliseconds) const
{
	if (!game_config::no_delay)
		SDL_Delay(milliseconds);
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

void display::add_highlighted_loc(const gamemap::location &hex)
{
	// Only invalidate and insert if this is a new addition,
	// for efficiency.
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
				 map_outside_area().w/2,
				 map_outside_area().h/3,
				 0.0,0.0,100,
				 map_outside_area(),
				 font::CENTER_ALIGN);
}

void display::draw_border(const gamemap::location& loc, const int xpos, const int ypos)
{
	/**
	 * at the moment the border must be between 0.0 and 0.5
	 * and the image should always be prepared for a 0.5 border.
	 * This way this code doesn't need modifications for other border sizes.
	 */

	// First handle the corners :
	if(loc.x == -1 && loc.y == -1) { // top left corner
		SDL_Rect rect = { xpos + zoom_/4, ypos, 3 * zoom_/4, zoom_ } ;
		const surface border(image::get_image(theme_.border().corner_image_top_left, image::SCALED_TO_ZOOM));

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);
	} else if(loc.x == map_.w() && loc.y == -1) { // top right corner
		SDL_Rect rect = { xpos, -1, 3 * zoom_/4, zoom_ } ;
		surface border;
		if(loc.x%2 == 0) {
			rect.y = ypos + zoom_/2;
			rect.h = zoom_/2;
			// We use the map idea of odd and even,
			// and map coords are internal coords + 1
			border = image::get_image(theme_.border().corner_image_top_right_odd, image::SCALED_TO_ZOOM);
		} else {
			rect.y = ypos;
			border = image::get_image(theme_.border().corner_image_top_right_even, image::SCALED_TO_ZOOM);
		}

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	} else if(loc.x == -1 && loc.y == map_.h()) { // bottom left corner
		SDL_Rect rect = { xpos + zoom_/4, ypos, 3 * zoom_/4, zoom_/2 } ;

		const surface border(image::get_image(theme_.border().corner_image_bottom_left, image::SCALED_TO_ZOOM));

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	} else if(loc.x == map_.w() && loc.y == map_.h()) { // bottom right corner
		SDL_Rect rect = { xpos, ypos, 3 * zoom_/4, zoom_/2 } ;
		surface border;
		if(loc.x%2 == 1) {
			// We use the map idea of odd and even, and map coords are internal coords + 1
			border = image::get_image(theme_.border().corner_image_bottom_right_even, image::SCALED_TO_ZOOM);
		} else {
			border = image::get_image(theme_.border().corner_image_bottom_right_odd, image::SCALED_TO_ZOOM);
		}

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	// Now handle the sides:
	} else if(loc.x == -1) { // left side
		SDL_Rect rect = { xpos + zoom_/4 , ypos, zoom_/2, zoom_ } ;
		const surface border(image::get_image(theme_.border().border_image_left, image::SCALED_TO_ZOOM));

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	} else if(loc.x == map_.w()) { // right side
		SDL_Rect rect = { xpos + zoom_/4 , ypos, zoom_/2, zoom_ } ;
		const surface border(image::get_image(theme_.border().border_image_right, image::SCALED_TO_ZOOM));

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	} else if(loc.y == -1) { // top side
		SDL_Rect rect = { xpos, -1, zoom_, zoom_/2 } ;
		surface border;

		if(loc.x%2 == 1) {
			rect.y = ypos;
			border = image::get_image(theme_.border().border_image_top_even, image::SCALED_TO_ZOOM);
		} else {
			rect.y = ypos + zoom_/2;
			border = image::get_image(theme_.border().border_image_top_odd, image::SCALED_TO_ZOOM);
		}

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);

	} else if(loc.y == map_.h()) { // bottom side
		SDL_Rect rect = { xpos, -1, zoom_, zoom_/2 } ;
		surface border;

		if(loc.x%2 == 1) {
			rect.y = ypos;
			border = image::get_image(theme_.border().border_image_bottom_even, image::SCALED_TO_ZOOM);
		} else {
			rect.y = ypos + zoom_/2;
			border = image::get_image(theme_.border().border_image_bottom_odd, image::SCALED_TO_ZOOM);
		}

		SDL_BlitSurface( border, NULL, screen_.getSurface(), &rect);
	}
}

void display::draw_minimap()
{
	const SDL_Rect& area = minimap_area();
	if(minimap_ == NULL || minimap_->w > area.w || minimap_->h > area.h) {
		minimap_ = image::getMinimap(area.w, area.h, map_, viewpoint_);
	}

	const surface screen(screen_.getSurface());
	clip_rect_setter clip_setter(screen, area);

	SDL_Color back_color = {0,0,0,255};
	draw_centered_on_background(minimap_, area, back_color, screen);

	//update the minimap location for mouse and units functions
	minimap_location_.x = area.x + (area.w - minimap_->w) / 2;
	minimap_location_.y = area.y + (area.h - minimap_->h) / 2;
	minimap_location_.w = minimap_->w;
	minimap_location_.h = minimap_->h;

	draw_minimap_units();

	// calculate the visible portion of the map:
	// scaling between minimap and full map images
	double xscaling = 1.0*minimap_->w / (map_.w()*hex_width());
	double yscaling = 1.0*minimap_->h / (map_.h()*hex_size());

	// we need to shift with the border size
	// and the 0.25 from the minimap balanced drawing
	// and the possible difference between real map and outside off-map
	SDL_Rect map_rect = map_area();
	SDL_Rect map_out_rect = map_outside_area();
	double border = theme_.border().size;
	double shift_x = - border*hex_width() - (map_out_rect.w - map_rect.w) / 2;
	double shift_y = - (border+0.25)*hex_size() - (map_out_rect.h - map_rect.h) / 2;

	int view_x = static_cast<int>((xpos_ + shift_x) * xscaling);
	int view_y = static_cast<int>((ypos_ + shift_y) * yscaling);
	int view_w = static_cast<int>(map_out_rect.w * xscaling);
	int view_h = static_cast<int>(map_out_rect.h * yscaling);

	const Uint32 box_color = SDL_MapRGB(minimap_->format,0xFF,0xFF,0xFF);
	draw_rectangle(minimap_location_.x + view_x - 1,
                   minimap_location_.y + view_y - 1,
                   view_w + 2, view_h + 2,
                   box_color, screen);
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

	// Only invalidate if we've actually moved
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

	// Invalidate locations in the newly visible rects

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

	redrawMinimap_ = true;
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

		zoom_redraw_hook();
		image::set_zoom(zoom_);
		map_labels_.recalculate_labels();
		redraw_background_ = true;
		invalidate_all();

		// Forces a redraw after zooming.
		// This prevents some graphic glitches from occurring.
		draw();
	}
}

void display::set_default_zoom()
{
	if (zoom_ != DefaultZoom) {
		last_zoom_ = zoom_;
		set_zoom(DefaultZoom - zoom_ );
	} else {
		// When we are already at the default zoom,
		// switch to the last zoom used
		set_zoom(last_zoom_ - zoom_);
		last_zoom_ = DefaultZoom;
	}
}

void display::scroll_to_tile(const gamemap::location& loc, SCROLL_TYPE scroll_type, bool check_fogged)
{
	if(screen_.update_locked() || (check_fogged && fogged(loc))) {
		return;
	}

	if(map_.on_board(loc) == false) {
		return;
	}

	// Current position of target (upper left tile corner) in screen coordinates
	const int screenxpos = get_location_x(loc);
	const int screenypos = get_location_y(loc);

	if (scroll_type == ONSCREEN) {
		// The tile must be fully visible
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

	// Doing an animated scroll, with acceleration etc.

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
			// Do not skip too many frames on slow PCs
			dt = 0.200;
		}
		t_prev = t;

		//std::cout << t << " " << hypot(x_old, y_old) << "\n";

		//! @todo Those values might need some fine-tuning:
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

	// If rectangle formed by corners loc1 and loc2
	// is larger than map area, then just scroll to loc1.
	if(diffx > map_area().w || diffy > map_area().h) {
		scroll_to_tile(loc1,scroll_type,check_fogged);
	} else {
		// Only scroll if rectangle is not completely inside map area.
		// Assume most paths are within rectangle.
		// Sometimes with rugged terrain this is not true -- but use
		// common cases to determine behaviour instead of exceptions.
		if (outside_area(map_area(),minx,miny) ||
		    outside_area(map_area(),maxx,maxy)) {
			// Scroll to middle point of rectangle
			scroll_to_tile(gamemap::location((loc1.x+loc2.x)/2,(loc1.y+loc2.y)/2),scroll_type,check_fogged);
		} // else don't scroll, rectangle is already on screen
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

	// Adjust for the border 2 times
	const int xend = static_cast<int>(tile_width * (map_.w() + 2 * theme_.border().size) + tile_width/3);
	const int yend = static_cast<int>(zoom_ * (map_.h() + 2 * theme_.border().size) + zoom_/2);

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

void display::invalidate_all()
{
	INFO_DP << "invalidate_all()\n";
	invalidateAll_ = true;
	invalidated_.clear();
	update_rect(map_area());
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

void display::set_idle_anim_rate(int rate)
{
	idle_anim_rate_ = pow(2.0, -rate/10.0);
}

void display::redraw_everything()
{
	if(screen_.update_locked())
		return;

	invalidateGameStatus_ = true;

	for(size_t n = 0; n != reports::NUM_REPORTS; ++n) {
		reportRects_[n] = empty_rect;
		reportSurfaces_[n].assign(NULL);
		reports_[n] = reports::report();
	}

	bounds_check_position();

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

void display:: set_report_content(const reports::TYPE which_report, const std::string &content) {
	report_[which_report] = content;
}

void display::refresh_report(reports::TYPE report_num, reports::report report,
			     bool brighten)
{
	const theme::status_item* const item = theme_.get_status_item(reports::report_name(report_num));
	if(item != NULL) {
		SDL_Rect& rect = reportRects_[report_num];
		const SDL_Rect& new_rect = item->location(screen_area());

		// Report and its location is unchanged since last time. Do nothing.
		if(rect == new_rect && reports_[report_num] == report) {
			return;
		}

		reports_[report_num] = report;

		surface& surf = reportSurfaces_[report_num];

		if(surf != NULL) {
			SDL_BlitSurface(surf,NULL,screen_.getSurface(),&rect);
			update_rect(rect);
		}
		// If the rectangle has just changed, assign the surface to it
		if(new_rect != rect || surf == NULL) {
			surf.assign(NULL);
			rect = new_rect;

			// If the rectangle is present, and we are blitting text,
			// then we need to backup the surface.
			// (Images generally won't need backing up,
			// unless they are transperant, but that is done later).
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
			// Add prefix, postfix elements.
			// Make sure that they get the same tooltip
			// as the guys around them.
			std::stringstream temp;
			Uint32 RGB = item->font_rgb();
			int red   = (RGB & 0x00FF0000)>>16;
			int green = (RGB & 0x0000FF00)>>8;
			int blue  = (RGB & 0x000000FF);

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
					surface img(image::get_image(i->image));

					if(img == NULL) {
						ERR_DP << "could not find image for report: '" << i->image.get_filename() << "'\n";
						continue;
					}

					if(rect.w + rect.x - x < img->w && image_count) {
					  // We have more than one image, and this one doesn't fit.
					  img=surface(image::get_image(game_config::ellipsis_image));
					  used_ellipsis=true;
					}

					area.x = x;
					area.y = y;
					area.w = minimum<int>(rect.w + rect.x - x, img->w);
					area.h = minimum<int>(rect.h + rect.y - y, img->h);
					draw_image_for_report(img, area);

					if(brighten) {
						surface tod_bright(image::get_image(game_config:: tod_bright_image));
						if(tod_bright != NULL) {
							draw_image_for_report(tod_bright,area);
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
					} else { // Collect all tooltips for the ellipsis
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
