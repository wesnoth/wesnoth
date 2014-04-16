/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Routines to set up the display, scroll and zoom the map.
 */

#include "builder.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "minimap.hpp"
#include "reports.hpp"
#include "text.hpp"
#include "time_of_day.hpp"
#include "tooltips.hpp"
#include "arrow.hpp"
#include "tod_manager.hpp"
#include "resources.hpp"
#include "whiteboard/manager.hpp"
#include "overlay.hpp"
#include "synced_context.hpp"

#include "SDL_image.h"

#include <boost/foreach.hpp>

#ifdef __SUNPRO_CC
// GCC doesn't have hypot in cmath so include it for Sun Studio
#include <math.h>
#endif
#include <cmath>

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
	const int DefaultZoom = game_config::tile_size;
	const int SmallZoom   = DefaultZoom / 2;

	const int MinZoom = 4;
	const int MaxZoom = 200;
	size_t sunset_delay = 0;

	bool benchmark = false;

	bool debug_foreground = false;
}

int display::last_zoom_ = SmallZoom;

void display::parse_team_overlays()
{
	const team& curr_team = (*teams_)[playing_team()];
	const team& prev_team = (*teams_)[playing_team()-1 < teams_->size() ? playing_team()-1 : teams_->size()-1];
	BOOST_FOREACH(const game_display::overlay_map::value_type i, *overlays_) {
		const overlay& ov = i.second;
		if (!ov.team_name.empty() &&
			((ov.team_name.find(curr_team.team_name()) + 1) != 0) !=
			((ov.team_name.find(prev_team.team_name()) + 1) != 0))
		{
			invalidate(i.first);
		}
	}
}


void display::add_overlay(const map_location& loc, const std::string& img, const std::string& halo,const std::string& team_name, bool visible_under_fog)
{
	const int halo_handle = halo::add(get_location_x(loc) + hex_size() / 2,
			get_location_y(loc) + hex_size() / 2, halo, loc);

	const overlay item(img, halo, halo_handle, team_name, visible_under_fog);
	overlays_->insert(overlay_map::value_type(loc,item));
}

void display::remove_overlay(const map_location& loc)
{
	typedef overlay_map::const_iterator Itor;
	std::pair<Itor,Itor> itors = overlays_->equal_range(loc);
	while(itors.first != itors.second) {
		halo::remove(itors.first->second.halo_handle);
		++itors.first;
	}

	overlays_->erase(loc);
}

void display::remove_single_overlay(const map_location& loc, const std::string& toDelete)
{
	//Iterate through the values with key of loc
	typedef overlay_map::iterator Itor;
	overlay_map::iterator iteratorCopy;
	std::pair<Itor,Itor> itors = overlays_->equal_range(loc);
	while(itors.first != itors.second) {
		//If image or halo of overlay struct matches toDelete, remove the overlay
		if(itors.first->second.image == toDelete || itors.first->second.halo == toDelete) {
			iteratorCopy = itors.first;
			++itors.first;
			halo::remove(iteratorCopy->second.halo_handle);
			overlays_->erase(iteratorCopy);
		}
		else {
			++itors.first;
		}
	}
}




display::display(unit_map* units, CVideo& video, const gamemap* map, const std::vector<team>* t,const config& theme_cfg, const config& level) :
	units_(units),
	exclusive_unit_draw_requests_(),
	screen_(video),
	map_(map),
	currentTeam_(0),
	teams_(t),
	viewpoint_(NULL),
	energy_bar_rects_(),
	xpos_(0),
	ypos_(0),
	view_locked_(false),
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
	scroll_event_("scrolled"),
	complete_redraw_event_("completely_redrawn"),
	nextDraw_(0),
	reportRects_(),
	reportSurfaces_(),
	reports_(),
	menu_buttons_(),
	action_buttons_(),
	sliders_(),
	invalidated_(),
	previous_invalidated_(),
	mouseover_hex_overlay_(NULL),
	tod_hex_mask1(NULL),
	tod_hex_mask2(NULL),
	fog_images_(),
	shroud_images_(),
	selectedHex_(),
	mouseoverHex_(),
	keys_(),
	animate_map_(true),
	local_tod_light_(false),
	flags_(),
	activeTeam_(0),
	drawing_buffer_(),
	map_screenshot_(false),
	reach_map_(),
	reach_map_old_(),
	reach_map_changed_(true),
	overlays_(NULL),
	fps_handle_(0),
	invalidated_hexes_(0),
	drawn_hexes_(0),
	idle_anim_(preferences::idle_anim()),
	idle_anim_rate_(1.0),
	map_screenshot_surf_(NULL),
	redraw_observers_(),
	draw_coordinates_(false),
	draw_terrain_codes_(false),
	arrows_map_(),
	color_adjust_()
#if defined(__GLIBC__)
	, do_reverse_memcpy_workaround_(false)
#endif
{
	singleton_ = this;

	blindfold_ctr_ = 0;

	read(level.child_or_empty("display"));

	if(non_interactive()
		&& (get_video_surface() != NULL
		&& video.faked())) {
		screen_.lock_updates(true);
	}

	fill_images_list(game_config::fog_prefix, fog_images_);
	fill_images_list(game_config::shroud_prefix, shroud_images_);

	set_idle_anim_rate(preferences::idle_anim_rate());

	image::set_zoom(zoom_);

	init_flags();

#if defined(__GLIBC__) && !SDL_VERSION_ATLEAST(2,0,0)
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
	singleton_ = NULL;
}


void display::init_flags() {

	flags_.clear();
	if (!teams_) return;
	flags_.resize(teams_->size());

	std::vector<std::string> side_colors;
	side_colors.reserve(teams_->size());

	for(size_t i = 0; i != teams_->size(); ++i) {
		std::string side_color = team::get_side_color_index(i+1);
		side_colors.push_back(side_color);
		init_flags_for_side_internal(i, side_color);
	}
	image::set_team_colors(&side_colors);
}

void display::reinit_flags_for_side(size_t side)
{
	if (!teams_ || side >= teams_->size()) {
		ERR_DP << "Cannot rebuild flags for inexistent or unconfigured side " << side << '\n';
		return;
	}

	init_flags_for_side_internal(side, team::get_side_color_index(side + 1));
}

void display::init_flags_for_side_internal(size_t n, const std::string& side_color)
{
	assert(teams_ != NULL);
	assert(n < teams_->size());
	assert(n < flags_.size());

	std::string flag = (*teams_)[n].flag();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = side_color;

	if(flag.empty()) {
		flag = game_config::images::flag;
	}

	LOG_DP << "Adding flag for team " << n << " from animation " << flag << "\n";

	// Must recolor flag image
	animated<image::locator> temp_anim;

	std::vector<std::string> items = utils::square_parenthetical_split(flag);
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

	animated<image::locator>& f = flags_[n];

	f = temp_anim;
	f.start_animation(rand() % f.get_end_time(), true);
}

struct is_energy_color {
	bool operator()(Uint32 color) const { return (color&0xFF000000) > 0x10000000 &&
	                                              (color&0x00FF0000) < 0x00100000 &&
												  (color&0x0000FF00) < 0x00001000 &&
												  (color&0x000000FF) < 0x00000010; }
};

surface display::get_flag(const map_location& loc)
{
	t_translation::t_terrain terrain = get_map().get_terrain(loc);

	if(!get_map().is_village(terrain)) {
		return surface(NULL);
	}

	for(size_t i = 0; i != teams_->size(); ++i) {
		if((*teams_)[i].owns_village(loc) &&
		  (!fogged(loc) || !(*teams_)[currentTeam_].is_enemy(i+1)))
		{
			flags_[i].update_last_draw_time();
			const image::locator &image_flag = animate_map_ ?
				flags_[i].get_current_frame() : flags_[i].get_first_frame();
			return image::get_image(image_flag, image::TOD_COLORED);
		}
	}

	return surface(NULL);
}

