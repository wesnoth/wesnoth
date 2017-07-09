/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "arrow.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "fake_unit_manager.hpp"
#include "font/standard_colors.hpp"
#include "font/text.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkey/command_executor.hpp"
#include "language.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "minimap.hpp"
#include "overlay.hpp"
#include "play_controller.hpp" //note: this can probably be refactored out
#include "reports.hpp"
#include "resources.hpp"
#include "color.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "time_of_day.hpp"
#include "tooltips.hpp"
#include "tod_manager.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/drawer.hpp"
#include "whiteboard/manager.hpp"
#include "show_dialog.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "sdl/render_utils.hpp"
#include "sdl/texture.hpp"

#include <SDL_image.h>

#include <array>
#include <cmath>
#include <iomanip>
#include <utility>

// Includes for bug #17573

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)
#define DBG_DP LOG_STREAM(debug, log_display)

// These are macros instead of proper constants so that they auto-update if the game config is reloaded.
#define zoom_levels      (game_config::zoom_levels)
#define final_zoom_index (static_cast<int>(zoom_levels.size()) - 1)
#define DefaultZoom      (game_config::tile_size)
#define SmallZoom        (DefaultZoom / 2)
#define MinZoom          (zoom_levels.front())
#define MaxZoom          (zoom_levels.back())

namespace {
	bool benchmark = false;

	bool debug_foreground = false;

	int prevLabel = 0;
}

unsigned int display::zoom_ = DefaultZoom;
unsigned int display::last_zoom_ = SmallZoom;

void display::add_overlay(const map_location& loc, const std::string& img, const std::string& halo, const std::string& team_name, const std::string& item_id, bool visible_under_fog)
{
	if (halo_man_) {
		halo::handle halo_handle;
		if(halo != "") {
			halo_handle = halo_man_->add(get_location_x(loc) + hex_size() / 2,
				get_location_y(loc) + hex_size() / 2, halo, loc);
		}

		overlays_->emplace(loc, overlay(img, halo, halo_handle, team_name, item_id, visible_under_fog));
	}
}

void display::remove_overlay(const map_location& loc)
{
	/* This code no longer needed because of RAII in halo::handles
	if (halo_man_) {
		typedef overlay_map::const_iterator Itor;
		std::pair<Itor,Itor> itors = overlays_->equal_range(loc);
		while(itors.first != itors.second) {
			halo_man_->remove(itors.first->second.halo_handle);
			++itors.first;
		}
	}
	*/

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
		if(itors.first->second.image == toDelete || itors.first->second.halo == toDelete || itors.first->second.id == toDelete) {
			iteratorCopy = itors.first;
			++itors.first;
			//Not needed because of RAII --> halo_man_->remove(iteratorCopy->second.halo_handle);
			overlays_->erase(iteratorCopy);
		}
		else {
			++itors.first;
		}
	}
}

display::display(const display_context * dc, std::weak_ptr<wb::manager> wb, reports & reports_object, const config& theme_cfg, const config& level, bool auto_join)
	: video2::draw_layering(auto_join)
	, dc_(dc)
	, halo_man_(new halo::manager())
	, wb_(wb)
	, exclusive_unit_draw_requests_()
	, screen_(CVideo::get_singleton())
	, currentTeam_(0)
	, dont_show_all_(false)
	, xpos_(0)
	, ypos_(0)
	, view_locked_(false)
	, theme_(theme_cfg, screen_.screen_area())
	, zoom_index_(0)
	, fake_unit_man_(new fake_unit_manager(*this))
	, builder_(new terrain_builder(level, (dc_ ? &dc_->map() : nullptr), theme_.border().tile_image, theme_.border().show_border))
	, minimap_(nullptr)
	, minimap_location_(sdl::empty_rect)
	, redrawMinimap_(false)
	, grid_(false)
	, diagnostic_label_(0)
	, turbo_speed_(2)
	, turbo_(false)
	, invalidateGameStatus_(true)
	, map_labels_(new map_labels(0))
	, reports_object_(&reports_object)
	, scroll_event_("scrolled")
	, complete_redraw_event_("completely_redrawn")
	, fps_counter_()
	, fps_start_()
	, fps_actual_()
	, reportRects_()
	, reportSurfaces_()
	, reports_()
	, menu_buttons_()
	, action_buttons_()
	, mouseover_hex_overlay_(nullptr)
	, tod_hex_mask1(nullptr)
	, tod_hex_mask2(nullptr)
	, fog_images_()
	, shroud_images_()
	, selectedHex_()
	, mouseoverHex_()
	, keys_()
	, animate_map_(true)
	, animate_water_(true)
	, flags_()
	, activeTeam_(0)
	, drawing_queue_()
	, map_screenshot_(false)
	, reach_map_()
	, overlays_(nullptr)
	, fps_handle_(0)
	, drawn_hexes_(0)
	, idle_anim_(preferences::idle_anim())
	, idle_anim_rate_(1.0)
	, map_screenshot_surf_(nullptr)
	, redraw_observers_()
	, draw_coordinates_(false)
	, draw_terrain_codes_(false)
	, draw_num_of_bitmaps_(false)
	, arrows_map_()
	, color_adjust_()
, 
{
	//The following assertion fails when starting a campaign
	assert(singleton_ == nullptr);
	singleton_ = this;

	resources::fake_units = fake_unit_man_.get();

	blindfold_ctr_ = 0;

	read(level.child_or_empty("display"));

	if(screen_.non_interactive()
		&& (screen_.getSurface() != nullptr
		&& screen_.faked())) {
		video_.lock_updates(true);
	}

	fill_images_list(game_config::fog_prefix, fog_images_);
	fill_images_list(game_config::shroud_prefix, shroud_images_);

	set_idle_anim_rate(preferences::idle_anim_rate());

	zoom_index_ = std::distance(zoom_levels.begin(), std::find(zoom_levels.begin(), zoom_levels.end(), zoom_));

	image::set_zoom(zoom_);

	init_flags();

	if(!menu_buttons_.empty() || !action_buttons_.empty()) {
		create_buttons();
	}

	rebuild_all();
	assert(builder_);
	//builder_->rebuild_cache_all();
}

display::~display()
{
	singleton_ = nullptr;
	resources::fake_units = nullptr;
}

void display::set_theme(config theme_cfg) {
	theme_ = theme(theme_cfg, screen_.screen_area());
	builder_->set_draw_border(theme_.border().show_border);
	menu_buttons_.clear();
	action_buttons_.clear();
	create_buttons();
}

void display::init_flags() {

	flags_.clear();
	if (!dc_) return;
	flags_.resize(dc_->teams().size());

	std::vector<std::string> side_colors;
	side_colors.reserve(dc_->teams().size());

	for(const team& t : dc_->teams()) {
		std::string side_color = t.color();
		side_colors.push_back(side_color);
		init_flags_for_side_internal(t.side() - 1, side_color);
	}
	image::set_team_colors(&side_colors);
}

void display::reinit_flags_for_side(size_t side)
{
	if (!dc_ || side >= dc_->teams().size()) {
		ERR_DP << "Cannot rebuild flags for inexistent or unconfigured side " << side << '\n';
		return;
	}

	init_flags_for_side_internal(side, dc_->teams()[side].color());
}

