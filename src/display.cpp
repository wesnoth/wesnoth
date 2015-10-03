/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file display.cpp
 * Routines to set up the display, scroll and zoom the map.
 */

#include "global.hpp"

#include "builder.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "foreach.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "minimap.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "text.hpp"
#include "tooltips.hpp"

#include "SDL_image.h"

#include <algorithm>
#include <cassert>
#ifdef __SUNPRO_CC
// GCC doesn't have hypot in cmath so include it for Sun Studio
#include <math.h>
#endif
#include <cmath>
#include <iostream>
#include <sstream>

// Includes for bug #17573
#if defined(__GLIBC__)
#include <gnu/libc-version.h> 
#include <cstdio>
#endif

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)
#define DBG_DP LOG_STREAM(debug, log_display)

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

int display::last_zoom_ = SmallZoom;

display::display(CVideo& video, const gamemap* map, const config& theme_cfg, const config& level) :
	screen_(video),
	map_(map),
	viewpoint_(NULL),
	xpos_(0),
	ypos_(0),
	theme_(theme_cfg, screen_area()),
	zoom_(DefaultZoom),
	builder_(new terrain_builder(level, map, theme_.border().tile_image)),
	minimap_(NULL),
	minimap_location_(empty_rect),
	redrawMinimap_(false),
	redraw_background_(true),
	invalidateAll_(true),
	grid_(false),
	diagnostic_label_(0),
	panelsDrawn_(false),
	turbo_speed_(2),
	turbo_(false),
	invalidateGameStatus_(true),
	map_labels_(new map_labels(*this, 0)),
	shroud_image_("terrain/" + get_map().get_terrain_info(t_translation::VOID_TERRAIN).minimap_image() + ".png"),
	fog_image_("terrain/" + get_map().get_terrain_info(t_translation::FOGGED).minimap_image() + ".png"),
	tod_(time_of_day()),
	scroll_event_("scrolled"),
	complete_redraw_event_("completely_redrawn"),
	nextDraw_(0),
	report_(),
	buttons_(),
	invalidated_(),
	previous_invalidated_(),
	selected_hex_overlay_(0),
	mouseover_hex_overlay_(0),
	selectedHex_(),
	mouseoverHex_(),
	keys_(),
#if TDRAWING_BUFFER_USES_VECTOR
	drawing_buffer_(LAYER_LAST_LAYER),
#else
	drawing_buffer_(),
#endif
	map_screenshot_(false),
	fps_handle_(0),
	invalidated_hexes_(0),
	drawn_hexes_(0),
	idle_anim_(preferences::idle_anim()),
	idle_anim_rate_(1.0),
	map_screenshot_surf_(NULL),
	redraw_observers_(),
	draw_coordinates_(false),
	draw_terrain_codes_(false)
{
	if(non_interactive()
		&& (get_video_surface() != NULL
		&& video.faked())) {
		screen_.lock_updates(true);
	}

	set_idle_anim_rate(preferences::idle_anim_rate());

	std::fill(reportRects_,reportRects_+reports::NUM_REPORTS,empty_rect);

	image::set_zoom(zoom_);

#if defined(__GLIBC__)
	// Runtime checks for bug #17573
	// Get glibc runtime version information
	int glibc, glibc_minor; 
	sscanf(gnu_get_libc_version(), "%d.%d", &glibc, &glibc_minor);

	// Get SDL runtime version information
	const SDL_version* v = SDL_Linked_Version();

	do_reverse_memcpy_workaround_ = (glibc > 2 || (glibc == 2 && glibc_minor >= 13)) &&
		(v->major < 1 || (v->major == 1 && v->minor < 2) || 
			(v->major == 1 && v->minor == 2 && v->patch < 15) );
#endif
}

display::~display()
{
}

void display::rebuild_all()
{
	builder_->rebuild_all();
}

void display::reload_map()
{
	builder_->reload_map();
}

void display::change_map(const gamemap* m)
{
	map_ = m;
	builder_->change_map(m);
}

const SDL_Rect& display::max_map_area() const
{
	static SDL_Rect max_area = {0, 0, 0, 0};

	// hex_size() is always a multiple of 4
	// and hex_width() a multiple of 3,
	// so there shouldn't be off-by-one-errors
	// due to rounding.
	// To display a hex fully on screen,
	// a little bit extra space is needed.
	// Also added the border two times.
	max_area.w  = static_cast<int>((get_map().w() + 2 * theme_.border().size + 1.0/3.0) * hex_width());
	max_area.h = static_cast<int>((get_map().h() + 2 * theme_.border().size + 0.5) * hex_size());

	return max_area;
}

const SDL_Rect& display::map_area() const
{
	static SDL_Rect max_area;
	max_area = max_map_area();

	// if it's for map_screenshot, maximize and don't recenter
	if (map_screenshot_) {
		return max_area;
	}

	static SDL_Rect res;
	res = map_outside_area();

	if(max_area.w < res.w) {
		// map is smaller, center
		res.x += (res.w - max_area.w)/2;
		res.w = max_area.w;
	}

	if(max_area.h < res.h) {
		// map is smaller, center
		res.y += (res.h - max_area.h)/2;
		res.h = max_area.h;
	}

	return res;
}

bool display::outside_area(const SDL_Rect& area, const int x, const int y) const
{
	const int x_thresh = hex_size();
	const int y_thresh = hex_size();
	return (x < area.x || x > area.x + area.w - x_thresh ||
		y < area.y || y > area.y + area.h - y_thresh);
}

// This function use the screen as reference
const map_location display::hex_clicked_on(int xclick, int yclick) const
{
	const SDL_Rect& rect = map_area();
	if(point_in_rect(xclick,yclick,rect) == false) {
		return map_location();
	}

	xclick -= rect.x;
	yclick -= rect.y;

	return pixel_position_to_hex(xpos_ + xclick, ypos_ + yclick);
}