void display::set_team(size_t teamindex, bool show_everything)
{
	assert(teamindex < teams_->size());
	currentTeam_ = teamindex;
	if (!show_everything)
	{
		labels().set_team(&(*teams_)[teamindex]);
		viewpoint_ = &(*teams_)[teamindex];
	}
	else
	{
		labels().set_team(NULL);
		viewpoint_ = NULL;
	}
	labels().recalculate_labels();
	if(resources::whiteboard)
		resources::whiteboard->on_viewer_change(teamindex);
}

void display::set_playing_team(size_t teamindex)
{
	assert(teamindex < teams_->size());
	activeTeam_ = teamindex;
	invalidate_game_status();
}

const SDL_Rect& display::calculate_energy_bar(surface surf)
{
	const std::map<surface,SDL_Rect>::const_iterator i = energy_bar_rects_.find(surf);
	if(i != energy_bar_rects_.end()) {
		return i->second;
	}

	int first_row = -1, last_row = -1, first_col = -1, last_col = -1;

	surface image(make_neutral_surface(surf));

	const_surface_lock image_lock(image);
	const Uint32* const begin = image_lock.pixels();

	for(int y = 0; y != image->h; ++y) {
		const Uint32* const i1 = begin + image->w*y;
		const Uint32* const i2 = i1 + image->w;
		const Uint32* const itor = std::find_if(i1,i2,is_energy_color());
		const int count = std::count_if(itor,i2,is_energy_color());

		if(itor != i2) {
			if(first_row == -1) {
				first_row = y;
			}

			first_col = itor - i1;
			last_col = first_col + count;
			last_row = y;
		}
	}

	const SDL_Rect res = create_rect(first_col
			, first_row
			, last_col-first_col
			, last_row+1-first_row);
	energy_bar_rects_.insert(std::pair<surface,SDL_Rect>(surf,res));
	return calculate_energy_bar(surf);
}