void display::init_flags_for_side_internal(size_t n, const std::string& side_color)
{
	assert(dc_ != nullptr);
	assert(n < dc_->teams().size());
	assert(n < flags_.size());

	std::string flag = dc_->teams()[n].flag();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = side_color;

	if(flag.empty()) {
		flag = game_config::images::flag;
	}

	LOG_DP << "Adding flag for team " << n << " from animation " << flag << "\n";

	// Must recolor flag image
	animated<image::locator> temp_anim;

	std::vector<std::string> items = utils::square_parenthetical_split(flag);

	for(const std::string& item : items) {
		const std::vector<std::string>& sub_items = utils::split(item, ':');
		std::string str = item;
		int time = 100;

		if(sub_items.size() > 1) {
			str = sub_items.front();
			try {
				time = std::max<int>(1, std::stoi(sub_items.back()));
			} catch(std::invalid_argument&) {
				ERR_DP << "Invalid time value found when constructing flag for side " << n << ": " << sub_items.back() << "\n";
			}
		}

		std::stringstream temp;
		temp << str << "~RC(" << old_rgb << ">"<< new_rgb << ")";
		image::locator flag_image(temp.str());
		temp_anim.add_frame(time, flag_image);
	}

	animated<image::locator>& f = flags_[n];
	f = temp_anim;
	auto time = f.get_end_time();
	if (time > 0) {
		f.start_animation(randomness::rng::default_instance().get_random_int(0, time-1), true);
	}
	else {
		// this can happen if both flag and game_config::images::flag are empty.
		ERR_DP << "missing flag for team" << n << "\n";
	}
}

void display::set_team(size_t teamindex, bool show_everything)
{
	assert(teamindex < dc_->teams().size());
	currentTeam_ = teamindex;
	if (!show_everything)
	{
		labels().set_team(&dc_->teams()[teamindex]);
		dont_show_all_ = true;
	}
	else
	{
		labels().set_team(nullptr);
		dont_show_all_ = false;
	}
	labels().recalculate_labels();
	if(std::shared_ptr<wb::manager> w = wb_.lock())
		w->on_viewer_change(teamindex);
}