// This function use the rect of map_area as reference
const map_location display::pixel_position_to_hex(int x, int y) const
{
	// adjust for the border
	x -= static_cast<int>(theme_.border().size * hex_width());
	y -= static_cast<int>(theme_.border().size * hex_size());
	// The editor can modify the border and this will result in a negative y
	// value. Instead of adding extra cases we just shift the hex. Since the
	// editor doesn't use the direction this is no problem.
	const int offset = y < 0 ? 1 : 0;
	if(offset) {
		x += hex_width();
		y += hex_size();
	}
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

	return map_location(x_base + x_modifier - offset, y_base + y_modifier - offset);


	// NOTE: This code to get nearest_hex and second_nearest_hex
	// is not used anymore. However, it can be usefull later.
	// So, keep it here for the moment.
	/*
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
				*nearest_hex = map_location::SOUTH_EAST;
				if(second_nearest_hex != NULL) {
					if(x_offset/2 > y_offset) {
						*second_nearest_hex = map_location::NORTH_EAST;
					} else {
						*second_nearest_hex = map_location::SOUTH;
					}
				}
			} else if(-x_offset > y_offset/2) {
				*nearest_hex = map_location::SOUTH_WEST;
				if(second_nearest_hex != NULL) {
					if(-x_offset/2 > y_offset) {
						*second_nearest_hex = map_location::NORTH_WEST;
					} else {
						*second_nearest_hex = map_location::SOUTH;
					}
				}
			} else {
				*nearest_hex = map_location::SOUTH;
				if(second_nearest_hex != NULL) {
					if(x_offset > 0) {
						*second_nearest_hex = map_location::SOUTH_EAST;
					} else {
						*second_nearest_hex = map_location::SOUTH_WEST;
					}
				}
			}
		} else { // y_offset <= 0
			if(x_offset > -y_offset/2) {
				*nearest_hex = map_location::NORTH_EAST;
				if(second_nearest_hex != NULL) {
					if(x_offset/2 > -y_offset) {
						*second_nearest_hex = map_location::SOUTH_EAST;
					} else {
						*second_nearest_hex = map_location::NORTH;
					}
				}
			} else if(-x_offset > -y_offset/2) {
				*nearest_hex = map_location::NORTH_WEST;
				if(second_nearest_hex != NULL) {
					if(-x_offset/2 > -y_offset) {
						*second_nearest_hex = map_location::SOUTH_WEST;
					} else {
						*second_nearest_hex = map_location::NORTH;
					}
				}
			} else {
				*nearest_hex = map_location::NORTH;
				if(second_nearest_hex != NULL) {
					if(x_offset > 0) {
						*second_nearest_hex = map_location::NORTH_EAST;
					} else {
						*second_nearest_hex = map_location::NORTH_WEST;
					}
				}
			}
		}
	}
	*/
}

void display::rect_of_hexes::iterator::operator++()
{
	if (loc_.y < rect_.bottom[loc_.x & 1])
		++loc_.y;
	else {
		++loc_.x;
		loc_.y = rect_.top[loc_.x & 1];
	}
}

// begin is top left, and end is after bottom right
display::rect_of_hexes::iterator display::rect_of_hexes::begin() const
{
	return iterator(map_location(left, top[left & 1]), *this);
}
display::rect_of_hexes::iterator display::rect_of_hexes::end() const
{
	return iterator(map_location(right+1, top[(right+1) & 1]), *this);
}

const display::rect_of_hexes display::hexes_under_rect(const SDL_Rect& r) const
{
	rect_of_hexes res;

	if (r.w<=0 || r.h<=0) {
		// empty rect, return dummy values giving begin=end
		res.left = 0;
		res.right = -1; // end is right+1
		res.top[0] = 0;
		res.top[1] = 0;
		res.bottom[0] = 0;
		res.bottom[1] = 0;
		return res;
	}

	SDL_Rect map_rect = map_area();
	// translate rect coordinates from screen-based to map_area-based
	int x = xpos_ - map_rect.x + r.x;
	int y = ypos_ - map_rect.y + r.y;
	// we use the "double" type to avoid important rounding error (size of an hex!)
	// we will also need to use std::floor to avoid bad rounding at border (negative values)
	double tile_width = hex_width();
	double tile_size = hex_size();
	double border = theme_.border().size;
	// the "-0.25" is for the horizontal imbrication of hexes (1/4 overlaps).
	res.left = static_cast<int>(std::floor(-border + x / tile_width - 0.25));
	// we remove 1 pixel of the rectangle dimensions
	// (the rounded division take one pixel more than needed)
	res.right = static_cast<int>(std::floor(-border + (x + r.w-1) / tile_width));

	// for odd x, we must shift up one half-hex. Since x will vary along the edge,
	// we store here the y values for even and odd x, respectively
	res.top[0] = static_cast<int>(std::floor(-border + y / tile_size));
	res.top[1] = static_cast<int>(std::floor(-border + y / tile_size - 0.5));
	res.bottom[0] = static_cast<int>(std::floor(-border + (y + r.h-1) / tile_size));
	res.bottom[1] = static_cast<int>(std::floor(-border + (y + r.h-1) / tile_size - 0.5));

	// TODO: in some rare cases (1/16), a corner of the big rect is on a tile
	// (the 72x72 rectangle containing the hex) but not on the hex itself
	// Can maybe be optimized by using pixel_position_to_hex

	return res;
}

int display::get_location_x(const map_location& loc) const
{
	return static_cast<int>(map_area().x + (loc.x + theme_.border().size) * hex_width() - xpos_);
}

int display::get_location_y(const map_location& loc) const
{
	return static_cast<int>(map_area().y + (loc.y + theme_.border().size) * zoom_ - ypos_ + (is_odd(loc.x) ? zoom_/2 : 0));
}

map_location display::minimap_location_on(int x, int y)
{
	//TODO: don't return location for this,
	// instead directly scroll to the clicked pixel position

	if (!point_in_rect(x, y, minimap_area())) {
		return map_location();
	}

	// we transfom the coordinates from minimap to the full map image
	// probably more adjustements to do (border, minimap shift...)
	// but the mouse and human capacity to evaluate the rectangle center
	// is not pixel precise.
	int px = (x - minimap_location_.x) * get_map().w()*hex_width() / minimap_location_.w;
	int py = (y - minimap_location_.y) * get_map().h()*hex_size() / minimap_location_.h;

	map_location loc = pixel_position_to_hex(px, py);
	if (loc.x < 0)
		loc.x = 0;
	else if (loc.x >= get_map().w())
		loc.x = get_map().w() - 1;

	if (loc.y < 0)
		loc.y = 0;
	else if (loc.y >= get_map().h())
		loc.y = get_map().h() - 1;

	return loc;
}