void display::draw_bar(const std::string& image, int xpos, int ypos,
		const map_location& loc, size_t height, double filled,
		const SDL_Color& col, fixed_t alpha)
{

	filled = std::min<double>(std::max<double>(filled,0.0),1.0);
	height = static_cast<size_t>(height*get_zoom_factor());

	surface surf(image::get_image(image,image::SCALED_TO_HEX));

	// We use UNSCALED because scaling (and bilinear interpolation)
	// is bad for calculate_energy_bar.
	// But we will do a geometric scaling later.
	surface bar_surf(image::get_image(image));
	if(surf == NULL || bar_surf == NULL) {
		return;
	}

	// calculate_energy_bar returns incorrect results if the surface colors
	// have changed (for example, due to bilinear interpolation)
	const SDL_Rect& unscaled_bar_loc = calculate_energy_bar(bar_surf);

	SDL_Rect bar_loc;
	if (surf->w == bar_surf->w && surf->h == bar_surf->h)
	  bar_loc = unscaled_bar_loc;
	else {
	  const fixed_t xratio = fxpdiv(surf->w,bar_surf->w);
	  const fixed_t yratio = fxpdiv(surf->h,bar_surf->h);
	  const SDL_Rect scaled_bar_loc = create_rect(
			    fxptoi(unscaled_bar_loc. x * xratio)
			  , fxptoi(unscaled_bar_loc. y * yratio + 127)
			  , fxptoi(unscaled_bar_loc. w * xratio + 255)
			  , fxptoi(unscaled_bar_loc. h * yratio + 255));
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

	SDL_Rect top = create_rect(0, 0, surf->w, bar_loc.y);
	SDL_Rect bot = create_rect(0, bar_loc.y + skip_rows, surf->w, 0);
	bot.h = surf->w - bot.y;

	drawing_buffer_add(LAYER_UNIT_BAR, loc, xpos, ypos, surf, top);
	drawing_buffer_add(LAYER_UNIT_BAR, loc, xpos, ypos + top.h, surf, bot);

	size_t unfilled = static_cast<size_t>(height * (1.0 - filled));

	if(unfilled < height && alpha >= ftofxp(0.3)) {
		const Uint8 r_alpha = std::min<unsigned>(unsigned(fxpmult(alpha,255)),255);
		surface filled_surf = create_compatible_surface(bar_surf, bar_loc.w, height - unfilled);
		SDL_Rect filled_area = create_rect(0, 0, bar_loc.w, height-unfilled);
		sdl_fill_rect(filled_surf,&filled_area,SDL_MapRGBA(bar_surf->format,col.r,col.g,col.b, r_alpha));
		drawing_buffer_add(LAYER_UNIT_BAR, loc, xpos + bar_loc.x, ypos + bar_loc.y + unfilled, filled_surf);
	}
}


bool display::add_exclusive_draw(const map_location& loc, unit& unit)
{
	if (loc.valid() && exclusive_unit_draw_requests_.find(loc) == exclusive_unit_draw_requests_.end())
	{
		exclusive_unit_draw_requests_[loc] = unit.id();
		return true;
	}
	else
	{
		return false;
	}
}

std::string display::remove_exclusive_draw(const map_location& loc)
{
	std::string id = "";
	if(loc.valid())
	{
		id = exclusive_unit_draw_requests_[loc];
		//id will be set to the default "" string by the [] operator if the map doesn't have anything for that loc.
		exclusive_unit_draw_requests_.erase(loc);
	}
	return id;
}

const time_of_day & display::get_time_of_day(const map_location& /*loc*/) const
{
	static time_of_day tod;
	return tod;
}

void display::update_tod() {
	const time_of_day& tod = get_time_of_day();
	tod_color col = color_adjust_ + tod.color;
	image::set_color_adjustment(col.r, col.g, col.b);
}

void display::adjust_color_overlay(int r, int g, int b) {
	color_adjust_ = tod_color(r, g, b);
	update_tod();
}


void display::fill_images_list(const std::string& prefix, std::vector<std::string>& images)
{
	// search prefix.png, prefix1.png, prefix2.png ...
	for(int i=0; ; ++i){
		std::ostringstream s;
		s << prefix;
		if(i != 0)
			s << i;
		s << ".png";
		if(image::exists(s.str()))
			images.push_back(s.str());
		else if(i>0)
			break;
	}
	if (images.empty())
		images.push_back("");
}

const std::string& display::get_variant(const std::vector<std::string>& variants, const map_location &loc) const
{
	//TODO use better noise function
	return variants[abs(loc.x + loc.y) % variants.size()];
}

void display::rebuild_all()
{
	builder_->rebuild_all();
}

void display::reload_map()
{
	redraw_background_ = true;
	builder_->reload_map();
}

void display::change_map(const gamemap* m)
{
	map_ = m;
	builder_->change_map(m);
}

void display::change_units(unit_map* umap)
{
	units_ = umap;
}

void display::change_teams(const std::vector<team>* teams)
{
	teams_ = teams;
}

void display::blindfold(bool value)
{
	if(value == true)
		++blindfold_ctr_;
	else
		--blindfold_ctr_;
}

bool display::is_blindfolded() const
{
	return blindfold_ctr_ > 0;
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

// This function uses the screen as reference
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


// This function uses the rect of map_area as reference
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
}

display::rect_of_hexes::iterator& display::rect_of_hexes::iterator::operator++()
{
	if (loc_.y < rect_.bottom[loc_.x & 1])
		++loc_.y;
	else {
		++loc_.x;
		loc_.y = rect_.top[loc_.x & 1];
	}

	return *this;
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
	// we minus "0.(3)", for horizontal imbrication.
	// reason is: two adjacent hexes each overlap 1/4 of their width, so for
	// grid calculation 3/4 of tile width is used, which by default gives
	// 18/54=0.(3). Note that, while tile_width is zoom dependand, 0.(3) is not.
	res.left = static_cast<int>(std::floor(-border + x / tile_width - 0.3333333));
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

	// we transform the coordinates from minimap to the full map image
	// probably more adjustments to do (border, minimap shift...)
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

gui::button* display::find_action_button(const std::string& id)
{
	for (size_t i = 0; i < action_buttons_.size(); ++i) {
		if(action_buttons_[i].id() == id) {
			return &action_buttons_[i];
		}
	}
	return NULL;
}

gui::button* display::find_menu_button(const std::string& id)
{
	for (size_t i = 0; i < menu_buttons_.size(); ++i) {
		if(menu_buttons_[i].id() == id) {
			return &menu_buttons_[i];
		}
	}
	return NULL;
}

gui::slider* display::find_slider(const std::string& id)
{
	for (size_t i = 0; i < sliders_.size(); ++i) {
		if(sliders_[i].id() == id) {
			return &sliders_[i];
		}
	}
	return NULL;
}

void display::create_buttons()
{
	std::vector<gui::button> menu_work;
	std::vector<gui::button> action_work;
	std::vector<gui::slider> slider_work;

	DBG_DP << "creating sliders...\n";
	const std::vector<theme::slider>& sliders = theme_.sliders();
	for(std::vector<theme::slider>::const_iterator i = sliders.begin(); i != sliders.end(); ++i) {
		gui::slider s(screen_, i->image(), i->black_line());
		DBG_DP << "drawing button " << i->get_id() << "\n";
		s.set_id(i->get_id());
		const SDL_Rect& loc = i->location(screen_area());
		s.set_location(loc);
		//TODO support for non zoom sliders
		s.set_max(MaxZoom);
		s.set_min(MinZoom);
		s.set_value(zoom_);
		if (!i->tooltip().empty()){
			s.set_tooltip_string(i->tooltip());
		}
		if(rects_overlap(s.location(),map_outside_area())) {
			s.set_volatile(true);
		}

		gui::slider* s_prev = find_slider(s.id());
		if(s_prev) {
			s.set_max(s_prev->max_value());
			s.set_min(s_prev->min_value());
			s.set_value(s_prev->value());
			s.enable(s_prev->enabled());
		}

		slider_work.push_back(s);
	}

	DBG_DP << "creating menu buttons...\n";
	const std::vector<theme::menu>& buttons = theme_.menus();
	for(std::vector<theme::menu>::const_iterator i = buttons.begin(); i != buttons.end(); ++i) {

		if (!i->is_button()) continue;

		gui::button b(screen_, i->title(), gui::button::TYPE_PRESS, i->image(),
				gui::button::DEFAULT_SPACE, true, i->overlay());
		DBG_DP << "drawing button " << i->get_id() << "\n";
		b.set_id(i->get_id());
		const SDL_Rect& loc = i->location(screen_area());
		b.set_location(loc.x,loc.y);
		if (!i->tooltip().empty()){
			b.set_tooltip_string(i->tooltip());
		}
		if(rects_overlap(b.location(),map_outside_area())) {
			b.set_volatile(true);
		}

		gui::button* b_prev = find_menu_button(b.id());
		if(b_prev) b.enable(b_prev->enabled());

		menu_work.push_back(b);
	}
	DBG_DP << "creating action buttons...\n";
	const std::vector<theme::action>& actions = theme_.actions();
	for(std::vector<theme::action>::const_iterator i = actions.begin(); i != actions.end(); ++i) {
		gui::button b(screen_, i->title(), string_to_button_type(i->type()), i->image(),
				gui::button::DEFAULT_SPACE, true, i->overlay());

		DBG_DP << "drawing button " << i->get_id() << "\n";
		b.set_id(i->get_id());
		const SDL_Rect& loc = i->location(screen_area());
		b.set_location(loc.x,loc.y);
		if (!i->tooltip(0).empty()){
			b.set_tooltip_string(i->tooltip(0));
		}
		if(rects_overlap(b.location(),map_outside_area())) {
			b.set_volatile(true);
		}

		gui::button* b_prev = find_action_button(b.id());
		if(b_prev) b.enable(b_prev->enabled());

		action_work.push_back(b);
	}

	menu_buttons_.swap(menu_work);
	action_buttons_.swap(action_work);
	sliders_.swap(slider_work);
	DBG_DP << "buttons created\n";
}

gui::button::TYPE display::string_to_button_type(std::string type)
{
	gui::button::TYPE res = gui::button::TYPE_PRESS;
	if (type == "checkbox") { res = gui::button::TYPE_CHECK; }
	else if (type == "image") { res = gui::button::TYPE_IMAGE; }
	else if (type == "radiobox") { res = gui::button::TYPE_RADIO; }
	else if (type == "turbo") { res = gui::button::TYPE_TURBO; }
	return res;
}

static const std::string& get_direction(size_t n)
{
	static std::string const dirs[6] = { "-n", "-ne", "-se", "-s", "-sw", "-nw" };
	return dirs[n >= sizeof(dirs)/sizeof(*dirs) ? 0 : n];
}

std::vector<surface> display::get_fog_shroud_images(const map_location& loc, image::TYPE image_type)
{
	std::vector<std::string> names;

	map_location adjacent[6];
	get_adjacent_tiles(loc,adjacent);

	enum visibility {FOG=0, SHROUD=1, CLEAR=2};
	visibility tiles[6];

	const std::string* image_prefix[] =
		{ &game_config::fog_prefix, &game_config::shroud_prefix};

	for(int i = 0; i != 6; ++i) {
		if(shrouded(adjacent[i])) {
			tiles[i] = SHROUD;
		} else if(!fogged(loc) && fogged(adjacent[i])) {
			tiles[i] = FOG;
		} else {
			tiles[i] = CLEAR;
		}
	}

	for(int v = FOG; v != CLEAR; ++v) {
		// Find somewhere that doesn't have overlap to use as a starting point
		int start;
		for(start = 0; start != 6; ++start) {
			if(tiles[start] != v) {
				break;
			}
		}

		if(start == 6) {
			// Completely surrounded by fog or shroud. This might have
			// a special graphic.
			const std::string name = *image_prefix[v] + "-all.png";
			if ( image::exists(name) ) {
				names.push_back(name);
				// Proceed to the next visibility (fog -> shroud -> clear).
				continue;
			}
			// No special graphic found. We'll just combine some other images
			// and hope it works out.
			start = 0;
		}

		// Find all the directions overlap occurs from
		for (int i = (start+1)%6, cap1 = 0;  i != start && cap1 != 6;  ++cap1) {
			if(tiles[i] == v) {
				std::ostringstream stream;
				std::string name;
				stream << *image_prefix[v];

				for (int cap2 = 0;  v == tiles[i] && cap2 != 6;  i = (i+1)%6, ++cap2) {
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
					names.push_back(name + ".png");
				}
			} else {
				i = (i+1)%6;
			}
		}
	}

	// now get the surfaces
	std::vector<surface> res;

	BOOST_FOREACH(std::string& name, names) {
		const surface surf(image::get_image(name, image_type));
		if (surf)
			res.push_back(surf);
	}

	return res;
}

std::vector<surface> display::get_terrain_images(const map_location &loc,
						     const std::string& timeid,
		image::TYPE image_type,
		TERRAIN_TYPE terrain_type)
{
	std::vector<surface> res;

	terrain_builder::TERRAIN_TYPE builder_terrain_type =
	      (terrain_type == FOREGROUND ?
		  terrain_builder::FOREGROUND : terrain_builder::BACKGROUND);

	const terrain_builder::imagelist* const terrains = builder_->get_terrain_at(loc,
			timeid, builder_terrain_type);

	image::light_string lt;
	bool use_local_light = local_tod_light_;
	if(use_local_light){
		const time_of_day& tod = get_time_of_day(loc);

		//get all the light transitions
		map_location adjs[6];
		get_adjacent_tiles(loc,adjs);
		for(int d=0; d<6; ++d){
			const time_of_day& atod = get_time_of_day(adjs[d]);
			if(atod.color == tod.color)
				continue;

			if(lt.empty()) {
				//color the full hex before adding transitions
				tod_color col = tod.color + color_adjust_;
				lt = image::get_light_string(6, col.r, col.g, col.b);
			}

			// add the directional transitions
			tod_color acol = atod.color + color_adjust_;
			lt += image::get_light_string(d, acol.r, acol.g, acol.b);
		}

		if(lt.empty()){
			if(tod.color == get_time_of_day().color) {
				use_local_light = false;
			} else {
				tod_color col = tod.color + color_adjust_;
				if(!col.is_zero()){
					// no real lightmap needed but still color the hex
					lt = image::get_light_string(-1, col.r, col.g, col.b);
				}
			}
		}
	}

	if(terrains != NULL) {
		// Cache the offmap name.
		// Since it is themeable it can change,
		// so don't make it static.
		const std::string off_map_name = "terrain/" + theme_.border().tile_image;
		for(std::vector<animated<image::locator> >::const_iterator it =
				terrains->begin(); it != terrains->end(); ++it) {

			const image::locator &image = animate_map_ ?
				it->get_current_frame() : it->get_first_frame();

			// We prevent ToD coloring and brightening of off-map tiles,
			// We need to test for the tile to be rendered and
			// not the location, since the transitions are rendered
			// over the offmap-terrain and these need a ToD coloring.

			surface surf;
			const bool off_map = image.get_filename() == off_map_name;
			if(!use_local_light || off_map) {
				surf = image::get_image(image, off_map ? image::SCALED_TO_HEX : image_type);
			} else if(lt.empty()) {
				surf = image::get_image(image, image::SCALED_TO_HEX);
			} else {
				surf = image::get_lighted_image(image, lt, image::SCALED_TO_HEX);
			}

			if (!surf.null()) {
				res.push_back(surf);
			}
		}
	}

	return res;
}

void display::drawing_buffer_add(const tdrawing_layer layer,
		const map_location& loc, int x, int y, const surface& surf,
		const SDL_Rect &clip)
{
	drawing_buffer_.push_back(tblit(layer, loc, x, y, surf, clip));
}

void display::drawing_buffer_add(const tdrawing_layer layer,
		const map_location& loc, int x, int y,
		const std::vector<surface> &surf,
		const SDL_Rect &clip)
{
	drawing_buffer_.push_back(tblit(layer, loc, x, y, surf, clip));
}

// FIXME: temporary method. Group splitting should be made
// public into the definition of tdrawing_layer
//
// The drawing is done per layer_group, the range per group is [low, high).
const display::tdrawing_layer display::drawing_buffer_key::layer_groups[] = {
	LAYER_TERRAIN_BG,
	LAYER_UNIT_FIRST,
	LAYER_UNIT_MOVE_DEFAULT,
	// Make sure the movement doesn't show above fog and reachmap.
	LAYER_REACHMAP,
	LAYER_LAST_LAYER
};

// no need to change this if layer_groups above is changed
const unsigned int display::drawing_buffer_key::max_layer_group = sizeof(display::drawing_buffer_key::layer_groups) / sizeof(display::tdrawing_layer) - 2;

enum {
	// you may adjust the following when needed:

	// maximum border. 3 should be safe even if a larger border is in use somewhere
	MAX_BORDER           = 3,

	// store x, y, and layer in one 32 bit integer
	// 4 most significant bits == layer group   => 16
	BITS_FOR_LAYER_GROUP = 4,

	// 10 second most significant bits == y     => 1024
	BITS_FOR_Y           = 10,

	// 1 third most significant bit == x parity => 2
	BITS_FOR_X_PARITY    = 1,

	// 8 fourth most significant bits == layer   => 256
	BITS_FOR_LAYER       = 8,

	// 9 least significant bits == x / 2        => 512 (really 1024 for x)
	BITS_FOR_X_OVER_2    = 9
};

inline display::drawing_buffer_key::drawing_buffer_key(const map_location &loc, tdrawing_layer layer)
	: key_(0)
{
	// max_layer_group + 1 is the last valid entry in layer_groups, but it is always > layer
	// thus the first --g is a given => start with max_layer_groups right away
	unsigned int g = max_layer_group;
	while (layer < layer_groups[g]) {
		--g;
	}

	enum {
		SHIFT_LAYER          = BITS_FOR_X_OVER_2,
		SHIFT_X_PARITY       = BITS_FOR_LAYER + SHIFT_LAYER,
		SHIFT_Y              = BITS_FOR_X_PARITY + SHIFT_X_PARITY,
		SHIFT_LAYER_GROUP    = BITS_FOR_Y + SHIFT_Y
	};
	BOOST_STATIC_ASSERT(SHIFT_LAYER_GROUP + BITS_FOR_LAYER_GROUP == sizeof(key_) * 8);

	// the parity of x must be more significant than the layer but less significant than y.
	// Thus basically every row is split in two: First the row containing all the odd x
	// then the row containing all the even x. Since thus the least significant bit of x is
	// not required for x ordering anymore it can be shifted out to the right.
	const unsigned int x_parity = static_cast<unsigned int>(loc.x) & 1;
	key_  = (g << SHIFT_LAYER_GROUP) | (static_cast<unsigned int>(loc.y + MAX_BORDER) << SHIFT_Y);
	key_ |= (x_parity << SHIFT_X_PARITY);
	key_ |= (static_cast<unsigned int>(layer) << SHIFT_LAYER) | static_cast<unsigned int>(loc.x + MAX_BORDER) / 2;
}

void display::drawing_buffer_commit()
{
	// std::list::sort() is a stable sort
	drawing_buffer_.sort();

	SDL_Rect clip_rect = map_area();
	surface screen = get_screen_surface();
	clip_rect_setter set_clip_rect(screen, &clip_rect);

	/*
	 * Info regarding the rendering algorithm.
	 *
	 * In order to render a hex properly it needs to be rendered per row. On
	 * this row several layers need to be drawn at the same time. Mainly the
	 * unit and the background terrain. This is needed since both can spill
	 * in the next hex. The foreground terrain needs to be drawn before to
	 * avoid decapitation a unit.
	 *
	 * This ended in the following priority order:
	 * layergroup > location > layer > 'tblit' > surface
	 */

	BOOST_FOREACH(const tblit &blit, drawing_buffer_) {
		BOOST_FOREACH(const surface& surf, blit.surf()) {
			// Note that dstrect can be changed by sdl_blit
			// and so a new instance should be initialized
			// to pass to each call to sdl_blit.
			SDL_Rect dstrect = create_rect(blit.x(), blit.y(), 0, 0);
			SDL_Rect srcrect = blit.clip();
			SDL_Rect *srcrectArg = (srcrect.x | srcrect.y | srcrect.w | srcrect.h)
				? &srcrect : NULL;
			sdl_blit(surf, srcrectArg, screen, &dstrect);
			//NOTE: the screen part should already be marked as 'to update'
		}
	}
	drawing_buffer_clear();
}

void display::drawing_buffer_clear()
{
	drawing_buffer_.clear();
}

void display::sunset(const size_t delay)
{
	// This allow both parametric and toggle use
	sunset_delay = (sunset_delay == 0 && delay == 0) ? 3 : delay;
}

void display::toggle_benchmark()
{
	benchmark = !benchmark;
}

void display::toggle_debug_foreground()
{
	debug_foreground = !debug_foreground;
}

void display::flip()
{
	if(video().faked()) {
		return;
	}

	surface frameBuffer = get_video_surface();

	// This is just the debug function "sunset" to progressively darken the map area
	static size_t sunset_timer = 0;
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

			font::floating_label flabel(stream.str());
			flabel.set_font_size(12);
			flabel.set_color(benchmark ? font::BAD_COLOR : font::NORMAL_COLOR);
			flabel.set_position(10, 100);
			flabel.set_alignment(font::LEFT_ALIGN);

			fps_handle_ = font::add_floating_label(flabel);
		}
	} else if(fps_handle_ != 0) {
		font::remove_floating_label(fps_handle_);
		fps_handle_ = 0;
		drawn_hexes_ = 0;
		invalidated_hexes_ = 0;
	}

	flip();
}