void display::set_playing_team(size_t teamindex)
{
	assert(teamindex < dc_->teams().size());
	activeTeam_ = teamindex;
	invalidate_game_status();
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

void display::update_tod(const time_of_day* tod_override)
{
	const time_of_day* tod = tod_override;
	if(tod == nullptr) {
		tod = &get_time_of_day();
	}

	const tod_color col = color_adjust_ + tod->color;
	image::set_color_adjustment(col.r, col.g, col.b);
}

void display::adjust_color_overlay(int r, int g, int b)
{
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
		images.emplace_back();
}

const std::string& display::get_variant(const std::vector<std::string>& variants, const map_location &loc)
{
	//TODO use better noise function
	return variants[std::abs(loc.x + loc.y) % variants.size()];
}

void display::rebuild_all()
{
	builder_->rebuild_all();
}

void display::reload_map()
{
	builder_->reload_map();
}

void display::change_display_context(const display_context * dc)
{
	dc_ = dc;
 	builder_->change_map(&dc_->map()); //TODO: Should display_context own and initialize the builder object?
}

void display::reset_halo_manager()
{
	halo_man_.reset(new halo::manager());
}

void display::reset_halo_manager(halo::manager & halo_man)
{
	halo_man_.reset(&halo_man);
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
	static SDL_Rect max_area {0, 0, 0, 0};

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

bool display::outside_area(const SDL_Rect& area, const int x, const int y)
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
	if(sdl::point_in_rect(xclick,yclick,rect) == false) {
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
		res.right = -1; // end is right + 1
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
	// 18/54=0.(3). Note that, while tile_width is zoom dependent, 0.(3) is not.
	res.left = static_cast<int>(std::floor(-border + x / tile_width - 0.3333333));

	// we remove 1 pixel of the rectangle dimensions
	// (the rounded division take one pixel more than needed)
	res.right = static_cast<int>(std::floor(-border + (x + r.w - 1) / tile_width));

	// for odd x, we must shift up one half-hex. Since x will vary along the edge,
	// we store here the y values for even and odd x, respectively
	res.top[0] = static_cast<int>(std::floor(-border + y / tile_size));
	res.top[1] = static_cast<int>(std::floor(-border + y / tile_size - 0.5));
	res.bottom[0] = static_cast<int>(std::floor(-border + (y + r.h - 1) / tile_size));
	res.bottom[1] = static_cast<int>(std::floor(-border + (y + r.h - 1) / tile_size - 0.5));

	// TODO: in some rare cases (1/16), a corner of the big rect is on a tile
	// (the 72x72 rectangle containing the hex) but not on the hex itself
	// Can maybe be optimized by using pixel_position_to_hex

	return res;
}

bool display::team_valid() const
{
	return currentTeam_ < dc_->teams().size();
}

bool display::shrouded(const map_location& loc) const
{
	return is_blindfolded() || (dont_show_all_ && dc_->teams()[currentTeam_].shrouded(loc));
}

bool display::fogged(const map_location& loc) const
{
	return is_blindfolded() || (dont_show_all_ && dc_->teams()[currentTeam_].fogged(loc));
}

int display::get_location_x(const map_location& loc) const
{
	return static_cast<int>(map_area().x + (loc.x + theme_.border().size) * hex_width() - xpos_);
}

int display::get_location_y(const map_location& loc) const
{
	return static_cast<int>(map_area().y + (loc.y + theme_.border().size) * zoom_ - ypos_ + (is_odd(loc.x) ? zoom_/2 : 0));
}

SDL_Point display::get_loc_drawing_origin(const map_location& loc) const
{
	return {
		get_location_x(loc),
		get_location_y(loc)
	};
}

map_location display::minimap_location_on(int x, int y)
{
	//TODO: don't return location for this,
	// instead directly scroll to the clicked pixel position

	if (!sdl::point_in_rect(x, y, minimap_area())) {
		return map_location();
	}

	// we transform the coordinates from minimap to the full map image
	// probably more adjustments to do (border, minimap shift...)
	// but the mouse and human capacity to evaluate the rectangle center
	// is not pixel precise.
	int px = (x - minimap_location_.x) * get_map().w()*hex_width() / std::max (minimap_location_.w, 1);
	int py = (y - minimap_location_.y) * get_map().h()*hex_size() / std::max(minimap_location_.h, 1);

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

bool display::screenshot(const std::string& filename, bool map_screenshot)
{
	bool res = false;

	if (!map_screenshot) {
		surface& screenshot_surf = video_.getSurface();

		res = image::save_image(screenshot_surf, filename);
#if 0
		// FIXME: the SDL_SavePNG path results in oblique errors that don't
		//        make sense to anyone who's not familiarized with it, so
		//        we can't use this.
		if (!res) {
			ERR_DP << "Screenshot failed: " << SDL_GetError() << '\n';
		}
#endif
	} else {
		if (get_map().empty()) {
			ERR_DP << "No map loaded, cannot create a map screenshot.\n";
			return false;
		}

		SDL_Rect area = max_map_area();
		map_screenshot_surf_ = create_compatible_surface(video_.getSurface(), area.w, area.h);

		if (map_screenshot_surf_ == nullptr) {
			// Memory problem ?
			ERR_DP << "Could not create screenshot surface, try zooming out.\n";
			return false;
		}

		// back up the current map view position and move to top-left
		int old_xpos = xpos_;
		int old_ypos = ypos_;
		xpos_ = 0;
		ypos_ = 0;

		// we reroute render output to the screenshot surface and invalidate all
		map_screenshot_= true ;
		DBG_DP << "draw() with map_screenshot\n";
		//draw(true,true);

		// finally save the image on disk
		res = image::save_image(map_screenshot_surf_,filename);

#if 0
		// FIXME: the SDL_SavePNG path results in oblique errors that don't
		//        make sense to anyone who's not familiarized with it, so
		//        we can't use this.
		if (!res) {
			// Need to do this ASAP or spurious messages result due to
			// redraw_everything calling SDL too (e.g. "SDL_UpperBlit: passed
			// a nullptr surface")
			ERR_DP << "Map screenshot failed: " << SDL_GetError() << '\n';
		}
#endif

		//NOTE: need to be sure that we free this huge surface (is it enough?)
		map_screenshot_surf_ = nullptr;

		// restore normal rendering
		map_screenshot_= false;
		xpos_ = old_xpos;
		ypos_ = old_ypos;
		// some drawing functions are confused by the temporary change
		// of the map_area and thus affect the UI outside of the map
		redraw_everything();
	}

	return res;
}

std::shared_ptr<gui::button> display::find_action_button(const std::string& id)
{
	for (size_t i = 0; i < action_buttons_.size(); ++i) {
		if(action_buttons_[i]->id() == id) {
			return action_buttons_[i];
		}
	}
	return nullptr;
}

std::shared_ptr<gui::button> display::find_menu_button(const std::string& id)
{
	for (size_t i = 0; i < menu_buttons_.size(); ++i) {
		if(menu_buttons_[i]->id() == id) {
			return menu_buttons_[i];
		}
	}
	return nullptr;
}

void display::layout_buttons()
{
	DBG_DP << "positioning menu buttons...\n";
	for(const auto& menu : theme_.menus()) {
		std::shared_ptr<gui::button> b = find_menu_button(menu.get_id());
		if(b) {
			const SDL_Rect& loc = menu.location(screen_.screen_area());
			b->set_location(loc);
			b->set_measurements(0,0);
			b->set_label(menu.title());
			b->set_image(menu.image());
		}
	}

	DBG_DP << "positioning action buttons...\n";
	for(const auto& action : theme_.actions()) {
		std::shared_ptr<gui::button> b = find_action_button(action.get_id());
		if(b) {
			const SDL_Rect& loc = action.location(screen_.screen_area());
			b->set_location(loc);
			b->set_measurements(0,0);
			b->set_label(action.title());
			b->set_image(action.image());
		}
	}
}

void display::create_buttons()
{
	std::vector<std::shared_ptr<gui::button>> menu_work;
	std::vector<std::shared_ptr<gui::button>> action_work;

	DBG_DP << "creating menu buttons...\n";
	for(const auto& menu : theme_.menus()) {
		if (!menu.is_button()) continue;

 		std::shared_ptr<gui::button> b(new gui::button(video_, menu.title(), gui::button::TYPE_PRESS, menu.image(),
				gui::button::DEFAULT_SPACE, false, menu.overlay()));
		DBG_DP << "drawing button " << menu.get_id() << "\n";
		b->join_same(this);
		b->set_id(menu.get_id());
		if (!menu.tooltip().empty()){
			b->set_tooltip_string(menu.tooltip());
		}

		std::shared_ptr<gui::button> b_prev = find_menu_button(b->id());
		if(b_prev) {
			b->enable(b_prev->enabled());
		}

		menu_work.push_back(b);
	}

	DBG_DP << "creating action buttons...\n";
	for(const auto& action : theme_.actions()) {
		std::shared_ptr<gui::button> b(new gui::button(video_, action.title(), string_to_button_type(action.type()), action.image(),
				gui::button::DEFAULT_SPACE, false, action.overlay()));

		DBG_DP << "drawing button " << action.get_id() << "\n";
		b->set_id(action.get_id());
		b->join_same(this);
		if (!action.tooltip(0).empty()){
			b->set_tooltip_string(action.tooltip(0));
		}

		std::shared_ptr<gui::button> b_prev = find_action_button(b->id());
		if(b_prev) {
			b->enable(b_prev->enabled());
			if (b_prev->get_type() == gui::button::TYPE_CHECK) {
				b->set_check(b_prev->checked());
			}
		}

		action_work.push_back(b);
	}


	menu_buttons_.clear();
	menu_buttons_.assign(menu_work.begin(), menu_work.end());
	action_buttons_.clear();
	action_buttons_.assign(action_work.begin(), action_work.end());

	layout_buttons();
	DBG_DP << "buttons created\n";
}

void display::render_buttons()
{
	for (std::shared_ptr<gui::button> btn : menu_buttons_) {
		btn->set_dirty(true);
		btn->draw();
	}

	for (std::shared_ptr<gui::button> btn : action_buttons_) {
		btn->set_dirty(true);
		btn->draw();
	}
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
	static const std::array<std::string, 6> dirs {{ "-n", "-ne", "-se", "-s", "-sw", "-nw" }};
	return dirs[n >= dirs.size() ? 0 : n];
}

std::vector<texture> display::get_fog_shroud_images(const map_location& loc, image::TYPE /*image_type*/)
{
	std::vector<std::string> names;

	adjacent_loc_array_t adjacent;
	get_adjacent_tiles(loc, adjacent.data());

	enum visibility {FOG = 0, SHROUD = 1, CLEAR = 2};
	visibility tiles[6];

	const std::string* image_prefix[] { &game_config::fog_prefix, &game_config::shroud_prefix};

	for(int i = 0; i < 6; ++i) {
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
			// Completely surrounded by fog or shroud. This might have a special graphic.
			const std::string name = *image_prefix[v] + "-all.png";
			if(image::exists(name)) {
				names.push_back(name);
				// Proceed to the next visibility (fog -> shroud -> clear).
				continue;
			}

			// No special graphic found. We'll just combine some other images
			// and hope it works out.
			start = 0;
		}

		// Find all the directions overlap occurs from
		for(int i = (start + 1) %6, cap1 = 0; i != start && cap1 != 6; ++cap1) {
			if(tiles[i] == v) {
				std::ostringstream stream;
				stream << *image_prefix[v];

				std::string name;
				for(int cap2 = 0; v == tiles[i] && cap2 != 6; i = (i + 1) % 6, ++cap2) {
					stream << get_direction(i);

					if(!image::exists(stream.str() + ".png")) {
						// If we don't have any surface at all,
						// then move onto the next overlapped area
						if(name.empty()) {
							i = (i + 1) % 6;
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
				i = (i + 1) % 6;
			}
		}
	}

	// now get the surfaces
	std::vector<texture> res;
	res.reserve(names.size());

	for(std::string& name : names) {
		texture tex(image::get_texture(name)); // TODO: image_type
		if(tex) {
			res.push_back(std::move(tex));
		}
	}

	return res;
}

void display::get_terrain_images(const map_location &loc,
	const std::string& timeid,
	TERRAIN_TYPE terrain_type)
{
	terrain_image_vector_.clear();

	terrain_builder::TERRAIN_TYPE builder_terrain_type = terrain_type == FOREGROUND
		? terrain_builder::FOREGROUND
		: terrain_builder::BACKGROUND;

	assert(builder_);
	const terrain_builder::imagelist* const terrains = builder_->get_terrain_at(loc, timeid, builder_terrain_type);

#if 0
	image::light_string lt;

	const time_of_day& tod = get_time_of_day(loc);

	//get all the light transitions
	adjacent_loc_array_t adjs;
	std::array<const time_of_day*, 6> atods;
	get_adjacent_tiles(loc, adjs.data());
	for(size_t d = 0; d < adjs.size(); ++d){
		atods[d] = &get_time_of_day(adjs[d]);
	}

	for(int d=0; d<6; ++d){
		/* concave
		  _____
		 /     \
		/ atod1 \_____
		\ !tod  /     \
		 \_____/ atod2 \
		 /  \__\ !tod  /
		/       \_____/
		\  tod  /
		 \_____/*/

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod1.color == tod.color || atod2.color == tod.color || atod1.color != atod2.color)
			continue;

		if(lt.empty()) {
			//color the full hex before adding transitions
			tod_color col = tod.color + color_adjust_;
			lt = image::get_light_string(0, col.r, col.g, col.b);
		}

		// add the directional transitions
		tod_color acol = atod1.color + color_adjust_;
		lt += image::get_light_string(d + 1, acol.r, acol.g, acol.b);
	}
	for(int d=0; d<6; ++d){
		/* convex 1
		  _____
		 /     \
		/ atod1 \_____
		\ !tod  /     \
		 \_____/ atod2 \
		 /  \__\  tod  /
		/       \_____/
		\  tod  /
		 \_____/*/

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod1.color == tod.color || atod1.color == atod2.color)
			continue;

		if(lt.empty()) {
			//color the full hex before adding transitions
			tod_color col = tod.color + color_adjust_;
			lt = image::get_light_string(0, col.r, col.g, col.b);
		}

		// add the directional transitions
		tod_color acol = atod1.color + color_adjust_;
		lt += image::get_light_string(d + 7, acol.r, acol.g, acol.b);
	}
	for(int d=0; d<6; ++d){
		/* convex 2
		  _____
		 /     \
		/ atod1 \_____
		\  tod  /     \
		 \_____/ atod2 \
		 /  \__\ !tod  /
		/       \_____/
		\  tod  /
		 \_____/*/

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod2.color == tod.color || atod1.color == atod2.color)
			continue;

		if(lt.empty()) {
			//color the full hex before adding transitions
			tod_color col = tod.color + color_adjust_;
			lt = image::get_light_string(0, col.r, col.g, col.b);
		}

		// add the directional transitions
		tod_color acol = atod2.color + color_adjust_;
		lt += image::get_light_string(d + 13, acol.r, acol.g, acol.b);
	}

	if(lt.empty()){
		tod_color col = tod.color + color_adjust_;
		if(!col.is_zero()){
			// no real lightmap needed but still color the hex
			lt = image::get_light_string(-1, col.r, col.g, col.b);
		}
	}
#endif

	if(terrains != nullptr) {
		// Pre-allocate storage appropriately.
		// Since this function is called extremely often this should save some time.
		res.reserve(terrains->size());

		// Cache the offmap name.
		// Since it is themeable it can change, so don't make it static.
		const std::string off_map_name = "terrain/" + theme_.border().tile_image;

		for(const auto& terrain : *terrains) {
			const image::locator &image = animate_map_
				? terrain.get_current_frame()
				: terrain.get_first_frame();

			// We prevent ToD coloring and brightening of off-map tiles,
			// We need to test for the tile to be rendered and
			// not the location, since the transitions are rendered
			// over the offmap-terrain and these need a ToD coloring.
			texture tex = image::get_texture(image, image::HEXED); // TODO: scaled to hex?

			//const bool off_map = (image.get_filename() == off_map_name || image.get_modifications().find("NO_TOD_SHIFT()") != std::string::npos);
#if 0
			if(off_map) {
				surf = image::get_image(image, image::SCALED_TO_HEX);
			} else if(lt.empty()) {
				surf = image::get_image(image, image::SCALED_TO_HEX);
			} else {
				surf = image::get_lighted_image(image, lt, image::SCALED_TO_HEX);
			}

			if(!tex.null()) {
				terrain_image_vector_.push_back(std::move(tex));
			}
		}
	}
}

void display::toggle_benchmark()
{
	benchmark = !benchmark;
}

void display::toggle_debug_foreground()
{
	debug_foreground = !debug_foreground;
}

void display::draw_debugging_aids()
{
	// TODO: enable.
	return;

	if(video_.update_locked()) {
		return;
	}

	if(preferences::show_fps() || benchmark) {
		static int frames = 0;
		++frames;

		const int sample_freq = 10;
		if(frames == sample_freq) {
			const auto minmax_it = std::minmax_element(frametimes_.begin(), frametimes_.end());
			const unsigned render_avg = std::accumulate(frametimes_.begin(), frametimes_.end(), 0) / frametimes_.size();
			const int avg_fps = calculate_fps(render_avg);
			const int max_fps = calculate_fps(*minmax_it.first);
			const int min_fps = calculate_fps(*minmax_it.second);
			frames = 0;

			if(fps_handle_ != 0) {
				font::remove_floating_label(fps_handle_);
				fps_handle_ = 0;
			}

			std::ostringstream stream;
			stream << "<tt>      min/avg/max/act</tt>\n";
			stream << "<tt>FPS:  " << std::setfill(' ') << std::setw(3) << min_fps << '/'<< std::setw(3) << avg_fps << '/' << std::setw(3) << max_fps << '/' << std::setw(3) << fps_actual_ << "</tt>\n";
			stream << "<tt>Time: " << std::setfill(' ') << std::setw(3) << *minmax_it.first << '/' << std::setw(3) << render_avg << '/' << std::setw(3) << *minmax_it.second << " ms</tt>\n";
			if(game_config::debug) {
				stream << "\nhex: " << drawn_hexes_ * 1.0 / sample_freq;
			}

			drawn_hexes_ = 0;

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
	}
}

static void draw_panel(CVideo &video, const theme::panel& panel, std::vector<std::shared_ptr<gui::button>>& /*buttons*/)
{
	//log_scope("draw panel");
	DBG_DP << "drawing panel " << panel.get_id() << "\n";

	texture tex(image::get_texture(panel.image()));

	const SDL_Rect screen = video.screen_area();
	SDL_Rect& loc = panel.location(screen);

	DBG_DP << "panel location: x=" << loc.x << ", y=" << loc.y
			<< ", w=" << loc.w << ", h=" << loc.h << "\n";

	if(!tex.null()) {
		// TODO: should probably tile this as before.
		video.render_copy(tex, nullptr, &loc);
	}
}

static void draw_label(CVideo& video, surface target, const theme::label& label)
{
	//log_scope("draw label");

	const color_t& RGB = label.font_rgb();

	std::string c_start="<";
	std::string c_sep=",";
	std::string c_end=">";
	std::stringstream color;
	color<< c_start << RGB.r << c_sep << RGB.g << c_sep << RGB.b << c_end;
	std::string text = label.text();

	if(label.font_rgb_set()) {
		color<<text;
		text = color.str();
	}
	const std::string& icon = label.icon();
	SDL_Rect& loc = label.location(video.screen_area());

	if(icon.empty() == false) {
		surface surf(image::get_image(icon));
		if(!surf.null()) {
			if(surf->w > loc.w || surf->h > loc.h) {
				surf.assign(scale_surface(surf,loc.w,loc.h));
			}

			sdl_blit(surf,nullptr,target,&loc);
		}

		if(text.empty() == false) {
			tooltips::add_tooltip(loc,text);
		}
	} else if(text.empty() == false) {
		font::draw_text(&video,loc,label.font_size(),font::NORMAL_COLOR,text,loc.x,loc.y);
	}

}

void display::draw_all_panels()
{
	surface& screen(video_.getSurface());

	/*
	 * The minimap is also a panel, force it to update its contents.
	 * This is required when the size of the minimap has been modified.
	 */
	//recalculate_minimap();

	for(const auto& panel : theme_.panels()) {
		draw_panel(video(), panel, menu_buttons_);
	}

	for(const auto& label : theme_.labels()) {
		draw_label(video(), screen, label);
	}

	render_buttons();
}

static void draw_background(const SDL_Rect& area, const std::string& image)
{
	// No background image, just fill in black.
	if(image.empty()) {
		sdl::fill_rectangle(area, color_t(0, 0, 0));
		return;
	}

	const texture background(image::get_texture(image));
	if(background.null()) {
		return;
	}

	// TODO: should probably tile this as before.
	SDL_Rect a = area;
	CVideo::get_singleton().render_copy(background, nullptr, &a);
}

void display::draw_text_in_hex(const map_location& loc,
		const drawing_queue::layer layer, const std::string& text,
		size_t font_size, color_t color, double x_in_hex, double y_in_hex)
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
				drawing_queue_add(layer, loc, x + dx, y + dy, back_surf);
			}
		}
	}

	drawing_queue_add(layer, loc, x, y, text_surf);
}