int display::screenshot(std::string filename, bool map_screenshot)
{
	int size = 0;
	if (!map_screenshot) {
		surface screenshot_surf = screen_.getSurface();
		SDL_SaveBMP(screenshot_surf, filename.c_str());
		size = screenshot_surf->w * screenshot_surf->h;
	} else {
		if (get_map().empty()) {
			// Map Screenshot are big, abort and warn the user if he does strange things
			std::cerr << "No map, can't do a Map Screenshot. If it was not wanted, check your hotkey.\n";
			return -1;
		}

		SDL_Rect area = max_map_area();
		map_screenshot_surf_ = create_compatible_surface(screen_.getSurface(), area.w, area.h);

		if (map_screenshot_surf_ == NULL) {
			// Memory problem ?
			std::cerr << "Can't create the screenshot surface. Maybe too big, try dezooming.\n";
			return -1;
		}
		size = map_screenshot_surf_->w * map_screenshot_surf_->h;

		// back up the current map view position and move to top-left
		int old_xpos = xpos_;
		int old_ypos = ypos_;
		xpos_ = 0;
		ypos_ = 0;

		// we reroute render output to the screenshot surface and invalidate all
		map_screenshot_= true ;
		invalidateAll_ = true;
		DBG_DP << "draw() with map_screenshot\n";
		draw(true,true);

		// finally save the image on disk
		SDL_SaveBMP(map_screenshot_surf_, filename.c_str());

		//NOTE: need to be sure that we free this huge surface (is it enough?)
		map_screenshot_surf_ = NULL;

		// restore normal rendering
		map_screenshot_= false;
		xpos_ = old_xpos;
		ypos_ = old_ypos;
		// some drawing functions are confused by the temporary change
		// of the map_area and thus affect the UI outside of the map
		redraw_everything();
	}

	// convert pixel size to BMP size
	size = (2048 + size*3);
	return size;
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

	DBG_DP << "creating buttons...\n";
	const std::vector<theme::menu>& buttons = theme_.menus();
	for(std::vector<theme::menu>::const_iterator i = buttons.begin(); i != buttons.end(); ++i) {
		gui::button b(screen_,i->title(),string_to_button_type(i->type()),i->image());
		DBG_DP << "drawing button " << i->get_id() << "\n";
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
	DBG_DP << "buttons created\n";
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

std::vector<std::string> display::get_fog_shroud_graphics(const map_location& loc)
{
	std::vector<std::string> res;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	t_translation::t_terrain tiles[6];

	static const t_translation::t_terrain terrain_types[] =
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


	for(const t_translation::t_terrain *terrain = terrain_types;
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
				stream << "terrain/" << get_map().get_terrain_info(*terrain).minimap_image();

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

std::vector<surface> display::get_terrain_images(const map_location &loc,
						     const std::string& timeid,
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
	const terrain_builder::imagelist* const terrains = builder_->get_terrain_at(loc,
			timeid, builder_terrain_type);

	if(terrains != NULL) {
		// Cache the offmap name.
		// Since it is themabel it can change,
		// so don't make it static.
		const std::string off_map_name = "terrain/" + theme_.border().tile_image + ".png";
		for(std::vector<animated<image::locator> >::const_iterator it =
				terrains->begin(); it != terrains->end(); ++it) {

			const image::locator& image = preferences::animate_map() ?
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

void display::drawing_buffer_commit()
{
	SDL_Rect clip_rect = map_area();
	surface const dst(get_screen_surface());
	clip_rect_setter set_clip_rect(dst, clip_rect);

	/*
	 * Info regarding the rendering algorithm.
	 *
	 * In order to render a hex properly it needs to be rendered per row. On
	 * this row several layers need to be drawn at the same time. Mainly the
	 * unit and the background terrain. This is needed since both can spill
	 * in the next hex. The foreground terrain needs to be drawn before to
	 * avoid decapitation a unit.
	 *
	 * This ended in the following algorithm.
	 *
	 * - Loop over all layer groups.
	 * - For all layers in this group proceed to render from low to high.
	 *   - Render all rows (a row has a constant y) from low to high.
	 *   - In the row render the columns from low to high.
	 */

	// The drawing is done per layer_group, the range per group is [low, high).
	static const tdrawing_layer layer_groups[] = {
		LAYER_TERRAIN_BG,
		LAYER_UNIT_FIRST,
		LAYER_UNIT_MOVE_DEFAULT,
		// Make sure the movement doesn't show above fog and reachmap.
		LAYER_REACHMAP,
		LAYER_LAST_LAYER };

	for(size_t z = 1; z < sizeof(layer_groups)/sizeof(layer_groups[0]); ++z) {

		for(int y = -get_map().border_size();
				y < get_map().total_height(); ++y) {

#if TDRAWING_BUFFER_USES_VECTOR
			int layer = -1;
#endif
			for(tdrawing_buffer::const_iterator
					layer_itor = drawing_buffer_.begin(),
					layer_itor_end = drawing_buffer_.end();
					layer_itor != layer_itor_end; ++layer_itor) {

#if TDRAWING_BUFFER_USES_VECTOR
				++layer;
				if(!(layer >= layer_groups[z - 1] &&
							layer < layer_groups[z])) {
					// The current layer is not in the layer group, skip.
					continue;
				}
#else
				if(!(layer_itor->first >= layer_groups[z - 1] &&
							layer_itor->first < layer_groups[z])) {
					// The current layer is not in the layer group, skip.
					continue;
				}
#endif

				for(tblit_map::const_iterator
#if TDRAWING_BUFFER_USES_VECTOR
						drawing_iterator = layer_itor->begin(),
						drawing_iterator_end = layer_itor->end();
#else
						drawing_iterator = layer_itor->second.begin(),
						drawing_iterator_end = layer_itor->second.end();
#endif
						drawing_iterator != drawing_iterator_end;
						++drawing_iterator) {

					if(drawing_iterator->first.y != y) {
						// The current row is not the row to render, skip.
						continue;
					}

					for(std::vector<tblit>::const_iterator
							blit_itor = drawing_iterator->second.begin(),
							blit_itor_end = drawing_iterator->second.end();
							blit_itor != blit_itor_end; ++blit_itor) {

						for(std::vector<surface>::const_iterator
								surface_itor = blit_itor->surf.begin(),
								surface_itor_end = blit_itor->surf.end();
								surface_itor != surface_itor_end; ++surface_itor) {

							// Note that dstrect can be changed by SDL_BlitSurface
							// and so a new instance should be initialized
							// to pass to each call to SDL_BlitSurface.
							SDL_Rect dstrect = { blit_itor->x, blit_itor->y, 0, 0 };

							if(blit_itor->clip.x || blit_itor->clip.y
									||blit_itor->clip.w ||blit_itor->clip.h) {

								SDL_Rect srcrect = blit_itor->clip;
								SDL_BlitSurface(*surface_itor,
										&srcrect, dst, &dstrect);
							} else {
								SDL_BlitSurface(*surface_itor,
										NULL, dst, &dstrect);
							}
						}
						update_rect(blit_itor->x, blit_itor->y, zoom_, zoom_);
					} // for blit_itor
				} // for drawing_iterator
			} // for layer_itor
		} // for y
	} // for z

	drawing_buffer_clear();
}

void display::drawing_buffer_clear()
{
#if TDRAWING_BUFFER_USES_VECTOR
	// Note clear the items, the vector should remain the same size.
	for(tdrawing_buffer::iterator layer_itor =
			drawing_buffer_.begin(),
			layer_itor_end = drawing_buffer_.end();
			layer_itor != layer_itor_end; ++layer_itor) {

		layer_itor->clear();
	}
#else
	drawing_buffer_.clear();
#endif
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
	if (screen_.update_locked()) {
		return;
	}

	if(preferences::show_fps() || benchmark) {
		static int last_sample = SDL_GetTicks();
		static int frames = 0;
		++frames;
		const int sample_freq = 10;
		if(frames == sample_freq) {
			const int this_sample = SDL_GetTicks();

			const int fps = (frames*1000)/(this_sample - last_sample);
			last_sample = this_sample;
			frames = 0;

			if(fps_handle_ != 0) {
				font::remove_floating_label(fps_handle_);
				fps_handle_ = 0;
			}
			std::ostringstream stream;
			stream << "fps: " << fps;
			if (game_config::debug) {
				stream << "\nhex: " << drawn_hexes_*1.0/sample_freq;
				if (drawn_hexes_ != invalidated_hexes_)
					stream << " (" << (invalidated_hexes_-drawn_hexes_)*1.0/sample_freq << ")";
			}
			drawn_hexes_ = 0;
			invalidated_hexes_ = 0;

			fps_handle_ = font::add_floating_label(stream.str(),12,
				benchmark ? font::BAD_COLOUR : font::NORMAL_COLOUR,
				10,100,0,0,-1,screen_area(),font::LEFT_ALIGN);
		}
	} else if(fps_handle_ != 0) {
		font::remove_floating_label(fps_handle_);
		fps_handle_ = 0;
		drawn_hexes_ = 0;
		invalidated_hexes_ = 0;
	}

	flip();
}

static void draw_panel(CVideo& video, const theme::panel& panel, std::vector<gui::button>& buttons)
{
	//log_scope("draw panel");
	DBG_DP << "drawing panel " << panel.get_id() << "\n";

	surface surf(image::get_image(panel.image()));

	const SDL_Rect screen = screen_area();
	SDL_Rect& loc = panel.location(screen);

	DBG_DP << "panel location: x=" << loc.x << ", y=" << loc.y
			<< ", w=" << loc.w << ", h=" << loc.h << "\n";

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
				/**
				 * @todo FixMe YogiHH:
				 * This is only made to have the buttons store their background
				 * information, otherwise the background will appear completely
				 * black. It would more straightforward to call bg_update, but
				 * that is not public and there seems to be no other way atm to
				 * call it. I will check if bg_update can be made public.
				 */
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

	const unsigned int w_count = static_cast<int>(std::ceil(static_cast<double>(area.w) / static_cast<double>(width)));
	const unsigned int h_count = static_cast<int>(std::ceil(static_cast<double>(area.h) / static_cast<double>(height)));

	for(unsigned int w = 0, w_off = area.x; w < w_count; ++w, w_off += width) {
		for(unsigned int h = 0, h_off = area.y; h < h_count; ++h, h_off += height) {
			SDL_Rect clip = {w_off, h_off, 0, 0};
			SDL_BlitSurface(background, NULL, screen, &clip);
		}
	}
}

void display::draw_text_in_hex(const map_location& loc,
		const tdrawing_layer layer, const std::string& text,
		size_t font_size, SDL_Color color, double x_in_hex, double y_in_hex)
{
	if (text.empty()) return;

	const size_t font_sz = static_cast<size_t>(font_size * get_zoom_factor()
#ifdef USE_TINY_GUI
		/ 2	// the hex is only half size
#endif
	);

	surface text_surf = font::get_rendered_text(text, font_sz, color);
	surface back_surf = font::get_rendered_text(text, font_sz, font::BLACK_COLOUR);
	const int x = get_location_x(loc) - text_surf->w/2
	              + static_cast<int>(x_in_hex* hex_size());
	const int y = get_location_y(loc) - text_surf->h/2
	              + static_cast<int>(y_in_hex* hex_size());

	for (int dy=-1; dy <= 1; ++dy) {
		for (int dx=-1; dx <= 1; ++dx) {
			if (dx!=0 || dy!=0) {
				drawing_buffer_add(layer, loc, tblit(x + dx, y + dy, back_surf));
			}
		}
	}
	drawing_buffer_add(layer, loc, tblit(x, y, text_surf));
}

void display::render_unit_image(int x, int y,const display::tdrawing_layer drawing_layer,
		const map_location& loc, surface image,
		bool hreverse, bool greyscale, fixed_t alpha,
		Uint32 blendto, double blend_ratio, double submerged,bool vreverse)
{

	if (image==NULL)
		return;

	SDL_Rect image_rect = {x, y, image->w, image->h};
	SDL_Rect clip_rect = map_area();
	if (!rects_overlap(image_rect, clip_rect))
		return;

	surface surf(image);

	if(hreverse) {
		surf = image::reverse_image(surf);
	}
	if(vreverse) {
		surf = flop_surface(surf, false);
	}

	if(greyscale) {
		surf = greyscale_image(surf, false);
	}

	if(blend_ratio != 0) {
		surf = blend_surface(surf, blend_ratio, blendto, false);
	}
	if(alpha > ftofxp(1.0)) {
		surf = brighten_image(surf, alpha, false);
	//} else if(alpha != 1.0 && blendto != 0) {
	//	surf.assign(blend_surface(surf,1.0-alpha,blendto));
	} else if(alpha != ftofxp(1.0)) {
		surf = adjust_surface_alpha(surf, alpha, false);
	}

	if(surf == NULL) {
		ERR_DP << "surface lost...\n";
		return;
	}

	const int submerge_height = std::min<int>(surf->h,std::max<int>(0,int(surf->h*(1.0-submerged))));


	SDL_Rect srcrect = {0,0,surf->w,submerge_height};

	drawing_buffer_add(drawing_layer, loc, tblit(x, y, surf, srcrect));

	if(submerge_height != surf->h) {
		surf.assign(adjust_surface_alpha(surf,ftofxp(0.2),false));

		srcrect.y = submerge_height;
		srcrect.h = surf->h-submerge_height;
		y += submerge_height;

		drawing_buffer_add(drawing_layer, loc, tblit(x, y, surf, srcrect));
	}

}

void display::select_hex(map_location hex)
{
	invalidate(selectedHex_);
	selectedHex_ = hex;
	invalidate(selectedHex_);
}

void display::highlight_hex(map_location hex)
{
	invalidate(mouseoverHex_);
	mouseoverHex_ = hex;
	invalidate(mouseoverHex_);
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

bool display::draw_init()
{
	bool changed = false;

	if (get_map().empty()) {
		return changed;
	}

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
		const surface screen = get_screen_surface();
		clip_rect_setter set_clip_rect(screen, clip_rect);
		draw_background(screen, clip_rect, theme_.border().background_image);
		update_rect(clip_rect);

		redraw_background_ = false;

		// Force a full map redraw
		invalidateAll_ = true;
	}

	if(invalidateAll_) {
		DBG_DP << "draw() with invalidateAll\n";

		// toggle invalidateAll_ first to allow regular invalidations
		invalidateAll_ = false;
		invalidate_locations_in_rect(map_area());

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
		nextDraw_ = std::max<int>(nextDraw_, SDL_GetTicks());
	}
}

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
			if(index >= theme_.menus().size()) {
				assert(false);
				return 0;
			}
			return &theme_.menus()[index];
		}
	}

	return 0;
}

void display::enable_menu(const std::string& item, bool enable)
{
	for(std::vector<theme::menu>::const_iterator menu = theme_.menus().begin();
			menu != theme_.menus().end(); ++menu) {

		std::vector<std::string>::const_iterator hasitem =
			std::find(menu->items().begin(), menu->items().end(), item);

		if(hasitem != menu->items().end()) {
			const size_t index = menu - theme_.menus().begin();
			if(index >= buttons_.size()) {
				assert(false);
				return;
			}
			buttons_[index].enable(enable);
		}
	}
}

void display::announce(const std::string& message, const SDL_Color& colour)
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

void display::draw_border(const map_location& loc, const int xpos, const int ypos)
{
	/**
	 * at the moment the border must be between 0.0 and 0.5
	 * and the image should always be prepared for a 0.5 border.
	 * This way this code doesn't need modifications for other border sizes.
	 */

	// First handle the corners :
	if(loc.x == -1 && loc.y == -1) { // top left corner
		drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos + zoom_/4, ypos,
			image::get_image(theme_.border().corner_image_top_left, image::SCALED_TO_ZOOM)));
	} else if(loc.x == get_map().w() && loc.y == -1) { // top right corner
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 0) {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos + zoom_/2,
				image::get_image(theme_.border().corner_image_top_right_odd, image::SCALED_TO_ZOOM)));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos,
				image::get_image(theme_.border().corner_image_top_right_even, image::SCALED_TO_ZOOM)));
		}
	} else if(loc.x == -1 && loc.y == get_map().h()) { // bottom left corner
		drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos + zoom_/4, ypos,
			image::get_image(theme_.border().corner_image_bottom_left, image::SCALED_TO_ZOOM)));

	} else if(loc.x == get_map().w() && loc.y == get_map().h()) { // bottom right corner
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos,
				image::get_image(theme_.border().corner_image_bottom_right_even, image::SCALED_TO_ZOOM)));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos,
				image::get_image(theme_.border().corner_image_bottom_right_odd, image::SCALED_TO_ZOOM)));
		}

	// Now handle the sides:
	} else if(loc.x == -1) { // left side
		drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos + zoom_/4, ypos,
			image::get_image(theme_.border().border_image_left, image::SCALED_TO_ZOOM)));
	} else if(loc.x == get_map().w()) { // right side
		drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos + zoom_/4, ypos,
			image::get_image(theme_.border().border_image_right, image::SCALED_TO_ZOOM)));
	} else if(loc.y == -1) { // top side
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos,
				image::get_image(theme_.border().border_image_top_even, image::SCALED_TO_ZOOM)));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos + zoom_/2,
				image::get_image(theme_.border().border_image_top_odd, image::SCALED_TO_ZOOM)));
		}
	} else if(loc.y == get_map().h()) { // bottom side
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos,
				image::get_image(theme_.border().border_image_bottom_even, image::SCALED_TO_ZOOM)));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, tblit(xpos, ypos + zoom_/2,
				image::get_image(theme_.border().border_image_bottom_odd, image::SCALED_TO_ZOOM)));
		}
	}
}