static void draw_panel(CVideo& video, const theme::panel& panel, std::vector<gui::button>& /*buttons*/)
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
			surf.assign(tile_surface(surf,loc.w,loc.h));
		}

		video.blit_surface(loc.x,loc.y,surf);
		update_rect(loc);
	}
}

static void draw_label(CVideo& video, surface target, const theme::label& label)
{
	//log_scope("draw label");

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

			sdl_blit(surf,NULL,target,&loc);
		}

		if(text.empty() == false) {
			tooltips::add_tooltip(loc,text);
		}
	} else if(text.empty() == false) {
		font::draw_text(&video,loc,label.font_size(),font::NORMAL_COLOR,text,loc.x,loc.y);
	}

	update_rect(loc);
}

void display::draw_all_panels()
{
	const surface& screen(screen_.getSurface());

	/*
	 * The minimap is also a panel, force it to update its contents.
	 * This is required when the size of the minimap has been modified.
	 */
	recalculate_minimap();

	const std::vector<theme::panel>& panels = theme_.panels();
	for(std::vector<theme::panel>::const_iterator p = panels.begin(); p != panels.end(); ++p) {
		draw_panel(video(), *p, menu_buttons_);
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
			SDL_Rect clip = create_rect(w_off, h_off, 0, 0);
			sdl_blit(background, NULL, screen, &clip);
		}
	}
}