void display::select_hex(map_location hex)
{
	selectedHex_ = hex;
	recalculate_minimap();
}

void display::highlight_hex(map_location hex)
{
	mouseoverHex_ = hex;
}

void display::set_diagnostic(const std::string& msg)
{
	if(diagnostic_label_ != 0) {
		font::remove_floating_label(diagnostic_label_);
		diagnostic_label_ = 0;
	}

	if(!msg.empty()) {
		font::floating_label flabel(msg);
		flabel.set_font_size(font::SIZE_PLUS);
		flabel.set_color(font::YELLOW_COLOR);
		flabel.set_position(300, 50);
		flabel.set_clip_rect(map_outside_area());

		diagnostic_label_ = font::add_floating_label(flabel);
	}
}

const theme::action* display::action_pressed()
{
	for(auto i = action_buttons_.begin(); i != action_buttons_.end(); ++i) {
		if((*i)->pressed()) {
			const size_t index = std::distance(action_buttons_.begin(), i);
			if(index >= theme_.actions().size()) {
				assert(false);
				return nullptr;
			}
			return &theme_.actions()[index];
		}
	}

	return nullptr;
}

const theme::menu* display::menu_pressed()
{
	for(auto i = menu_buttons_.begin(); i != menu_buttons_.end(); ++i) {
		if((*i)->pressed()) {
			const size_t index = std::distance(menu_buttons_.begin(), i);
			if(index >= theme_.menus().size()) {
				assert(false);
				return nullptr;
			}
			return theme_.get_menu_item((*i)->id());
		}
	}

	return nullptr;
}