void display::draw_minimap()
{
	const SDL_Rect& area = minimap_area();
	if(minimap_ == NULL || minimap_->w > area.w || minimap_->h > area.h) {
		minimap_ = image::getMinimap(area.w, area.h, get_map(), viewpoint_);
		if(minimap_ == NULL) {
			return;
		}
	}

	const surface screen(screen_.getSurface());
	clip_rect_setter clip_setter(screen, area);

	SDL_Color back_color = {31,31,23,255};
	draw_centered_on_background(minimap_, area, back_color, screen);

	//update the minimap location for mouse and units functions
	minimap_location_.x = area.x + (area.w - minimap_->w) / 2;
	minimap_location_.y = area.y + (area.h - minimap_->h) / 2;
	minimap_location_.w = minimap_->w;
	minimap_location_.h = minimap_->h;

	draw_minimap_units();

	// calculate the visible portion of the map:
	// scaling between minimap and full map images
	double xscaling = 1.0*minimap_->w / (get_map().w()*hex_width());
	double yscaling = 1.0*minimap_->h / (get_map().h()*hex_size());

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

bool display::scroll(int xmove, int ymove)
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
		return false;

	labels().scroll(dx, dy);
	font::scroll_floating_labels(dx, dy);

	surface screen(screen_.getSurface());

	SDL_Rect dstrect = map_area();
	dstrect.x += dx;
	dstrect.y += dy;
	dstrect = intersect_rects(dstrect, map_area());

	SDL_Rect srcrect = dstrect;
	srcrect.x -= dx;
	srcrect.y -= dy;
	if (!screen_.update_locked()) {

// Hack to workaround bug #17573
#if defined(__GLIBC__)
		if (do_reverse_memcpy_workaround_) {
			surface screen_copy = make_neutral_surface(screen);
			SDL_BlitSurface(screen_copy,&srcrect,screen,&dstrect);
		} else {
			SDL_BlitSurface(screen,&srcrect,screen,&dstrect);
		}
#else 
		SDL_BlitSurface(screen,&srcrect,screen,&dstrect);
#endif
	}

//This is necessary to avoid a crash in some SDL versions on some systems
//see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=462794
//FIXME remove this once the latest stable SDL release doesn't crash as 1.2.13 does
#ifdef _MSC_VER
    __asm{cld};
#elif defined(__GNUG__) && (defined(__i386__) || defined(__x86_64__))
    asm("cld");
#endif

	// Invalidate locations in the newly visible rects

	if (dy != 0) {
		SDL_Rect r = map_area();
		if(dy < 0)
			r.y = r.y + r.h + dy;
		r.h = abs(dy);
		invalidate_locations_in_rect(r);
	}
	if (dx != 0) {
		SDL_Rect r = map_area();
		if (dx < 0)
			r.x = r.x + r.w + dx;
		r.w = abs(dx);
		invalidate_locations_in_rect(r);
	}
	scroll_event_.notify_observers();
	update_rect(map_area());

	redrawMinimap_ = true;
	return true;
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
		if (zoom_ != DefaultZoom) {
			last_zoom_ = zoom_;
		}
		image::set_zoom(zoom_);

		labels().recalculate_labels();
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
	}
}