void display::draw_text_in_hex(const map_location& loc,
		const tdrawing_layer layer, const std::string& text,
		size_t font_size, SDL_Color color, double x_in_hex, double y_in_hex)
{
	if (text.empty()) return;

	const size_t font_sz = static_cast<size_t>(font_size * get_zoom_factor());

	surface text_surf = font::get_rendered_text(text, font_sz, color);
	surface back_surf = font::get_rendered_text(text, font_sz, font::BLACK_COLOR);
	const int x = get_location_x(loc) - text_surf->w/2
	              + static_cast<int>(x_in_hex* hex_size());
	const int y = get_location_y(loc) - text_surf->h/2
	              + static_cast<int>(y_in_hex* hex_size());

	for (int dy=-1; dy <= 1; ++dy) {
		for (int dx=-1; dx <= 1; ++dx) {
			if (dx!=0 || dy!=0) {
				drawing_buffer_add(layer, loc, x + dx, y + dy, back_surf);
			}
		}
	}
	drawing_buffer_add(layer, loc, x, y, text_surf);
}

void display::render_image(int x, int y, const display::tdrawing_layer drawing_layer,
		const map_location& loc, surface image,
		bool hreverse, bool greyscale, fixed_t alpha,
		Uint32 blendto, double blend_ratio, double submerged, bool vreverse)
{

	if (image==NULL)
		return;

	SDL_Rect image_rect = create_rect(x, y, image->w, image->h);
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

	if(submerged > 0.0) {
		// divide the surface into 2 parts
		const int submerge_height = std::max<int>(0, surf->h*(1.0-submerged));
		const int depth = surf->h - submerge_height;
		SDL_Rect srcrect = create_rect(0, 0, surf->w, submerge_height);
		drawing_buffer_add(drawing_layer, loc, x, y, surf, srcrect);

		if(submerge_height != surf->h) {
			//the lower part will be transparent
			float alpha_base = 0.3f; // 30% alpha at surface of water
			float alpha_delta = 0.015f; // lose 1.5% per pixel depth
			alpha_delta *= zoom_ / DefaultZoom; // adjust with zoom
			surf = submerge_alpha(surf, depth, alpha_base, alpha_delta, false);

			srcrect.y = submerge_height;
			srcrect.h = surf->h-submerge_height;
			y += submerge_height;

			drawing_buffer_add(drawing_layer, loc, x, y, surf, srcrect);
		}
	} else {
		// simple blit
		drawing_buffer_add(drawing_layer, loc, x, y, surf);
	}

}

void display::select_hex(map_location hex)
{
	invalidate(selectedHex_);
	selectedHex_ = hex;
	invalidate(selectedHex_);
	recalculate_minimap();
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
		font::floating_label flabel(msg);
		flabel.set_font_size(font::SIZE_PLUS);
		flabel.set_color(font::YELLOW_COLOR);
		flabel.set_position(300, 50);
		flabel.set_clip_rect(map_outside_area());

		diagnostic_label_ = font::add_floating_label(flabel);
	}
}

void display::draw_init()
{
	if (get_map().empty()) {
		return;
	}

	if(benchmark) {
		invalidateAll_ = true;
	}

	if(!panelsDrawn_) {
		draw_all_panels();
		panelsDrawn_ = true;
	}

	if(redraw_background_) {
		// Full redraw of the background
		const SDL_Rect clip_rect = map_outside_area();
		const surface screen = get_screen_surface();
		clip_rect_setter set_clip_rect(screen, &clip_rect);
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
}

void display::draw_wrap(bool update, bool force)
{
	static const int time_between_draws = preferences::draw_delay();
	const int current_time = SDL_GetTicks();
	const int wait_time = nextDraw_ - current_time;

	if(redrawMinimap_) {
		redrawMinimap_ = false;
		draw_minimap();
	}

	if(update) {
		update_display();
		if(!force && !benchmark && wait_time > 0) {
			// If it's not time yet to draw, delay until it is
			SDL_Delay(wait_time);
		}

		// Set the theoretical next draw time
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

const theme::action* display::action_pressed()
{
	for(std::vector<gui::button>::iterator i = action_buttons_.begin();
			i != action_buttons_.end(); ++i) {
		if(i->pressed()) {
			const size_t index = i - action_buttons_.begin();
			if(index >= theme_.actions().size()) {
				assert(false);
				return NULL;
			}
			return &theme_.actions()[index];
		}
	}

	return NULL;
}

const theme::menu* display::menu_pressed()
{
	for(std::vector<gui::button>::iterator i = menu_buttons_.begin(); i != menu_buttons_.end(); ++i) {
		if(i->pressed()) {
			const size_t index = i - menu_buttons_.begin();
			if(index >= theme_.menus().size()) {
				assert(false);
				return NULL;
			}
			return theme_.get_menu_item(i->id());
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
			if(index >= menu_buttons_.size()) {
				continue;
			}
			menu_buttons_[index].enable(enable);
		}
	}
}

void display::announce(const std::string& message, const SDL_Color& color)
{
	font::floating_label flabel(message);
	flabel.set_font_size(font::SIZE_XLARGE);
	flabel.set_color(color);
	flabel.set_position(map_outside_area().w/2, map_outside_area().h/3);
	flabel.set_lifetime(100);
	flabel.set_clip_rect(map_outside_area());

	font::add_floating_label(flabel);
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
		drawing_buffer_add(LAYER_BORDER, loc, xpos + zoom_/4, ypos,
			image::get_image(theme_.border().corner_image_top_left, image::SCALED_TO_ZOOM));
	} else if(loc.x == get_map().w() && loc.y == -1) { // top right corner
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 0) {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos + zoom_/2,
				image::get_image(theme_.border().corner_image_top_right_odd, image::SCALED_TO_ZOOM));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos,
				image::get_image(theme_.border().corner_image_top_right_even, image::SCALED_TO_ZOOM));
		}
	} else if(loc.x == -1 && loc.y == get_map().h()) { // bottom left corner
		drawing_buffer_add(LAYER_BORDER, loc, xpos + zoom_/4, ypos,
			image::get_image(theme_.border().corner_image_bottom_left, image::SCALED_TO_ZOOM));

	} else if(loc.x == get_map().w() && loc.y == get_map().h()) { // bottom right corner
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos,
				image::get_image(theme_.border().corner_image_bottom_right_even, image::SCALED_TO_ZOOM));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos,
				image::get_image(theme_.border().corner_image_bottom_right_odd, image::SCALED_TO_ZOOM));
		}

	// Now handle the sides:
	} else if(loc.x == -1) { // left side
		drawing_buffer_add(LAYER_BORDER, loc, xpos + zoom_/4, ypos,
			image::get_image(theme_.border().border_image_left, image::SCALED_TO_ZOOM));
	} else if(loc.x == get_map().w()) { // right side
		drawing_buffer_add(LAYER_BORDER, loc, xpos + zoom_/4, ypos,
			image::get_image(theme_.border().border_image_right, image::SCALED_TO_ZOOM));
	} else if(loc.y == -1) { // top side
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos,
				image::get_image(theme_.border().border_image_top_even, image::SCALED_TO_ZOOM));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos + zoom_/2,
				image::get_image(theme_.border().border_image_top_odd, image::SCALED_TO_ZOOM));
		}
	} else if(loc.y == get_map().h()) { // bottom side
		// We use the map idea of odd and even, and map coords are internal coords + 1
		if(loc.x%2 == 1) {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos,
				image::get_image(theme_.border().border_image_bottom_even, image::SCALED_TO_ZOOM));
		} else {
			drawing_buffer_add(LAYER_BORDER, loc, xpos, ypos + zoom_/2,
				image::get_image(theme_.border().border_image_bottom_odd, image::SCALED_TO_ZOOM));
		}
	}
}