void display::enable_menu(const std::string& item, bool enable)
{
	for(auto menu = theme_.menus().begin(); menu != theme_.menus().end(); ++menu) {

		const auto hasitem = std::find_if(menu->items().begin(), menu->items().end(),
			[&item](const config& c) { return c["id"].str() == item; }
		);

		if(hasitem != menu->items().end()) {
			const size_t index = std::distance(theme_.menus().begin(), menu);
			if(index >= menu_buttons_.size()) {
				continue;
			}
			menu_buttons_[index]->enable(enable);
		}
	}
}

void display::announce(const std::string& message, const color_t& color, const announce_options& options)
{
	if(options.discard_previous) {
		font::remove_floating_label(prevLabel);
	}

	font::floating_label flabel(message);
	flabel.set_font_size(font::SIZE_XLARGE);
	flabel.set_color(color);
	flabel.set_position(map_outside_area().w/2, map_outside_area().h/3);
	flabel.set_lifetime(options.lifetime);
	flabel.set_clip_rect(map_outside_area());

	prevLabel = font::add_floating_label(flabel);
}

void display::draw_minimap()
{
	const SDL_Rect& area = minimap_area();

	if(area.w == 0 || area.h == 0) {
		return;
	}

	if(minimap_ == nullptr || minimap_->w > area.w || minimap_->h > area.h) {
		minimap_ = image::getMinimap(area.w, area.h, get_map(),
			dc_->teams().empty() ? nullptr : &dc_->teams()[currentTeam_],
			(selectedHex_.valid() && !is_blindfolded()) ? &reach_map_ : nullptr);
		if(minimap_ == nullptr) {
			return;
		}
	}

	color_t back_color {31,31,23,SDL_ALPHA_OPAQUE};
	draw_centered_on_background(minimap_, area, back_color);

	//update the minimap location for mouse and units functions
	minimap_location_.x = area.x + (area.w - minimap_->w) / 2;
	minimap_location_.y = area.y + (area.h - minimap_->h) / 2;
	minimap_location_.w = minimap_->w;
	minimap_location_.h = minimap_->h;

	draw_minimap_units();

	// calculate the visible portion of the map:
	// scaling between minimap and full map images
	double xscaling = 1.0 * minimap_->w / (get_map().w() * hex_width());
	double yscaling = 1.0  *minimap_->h / (get_map().h() * hex_size());

	// we need to shift with the border size
	// and the 0.25 from the minimap balanced drawing
	// and the possible difference between real map and outside off-map
	SDL_Rect map_rect = map_area();
	SDL_Rect map_out_rect = map_outside_area();

	double border = theme_.border().size;

	double shift_x = -  border         * hex_width() - (map_out_rect.w - map_rect.w) / 2;
	double shift_y = - (border + 0.25) * hex_size()  - (map_out_rect.h - map_rect.h) / 2;

	int view_x = static_cast<int>((xpos_ + shift_x) * xscaling);
	int view_y = static_cast<int>((ypos_ + shift_y) * yscaling);
	int view_w = static_cast<int>(map_out_rect.w * xscaling);
	int view_h = static_cast<int>(map_out_rect.h * yscaling);

	SDL_Rect outline_rect {
		minimap_location_.x + view_x - 1,
		minimap_location_.y + view_y - 1,
		view_w + 2,
		view_h + 2
	};

	sdl::draw_rectangle(outline_rect, {255, 255, 255, 255});
}

void display::draw_minimap_units()
{
	if (!preferences::minimap_draw_units() || is_blindfolded()) return;

	double xscaling = 1.0 * minimap_location_.w / get_map().w();
	double yscaling = 1.0 * minimap_location_.h / get_map().h();

	for(const auto& u : dc_->units()) {
		if (fogged(u.get_location()) ||
		    (dc_->teams()[currentTeam_].is_enemy(u.side()) &&
		     u.invisible(u.get_location(), *dc_)) ||
			 u.get_hidden()) {
			continue;
		}

		int side = u.side();
		color_t col = team::get_minimap_color(side);

		if (!preferences::minimap_movement_coding()) {

			if (dc_->teams()[currentTeam_].is_enemy(side)) {
				col = game_config::color_info(preferences::enemy_color()).rep();
			} else {

				if (currentTeam_ +1 == static_cast<unsigned>(side)) {

					if (u.movement_left() == u.total_movement())
						col = game_config::color_info(preferences::unmoved_color()).rep();
					else if (u.movement_left() == 0)
						col = game_config::color_info(preferences::moved_color()).rep();
					else
						col = game_config::color_info(preferences::partial_color()).rep();

				} else
					col = game_config::color_info(preferences::allied_color()).rep();
			}
		}

		double u_x = u.get_location().x * xscaling;
		double u_y = (u.get_location().y + (is_odd(u.get_location().x) ? 1 : -1)/4.0) * yscaling;
 		// use 4/3 to compensate the horizontal hexes imbrication
		double u_w = 4.0 / 3.0 * xscaling;
		double u_h = yscaling;

		SDL_Rect r {
				  minimap_location_.x + round_double(u_x)
				, minimap_location_.y + round_double(u_y)
				, round_double(u_w)
				, round_double(u_h)
		};

		sdl::fill_rectangle(r, col);
	}
}

bool display::scroll(int xmove, int ymove, bool force)
{
	if(view_locked_ && !force) {
		return false;
	}

	// No move offset, do nothing.
	if(xmove == 0 && ymove == 0) {
		return false;
	}

	int new_x = xpos_ + xmove;
	int new_y = ypos_ + ymove;

	bounds_check_position(new_x, new_y);

	// Camera position doesn't change, exit.
	if(xpos_ == new_x && ypos_ == new_y) {
		return false;
	}

	const int diff_x = xpos_ - new_x;
	const int diff_y = ypos_ - new_y;

	xpos_ = new_x;
	ypos_ = new_y;

	/* Adjust floating label positions. This only affects labels whose position is anchored
	 * to the map instead of the screen. In order to do that, we want to adjust their drawing
	 * coordinates in the opposite direction of the screen scroll.
	 *
	 * The check a few lines up prevents any scrolling from happening if the camera position
	 * doesn't change. Without that, the label still scroll even when the map edge is reached.
	 * If that's removed, the following formula should work instead:
	 *
	 * const int label_[x,y]_adjust = [x,y]pos_ - new_[x,y];
	 */
	font::scroll_floating_labels(diff_x, diff_y);

	labels().recalculate_shroud();


	if(!screen_.update_locked()) {
		surface& screen(screen_.getSurface());


		SDL_Rect srcrect = dstrect;
		srcrect.x -= diff_x;
		srcrect.y -= diff_y;

		// This is a workaround for a SDL2 bug when blitting on overlapping surfaces. The bug
		// only strikes during scrolling, but will then duplicate textures across the entire map.
		//surface screen_copy = make_neutral_surface(screen);

		//SDL_SetSurfaceBlendMode(screen_copy, SDL_BLENDMODE_NONE);
		//SDL_BlitSurface(screen_copy, &srcrect, screen, &dstrect);
	}

	if(diff_y != 0) {
		SDL_Rect r = map_area();

		if(diff_y < 0) {
			r.y = r.y + r.h + diff_y;
		}

		r.h = std::abs(diff_y);
		invalidate_locations_in_rect(r);
	}

	if(diff_x != 0) {
		SDL_Rect r = map_area();

		if(diff_x < 0) {
			r.x = r.x + r.w + diff_x;
		}

		r.w = std::abs(diff_x);
		invalidate_locations_in_rect(r);
	}

	scroll_event_.notify_observers();

	redrawMinimap_ = true;

	return true;
}