bool display::tile_fully_on_screen(const map_location& loc)
{
	int x = get_location_x(loc);
	int y = get_location_y(loc);
	return !outside_area(map_area(), x, y);
}

bool display::tile_nearly_on_screen(const map_location& loc) const
{
	int x = get_location_x(loc);
	int y = get_location_y(loc);
	const SDL_Rect &area = map_area();
	int hw = hex_width(), hs = hex_size();
	return x + hs >= area.x - hw && x < area.x + area.w + hw &&
	       y + hs >= area.y - hs && y < area.y + area.h + hs;
}

void display::scroll_to_xy(int screenxpos, int screenypos, SCROLL_TYPE scroll_type, bool force)
{
	if(!force && !preferences::scroll_to_action()) return;
	if(screen_.update_locked()) {
		return;
	}
	const SDL_Rect area = map_area();
	const int xmove_expected = screenxpos - (area.x + area.w/2);
	const int ymove_expected = screenypos - (area.y + area.h/2);

	int xpos = xpos_ + xmove_expected;
	int ypos = ypos_ + ymove_expected;
	bounds_check_position(xpos, ypos);
	int xmove = xpos - xpos_;
	int ymove = ypos - ypos_;

	if(scroll_type == WARP || turbo_speed() > 2.0 || preferences::scroll_speed() > 99) {
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

		/** @todo Those values might need some fine-tuning: */
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

void display::scroll_to_tile(const map_location& loc, SCROLL_TYPE scroll_type, bool check_fogged, bool force)
{
	if(get_map().on_board(loc) == false) {
		ERR_DP << "Tile at " << loc << " isn't on the map, can't scroll to the tile.\n";
		return;
	}

	std::vector<map_location> locs;
	locs.push_back(loc);
	scroll_to_tiles(locs, scroll_type, check_fogged,false,0.0,force);
}

void display::scroll_to_tiles(map_location loc1, map_location loc2,
                              SCROLL_TYPE scroll_type, bool check_fogged,
			      double add_spacing, bool force)
{
	std::vector<map_location> locs;
	locs.push_back(loc1);
	locs.push_back(loc2);
	scroll_to_tiles(locs, scroll_type, check_fogged, false, add_spacing,force);
}

void display::scroll_to_tiles(const std::vector<map_location>& locs,
                              SCROLL_TYPE scroll_type, bool check_fogged,
			      bool only_if_possible, double add_spacing, bool force)
{
	// basically we calculate the min/max coordinates we want to have on-screen
	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;
	bool valid = false;

	for(std::vector<map_location>::const_iterator itor = locs.begin(); itor != locs.end() ; ++itor) {
		if(get_map().on_board(*itor) == false) continue;
		if(check_fogged && fogged(*itor)) continue;

		int x = get_location_x(*itor);
		int y = get_location_y(*itor);

		if (!valid) {
			minx = x;
			maxx = x;
			miny = y;
			maxy = y;
			valid = true;
		} else {
			int minx_new = std::min<int>(minx,x);
			int miny_new = std::min<int>(miny,y);
			int maxx_new = std::max<int>(maxx,x);
			int maxy_new = std::max<int>(maxy,y);
			SDL_Rect r = map_area();
			r.x = minx_new;
			r.y = miny_new;
			if(outside_area(r, maxx_new, maxy_new)) {
				// we cannot fit all locations to the screen
				if (only_if_possible) return;
				break;
			}
			minx = minx_new;
			miny = miny_new;
			maxx = maxx_new;
			maxy = maxy_new;

		}
	}
	//if everything is fogged or the locs list is empty
	if(!valid) return;

	if (scroll_type == ONSCREEN) {
		SDL_Rect r = map_area();
		int spacing = round_double(add_spacing*hex_size());
		r.x += spacing;
		r.y += spacing;
		r.w -= 2*spacing;
		r.h -= 2*spacing;
		if (!outside_area(r, minx,miny) && !outside_area(r, maxx,maxy)) {
			return;
		}
	}

	// let's do "normal" rectangle math from now on
	SDL_Rect locs_bbox;
	locs_bbox.x = minx;
	locs_bbox.y = miny;
	locs_bbox.w = maxx - minx + hex_size();
	locs_bbox.h = maxy - miny + hex_size();

	// target the center
	int target_x = locs_bbox.x + locs_bbox.w/2;
	int target_y = locs_bbox.y + locs_bbox.h/2;

	if (scroll_type == ONSCREEN) {
		// when doing an ONSCREEN scroll we do not center the target unless needed
		SDL_Rect r = map_area();
		int map_center_x = r.x + r.w/2;
		int map_center_y = r.y + r.h/2;

		int h = r.h;
		int w = r.w;

		// we do not want to be only inside the screen rect, but center a bit more
		double inside_frac = 0.5; // 0.0 = always center the target, 1.0 = scroll the minimum distance
		w = static_cast<int>(w * inside_frac);
		h = static_cast<int>(h * inside_frac);

		// shrink the rectangle by the size of the locations rectangle we found
		// such that the new task to fit a point into a rectangle instead of rectangle into rectangle
		w -= locs_bbox.w;
		h -= locs_bbox.h;

		if (w < 1) w = 1;
		if (h < 1) h = 1;

		r.x = target_x - w/2;
		r.y = target_y - h/2;
		r.w = w;
		r.h = h;

		// now any point within r is a possible target to scroll to
		// we take the one with the minimum distance to map_center
		// which will always be at the border of r

		if (map_center_x < r.x) {
			target_x = r.x;
			target_y = map_center_y;
			if (target_y < r.y) target_y = r.y;
			if (target_y > r.y+r.h-1) target_y = r.y+r.h-1;
		} else if (map_center_x > r.x+r.w-1) {
			target_x = r.x+r.w-1;
			target_y = map_center_y;
			if (target_y < r.y) target_y = r.y;
			if (target_y >= r.y+r.h) target_y = r.y+r.h-1;
		} else if (map_center_y < r.y) {
			target_y = r.y;
			target_x = map_center_x;
			if (target_x < r.x) target_x = r.x;
			if (target_x > r.x+r.w-1) target_x = r.x+r.w-1;
		} else if (map_center_y > r.y+r.h-1) {
			target_y = r.y+r.h-1;
			target_x = map_center_x;
			if (target_x < r.x) target_x = r.x;
			if (target_x > r.x+r.w-1) target_x = r.x+r.w-1;
		} else {
			ERR_DP << "Bug in the scrolling code? Looks like we would not need to scroll after all...\n";
			// keep the target at the center
		}
	}

	scroll_to_xy(target_x, target_y,scroll_type,force);
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
	const int xend = static_cast<int>(tile_width * (get_map().w() + 2 * theme_.border().size) + tile_width/3);
	const int yend = static_cast<int>(zoom_ * (get_map().h() + 2 * theme_.border().size) + zoom_/2);

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
	idle_anim_rate_ = std::pow(2.0, -rate/10.0);
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

	labels().recalculate_labels();

	redraw_background_ = true;

	int ticks1 = SDL_GetTicks();
	invalidate_all();
	int ticks2 = SDL_GetTicks();
	draw(true,true);
	int ticks3 = SDL_GetTicks();
	LOG_DP << "invalidate and draw: " << (ticks3 - ticks2) << " and " << (ticks2 - ticks1) << "\n";

	BOOST_FOREACH (boost::function<void(display&)> f, redraw_observers_) {
		f(*this);
	}
	
	complete_redraw_event_.notify_observers();
}

void display::add_redraw_observer(boost::function<void(display&)> f)
{
	redraw_observers_.push_back(f);
}

void display::clear_redraw_observers()
{
	redraw_observers_.clear();
}

void display::draw(bool update,bool force) {
//	log_scope("display::draw");
	if (screen_.update_locked()) {
		return;
	}
	bool changed = draw_init();
	pre_draw();
	// invalidate all that needs to be invalidated
	invalidate_animations();
	update_time_of_day();
	// at this stage we have everything that needs to be invalidated for this redraw
	// save it as the previous invalidated, and merge with the previous invalidated_
	// we merge with the previous redraw because if a hex had a unit last redraw but
	// not this one, nobody will tell us to redraw (cleanup)
	previous_invalidated_.swap(invalidated_);
	invalidated_.insert(previous_invalidated_.begin(),previous_invalidated_.end());
	// these new invalidations can not cause any propagation because
	// if a hex was invalidated last turn but not this turn, then
	// * case of no unit in neighbour hex=> no propagation
	// * case of unit in hex but was there last turn=>its hexes are invalidated too
	// * case of unit inhex not there last turn => it moved, so was invalidated previously
	if(!get_map().empty()) {
		//int simulate_delay = 0;

		/*
		 * draw_invalidated() also invalidates the halos, so also needs to be
		 * ran if invalidated_.empty() == true.
		 */
		if(!invalidated_.empty() || preferences::show_haloes()) {
			changed = true;
			draw_invalidated();
			invalidated_.clear();
		}
		drawing_buffer_commit();
		post_commit();
		draw_sidebar();
		/** @todo FIXME: This changed can probably be smarter */
		changed = true;
		// Simulate slow PC:
		//SDL_Delay(2*simulate_delay + rand() % 20);
	}
	draw_wrap(update, force, changed);
}

map_labels& display::labels()
{
	return *map_labels_;
}

const map_labels& display::labels() const
{
	return *map_labels_;
}

void display::clear_screen()
{
	surface const disp(screen_.getSurface());
	SDL_Rect area = screen_area();
	SDL_FillRect(disp, &area, SDL_MapRGB(disp->format, 0, 0, 0));
}

const SDL_Rect& display::get_clip_rect()
{
	return map_area();
}

void display::draw_invalidated() {
//	log_scope("display::draw_invalidated");
	SDL_Rect clip_rect = get_clip_rect();
	surface screen = get_screen_surface();
	clip_rect_setter set_clip_rect(screen, clip_rect);
	BOOST_FOREACH (map_location loc, invalidated_) {
		int xpos = get_location_x(loc);
		int ypos = get_location_y(loc);
		const bool on_map = get_map().on_board(loc);
		SDL_Rect hex_rect = {xpos, ypos, zoom_, zoom_};
		if(!rects_overlap(hex_rect,clip_rect)) {
			continue;
		}
		draw_hex(loc);
		drawn_hexes_+=1;
		// If the tile is at the border, we start to blend it
		if(!on_map) {
			 draw_border(loc, xpos, ypos);
		}
	}
	invalidated_hexes_ += invalidated_.size();
}

void display::draw_hex(const map_location& loc) {
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	image::TYPE image_type = get_image_type(loc);
	const bool on_map = get_map().on_board(loc);
	const bool off_map_tile = (get_map().get_terrain(loc) == t_translation::OFF_MAP_USER);
	if(!shrouded(loc)) {
		// unshrouded terrain (the normal case)
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, tblit(xpos, ypos,
			get_terrain_images(loc,tod_.id, image_type, ADJACENT_BACKGROUND)));

		 drawing_buffer_add(LAYER_TERRAIN_FG, loc, tblit(xpos, ypos,
			get_terrain_images(loc,tod_.id,image_type,ADJACENT_FOREGROUND)));

	// Draw the grid, if that's been enabled
		if(grid_ && on_map && !off_map_tile) {
			drawing_buffer_add(LAYER_TERRAIN_TMP_BG, loc, tblit(xpos, ypos,
				image::get_image(game_config::grid_image, image::SCALED_TO_HEX)));
		}
	}

	// Paint selection and mouseover overlays
	if(loc == selectedHex_ && on_map && selected_hex_overlay_ != NULL) {
		drawing_buffer_add(LAYER_TERRAIN_TMP_BG, loc, tblit(xpos, ypos, selected_hex_overlay_));
	}
	if(loc == mouseoverHex_ && (on_map || (in_editor() && get_map().on_board_with_border(loc))) && mouseover_hex_overlay_ != NULL) {
		drawing_buffer_add(LAYER_TERRAIN_TMP_BG, loc, tblit(xpos, ypos, mouseover_hex_overlay_));
	}

	// Apply shroud, fog and linger overlay
	if(shrouded(loc)) {
		// We apply void also on off-map tiles
		// to shroud the half-hexes too
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(xpos, ypos,
			image::get_image(shroud_image_, image_type)));
	} else if(fogged(loc)) {
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(xpos, ypos,
			image::get_image(fog_image_, image_type)));
	}

	if(!shrouded(loc)) {
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(xpos, ypos,
			get_terrain_images(loc, tod_.id, image_type, ADJACENT_FOGSHROUD)));
	}
	if (on_map) {
		if (draw_coordinates_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(loc), font::SIZE_SMALL, font::NORMAL_COLOUR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect = {0, 0, text->w, text->h};
			SDL_FillRect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			if (draw_terrain_codes_) {
				off_y -= text->h;
			} else {
				off_y -= text->h / 2;
			}
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(off_x, off_y, bg));
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(off_x, off_y, text));
		}
		if (draw_terrain_codes_ && (game_config::debug || !shrouded(loc))) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(get_map().get_terrain(loc)), font::SIZE_SMALL, font::NORMAL_COLOUR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect = {0, 0, text->w, text->h};
			SDL_FillRect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			if (!draw_coordinates_) {
				off_y -= text->h / 2;
			}
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(off_x, off_y, bg));
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(off_x, off_y, text));
		}
	}
}