void display::draw_minimap()
{
	const SDL_Rect& area = minimap_area();

	if(area.w == 0 || area.h == 0) {
		return;
	}

	if(minimap_ == NULL || minimap_->w > area.w || minimap_->h > area.h) {
		minimap_ = image::getMinimap(area.w, area.h, get_map(), viewpoint_, (selectedHex_.valid() && !is_blindfolded()) ? &reach_map_ : NULL);
		if(minimap_ == NULL) {
			return;
		}
	}

	const surface& screen(screen_.getSurface());
	clip_rect_setter clip_setter(screen, &area);

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

void display::draw_minimap_units()
{
	if (!preferences::minimap_draw_units() || is_blindfolded()) return;

	double xscaling = 1.0 * minimap_location_.w / get_map().w();
	double yscaling = 1.0 * minimap_location_.h / get_map().h();

	for(unit_map::const_iterator u = units_->begin(); u != units_->end(); ++u) {
		if (fogged(u->get_location()) ||
		    ((*teams_)[currentTeam_].is_enemy(u->side()) &&
		     u->invisible(u->get_location())) ||
			 u->get_hidden()) {
			continue;
		}

		int side = u->side();
		SDL_Color col = team::get_minimap_color(side);

		if (preferences::minimap_movement_coding()) {

			if ((*teams_)[currentTeam_].is_enemy(side)) {
				col = int_to_color(game_config::color_info(preferences::enemy_color()).rep());
			} else {

				if (currentTeam_ +1 == static_cast<unsigned>(side)) {

					if (u->movement_left() == u->total_movement())
						col = int_to_color(game_config::color_info(preferences::unmoved_color()).rep());
					else if (u->movement_left() == 0)
						col = int_to_color(game_config::color_info(preferences::moved_color()).rep());
					else
						col = int_to_color(game_config::color_info(preferences::partial_color()).rep());

				} else
					col = int_to_color(game_config::color_info(preferences::allied_color()).rep());
			}
		}

		const Uint32 mapped_col = SDL_MapRGB(video().getSurface()->format,col.r,col.g,col.b);

		double u_x = u->get_location().x * xscaling;
		double u_y = (u->get_location().y + (is_odd(u->get_location().x) ? 1 : -1)/4.0) * yscaling;
 		// use 4/3 to compensate the horizontal hexes imbrication
		double u_w = 4.0 / 3.0 * xscaling;
		double u_h = yscaling;

		SDL_Rect r = create_rect(minimap_location_.x + round_double(u_x)
				, minimap_location_.y + round_double(u_y)
				, round_double(u_w)
				, round_double(u_h));

		sdl_fill_rect(video().getSurface(), &r, mapped_col);
	}
}

bool display::scroll(int xmove, int ymove, bool force)
{
	if(view_locked_ && !force) {
		return false;
	}

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

bool display::zoom_at_max() const
{
	return zoom_ == MaxZoom;
}

bool display::zoom_at_min() const
{
	return zoom_ == MinZoom;
}

bool display::set_zoom(int amount, bool absolute)
{
	int new_zoom = zoom_ + amount;
	if (absolute)
		new_zoom = amount;
	if (new_zoom < MinZoom) {
		new_zoom = MinZoom;
	}
	if (new_zoom > MaxZoom) {
		new_zoom = MaxZoom;
	}
	if (new_zoom != zoom_) {
		gui::slider* zoom_slider = find_slider("map-zoom-slider");
		if (zoom_slider) {
			zoom_slider->set_value(new_zoom);
		}
		SDL_Rect const &area = map_area();
		xpos_ += (xpos_ + area.w / 2) * (absolute ? new_zoom - zoom_ : amount) / zoom_;
		ypos_ += (ypos_ + area.h / 2) * (absolute ? new_zoom - zoom_ : amount) / zoom_;

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
		return true;
	} else {
		return false;
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
	if(!force && (view_locked_ || !preferences::scroll_to_action())) return;
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

	if(scroll_type == WARP || scroll_type == ONSCREEN_WARP || turbo_speed() > 2.0 || preferences::scroll_speed() > 99) {
		scroll(xmove,ymove,true);
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

		scroll(dx,dy,true);
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

void display::scroll_to_tiles(const std::vector<map_location>::const_iterator & begin,
                              const std::vector<map_location>::const_iterator & end,
                              SCROLL_TYPE scroll_type, bool check_fogged,
                              bool only_if_possible, double add_spacing, bool force)
{
	// basically we calculate the min/max coordinates we want to have on-screen
	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;
	bool valid = false;

	for(std::vector<map_location>::const_iterator itor = begin; itor != end ; ++itor) {
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
	//if everything is fogged or the location list is empty
	if(!valid) return;

	if (scroll_type == ONSCREEN || scroll_type == ONSCREEN_WARP) {
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

	if (scroll_type == ONSCREEN || scroll_type == ONSCREEN_WARP) {
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

	reportRects_.clear();
	reportSurfaces_.clear();
	reports_.clear();

	bounds_check_position();

	tooltips::clear_tooltips();

	theme_.set_resolution(screen_area());

	if(!menu_buttons_.empty() || !action_buttons_.empty() || !sliders_.empty() ) {
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

	BOOST_FOREACH(boost::function<void(display&)> f, redraw_observers_) {
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
	set_scontext_leave_for_draw leave_synced_context;
	local_tod_light_ = has_time_area() && preferences::get("local_tod_lighting", true);

	draw_init();
	pre_draw();
	// invalidate all that needs to be invalidated
	invalidate_animations();
	// at this stage we have everything that needs to be invalidated for this redraw
	// save it as the previous invalidated, and merge with the previous invalidated_
	// we merge with the previous redraw because if a hex had a unit last redraw but
	// not this one, nobody will tell us to redraw (cleanup)
	previous_invalidated_.swap(invalidated_);
	invalidated_.insert(previous_invalidated_.begin(),previous_invalidated_.end());
	// these new invalidations cannot cause any propagation because
	// if a hex was invalidated last turn but not this turn, then
	// * case of no unit in neighbor hex=> no propagation
	// * case of unit in hex but was there last turn=>its hexes are invalidated too
	// * case of unit in hex not there last turn => it moved, so was invalidated previously
	if(!get_map().empty()) {
		//int simulate_delay = 0;

		/*
		 * draw_invalidated() also invalidates the halos, so also needs to be
		 * ran if invalidated_.empty() == true.
		 */
		if(!invalidated_.empty() || preferences::show_haloes()) {
			draw_invalidated();
			invalidated_.clear();
		}
		drawing_buffer_commit();
		post_commit();
		draw_sidebar();

		// Simulate slow PC:
		//SDL_Delay(2*simulate_delay + rand() % 20);
	}
	draw_wrap(update, force);
	post_draw();
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
	surface disp(screen_.getSurface());
	SDL_Rect area = screen_area();
	sdl_fill_rect(disp, &area, SDL_MapRGB(disp->format, 0, 0, 0));
}

const SDL_Rect& display::get_clip_rect()
{
	return map_area();
}

void display::draw_invalidated() {
//	log_scope("display::draw_invalidated");
	SDL_Rect clip_rect = get_clip_rect();
	surface screen = get_screen_surface();
	clip_rect_setter set_clip_rect(screen, &clip_rect);
	BOOST_FOREACH(const map_location& loc, invalidated_) {
		int xpos = get_location_x(loc);
		int ypos = get_location_y(loc);

		update_rect(xpos, ypos, zoom_, zoom_);

		const bool on_map = get_map().on_board(loc);
		SDL_Rect hex_rect = create_rect(xpos, ypos, zoom_, zoom_);
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

	BOOST_FOREACH(const map_location& loc, invalidated_) {
		unit_map::iterator u_it = units_->find(loc);
		exclusive_unit_draw_requests_t::iterator request = exclusive_unit_draw_requests_.find(loc);
		if (u_it != units_->end()
				&& (request == exclusive_unit_draw_requests_.end() || request->second == u_it->id()))
			u_it->redraw_unit();
	}

}

void display::draw_hex(const map_location& loc) {
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	image::TYPE image_type = get_image_type(loc);
	const bool on_map = get_map().on_board(loc);
	const bool off_map_tile = (get_map().get_terrain(loc) == t_translation::OFF_MAP_USER);
	const time_of_day& tod = get_time_of_day(loc);
	if(!shrouded(loc)) {
		// unshrouded terrain (the normal case)
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, xpos, ypos,
			get_terrain_images(loc,tod.id, image_type, BACKGROUND));

		drawing_buffer_add(LAYER_TERRAIN_FG, loc, xpos, ypos,
			get_terrain_images(loc,tod.id,image_type, FOREGROUND));

		// Draw the grid, if that's been enabled
		if(grid_ && on_map && !off_map_tile) {
			static const image::locator grid_top(game_config::images::grid_top);
			drawing_buffer_add(LAYER_GRID_TOP, loc, xpos, ypos,
				image::get_image(grid_top, image::TOD_COLORED));
			static const image::locator grid_bottom(game_config::images::grid_bottom);
			drawing_buffer_add(LAYER_GRID_BOTTOM, loc, xpos, ypos,
				image::get_image(grid_bottom, image::TOD_COLORED));
		}
		// village-control flags.
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, xpos, ypos, get_flag(loc));
	}

	if(!shrouded(loc)) {
		typedef overlay_map::const_iterator Itor;
		std::pair<Itor,Itor> overlays = overlays_->equal_range(loc);
		for( ; overlays.first != overlays.second; ++overlays.first) {
			if ((overlays.first->second.team_name == "" ||
					overlays.first->second.team_name.find((*teams_)[playing_team()].team_name()) != std::string::npos)
					&& !(fogged(loc) && !overlays.first->second.visible_in_fog))
			{
				drawing_buffer_add(LAYER_TERRAIN_BG, loc, xpos, ypos,
						image::get_image(overlays.first->second.image,image_type));
			}
		}
	}

	// Draw the time-of-day mask on top of the terrain in the hex.
	// tod may differ from tod if hex is illuminated.
	const std::string& tod_hex_mask = tod.image_mask;
	if(tod_hex_mask1 != NULL || tod_hex_mask2 != NULL) {
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, xpos, ypos, tod_hex_mask1);
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, xpos, ypos, tod_hex_mask2);
	} else if(!tod_hex_mask.empty()) {
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, xpos, ypos,
			image::get_image(tod_hex_mask,image::SCALED_TO_HEX));
	}

	// Paint mouseover overlays
	if(loc == mouseoverHex_ && (on_map || (in_editor() && get_map().on_board_with_border(loc)))
			&& mouseover_hex_overlay_ != NULL) {
		drawing_buffer_add(LAYER_MOUSEOVER_OVERLAY, loc, xpos, ypos, mouseover_hex_overlay_);
	}

	// Paint arrows
	arrows_map_t::const_iterator arrows_in_hex = arrows_map_.find(loc);
	if(arrows_in_hex != arrows_map_.end()) {
		BOOST_FOREACH(arrow* const a, arrows_in_hex->second) {
			a->draw_hex(loc);
		}
	}

	// Apply shroud, fog and linger overlay

	if(shrouded(loc)) {
		// We apply void also on off-map tiles
		// to shroud the half-hexes too
		const std::string& shroud_image = get_variant(shroud_images_, loc);
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, xpos, ypos,
			image::get_image(shroud_image, image_type));
	} else if(fogged(loc)) {
		const std::string& fog_image = get_variant(fog_images_, loc);
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, xpos, ypos,
			image::get_image(fog_image, image_type));
	}

	if(!shrouded(loc)) {
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, xpos, ypos, get_fog_shroud_images(loc, image_type));
	}

	if (on_map) {
		if (draw_coordinates_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(loc), font::SIZE_SMALL, font::NORMAL_COLOR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect = create_rect(0, 0, text->w, text->h);
			sdl_fill_rect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			if (draw_terrain_codes_) {
				off_y -= text->h;
			} else {
				off_y -= text->h / 2;
			}
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, off_x, off_y, bg);
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, off_x, off_y, text);
		}
		if (draw_terrain_codes_ && (game_config::debug || !shrouded(loc))) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(get_map().get_terrain(loc)), font::SIZE_SMALL, font::NORMAL_COLOR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect = create_rect(0, 0, text->w, text->h);
			sdl_fill_rect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			if (!draw_coordinates_) {
				off_y -= text->h / 2;
			}
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, off_x, off_y, bg);
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, off_x, off_y, text);
		}
	}

	if(debug_foreground) {
		drawing_buffer_add(LAYER_UNIT_DEFAULT, loc, xpos, ypos,
			image::get_image("terrain/foreground.png", image_type));
	}

}