bool display::zoom_at_max()
{
	return zoom_ == MaxZoom;
}

bool display::zoom_at_min()
{
	return zoom_ == MinZoom;
}

bool display::set_zoom(bool increase)
{
	// Ensure we don't try to access nonexistent vector indices.
	zoom_index_ = utils::clamp(increase ? zoom_index_ + 1 : zoom_index_ - 1, 0, final_zoom_index);

	// No validation check is needed in the next step since we've already set the index here and
	// know the new zoom value is indeed valid.
	return set_zoom(zoom_levels[zoom_index_], false);
}

bool display::set_zoom(unsigned int amount, const bool validate_value_and_set_index)
{
	unsigned int new_zoom = utils::clamp(amount, MinZoom, MaxZoom);

	LOG_DP << "new_zoom = " << new_zoom << std::endl;

	if(new_zoom == zoom_) {
		return false;
	}

	// Confirm this is indeed a valid zoom level.
	if(validate_value_and_set_index) {
		auto iter = std::lower_bound(zoom_levels.begin(), zoom_levels.end(), new_zoom);

		if(iter == zoom_levels.end()) {
			// This should never happen, since the value was already clamped earlier
			return false;
		} else if(iter != zoom_levels.begin()) {
			float diff = *iter - *(iter - 1);
			float lower = (new_zoom - *(iter - 1)) / diff;
			float upper = (*iter - new_zoom) / diff;
			if(lower < upper) {
				// It's actually closer to the previous element.
				iter--;
			}
		}

		new_zoom = *iter;
		zoom_index_ = std::distance(zoom_levels.begin(), iter);
	}

	const SDL_Rect& area = map_area();

	//Turn the zoom factor to a double in order to avoid rounding errors.
	double zoom_factor = double(new_zoom) / double(zoom_);

	xpos_ = round_double(((xpos_ + area.w / 2) * zoom_factor) - (area.w / 2));
	ypos_ = round_double(((ypos_ + area.h / 2) * zoom_factor) - (area.h / 2));

	zoom_ = new_zoom;
	bounds_check_position(xpos_, ypos_);
	if(zoom_ != DefaultZoom) {
		last_zoom_ = zoom_;
	}

	image::set_zoom(zoom_);

	labels().recalculate_labels();

	// Forces a redraw after zooming.
	// This prevents some graphic glitches from occurring.
	//draw();

	return true;
}

void display::set_default_zoom()
{
	if (zoom_ != DefaultZoom) {
		last_zoom_ = zoom_;
		set_zoom(DefaultZoom);
	} else {
		// When we are already at the default zoom,
		// switch to the last zoom used
		set_zoom(last_zoom_);
	}
}

bool display::tile_fully_on_screen(const map_location& loc) const
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
	if(video_.update_locked()) {
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
		//draw();
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
		//draw();
	}
}

void display::scroll_to_tile(const map_location& loc, SCROLL_TYPE scroll_type, bool check_fogged, bool force)
{
	if(get_map().on_board(loc) == false) {
		ERR_DP << "Tile at " << loc << " isn't on the map, can't scroll to the tile." << std::endl;
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
			ERR_DP << "Bug in the scrolling code? Looks like we would not need to scroll after all..." << std::endl;
			// keep the target at the center
		}
	}

	scroll_to_xy(target_x, target_y,scroll_type,force);
}


void display::bounds_check_position()
{
	const unsigned int orig_zoom = zoom_;

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

void display::bounds_check_position(int& xpos, int& ypos) const
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

	res |= video_.faked();
	if(res) {
		return turbo_speed_;
	}

	return 1.0;
}

void display::set_idle_anim_rate(int rate)
{
	idle_anim_rate_ = std::pow(2.0, -rate/10.0);
}

map_labels& display::labels()
{
	return *map_labels_;
}

const map_labels& display::labels() const
{
	return *map_labels_;
}

const SDL_Rect& display::get_clip_rect()
{
	return map_area();
}

image::TYPE display::get_image_type(const map_location& /*loc*/)
{
	return image::TOD_COLORED;
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

		sdl_blit(img,&visible_area,video_.getSurface(),&target);
	} else {
		if(img->w != rect.w || img->h != rect.h) {
			img.assign(scale_surface(img,rect.w,rect.h));
		}

		sdl_blit(img,nullptr,video_.getSurface(),&target);
	}
}

/**
 * Redraws the specified report (if anything has changed).
 * If a config is not supplied, it will be generated via
 * reports::generate_report().
 */