image::TYPE display::get_image_type(const map_location& /*loc*/) {
	return image::SCALED_TO_HEX;
}

void display::update_time_of_day() {
	//no action
}

void display::draw_sidebar() {

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
			img.assign(get_surface_portion(img,visible_area,false));
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

void display::refresh_report(reports::TYPE report_num, reports::report report)
{
	const theme::status_item* const item = theme_.get_status_item(reports::report_name(report_num));
	if (!item) {
		reportSurfaces_[report_num].assign(NULL);
		return;
	}

	SDL_Rect &rect = reportRects_[report_num];
	const SDL_Rect &new_rect = item->location(screen_area());

	// Report and its location is unchanged since last time. Do nothing.
	if (rect == new_rect && reports_[report_num] == report) {
		return;
	}

	reports_[report_num] = report;

	surface &surf = reportSurfaces_[report_num];

	if (surf) {
		SDL_BlitSurface(surf, NULL, screen_.getSurface(), &rect);
		update_rect(rect);
	}

	// If the rectangle has just changed, assign the surface to it
	if (new_rect != rect || !surf)
	{
		surf.assign(NULL);
		rect = new_rect;

		// If the rectangle is present, and we are blitting text,
		// then we need to backup the surface.
		// (Images generally won't need backing up,
		// unless they are transperant, but that is done later).
		if (rect.w > 0 && rect.h > 0) {
			surf.assign(get_surface_portion(screen_.getSurface(), rect));
			if (reportSurfaces_[report_num] == NULL) {
				ERR_DP << "Could not backup background for report!\n";
			}
		}
		update_rect(rect);
	}

	tooltips::clear_tooltips(rect);

	if (report.empty()) return;

	int x = rect.x, y = rect.y;

	// Add prefix, postfix elements.
	// Make sure that they get the same tooltip
	// as the guys around them.
	std::string str = item->prefix();
	if (!str.empty()) {
		report.insert(report.begin(), reports::element(str, "", report.begin()->tooltip));
	}
	str = item->postfix();
	if (!str.empty()) {
		report.push_back(reports::element(str, "", report.back().tooltip));
	}

	// Loop through and display each report element.
	int tallest = 0;
	int image_count = 0;
	bool used_ellipsis = false;
	std::ostringstream ellipsis_tooltip;
	SDL_Rect ellipsis_area = rect;

	BOOST_FOREACH (const reports::element &e, report)
	{
		SDL_Rect area = { x, y, rect.w + rect.x - x, rect.h + rect.y - y };
		if (area.h <= 0) break;

		if (!e.text.empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw a text element.
			font::ttext text;
			if (item->font_rgb_set()) {
				text.set_foreground_colour(item->font_rgb());
			}
			std::string t = e.text;
			bool eol = false;
			if (t[t.size() - 1] == '\n') {
				eol = true;
				t = t.substr(0, t.size() - 1);
			}
			text.set_font_size(item->font_size());
			text.set_text(t, true);
			text.set_maximum_width(area.w);
			text.set_maximum_height(area.h, false);
			surface s = text.render();
			screen_.blit_surface(x, y, s);
			area.w = s->w;
			area.h = s->h;
			if (area.h > tallest) {
				tallest = area.h;
			}
			if (eol) {
				x = rect.x;
				y += tallest;
				tallest = 0;
			} else {
				x += area.w;
			}
		}
		else if (!e.image.get_filename().empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw an image element.
			surface img(image::get_image(e.image));

			if (!img) {
				ERR_DP << "could not find image for report: '" << e.image.get_filename() << "'\n";
				continue;
			}

			if (area.w < img->w && image_count) {
				// We have more than one image, and this one doesn't fit.
				img = surface(image::get_image(game_config::ellipsis_image));
				used_ellipsis = true;
			}

			if (img->w < area.w) area.w = img->w;
			if (img->h < area.h) area.h = img->h;
			draw_image_for_report(img, area);

			++image_count;
			if (area.h > tallest) {
				tallest = area.h;
			}

			if (!used_ellipsis) {
				x += area.w;
			} else {
				ellipsis_area = area;
			}
		}
		else
		{
			// No text nor image, skip this element
			continue;
		}

		skip_element:
		if (!e.tooltip.empty()) {
			if (!used_ellipsis) {
				tooltips::add_tooltip(area, e.tooltip);
			} else {
				// Collect all tooltips for the ellipsis.
				ellipsis_tooltip << e.tooltip << '\n';
			}
		}
	}

	if (used_ellipsis) {
		tooltips::add_tooltip(ellipsis_area, ellipsis_tooltip.str());
	}
}

void display::invalidate_all()
{
	DBG_DP << "invalidate_all()\n";
	invalidateAll_ = true;
	invalidated_.clear();
	update_rect(map_area());
}

bool display::invalidate(const map_location& loc)
{
	if(invalidateAll_)
		return false;

	return invalidated_.insert(loc).second;
}

bool display::invalidate(const std::set<map_location>& locs)
{
	if(invalidateAll_)
		return false;
	bool ret = false;
	BOOST_FOREACH (const map_location& loc, locs) {
		ret = invalidated_.insert(loc).second || ret;
	}
	return ret;
}

bool display::propagate_invalidation(const std::set<map_location>& locs)
{
	if(invalidateAll_)
		return false;

	if(locs.size()<=1)
		return false; // propagation never needed

	// search the first hex invalidated (if any)
	std::set<map_location>::const_iterator i = locs.begin();
	for(; i != locs.end() && invalidated_.count(*i) == 0 ; ++i) {}

	if (i == locs.end())
		return false; // no invalidation, don't propagate

	// propagate invalidation
	// 'i' is already in, but I suspect that splitting the range is bad
	// especially because locs are often adjacents
	size_t previous_size = invalidated_.size();
	invalidated_.insert(locs.begin(), locs.end());
	return previous_size < invalidated_.size();
}

bool display::invalidate_visible_locations_in_rect(const SDL_Rect& rect)
{
	return invalidate_locations_in_rect(intersect_rects(map_area(),rect));
}

bool display::invalidate_locations_in_rect(const SDL_Rect& rect)
{
	if(invalidateAll_)
		return false;

	bool result = false;
	BOOST_FOREACH (const map_location &loc, hexes_under_rect(rect)) {
		result |= invalidate(loc);
	}
	return result;
}

void display::invalidate_animations()
{
	if (!preferences::animate_map()) {
		return;
	}

	BOOST_FOREACH (const map_location &loc, get_visible_hexes())
	{
		if (shrouded(loc)) continue;
		if (builder_->update_animation(loc)) {
			invalidate(loc);
		} else {
			invalidate_animations_location(loc);
		}
	}
}