image::TYPE display::get_image_type(const map_location& /*loc*/) {
	return image::TOD_COLORED;
}

/*void display::draw_sidebar() {

}*/

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

		sdl_blit(img,&visible_area,screen_.getSurface(),&target);
	} else {
		if(img->w != rect.w || img->h != rect.h) {
			img.assign(scale_surface(img,rect.w,rect.h));
		}

		sdl_blit(img,NULL,screen_.getSurface(),&target);
	}
}

/**
 * Redraws the specified report (if anything has changed).
 * If a config is not supplied, it will be generated via
 * reports::generate_report().
 */
void display::refresh_report(std::string const &report_name, const config * new_cfg)
{
	const theme::status_item *item = theme_.get_status_item(report_name);
	if (!item) {
		reportSurfaces_[report_name].assign(NULL);
		return;
	}

	// Now we will need the config. Generate one if needed.
	const config generated_cfg = new_cfg ? config() : reports::generate_report(report_name);
	if ( new_cfg == NULL )
		new_cfg = &generated_cfg;

	SDL_Rect &rect = reportRects_[report_name];
	const SDL_Rect &new_rect = item->location(screen_area());
	surface &surf = reportSurfaces_[report_name];
	config &report = reports_[report_name];

	// Report and its location is unchanged since last time. Do nothing.
	if (surf && rect == new_rect && report == *new_cfg) {
		return;
	}

	// Update the config in reports_.
	report = *new_cfg;

	if (surf) {
		sdl_blit(surf, NULL, screen_.getSurface(), &rect);
		update_rect(rect);
	}

	// If the rectangle has just changed, assign the surface to it
	if (!surf || new_rect != rect)
	{
		surf.assign(NULL);
		rect = new_rect;

		// If the rectangle is present, and we are blitting text,
		// then we need to backup the surface.
		// (Images generally won't need backing up,
		// unless they are transparent, but that is done later).
		if (rect.w > 0 && rect.h > 0) {
			surf.assign(get_surface_portion(screen_.getSurface(), rect));
			if (reportSurfaces_[report_name] == NULL) {
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
		config &e = report.add_child_at("element", config(), 0);
		e["text"] = str;
		e["tooltip"] = report.child("element")["tooltip"];
	}
	str = item->postfix();
	if (!str.empty()) {
		config &e = report.add_child("element");
		e["text"] = str;
		e["tooltip"] = report.child("element", -1)["tooltip"];
	}

	// Loop through and display each report element.
	int tallest = 0;
	int image_count = 0;
	bool used_ellipsis = false;
	std::ostringstream ellipsis_tooltip;
	SDL_Rect ellipsis_area = rect;

	for (config::const_child_itors elements = report.child_range("element");
	     elements.first != elements.second; ++elements.first)
	{
		SDL_Rect area = create_rect(x, y, rect.w + rect.x - x, rect.h + rect.y - y);
		if (area.h <= 0) break;

		std::string t = (*elements.first)["text"];
		if (!t.empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw a text element.
			font::ttext text;
			if (item->font_rgb_set()) {
				// font_rgb() has no alpha channel and uses a 0x00RRGGBB
				// layout instead of 0xRRGGBBAA which is what ttext expects,
				// so shift the value to the left and add fully-opaque alpha.
				text.set_foreground_color((item->font_rgb() << 8) + 0xFF);
			}
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

			// check if next element is text with almost no space to show it
			const int minimal_text = 12; // width in pixels
			config::const_child_iterator ee = elements.first;
			if (!eol && rect.w - (x - rect.x + s->w) < minimal_text &&
			    ++ee != elements.second && !(*ee)["text"].empty())
			{
				// make this element longer to trigger rendering of ellipsis
				// (to indicate that next elements have not enough space)
				//NOTE this space should be longer than minimal_text pixels
				t = t + "    ";
				text.set_text(t, true);
				s = text.render();
				// use the area of this element for next tooltips
				used_ellipsis = true;
				ellipsis_area.x = x;
				ellipsis_area.y = y;
				ellipsis_area.w = s->w;
				ellipsis_area.h = s->h;
			}

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
		else if (!(t = (*elements.first)["image"].str()).empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw an image element.
			surface img(image::get_image(t));

			if (!img) {
				ERR_DP << "could not find image for report: '" << t << "'\n";
				continue;
			}

			if (area.w < img->w && image_count) {
				// We have more than one image, and this one doesn't fit.
				img = image::get_image(game_config::images::ellipsis);
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
		t = (*elements.first)["tooltip"].t_str().base_str();
		if (!t.empty()) {
			if (!used_ellipsis) {
				tooltips::add_tooltip(area, t, (*elements.first)["help"].t_str().base_str());
			} else {
				// Collect all tooltips for the ellipsis.
				// TODO: need a better separator
				// TODO: assign an action
				ellipsis_tooltip << t;
				config::const_child_iterator ee = elements.first;
				if (++ee != elements.second)
					ellipsis_tooltip << "\n  _________\n\n";
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
#ifdef _OPENMP
#pragma omp critical(invalidated_)
#endif //_OPENMP
	invalidated_.clear();
	update_rect(map_area());
}

bool display::invalidate(const map_location& loc)
{
	if(invalidateAll_)
		return false;

	bool tmp;
#ifdef _OPENMP
#pragma omp critical(invalidated_)
#endif //_OPENMP
	tmp = invalidated_.insert(loc).second;
	return tmp;
}

bool display::invalidate(const std::set<map_location>& locs)
{
	if(invalidateAll_)
		return false;
	bool ret = false;
	BOOST_FOREACH(const map_location& loc, locs) {
#ifdef _OPENMP
#pragma omp critical(invalidated_)
#endif //_OPENMP
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

	bool result = false;
#ifdef _OPENMP
#pragma omp critical(invalidated_)
#endif //_OPENMP
	{
		// search the first hex invalidated (if any)
		std::set<map_location>::const_iterator i = locs.begin();
		for(; i != locs.end() && invalidated_.count(*i) == 0 ; ++i) {}

		if (i != locs.end()) {

			// propagate invalidation
			// 'i' is already in, but I suspect that splitting the range is bad
			// especially because locs are often adjacents
			size_t previous_size = invalidated_.size();
			invalidated_.insert(locs.begin(), locs.end());
			result = previous_size < invalidated_.size();
		}
	}
	return result;
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
	BOOST_FOREACH(const map_location &loc, hexes_under_rect(rect)) {
		result |= invalidate(loc);
	}
	return result;
}

void display::invalidate_animations_location(const map_location& loc) {
	if (get_map().is_village(loc)) {
		const int owner = village_owner(loc);
		if (owner >= 0 && flags_[owner].need_update()
		&& (!fogged(loc) || !(*teams_)[currentTeam_].is_enemy(owner+1))) {
			invalidate(loc);
		}
	}
}


std::vector<unit*> display::get_unit_list_for_invalidation() {
	std::vector<unit*> unit_list;
	BOOST_FOREACH(unit &u, *units_) {
		unit_list.push_back(&u);
	}
	return unit_list;
}
void display::invalidate_animations()
{
	new_animation_frame();
	animate_map_ = preferences::animate_map();
	if (animate_map_) {
		BOOST_FOREACH(const map_location &loc, get_visible_hexes())
		{
			if (shrouded(loc)) continue;
			if (builder_->update_animation(loc)) {
				invalidate(loc);
			} else {
				invalidate_animations_location(loc);
			}
		}
	}
	std::vector<unit*> unit_list=get_unit_list_for_invalidation();
	BOOST_FOREACH(unit* u, unit_list) {
		u->refresh();
	}
	bool new_inval;
	do {
		new_inval = false;
#ifdef _OPENMP
#pragma omp parallel for reduction(|:new_inval) shared(unit_list) schedule(guided)
#endif //_OPENMP
		for(int i=0; i < static_cast<int>(unit_list.size()); i++) {
			new_inval |=  unit_list[i]->invalidate(unit_list[i]->get_location());
		}
	}while(new_inval);
}

void display::add_arrow(arrow& arrow)
{
	const arrow_path_t & arrow_path = arrow.get_path();
	BOOST_FOREACH(const map_location& loc, arrow_path)
	{
		arrows_map_[loc].push_back(&arrow);
	}
}

void display::remove_arrow(arrow& arrow)
{
	const arrow_path_t & arrow_path = arrow.get_path();
	BOOST_FOREACH(const map_location& loc, arrow_path)
	{
		arrows_map_[loc].remove(&arrow);
	}
}

void display::update_arrow(arrow & arrow)
{
	const arrow_path_t & previous_path = arrow.get_previous_path();
	BOOST_FOREACH(const map_location& loc, previous_path)
	{
		arrows_map_[loc].remove(&arrow);
	}
	const arrow_path_t & arrow_path = arrow.get_path();
	BOOST_FOREACH(const map_location& loc, arrow_path)
	{
		arrows_map_[loc].push_back(&arrow);
	}
}

void display::write(config& cfg) const
{
	cfg["view_locked"] = view_locked_;
	cfg["color_adjust_red"] = color_adjust_.r;
	cfg["color_adjust_green"] = color_adjust_.g;
	cfg["color_adjust_blue"] = color_adjust_.b;
}

void display::read(const config& cfg)
{
	view_locked_ = cfg["view_locked"].to_bool(false);
	color_adjust_.r = cfg["color_adjust_red"].to_int(0);
	color_adjust_.g = cfg["color_adjust_green"].to_int(0);
	color_adjust_.b = cfg["color_adjust_blue"].to_int(0);
}

void display::process_reachmap_changes()
{
	if (!reach_map_changed_) return;
	if (reach_map_.empty() != reach_map_old_.empty()) {
		// Invalidate everything except the non-darkened tiles
		reach_map &full = reach_map_.empty() ? reach_map_old_ : reach_map_;

		rect_of_hexes hexes = get_visible_hexes();
		rect_of_hexes::iterator i = hexes.begin(), end = hexes.end();
		for (;i != end; ++i) {
			reach_map::iterator reach = full.find(*i);
			if (reach == full.end()) {
				// Location needs to be darkened or brightened
				invalidate(*i);
			} else if (reach->second != 1) {
				// Number needs to be displayed or cleared
				invalidate(*i);
			}
		}
	} else if (!reach_map_.empty()) {
		// Invalidate only changes
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

display *display::singleton_ = NULL;