void display::refresh_report(const std::string& report_name, const config * new_cfg)
{
	const theme::status_item *item = theme_.get_status_item(report_name);
	if (!item) {
		reportSurfaces_[report_name].assign(nullptr);
		return;
	}

	// Now we will need the config. Generate one if needed.

	boost::optional <events::mouse_handler &> mhb = boost::none;

	if (resources::controller) {
		mhb = resources::controller->get_mouse_handler_base();
	}

	reports::context temp_context = reports::context(*dc_, *this, *resources::tod_manager, wb_.lock(), mhb);

	const config generated_cfg = new_cfg ? config() : reports_object_->generate_report(report_name, temp_context);
	if ( new_cfg == nullptr )
		new_cfg = &generated_cfg;

	SDL_Rect &rect = reportRects_[report_name];
	const SDL_Rect &new_rect = item->location(screen_.screen_area());
	surface &surf = reportSurfaces_[report_name];
	config &report = reports_[report_name];

	// Report and its location is unchanged since last time. Do nothing.
	if (surf && rect == new_rect && report == *new_cfg) {
		return;
	}

	// Update the config in reports_.
	report = *new_cfg;

	if (surf) {
		sdl_blit(surf, nullptr, video_.getSurface(), &rect);
	}

	// If the rectangle has just changed, assign the surface to it
	if (!surf || new_rect != rect)
	{
		surf.assign(nullptr);
		rect = new_rect;

		// If the rectangle is present, and we are blitting text,
		// then we need to backup the surface.
		// (Images generally won't need backing up,
		// unless they are transparent, but that is done later).
		if (rect.w > 0 && rect.h > 0) {
			surf.assign(get_surface_portion(video_.getSurface(), rect));
			if (reportSurfaces_[report_name] == nullptr) {
				ERR_DP << "Could not backup background for report!" << std::endl;
			}
		}
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
		 elements.begin() != elements.end(); elements.pop_front())
	{
		SDL_Rect area {x, y, rect.w + rect.x - x, rect.h + rect.y - y};
		if (area.h <= 0) break;

		std::string t = elements.front()["text"];
		if (!t.empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw a text element.
			font::pango_text text;
			if (item->font_rgb_set()) {
				text.set_foreground_color(item->font_rgb());
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
			config::const_child_iterator ee = elements.begin();
			if (!eol && rect.w - (x - rect.x + s->w) < minimal_text &&
				++ee != elements.end() && !(*ee)["text"].empty())
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

			video_.blit_surface(x, y, s);
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
		else if (!(t = elements.front()["image"].str()).empty())
		{
			if (used_ellipsis) goto skip_element;

			// Draw an image element.
			surface img(image::get_image(t));

			if (!img) {
				ERR_DP << "could not find image for report: '" << t << "'" << std::endl;
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
		t = elements.front()["tooltip"].t_str().base_str();
		if (!t.empty()) {
			if (!used_ellipsis) {
				tooltips::add_tooltip(area, t, elements.front()["help"].t_str().base_str());
			} else {
				// Collect all tooltips for the ellipsis.
				// TODO: need a better separator
				// TODO: assign an action
				ellipsis_tooltip << t;
				config::const_child_iterator ee = elements.begin();
				if (++ee != elements.end())
					ellipsis_tooltip << "\n  _________\n\n";
			}
		}
	}

	if (used_ellipsis) {
		tooltips::add_tooltip(ellipsis_area, ellipsis_tooltip.str());
	}
}

void display::invalidate_animations()
{
	new_animation_frame();

	animate_map_ = preferences::animate_map();
	if(animate_map_) {
		for(const map_location& loc : get_visible_hexes()) {
			if(shrouded(loc)) {
				continue;
			}

			builder_->update_animation(loc);
		}
	}

#ifndef _OPENMP
	for(const unit& u : dc_->units()) {
		u.anim_comp().refresh();
	}

	for(const unit* u : *fake_unit_man_) {
		u->anim_comp().refresh();
	}
#else
	std::vector<const unit*> open_mp_list;
	for(const unit& u : dc_->units()) {
		open_mp_list.push_back(&u);
	}

	// Note that it is an important assumption of the system that the fake units are added to the list
	// after the real units, so that e.g. whiteboard planned moves are drawn over the real units.
	for(const unit* u : *fake_unit_man_) {
		open_mp_list.push_back(u);
	}

	// openMP can't iterate over size_t
	const int omp_iterations = open_mp_list.size();
	// #pragma omp parallel for shared(open_mp_list)

	// This loop must not be parallelized. Refresh is not thread-safe; for one, unit filters are not thread safe.
	// This is because, adding new "scoped" wml variables is not thread safe. Lua itself is not thread safe.
	// When this loop was parallelized, assertion failures were reported in windows openmp builds.
	for(int i = 0; i < omp_iterations; i++) {
		open_mp_list[i]->anim_comp().refresh();
	}
#endif
}

void display::reset_standing_animations()
{
	for(const unit & u : dc_->units()) {
		u.anim_comp().set_standing();
	}
}

void display::add_arrow(arrow& arrow)
{
	const arrow_path_t & arrow_path = arrow.get_path();
	for (const map_location& loc : arrow_path)
	{
		arrows_map_[loc].push_back(&arrow);
	}
}

void display::remove_arrow(arrow& arrow)
{
	const arrow_path_t & arrow_path = arrow.get_path();
	for (const map_location& loc : arrow_path)
	{
		arrows_map_[loc].remove(&arrow);
	}
}

void display::update_arrow(arrow & arrow)
{
	const arrow_path_t & previous_path = arrow.get_previous_path();
	for (const map_location& loc : previous_path)
	{
		arrows_map_[loc].remove(&arrow);
	}
	const arrow_path_t & arrow_path = arrow.get_path();
	for (const map_location& loc : arrow_path)
	{
		arrows_map_[loc].push_back(&arrow);
	}
}

map_location display::get_middle_location() const
{
	const SDL_Rect& rect = map_area();
	return pixel_position_to_hex(xpos_ + rect.x + rect.w / 2 , ypos_ + rect.y + rect.h / 2 );
}

void display::write(config& cfg) const
{
	cfg["view_locked"] = view_locked_;
	cfg["color_adjust_red"] = color_adjust_.r;
	cfg["color_adjust_green"] = color_adjust_.g;
	cfg["color_adjust_blue"] = color_adjust_.b;
	get_middle_location().write(cfg.add_child("location"));
}

void display::read(const config& cfg)
{
	view_locked_ = cfg["view_locked"].to_bool(false);
	color_adjust_.r = cfg["color_adjust_red"].to_int(0);
	color_adjust_.g = cfg["color_adjust_green"].to_int(0);
	color_adjust_.b = cfg["color_adjust_blue"].to_int(0);
}

/* Debugging aid to slow things down a bit. Don't ever enable this in a release! */
#ifdef SLOW_THINGS_DOWN
#undef SLOW_THINGS_DOWN
#endif

// #define SLOW_THINGS_DOWN

void display::redraw_everything()
{
	if(video_.update_locked()) {
		return;
	}

	invalidateGameStatus_ = true;

	reportRects_.clear();
	reportSurfaces_.clear();
	reports_.clear();

	bounds_check_position();

	tooltips::clear_tooltips();

	theme_.set_resolution(screen_area());

	if(!menu_buttons_.empty() || !action_buttons_.empty()) {
		create_buttons();
	}

	if(resources::controller) {
		hotkey::command_executor* command_executor = resources::controller->get_hotkey_command_executor();
		if(command_executor != nullptr)	{
			// This function adds button overlays,
			// it needs to be run after recreating the buttons.
			command_executor->set_button_state();
		}
	}

	if (!gui::in_dialog()) {
		labels().recalculate_labels();
	}

	for(std::function<void(display&)> f : redraw_observers_) {
		f(*this);
	}

	int ticks1 = SDL_GetTicks();
	//invalidate_all();
	int ticks2 = SDL_GetTicks();
	//draw(true,true);
	int ticks3 = SDL_GetTicks();
	LOG_DP << "invalidate and draw: " << (ticks3 - ticks2) << " and " << (ticks2 - ticks1) << "\n";

	complete_redraw_event_.notify_observers();
}


//
// NEW RENDERING CODE =========================================================================
//

void display::draw_hex_overlays()
{
	// DO NOTHING
}

void display::draw_visible_hexes(const rect_of_hexes& visible_hexes, TERRAIN_TYPE terrain_type)
{
	for(const map_location& loc : visible_hexes) {
		if(shrouded(loc)) {
			continue;
		}

		image::TYPE image_type = get_image_type(loc);
		const time_of_day& tod = get_time_of_day(loc);

		for(const texture& t : get_terrain_images(loc, tod.id, image_type, terrain_type)) {
			render_scaled_to_zoom(t, loc);
		}
	}
}

void display::draw_gamemap()
{
	// Currenty visible hexes.
	const rect_of_hexes& visible_hexes = get_visible_hexes();

	//
	// Background terrains
	//
	draw_visible_hexes(visible_hexes, BACKGROUND);

	//
	// On-map overlays, such as [item]s.
	//
	for(const auto& overlay_record : *overlays_) {
		const map_location& o_loc = overlay_record.first;

		if(shrouded(o_loc)) {
			continue;
		}

		const overlay& item = overlay_record.second;
		const std::string& current_team_name = dc_->teams()[viewing_team()].team_name();

		if((item.team_name.empty() || item.team_name.find(current_team_name) != std::string::npos) &&
			(!fogged(o_loc) || item.visible_in_fog))
		{
			const texture tex = item.image.find("~NO_TOD_SHIFT()") == std::string::npos
				? image::get_texture(item.image) // TODO
				: image::get_texture(item.image);

			// was: SCALED_TO_HEX
			render_scaled_to_zoom(tex, o_loc);
		}
	}

	//
	// Village flags
	//
	for(const team& t : dc_->teams()) {
		auto& flag = flags_[t.side() - 1];
		flag.update_last_draw_time();

		for(const map_location& v_loc : t.villages()) {
			if(!fogged(v_loc) || !dc_->get_team(viewing_side()).is_enemy(t.side())) {

				// TODO: move this if-animated check to a helper function.
				const image::locator& flag_image = animate_map_
					? flag.get_current_frame()
					: flag.get_first_frame();

				render_scaled_to_zoom(image::get_texture(flag_image), v_loc);
			}
		}
	}

	//
	// The grid overlay, if that's been enabled
	//
	if(grid_) {
		for(const map_location& loc : visible_hexes) {
			if(shrouded(loc)) {
				continue;
			}

			// TODO: split into drawing layers? If not, combine into one image or texture.
			static const texture grid_top = image::get_texture(game_config::images::grid_top);
			render_scaled_to_zoom(grid_top, loc);

			static const texture grid_bottom = image::get_texture(game_config::images::grid_bottom);
			render_scaled_to_zoom(grid_bottom, loc);
		}
	}

	//
	// Units
	//
	if(!dc_->teams().empty()) {
		unit_drawer drawer = unit_drawer(*this);

		// Real units
		for(const unit& real_unit : dc_->units()) {
			drawer.redraw_unit(real_unit);
		}

		// Fake (moving) units
		for(const unit* temp_unit : *fake_unit_man_) {
			drawer.redraw_unit(*temp_unit);
		}

		// TODO: re-add exclusionary checks for exclusive_unit_draw_requests_ later on, if necessary.
#if 0
		auto request = exclusive_unit_draw_requests_.find(loc);
		if(request == exclusive_unit_draw_requests_.end() || request->second == u_it->id()) {};
#endif
	}

	//
	// Foreground terrains. FIXME! sometimes cut off units...
	//
	draw_visible_hexes(visible_hexes, FOREGROUND);

	//
	// Hex cursor (TODO: split into layers?)
	//
	draw_hex_cursor(mouseoverHex_);

	//
	// Right now just handles halos - see game_display
	//
	post_commit();

	draw_hex_overlays();

	//
	// Shroud and fog
	//
	for(const map_location& loc : visible_hexes) {
		image::TYPE image_type = get_image_type(loc);

		const bool is_shrouded = shrouded(loc);
		const bool is_fogged = fogged(loc);

		// Main images.
		if(is_shrouded || is_fogged) {

			// If is_shrouded is false, is_fogged is true
			const std::string& weather_image = is_shrouded
				? get_variant(shroud_images_, loc)
				: get_variant(fog_images_, loc);

			// TODO: image type
			render_scaled_to_zoom(image::get_texture(weather_image), loc);
		}

		// Transitions to main hexes.
		if(!is_shrouded) {
			for(const texture& t : get_fog_shroud_images(loc, image_type)) {
				render_scaled_to_zoom(t, loc);
			}
		}
	}

	// ================================================================================
	// TODO: RE-ADD ALL THIS!
	// ================================================================================

#if 0

	//
	// Mouseover overlays (TODO: delegate to editor)
	//
	if(loc == mouseoverHex_ && (on_map || (in_editor() && get_map().on_board_with_border(loc)))
		&& mouseover_hex_overlay_ != nullptr)
	{
		render_scaled_to_zoom(mouseover_hex_overlay_, xpos, ypos);
	}

	//
	// Arrows (whiteboard?) TODO:
	//
	auto arrows_in_hex = arrows_map_.find(loc);
	if(arrows_in_hex != arrows_map_.end()) {
		for(arrow* const a : arrows_in_hex->second) {
			a->draw_hex(loc);
		}
	}

	//
	// ToD Mask
	//
	// Draw the time-of-day mask on top of the terrain in the hex.
	// tod may differ from tod if hex is illuminated.
	const std::string& tod_hex_mask = tod.image_mask;
	if(tod_hex_mask1 != nullptr || tod_hex_mask2 != nullptr) {
		drawing_queue_add(drawing_queue::LAYER_TERRAIN_FG, loc, xpos, ypos, tod_hex_mask1);
		drawing_queue_add(drawing_queue::LAYER_TERRAIN_FG, loc, xpos, ypos, tod_hex_mask2);
	} else if(!tod_hex_mask.empty()) {
		drawing_queue_add(drawing_queue::LAYER_TERRAIN_FG, loc, xpos, ypos,
			image::get_image(tod_hex_mask,image::SCALED_TO_HEX));
	}

	//
	// Debugging output - coordinates, etc.
	//
	if(on_map) {
		if(draw_coordinates_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(loc), font::SIZE_SMALL, font::NORMAL_COLOR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect {0, 0, text->w, text->h};
			sdl::fill_surface_rect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			off_y -= text->h / 2;
			if (draw_terrain_codes_) {
				off_y -= text->h / 2;
			}
			if (draw_num_of_bitmaps_) {
				off_y -= text->h / 2;
			}
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, bg);
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, text);
		}

		if(draw_terrain_codes_ && (game_config::debug || !shrouded(loc))) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(get_map().get_terrain(loc)), font::SIZE_SMALL, font::NORMAL_COLOR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect {0, 0, text->w, text->h};
			sdl::fill_surface_rect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			off_y -= text->h / 2;
			if (draw_coordinates_ && !draw_num_of_bitmaps_) {
				off_y += text->h / 2;
			} else if (draw_num_of_bitmaps_ && !draw_coordinates_) {
				off_y -= text->h / 2;
			}
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, bg);
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, text);
		}

		if(draw_num_of_bitmaps_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			surface text = font::get_rendered_text(lexical_cast<std::string>(num_images_bg + num_images_fg), font::SIZE_SMALL, font::NORMAL_COLOR);
			surface bg = create_neutral_surface(text->w, text->h);
			SDL_Rect bg_rect {0, 0, text->w, text->h};
			sdl::fill_surface_rect(bg, &bg_rect, 0xaa000000);
			off_x -= text->w / 2;
			off_y -= text->h / 2;
			if (draw_coordinates_) {
				off_y += text->h / 2;
			}
			if (draw_terrain_codes_) {
				off_y += text->h / 2;
			}
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, bg);
			drawing_queue_add(drawing_queue::LAYER_FOG_SHROUD, loc, off_x, off_y, text);
		}
	}

	if(debug_foreground) {
		drawing_queue_add(drawing_queue::LAYER_UNIT_DEFAULT, loc, xpos, ypos,
			image::get_image("terrain/foreground.png", image_type));
	}
#endif
}

void display::draw()
{
	// Execute any pre-draw actions from derived classes.
	pre_draw();

	// Draw theme background.
	const SDL_Rect outside_area = map_outside_area();
	draw_background(outside_area, theme_.border().background_image);

	// Progress animations.
	invalidate_animations();

	draw_all_panels();
	draw_minimap();

	// Draw the gamemap and its contents (units, etc)
	{
		SDL_Rect map_area_rect = map_area();
		render_clip_rect_setter setter(&map_area_rect);

		draw_gamemap();
	}

	// Draw map labels.
	font::draw_floating_labels();

	// TODO: what dis?
	//events::raise_volatile_draw_event();
	//events::raise_volatile_undraw_event();

	// Draw debugging aids such as the FPS counter.
	draw_debugging_aids();

	// Call any redraw observers.
	// FIXME: makes the editor slow.
#if 0
	for(std::function<void(display&)> f : redraw_observers_) {
		f(*this);
	}
#endif

	// Execute any post-draw actions from derived classes.
	post_draw();
}

void display::handle_window_event(const SDL_Event& event)
{
	if(event.type == SDL_WINDOWEVENT) {
		switch(event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_EXPOSED:
			// TODO: add additional handling here if needed.
			break;
		}
	}
}

void display::handle_event(const SDL_Event& /*event*/)
{
	if(gui2::dialogs::loading_screen::displaying()) {
		return;
	}
}

display *display::singleton_ = nullptr;
