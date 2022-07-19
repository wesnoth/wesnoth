/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "color.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "draw.hpp"
#include "draw_manager.hpp"
#include "fake_unit_manager.hpp"
#include "filesystem.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "font/text.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "halo.hpp"
#include "hotkey/command_executor.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "minimap.hpp"
#include "overlay.hpp"
#include "play_controller.hpp" //note: this can probably be refactored out
#include "reports.hpp"
#include "resources.hpp"
#include "sdl/utils.hpp" // fill_surface_rect
#include "show_dialog.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "time_of_day.hpp"
#include "tooltips.hpp"
#include "tod_manager.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/drawer.hpp"
#include "units/orb_status.hpp"
#include "video.hpp"
#include "whiteboard/manager.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

// Includes for bug #17573

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)
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
	// if this is enabled with :benchmark, then everything is marked as invalid and redrawn each time
	bool benchmark = false;

	bool debug_foreground = false;

	int prevLabel = 0;
}

unsigned int display::zoom_ = DefaultZoom;
unsigned int display::last_zoom_ = SmallZoom;

// Returns index of zoom_levels which is closest match to input zoom_level
// Assumption: zoom_levels is a sorted vector of ascending tile sizes
static int get_zoom_levels_index(unsigned int zoom_level)
{
	zoom_level = std::clamp(zoom_level, MinZoom, MaxZoom);	// ensure zoom_level is within zoom_levels bounds
	auto iter = std::lower_bound(zoom_levels.begin(), zoom_levels.end(), zoom_level);

	// find closest match
	if(iter != zoom_levels.begin() && iter != zoom_levels.end()) {
		float diff = *iter - *(iter - 1);
		float lower = (zoom_level - *(iter - 1)) / diff;
		float upper = (*iter - zoom_level) / diff;

		// the previous element is closer to zoom_level than the current one
		if(lower < upper) {
			iter--;
		}
	}

	return std::distance(zoom_levels.begin(), iter);
}

void display::parse_team_overlays()
{
	const team& curr_team = dc_->teams()[playing_team()];
	const team& prev_team = playing_team() == 0
		? dc_->teams().back()
		: dc_->get_team(playing_team());
	for (const auto& i : get_overlays()) {
		for(const overlay& ov : i.second) {
			if(!ov.team_name.empty() &&
				((ov.team_name.find(curr_team.team_name()) + 1) != 0) !=
				((ov.team_name.find(prev_team.team_name()) + 1) != 0))
			{
				invalidate(i.first);
			}
		}
	}
}


void display::add_overlay(const map_location& loc, const std::string& img, const std::string& halo, const std::string& team_name, const std::string& item_id, bool visible_under_fog, float z_order)
{
	halo::handle halo_handle;
	if(halo != "") {
		halo_handle = halo_man_.add(get_location_x(loc) + hex_size() / 2,
			get_location_y(loc) + hex_size() / 2, halo, loc);
	}

	std::vector<overlay>& overlays = get_overlays()[loc];
	auto it = std::find_if(overlays.begin(), overlays.end(), [z_order](const overlay& ov) { return ov.z_order > z_order; });
	overlays.emplace(it, img, halo, halo_handle, team_name, item_id, visible_under_fog, z_order);
}

void display::remove_overlay(const map_location& loc)
{
	get_overlays().erase(loc);
}

void display::remove_single_overlay(const map_location& loc, const std::string& toDelete)
{
	std::vector<overlay>& overlays = get_overlays()[loc];
	overlays.erase(
		std::remove_if(
			overlays.begin(), overlays.end(),
			[&toDelete](const overlay& ov) { return ov.image == toDelete || ov.halo == toDelete || ov.id == toDelete; }
		),
		overlays.end()
	);
}

display::display(const display_context* dc,
		std::weak_ptr<wb::manager> wb,
		reports& reports_object,
		const std::string& theme_id,
		const config& level)
	: dc_(dc)
	, halo_man_()
	, wb_(wb)
	, exclusive_unit_draw_requests_()
	, currentTeam_(0)
	, dont_show_all_(false)
	, xpos_(0)
	, ypos_(0)
	, view_locked_(false)
	, theme_(theme::get_theme_config(theme_id.empty() ? preferences::theme() : theme_id), video::game_canvas())
	, zoom_index_(0)
	, fake_unit_man_(new fake_unit_manager(*this))
	, builder_(new terrain_builder(level, (dc_ ? &dc_->map() : nullptr), theme_.border().tile_image, theme_.border().show_border))
	, minimap_(nullptr)
	, minimap_location_(sdl::empty_rect)
	, redraw_background_(false)
	, invalidateAll_(true)
	, diagnostic_label_(0)
	, invalidateGameStatus_(true)
	, map_labels_(new map_labels(nullptr))
	, reports_object_(&reports_object)
	, scroll_event_("scrolled")
	, frametimes_(50)
	, fps_counter_()
	, fps_start_()
	, fps_actual_()
	, reportLocations_()
	, reportSurfaces_()
	, reports_()
	, menu_buttons_()
	, action_buttons_()
	, invalidated_()
	, mouseover_hex_overlay_()
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
	, drawing_buffer_()
	, map_screenshot_(false)
	, reach_map_()
	, reach_map_old_()
	, reach_map_changed_(true)
	, fps_handle_(0)
	, invalidated_hexes_(0)
	, drawn_hexes_(0)
	, redraw_observers_()
	, draw_coordinates_(false)
	, draw_terrain_codes_(false)
	, draw_num_of_bitmaps_(false)
	, arrows_map_()
	, color_adjust_()
{
	//The following assertion fails when starting a campaign
	assert(singleton_ == nullptr);
	singleton_ = this;

	resources::fake_units = fake_unit_man_.get();

	blindfold_ctr_ = 0;

	read(level.child_or_empty("display"));

	fill_images_list(game_config::fog_prefix, fog_images_);
	fill_images_list(game_config::shroud_prefix, shroud_images_);

	unsigned int tile_size = preferences::tile_size();
	if(tile_size < MinZoom || tile_size > MaxZoom)
		tile_size = DefaultZoom;
	zoom_index_ = get_zoom_levels_index(tile_size);
	zoom_ = zoom_levels[zoom_index_];
	if(zoom_ != preferences::tile_size())	// correct saved tile_size if necessary
		preferences::set_tile_size(zoom_);

	image::set_zoom(zoom_);

	init_flags();

	if(!menu_buttons_.empty() || !action_buttons_.empty()) {
		create_buttons();
	}

#ifdef _WIN32
	// Increase timer resolution to prevent delays getting much longer than they should.
	timeBeginPeriod(1u);
#endif
}

display::~display()
{
#ifdef _WIN32
	timeEndPeriod(1u);
#endif

	singleton_ = nullptr;
	resources::fake_units = nullptr;
}

void display::set_theme(const std::string& new_theme)
{
	theme_ = theme{theme::get_theme_config(new_theme), video::game_canvas()};
	builder_->set_draw_border(theme_.border().show_border);
	menu_buttons_.clear();
	action_buttons_.clear();
	create_buttons();
	rebuild_all();
	queue_rerender();
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
}

void display::reinit_flags_for_team(const team& t)
{
	init_flags_for_side_internal(t.side() - 1, t.color());
}

void display::init_flags_for_side_internal(std::size_t n, const std::string& side_color)
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

	LOG_DP << "Adding flag for team " << n << " from animation " << flag;

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
			} catch(const std::invalid_argument&) {
				ERR_DP << "Invalid time value found when constructing flag for side " << n << ": " << sub_items.back();
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
		ERR_DP << "missing flag for team" << n;
	}
}

texture display::get_flag(const map_location& loc)
{
	if(!get_map().is_village(loc)) {
		return texture();
	}

	for (const team& t : dc_->teams()) {
		if (t.owns_village(loc) && (!fogged(loc) || !dc_->get_team(viewing_side()).is_enemy(t.side())))
		{
			auto& flag = flags_[t.side() - 1];
			flag.update_last_draw_time();
			const image::locator &image_flag = animate_map_ ?
				flag.get_current_frame() : flag.get_first_frame();
			return image::get_texture(image_flag, image::TOD_COLORED);
		}
	}

	return texture();
}

void display::set_team(std::size_t teamindex, bool show_everything)
{
	assert(teamindex < dc_->teams().size());
	currentTeam_ = teamindex;
	if(!show_everything) {
		labels().set_team(&dc_->teams()[teamindex]);
		dont_show_all_ = true;
	} else {
		labels().set_team(nullptr);
		dont_show_all_ = false;
	}
	labels().recalculate_labels();
	if(std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->on_viewer_change(teamindex);
	}
}

void display::set_playing_team(std::size_t teamindex)
{
	assert(teamindex < dc_->teams().size());
	activeTeam_ = teamindex;
	invalidate_game_status();
}

bool display::add_exclusive_draw(const map_location& loc, unit& unit)
{
	if(loc.valid() && exclusive_unit_draw_requests_.find(loc) == exclusive_unit_draw_requests_.end()) {
		exclusive_unit_draw_requests_[loc] = unit.id();
		return true;
	} else {
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
	if(prefix == ""){
		return;
	}

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
	redraw_background_ = true;
	builder_->reload_map();
}

void display::change_display_context(const display_context * dc)
{
	dc_ = dc;
	builder_->change_map(&dc_->map()); //TODO: Should display_context own and initialize the builder object?
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

const rect& display::minimap_area() const
{
	return theme_.mini_map_location(video::game_canvas());
}

const rect& display::palette_area() const
{
	return theme_.palette_location(video::game_canvas());
}

const rect& display::unit_image_area() const
{
	return theme_.unit_image_location(video::game_canvas());
}

rect display::max_map_area() const
{
	rect max_area{0, 0, 0, 0};

	// hex_size() is always a multiple of 4
	// and hex_width() a multiple of 3,
	// so there shouldn't be off-by-one-errors
	// due to rounding.
	// To display a hex fully on screen,
	// a little bit extra space is needed.
	// Also added the border two times.
	max_area.w = static_cast<int>((get_map().w() + 2 * theme_.border().size + 1.0 / 3.0) * hex_width());
	max_area.h = static_cast<int>((get_map().h() + 2 * theme_.border().size + 0.5) * hex_size());

	return max_area;
}

rect display::map_area() const
{
	rect max_area = max_map_area();

	// if it's for map_screenshot, maximize and don't recenter
	if(map_screenshot_) {
		return max_area;
	}

	rect res = map_outside_area();

	if(max_area.w < res.w) {
		// map is smaller, center
		res.x += (res.w - max_area.w) / 2;
		res.w = max_area.w;
	}

	if(max_area.h < res.h) {
		// map is smaller, center
		res.y += (res.h - max_area.h) / 2;
		res.h = max_area.h;
	}

	return res;
}

rect display::map_outside_area() const
{
	if(map_screenshot_) {
		return max_map_area();
	} else {
		return theme_.main_map_location(video::game_canvas());
	}
}

bool display::outside_area(const SDL_Rect& area, const int x, const int y)
{
	const int x_thresh = hex_size();
	const int y_thresh = hex_size();
	return (x < area.x || x > area.x + area.w - x_thresh || y < area.y || y > area.y + area.h - y_thresh);
}

// This function uses the screen as reference
const map_location display::hex_clicked_on(int xclick, int yclick) const
{
	rect r = map_area();
	if(!r.contains(xclick, yclick)) {
		return map_location();
	}

	xclick -= r.x;
	yclick -= r.y;

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

	if(r.w <= 0 || r.h <= 0) {
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

map_location display::minimap_location_on(int x, int y)
{
	// TODO: don't return location for this,
	//  instead directly scroll to the clicked pixel position

	if(!minimap_area().contains(x, y)) {
		return map_location();
	}

	// we transform the coordinates from minimap to the full map image
	// probably more adjustments to do (border, minimap shift...)
	// but the mouse and human capacity to evaluate the rectangle center
	// is not pixel precise.
	int px = (x - minimap_location_.x) * get_map().w() * hex_width() / std::max(minimap_location_.w, 1);
	int py = (y - minimap_location_.y) * get_map().h() * hex_size() / std::max(minimap_location_.h, 1);

	map_location loc = pixel_position_to_hex(px, py);
	if(loc.x < 0) {
		loc.x = 0;
	} else if(loc.x >= get_map().w()) {
		loc.x = get_map().w() - 1;
	}

	if(loc.y < 0) {
		loc.y = 0;
	} else if(loc.y >= get_map().h()) {
		loc.y = get_map().h() - 1;
	}

	return loc;
}

surface display::screenshot(bool map_screenshot)
{
	if (!map_screenshot) {
		LOG_DP << "taking ordinary screenshot";
		return video::read_pixels();
	}

	if (get_map().empty()) {
		ERR_DP << "No map loaded, cannot create a map screenshot.";
		return nullptr;
	}

	// back up the current map view position and move to top-left
	int old_xpos = xpos_;
	int old_ypos = ypos_;
	xpos_ = 0;
	ypos_ = 0;

	// Reroute render output to a separate texture until the end of scope.
	SDL_Rect area = max_map_area();
	if (area.w > 1 << 16 || area.h > 1 << 16) {
		WRN_DP << "Excessively large map screenshot area";
	}
	LOG_DP << "creating " << area.w << " by " << area.h
	       << " texture for map screenshot";
	texture output_texture(area.w, area.h, SDL_TEXTUREACCESS_TARGET);
	auto target_setter = draw::set_render_target(output_texture);
	auto clipper = draw::override_clip(area);

	map_screenshot_ = true;

	invalidate_locations_in_rect(map_area());
	draw();

	map_screenshot_ = false;

	// Restore map viewport position
	xpos_ = old_xpos;
	ypos_ = old_ypos;

	// Read rendered pixels back as an SDL surface.
	return video::read_pixels();
}

std::shared_ptr<gui::button> display::find_action_button(const std::string& id)
{
	for(auto& b : action_buttons_) {
		if(b->id() == id) {
			return b;
		}
	}
	return nullptr;
}

std::shared_ptr<gui::button> display::find_menu_button(const std::string& id)
{
	for(auto& b : menu_buttons_) {
		if(b->id() == id) {
			return b;
		}
	}
	return nullptr;
}

void display::layout_buttons()
{
	DBG_DP << "positioning menu buttons...";
	for(const auto& menu : theme_.menus()) {
		if(auto b = find_menu_button(menu.get_id())) {
			const SDL_Rect& loc = menu.location(video::game_canvas());
			b->set_location(loc);
			b->set_measurements(0,0);
			b->set_label(menu.title());
			b->set_image(menu.image());
		}
	}

	DBG_DP << "positioning action buttons...";
	for(const auto& action : theme_.actions()) {
		if(auto b = find_action_button(action.get_id())) {
			const SDL_Rect& loc = action.location(video::game_canvas());
			b->set_location(loc);
			b->set_measurements(0,0);
			b->set_label(action.title());
			b->set_image(action.image());
		}
	}
}

namespace
{
gui::button::TYPE string_to_button_type(const std::string& type)
{
	if(type == "checkbox") {
		return gui::button::TYPE_CHECK;
	} else if(type == "image") {
		return gui::button::TYPE_IMAGE;
	} else if(type == "radiobox") {
		return gui::button::TYPE_RADIO;
	} else if(type == "turbo") {
		return gui::button::TYPE_TURBO;
	} else {
		return gui::button::TYPE_PRESS;
	}
}

const std::string& get_direction(std::size_t n)
{
	using namespace std::literals::string_literals;
	static const std::array dirs{"-n"s, "-ne"s, "-se"s, "-s"s, "-sw"s, "-nw"s};
	return dirs[n >= dirs.size() ? 0 : n];
}
} // namespace

void display::create_buttons()
{
	if(video::headless()) {
		return;
	}

	menu_buttons_.clear();
	action_buttons_.clear();

	DBG_DP << "creating menu buttons...";
	for(const auto& menu : theme_.menus()) {
		if(!menu.is_button()) {
			continue;
		}

		auto b = std::make_shared<gui::button>(menu.title(), gui::button::TYPE_PRESS, menu.image(),
			gui::button::DEFAULT_SPACE, true, menu.overlay(), font::SIZE_BUTTON_SMALL);

		DBG_DP << "drawing button " << menu.get_id();
		b->set_id(menu.get_id());
		if(!menu.tooltip().empty()) {
			b->set_tooltip_string(menu.tooltip());
		}

		if(auto b_prev = find_menu_button(b->id())) {
			b->enable(b_prev->enabled());
		}

		menu_buttons_.push_back(std::move(b));
	}

	DBG_DP << "creating action buttons...";
	for(const auto& action : theme_.actions()) {
		auto b = std::make_shared<gui::button>(action.title(), string_to_button_type(action.type()),
			action.image(), gui::button::DEFAULT_SPACE, true, action.overlay(), font::SIZE_BUTTON_SMALL);

		DBG_DP << "drawing button " << action.get_id();
		b->set_id(action.get_id());
		if(!action.tooltip(0).empty()) {
			b->set_tooltip_string(action.tooltip(0));
		}

		if(auto b_prev = find_action_button(b->id())) {
			b->enable(b_prev->enabled());
			if(b_prev->get_type() == gui::button::TYPE_CHECK) {
				b->set_check(b_prev->checked());
			}
		}

		action_buttons_.push_back(std::move(b));
	}

	layout_buttons();
	DBG_DP << "buttons created";
}

void display::draw_buttons()
{
	const rect clip = draw::get_clip();
	for(auto& btn : menu_buttons_) {
		if(clip.overlaps(btn->location())) {
			//btn->set_dirty(true);
			// TODO: draw_manager - this won't actually draw, because it's not "dirty", but dirty can't be set because that invalidates. Overhaul.
			btn->draw();
		}
	}

	for(auto& btn : action_buttons_) {
		if(clip.overlaps(btn->location())) {
			//btn->set_dirty(true);
			btn->draw();
		}
	}
}

std::vector<texture> display::get_fog_shroud_images(const map_location& loc, image::TYPE image_type)
{
	std::vector<std::string> names;
	const auto adjacent = get_adjacent_tiles(loc);

	enum visibility { FOG = 0, SHROUD = 1, CLEAR = 2 };
	std::array<visibility, 6> tiles;

	const std::array image_prefix{&game_config::fog_prefix, &game_config::shroud_prefix};

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
			// Completely surrounded by fog or shroud. This might have
			// a special graphic.
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
		for(int i = (start + 1) % 6, cap1 = 0; i != start && cap1 != 6; ++cap1) {
			if(tiles[i] == v) {
				std::ostringstream stream;
				std::string name;
				stream << *image_prefix[v];

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

	// now get the textures
	std::vector<texture> res;

	for(const std::string& name : names) {
		if(texture tex = image::get_texture(name, image_type)) {
			res.push_back(std::move(tex));
		}
	}

	return res;
}

void display::get_terrain_images(const map_location& loc, const std::string& timeid, TERRAIN_TYPE terrain_type)
{
	terrain_image_vector_.clear();

	image::light_string lt;
	const time_of_day& tod = get_time_of_day(loc);

	// get all the light transitions
	const auto adjs = get_adjacent_tiles(loc);
	std::array<const time_of_day*, adjs.size()> atods;

	for(std::size_t d = 0; d < adjs.size(); ++d) {
		atods[d] = &get_time_of_day(adjs[d]);
	}

	for(int d = 0; d < 6; ++d) {
		/*
		concave
		  _____
		 /     \
		/ atod1 \_____
		\ !tod  /     \
		 \_____/ atod2 \
		 /  \__\ !tod  /
		/       \_____/
		\  tod  /
		 \_____/
		 */

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod1.color == tod.color || atod2.color == tod.color || atod1.color != atod2.color) {
			continue;
		}

		if(lt.empty()) {
			// color the full hex before adding transitions
			tod_color col = tod.color + color_adjust_;
			lt = image::get_light_string(0, col.r, col.g, col.b);
		}

		// add the directional transitions
		tod_color acol = atod1.color + color_adjust_;
		lt += image::get_light_string(d + 1, acol.r, acol.g, acol.b);
	}

	for(int d = 0; d < 6; ++d) {
		/*
		convex 1
		  _____
		 /     \
		/ atod1 \_____
		\ !tod  /     \
		 \_____/ atod2 \
		 /  \__\  tod  /
		/       \_____/
		\  tod  /
		 \_____/
		 */

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod1.color == tod.color || atod1.color == atod2.color) {
			continue;
		}

		if(lt.empty()) {
			// color the full hex before adding transitions
			tod_color col = tod.color + color_adjust_;
			lt = image::get_light_string(0, col.r, col.g, col.b);
		}

		// add the directional transitions
		tod_color acol = atod1.color + color_adjust_;
		lt += image::get_light_string(d + 7, acol.r, acol.g, acol.b);
	}

	for(int d = 0; d < 6; ++d) {
		/*
		convex 2
		  _____
		 /     \
		/ atod1 \_____
		\  tod  /     \
		 \_____/ atod2 \
		 /  \__\ !tod  /
		/       \_____/
		\  tod  /
		 \_____/
		 */

		const time_of_day& atod1 = *atods[d];
		const time_of_day& atod2 = *atods[(d + 1) % 6];

		if(atod2.color == tod.color || atod1.color == atod2.color) {
			continue;
		}

		if(lt.empty()) {
			// color the full hex before adding transitions
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

	const terrain_builder::TERRAIN_TYPE builder_terrain_type = terrain_type == FOREGROUND
		? terrain_builder::FOREGROUND
		: terrain_builder::BACKGROUND;

	if(const terrain_builder::imagelist* const terrains = builder_->get_terrain_at(loc, timeid, builder_terrain_type)) {
		// Cache the offmap name. Since it is themeable it can change, so don't make it static.
		const std::string off_map_name = "terrain/" + theme_.border().tile_image;
		for(const auto& terrain : *terrains) {
			const image::locator& image = animate_map_ ? terrain.get_current_frame() : terrain.get_first_frame();

			// We prevent ToD coloring and brightening of off-map tiles,
			// We need to test for the tile to be rendered and
			// not the location, since the transitions are rendered
			// over the offmap-terrain and these need a ToD coloring.
			texture tex;
			const bool off_map = (image.get_filename() == off_map_name
				|| image.get_modifications().find("NO_TOD_SHIFT()") != std::string::npos);

			if(off_map) {
				tex = image::get_texture(image, image::HEXED);
			} else if(lt.empty()) {
				tex = image::get_texture(image, image::HEXED);
			} else {
				tex = image::get_lighted_texture(image, lt);
			}

			if(tex) {
				terrain_image_vector_.push_back(std::move(tex));
			}
		}
	}
}

display::blit_helper& display::drawing_buffer_add(const drawing_layer layer,
		const map_location& loc, const SDL_Rect& dest, const texture& tex,
		const SDL_Rect &clip)
{
	drawing_buffer_.emplace_back(layer, loc, dest, tex, clip);
	return drawing_buffer_.back();
}

display::blit_helper& display::drawing_buffer_add(const drawing_layer layer,
		const map_location& loc, const SDL_Rect& dest,
		const std::vector<texture> &tex, const SDL_Rect &clip)
{
	drawing_buffer_.emplace_back(layer, loc, dest, tex, clip);
	return drawing_buffer_.back();
}

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

inline display::drawing_buffer_key::drawing_buffer_key(const map_location &loc, drawing_layer layer)
	: key_(0)
{
	// Start with the index of last group entry...
	unsigned int group_i = layer_groups.size() - 1;

	// ...and works backwards until the group containing the specified layer is found.
	while(layer < layer_groups[group_i]) {
		--group_i;
	}

	enum {
		SHIFT_LAYER          = BITS_FOR_X_OVER_2,
		SHIFT_X_PARITY       = BITS_FOR_LAYER + SHIFT_LAYER,
		SHIFT_Y              = BITS_FOR_X_PARITY + SHIFT_X_PARITY,
		SHIFT_LAYER_GROUP    = BITS_FOR_Y + SHIFT_Y
	};
	static_assert(SHIFT_LAYER_GROUP + BITS_FOR_LAYER_GROUP == sizeof(key_) * 8, "Bit field too small");

	// the parity of x must be more significant than the layer but less significant than y.
	// Thus basically every row is split in two: First the row containing all the odd x
	// then the row containing all the even x. Since thus the least significant bit of x is
	// not required for x ordering anymore it can be shifted out to the right.
	const unsigned int x_parity = static_cast<unsigned int>(loc.x) & 1;
	key_  = (group_i << SHIFT_LAYER_GROUP) | (static_cast<unsigned int>(loc.y + MAX_BORDER) << SHIFT_Y);
	key_ |= (x_parity << SHIFT_X_PARITY);
	key_ |= (static_cast<unsigned int>(layer) << SHIFT_LAYER) | static_cast<unsigned int>(loc.x + MAX_BORDER) / 2;
}

void display::drawing_buffer_commit()
{
	// std::list::sort() is a stable sort
	drawing_buffer_.sort();

	auto clipper = draw::reduce_clip(map_area());

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
	 * layergroup > location > layer > 'blit_helper' > surface
	 */

	for(const blit_helper& blit : drawing_buffer_) {
		for(texture tex : blit.tex()) {
			const SDL_Rect& src = blit.clip();
			if (blit.alpha_mod != SDL_ALPHA_OPAQUE) {
				tex.set_alpha_mod(blit.alpha_mod);
			}
			if (blit.r_mod != 255 || blit.g_mod != 255 || blit.b_mod != 255) {
				tex.set_color_mod(blit.r_mod, blit.g_mod, blit.b_mod);
			}
			// TODO: hwaccel - revist whether treating empty rect as full src clip is okay
			draw::flipped(tex, blit.dest(), src, blit.hflip, blit.vflip);
			if (blit.highlight) {
				tex.set_blend_mode(SDL_BLENDMODE_ADD);
				tex.set_alpha_mod(blit.highlight);
				draw::flipped(tex, blit.dest(), src, blit.hflip, blit.vflip);
				tex.set_blend_mode(SDL_BLENDMODE_BLEND);
			}
			if (blit.r_mod != 255 || blit.g_mod != 255 || blit.b_mod != 255) {
				tex.set_color_mod(255, 255, 255);
			}
			if (blit.alpha_mod != SDL_ALPHA_OPAQUE || blit.highlight) {
				tex.set_alpha_mod(SDL_ALPHA_OPAQUE);
			}
		}
	}
	drawing_buffer_clear();
}

void display::drawing_buffer_clear()
{
	drawing_buffer_.clear();
}

void display::toggle_benchmark()
{
	benchmark = !benchmark;
}

void display::toggle_debug_foreground()
{
	debug_foreground = !debug_foreground;
}

// frametime is in milliseconds
static unsigned calculate_fps(unsigned frametime)
{
	return frametime != 0u ? 1000u / frametime : 999u;
}

void display::update_fps_label()
{
	static int frames = 0;
	++frames;
	const int sample_freq = 10;

	if(frames != sample_freq) {
		return;
	}

	const auto minmax_it = std::minmax_element(frametimes_.begin(), frametimes_.end());
	const unsigned render_avg = std::accumulate(frametimes_.begin(), frametimes_.end(), 0) / frametimes_.size();
	const int avg_fps = calculate_fps(render_avg);
	const int max_fps = calculate_fps(*minmax_it.first);
	const int min_fps = calculate_fps(*minmax_it.second);
	fps_history_.emplace_back(min_fps, avg_fps, max_fps);
	frames = 0;

	// flush out the stored fps values every so often
	if(fps_history_.size() == 1000) {
		std::string filename = filesystem::get_user_data_dir()+"/fps_log.csv";
		filesystem::scoped_ostream fps_log = filesystem::ostream_file(filename, std::ios_base::binary | std::ios_base::app);
		for(const auto& fps : fps_history_) {
			*fps_log << std::get<0>(fps) << "," << std::get<1>(fps) << "," << std::get<2>(fps) << "\n";
		}
		fps_history_.clear();
	}

	if(fps_handle_ != 0) {
		font::remove_floating_label(fps_handle_);
		fps_handle_ = 0;
	}
	std::ostringstream stream;
	stream << "<tt>      min/avg/max/act</tt>\n";
	stream << "<tt>FPS:  " << std::setfill(' ') << std::setw(3) << min_fps << '/'<< std::setw(3) << avg_fps << '/' << std::setw(3) << max_fps << '/' << std::setw(3) << fps_actual_ << "</tt>\n";
	stream << "<tt>Time: " << std::setfill(' ') << std::setw(3) << *minmax_it.first << '/' << std::setw(3) << render_avg << '/' << std::setw(3) << *minmax_it.second << " ms</tt>\n";
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

void display::clear_fps_label()
{
	if(fps_handle_ != 0) {
		font::remove_floating_label(fps_handle_);
		fps_handle_ = 0;
		drawn_hexes_ = 0;
		invalidated_hexes_ = 0;
	}
}

void display::draw_panel(const theme::panel& panel)
{
	// Most panels are transparent.
	if (panel.image().empty()) {
		return;
	}

	// TODO: highdpi - draw area should probably be moved to new drawing API
	const rect& loc = panel.location(video::game_canvas());

	if (!loc.overlaps(draw::get_clip())) {
		return;
	}

	DBG_DP << "drawing panel " << panel.get_id() << ' ' << loc;

	texture tex(image::get_texture(panel.image()));
	if (!tex) {
		ERR_DP << "failed to load panel " << panel.get_id()
			<< " texture: " << panel.image();
		return;
	}

	draw::tiled(tex, loc);
}

void display::draw_label(const theme::label& label)
{
	const rect& loc = label.location(video::game_canvas());

	if (!loc.overlaps(draw::get_clip())) {
		return;
	}

	const std::string& text = label.text();
	const color_t text_color = label.font_rgb_set() ? label.font_rgb() : font::NORMAL_COLOR;
	const std::string& icon = label.icon();

	DBG_DP << "drawing label " << label.get_id() << ' ' << loc;

	if(icon.empty() == false) {
		draw::blit(image::get_texture(icon), loc);

		if(text.empty() == false) {
			tooltips::add_tooltip(loc,text);
		}
	} else if(text.empty() == false) {
		font::pango_draw_text(true, loc, label.font_size(),
			text_color, text, loc.x, loc.y
		);
	}
}

void display::draw_all_panels()
{
	for(const auto& panel : theme_.panels()) {
		draw_panel(panel);
	}

	for(const auto& label : theme_.labels()) {
		draw_label(label);
	}
}

void display::draw_text_in_hex(const map_location& loc,
		const drawing_layer layer,
		const std::string& text,
		std::size_t font_size,
		color_t color,
		double x_in_hex,
		double y_in_hex)
{
	if (text.empty()) return;

	const double zf = get_zoom_factor();
	const int font_sz = int(font_size * zf);

	// TODO: highdpi - use the same processing as floating_label::create_texture() and cache the effect result so it doesn't constantly rerender the same thing.
	texture text_surf = font::pango_render_text(text, font_sz, color);
	const int x = get_location_x(loc) - text_surf.w()/2
	              + static_cast<int>(x_in_hex* hex_size());
	const int y = get_location_y(loc) - text_surf.h()/2
	              + static_cast<int>(y_in_hex* hex_size());
	const int w = text_surf.w();
	const int h = text_surf.h();
	for (int dy=-1; dy <= 1; ++dy) {
		for (int dx=-1; dx <= 1; ++dx) {
			if (dx!=0 || dy!=0) {
				const SDL_Rect dest{int(x + dx*zf), int(y + dy*zf), w, h};
				drawing_buffer_add(layer, loc, dest, text_surf)
					.set_color_and_alpha({0, 0, 0, 128});
			}
		}
	}
	drawing_buffer_add(layer, loc, {x, y, w, h}, text_surf);
}

void display::render_image(int x, int y, const display::drawing_layer drawing_layer,
		const map_location& loc, const image::locator& i_locator,
		bool hreverse, bool greyscale, uint8_t alpha, double highlight,
		color_t blendto, double blend_ratio, double submerged, bool vreverse)
{
	if (alpha == 0) {
		return;
	}

	const point image_size = image::get_size(i_locator);
	if (!image_size.x || !image_size.y) {
		return;
	}

	rect dest = scaled_to_zoom({x, y, image_size.x, image_size.y});
	if (!dest.overlaps(map_area())) {
		return;
	}

	// For now, we add to the existing IPF modifications for the image.
	std::string new_modifications;

	if (greyscale) {
		new_modifications += "~GS()";
	}

	// general formula for submerged alpha:
	//   if (y > WATERLINE)
	//   then min(max(alpha_mod, 0), 1) * alpha
	//   else alpha
	//   where alpha_mod = alpha_base - (y - WATERLINE) * alpha_delta
	// formula variables: x, y, red, green, blue, alpha, width, height
	// full WFL string:
	// "~ADJUST_ALPHA(if(y>DL,clamp((AB-(y-DL)*AD),0,1)*alpha,alpha))"
	// where DL = submersion line in pixels from top of image
	//       AB = base alpha proportion at submersion line (30%)
	//       AD = proportional alpha delta per pixel (1.5%)
	if (submerged > 0.0) {
		const int submersion_line = image_size.y * (1.0 - submerged);
		const std::string sl_string = std::to_string(submersion_line);
		new_modifications += "~ADJUST_ALPHA(if(y>";
		new_modifications += sl_string;
		new_modifications += ",clamp((0.3-(y-";
		new_modifications += sl_string;
		new_modifications += ")*0.015),0,1)*alpha,alpha))";
	}
	// FUTURE: this submerge function could be done using SDL_RenderGeometry,
	// but that's not available below SDL 2.0.18.
	// Alternately it could also be done fairly easily using shaders,
	// if a graphics system supporting them (such as openGL) is moved to.

	texture tex;
	if (!new_modifications.empty()) {
		const image::locator modified_locator(
			i_locator.get_filename(),
			i_locator.get_modifications() + new_modifications
		);
		tex = image::get_texture(modified_locator);
	} else {
		tex = image::get_texture(i_locator);
	}

	// Clamp blend ratio so nothing weird happens
	blend_ratio = std::clamp(blend_ratio, 0.0, 1.0);

	blit_helper& bh = drawing_buffer_add(drawing_layer, loc, dest, tex);
	bh.hflip = hreverse;
	bh.vflip = vreverse;
	bh.alpha_mod = alpha;
	bh.highlight = float_to_color(highlight);

	// SDL hax to apply an active washout tint at the correct ratio
	if (blend_ratio > 0.0) {
		// Get a pure-white version of the texture
		const image::locator whiteout_locator(
			i_locator.get_filename(),
			i_locator.get_modifications()
				+ new_modifications
				+ "~CHAN(255, 255, 255, alpha)"
		);
		const texture& wt = image::get_texture(whiteout_locator);
		// Blit the pure white version, tinted to the right colour
		blit_helper &wh = drawing_buffer_add(drawing_layer, loc, dest, wt);
		wh.hflip = hreverse;
		wh.vflip = vreverse;
		wh.alpha_mod = uint8_t(alpha * blend_ratio);
		wh.r_mod = blendto.r;
		wh.g_mod = blendto.g;
		wh.b_mod = blendto.b;
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
	if(mouseoverHex_ == hex) {
		return;
	}
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

	if(!msg.empty()) {
		font::floating_label flabel(msg);
		flabel.set_font_size(font::SIZE_PLUS);
		flabel.set_color(font::YELLOW_COLOR);
		flabel.set_position(300, 50);
		flabel.set_clip_rect(map_outside_area());

		diagnostic_label_ = font::add_floating_label(flabel);
	}
}

void display::update_fps_count()
{
	static int time_between_draws = preferences::draw_delay();
	if(time_between_draws < 0) {
		time_between_draws = 1000 / video::current_refresh_rate();
	}

	frametimes_.push_back(SDL_GetTicks() - last_frame_finished_);
	fps_counter_++;
	using std::chrono::duration_cast;
	using std::chrono::seconds;
	using std::chrono::steady_clock;
	const seconds current_second = duration_cast<seconds>(steady_clock::now().time_since_epoch());
	if(current_second != fps_start_) {
		fps_start_ = current_second;
		fps_actual_ = fps_counter_;
		fps_counter_ = 0;
	}

	last_frame_finished_ = SDL_GetTicks();
}

const theme::action* display::action_pressed()
{
	for(auto i = action_buttons_.begin(); i != action_buttons_.end(); ++i) {
		if((*i)->pressed()) {
			const std::size_t index = std::distance(action_buttons_.begin(), i);
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
			const std::size_t index = std::distance(menu_buttons_.begin(), i);
			if(index >= theme_.menus().size()) {
				assert(false);
				return nullptr;
			}
			return theme_.get_menu_item((*i)->id());
		}
	}

	return nullptr;
}

void display::announce(const std::string& message, const color_t& color, const announce_options& options)
{
	if(options.discard_previous) {
		font::remove_floating_label(prevLabel);
	}
	font::floating_label flabel(message);
	flabel.set_font_size(font::SIZE_FLOAT_LABEL);
	flabel.set_color(color);
	flabel.set_position(
		map_outside_area().x + map_outside_area().w / 2, map_outside_area().y + map_outside_area().h / 3);
	flabel.set_lifetime(options.lifetime);
	flabel.set_clip_rect(map_outside_area());

	prevLabel = font::add_floating_label(flabel);
}

void display::recalculate_minimap()
{
	if(video::headless()) {
		return;
	}

	const rect& area = minimap_area();
	if(area.empty()){
		return;
	}

	minimap_ = texture(image::getMinimap(
		area.w, area.h, get_map(),
		dc_->teams().empty() ? nullptr : &dc_->teams()[currentTeam_],
		(selectedHex_.valid() && !is_blindfolded()) ? &reach_map_ : nullptr
	));

	redraw_minimap();
}

void display::redraw_minimap()
{
	draw_manager::invalidate_region(minimap_area());
}

void display::draw_minimap()
{
	const rect& area = minimap_area();

	if(area.empty() || !area.overlaps(draw::get_clip())) {
		return;
	}

	if(!minimap_) {
		ERR_DP << "trying to draw null minimap";
		return;
	}

	auto clipper = draw::reduce_clip(area);

	// Draw the minimap background.
	draw::fill(area, 31, 31, 23);

	// Update the minimap location for mouse and units functions
	minimap_location_.x = area.x + (area.w - minimap_.w()) / 2;
	minimap_location_.y = area.y + (area.h - minimap_.h()) / 2;
	minimap_location_.w = minimap_.w();
	minimap_location_.h = minimap_.h();

	// Draw the minimap.
	if (minimap_) {
		draw::blit(minimap_, minimap_location_);
	}

	draw_minimap_units();

	// calculate the visible portion of the map:
	// scaling between minimap and full map images
	double xscaling = 1.0*minimap_.w() / (get_map().w()*hex_width());
	double yscaling = 1.0*minimap_.h() / (get_map().h()*hex_size());

	// we need to shift with the border size
	// and the 0.25 from the minimap balanced drawing
	// and the possible difference between real map and outside off-map
	SDL_Rect map_rect = map_area();
	SDL_Rect map_out_rect = map_outside_area();
	double border = theme_.border().size;
	double shift_x = -border * hex_width() - (map_out_rect.w - map_rect.w) / 2;
	double shift_y = -(border + 0.25) * hex_size() - (map_out_rect.h - map_rect.h) / 2;

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

	draw::rect(outline_rect, 255, 255, 255);
}

void display::draw_minimap_units()
{
	if (!preferences::minimap_draw_units() || is_blindfolded()) return;

	double xscaling = 1.0 * minimap_location_.w / get_map().w();
	double yscaling = 1.0 * minimap_location_.h / get_map().h();

	for(const auto& u : dc_->units()) {
		if (fogged(u.get_location()) ||
		    (dc_->teams()[currentTeam_].is_enemy(u.side()) &&
		     u.invisible(u.get_location())) ||
			 u.get_hidden()) {
			continue;
		}

		int side = u.side();
		color_t col = team::get_minimap_color(side);

		if(!preferences::minimap_movement_coding()) {
			auto status = orb_status::allied;
			if(dc_->teams()[currentTeam_].is_enemy(side)) {
				status = orb_status::enemy;
			} else if(currentTeam_ + 1 == static_cast<unsigned>(side)) {
				status = dc_->unit_orb_status(u);
			} else {
				// no-op, status is already set to orb_status::allied;
			}
			col = game_config::color_info(orb_status_helper::get_orb_color(status)).rep();
		}

		double u_x = u.get_location().x * xscaling;
		double u_y = (u.get_location().y + (is_odd(u.get_location().x) ? 1 : -1)/4.0) * yscaling;
		// use 4/3 to compensate the horizontal hexes imbrication
		double u_w = 4.0 / 3.0 * xscaling;
		double u_h = yscaling;

		SDL_Rect r {
				  minimap_location_.x + int(std::round(u_x))
				, minimap_location_.y + int(std::round(u_y))
				, int(std::round(u_w))
				, int(std::round(u_h))
		};

		draw::fill(r, col.r, col.g, col.b, col.a);
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

	//
	// NOTE: the next three blocks can be removed once we switch to accelerated rendering.
	//

	if(!video::headless()) {
		rect dst = map_area();
		dst.x += diff_x;
		dst.y += diff_y;
		dst.clip(map_area());

		rect src = dst;
		src.x -= diff_x;
		src.y -= diff_y;

		src = video::to_output(src);

		// swap buffers
		std::swap(front_, back_);

		// copy from the back to the front buffer
		auto rts = draw::set_render_target(front_);
		draw::blit(back_, dst, src);

		// queue repaint
		draw_manager::invalidate_region(map_area());
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

	redraw_minimap();

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
	zoom_index_ = std::clamp(increase ? zoom_index_ + 1 : zoom_index_ - 1, 0, final_zoom_index);

	// No validation check is needed in the next step since we've already set the index here and
	// know the new zoom value is indeed valid.
	return set_zoom(zoom_levels[zoom_index_], false);
}

bool display::set_zoom(unsigned int amount, const bool validate_value_and_set_index)
{
	unsigned int new_zoom = std::clamp(amount, MinZoom, MaxZoom);

	LOG_DP << "new_zoom = " << new_zoom;

	if(new_zoom == zoom_) {
		return false;
	}

	if(validate_value_and_set_index) {
		zoom_index_ = get_zoom_levels_index (new_zoom);
		new_zoom = zoom_levels[zoom_index_];
	}

	const SDL_Rect& outside_area = map_outside_area();
	const SDL_Rect& area = map_area();

	// Turn the zoom factor to a double in order to avoid rounding errors.
	double zoom_factor = static_cast<double>(new_zoom) / static_cast<double>(zoom_);

	// INVARIANT: xpos_ + area.w == xend where xend is as in bounds_check_position()
	//
	// xpos_: Position of the leftmost visible map pixel of the viewport, in pixels.
	// Affected by the current zoom: this->zoom_ pixels to the hex.
	//
	// xpos_ + area.w/2: Position of the center of the viewport, in pixels.
	//
	// (xpos_ + area.w/2) * new_zoom/zoom_: Position of the center of the
	// viewport, as it would be under new_zoom.
	//
	// (xpos_ + area.w/2) * new_zoom/zoom_ - area.w/2: Position of the
	// leftmost visible map pixel, as it would be under new_zoom.
	xpos_ = std::round(((xpos_ + area.w / 2) * zoom_factor) - (area.w / 2));
	ypos_ = std::round(((ypos_ + area.h / 2) * zoom_factor) - (area.h / 2));
	xpos_ -= (outside_area.w - area.w) / 2;
	ypos_ -= (outside_area.h - area.h) / 2;

	zoom_ = new_zoom;
	bounds_check_position(xpos_, ypos_);
	if(zoom_ != DefaultZoom) {
		last_zoom_ = zoom_;
	}

	preferences::set_tile_size(zoom_);
	image::set_zoom(zoom_);

	labels().recalculate_labels();
	redraw_background_ = true;
	invalidate_all();

	return true;
}

void display::toggle_default_zoom()
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
	if(video::headless()) {
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
		redraw_minimap();
		events::raise_draw_event();
		return;
	}

	// Doing an animated scroll, with acceleration etc.

	int x_old = 0;
	int y_old = 0;

	const double dist_total = std::hypot(xmove, ymove);
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

		int x_new = std::round(xmove * dist_moved / dist_total);
		int y_new = std::round(ymove * dist_moved / dist_total);

		int dx = x_new - x_old;
		int dy = y_new - y_old;

		scroll(dx,dy,true);
		x_old += dx;
		y_old += dy;

		redraw_minimap();
		events::raise_draw_event();
	}
}

void display::scroll_to_tile(const map_location& loc, SCROLL_TYPE scroll_type, bool check_fogged, bool force)
{
	if(get_map().on_board(loc) == false) {
		ERR_DP << "Tile at " << loc << " isn't on the map, can't scroll to the tile.";
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
		int spacing = std::round(add_spacing * hex_size());
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
			ERR_DP << "Bug in the scrolling code? Looks like we would not need to scroll after all...";
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
	bool res = preferences::turbo();
	if(keys_[SDLK_LSHIFT] || keys_[SDLK_RSHIFT]) {
		res = !res;
	}

	res |= video::headless();
	if(res)
		return preferences::turbo_speed();
	else
		return 1.0;
}

void display::fade_tod_mask(
	const std::string& old_mask,
	const std::string& new_mask)
{
	// TODO: hwaccel - this needs testing as it's not used in mainline
	tod_hex_mask1 = image::get_texture(old_mask, image::HEXED);
	tod_hex_mask2 = image::get_texture(new_mask, image::HEXED);

	int duration = 300 / turbo_speed();
	int start = SDL_GetTicks();
	for(int now = start; now < start + duration; now = SDL_GetTicks()) {
		float prop_f = float(now - start) / float(duration);
		uint8_t p = float_to_color(prop_f);
		tod_hex_alpha2 = p;
		tod_hex_alpha1 = ~p;
		draw_manager::invalidate_region(map_outside_area());
		events::pump();
		events::raise_draw_event();
	}

	tod_hex_mask1.reset();
	tod_hex_mask2.reset();
}

void display::fade_to(const color_t& c, int duration)
{
	uint32_t start = SDL_GetTicks();
	color_t fade_start = fade_color_;

	// If we started transparent, assume the same colour
	if(fade_start.a == 0) {
		fade_start.r = c.r;
		fade_start.g = c.g;
		fade_start.b = c.b;
	}

	// Smoothly blend and display
	for(uint32_t now = start; now < start + duration; now = SDL_GetTicks()) {
		float prop_f = float(now - start) / float(duration);
		uint8_t p = float_to_color(prop_f);
		fade_color_ = fade_start.smooth_blend(c, p);
		draw_manager::invalidate_region(map_outside_area());
		events::pump();
		events::raise_draw_event();
	}
	fade_color_ = c;
}

void display::set_fade(const color_t& c)
{
	fade_color_ = c;
}

void display::queue_rerender()
{
	if(video::headless())
		return;

	DBG_DP << "redrawing everything";

	// TODO: draw_manager - ugh
	invalidateGameStatus_ = true;

	reportLocations_.clear();
	reportSurfaces_.clear();
	reports_.clear();

	bounds_check_position();

	tooltips::clear_tooltips();

	theme_.set_resolution(video::game_canvas());

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

	redraw_background_ = true;

	// TODO: draw_manager - kill this
	for(std::function<void(display&)> f : redraw_observers_) {
		f(*this);
	}

	invalidate_all();

	draw_manager::invalidate_all();
}

void display::queue_repaint()
{
	// Could redraw a smaller region if the display doesn't use it all,
	// but when does that ever happen?
	draw_manager::invalidate_all();
}

void display::add_redraw_observer(std::function<void(display&)> f)
{
	redraw_observers_.push_back(f);
}

void display::clear_redraw_observers()
{
	redraw_observers_.clear();
}

void display::draw()
{
	//	log_scope("display::draw");

	if(video::headless()) {
		DBG_DP << "display::draw denied";
		// TODO: draw_manager - deny drawing in draw_manager if appropriate
		return;
	}
	//DBG_DP << "display::draw";

	// TODO: draw_manager - why on earth does this need to mess with sync context?
	set_scontext_unsynced leave_synced_context;

	// TODO: draw_manager - redraw background more judiciously
	if(redraw_background_) {
		DBG_DP << "display::draw redraw background";
		render_map_outside_area();
		draw_manager::invalidate_region(map_outside_area());
		redraw_background_ = false;
	}

	if(!get_map().empty()) {
		if(!invalidated_.empty()) {
			draw_invalidated();
			invalidated_.clear();
		}
		drawing_buffer_commit();
		post_commit();
	}

	if(preferences::show_fps() || benchmark) {
		update_fps_label();
		update_fps_count();
	} else if(fps_handle_ != 0) {
		clear_fps_label();
	}
}

void display::update()
{
	//DBG_DP << "display::update";
	// Ensure render textures are correctly sized and up-to-date.
	update_render_textures();

	// Trigger cache rebuild if animated water preference has changed.
	if(animate_water_ != preferences::animate_water()) {
		animate_water_ = preferences::animate_water();
		builder_->rebuild_cache_all();
	}

	if(benchmark) {
		invalidate_all();
	}

	// TODO: draw_manager - can probably just override update() on subclasses
	pre_draw();
}

void display::layout()
{
	//DBG_DP << "display::layout";
	// TODO: draw_manager - theme layout for changed screen size?

	// Post-layout / Pre-render

	if (!get_map().empty()) {
		if(redraw_background_) {
			invalidateAll_ = true;
		}
		if(invalidateAll_) {
			DBG_DP << "draw() with invalidateAll";

			// toggle invalidateAll_ first to allow regular invalidations
			invalidateAll_ = false;
			invalidate_locations_in_rect(map_area());

			redraw_minimap();
		}
	}

	// invalidate animated terrain, units and haloes
	invalidate_animations();

	// TODO: draw_manager - split map labels from UI labels
	// Update and invalidate floating labels as necessary
	font::update_floating_labels();

	// TODO: draw_manager - should this just be moved into subclass layout() overrides?
	// update and invalidate reports
	refresh_reports();
}

void display::render()
{
	// This should render the game map and units.
	// It is not responsible for halos and floating labels.
	//DBG_DP << "display::render";

	// No need to render if we aren't going to draw anything.
	if(prevent_draw_) {
		DBG_DP << "render prevented";
		return;
	}

	// render to the offscreen buffer
	auto target_setter = draw::set_render_target(front_);
	draw();

	// update the minimap texture, if necessary
	// TODO: highdpi - high DPI minimap
	const rect& area = minimap_area();
	if(!area.empty() &&
		(!minimap_ || minimap_.w() > area.w || minimap_.h() > area.h))
	{
		recalculate_minimap();
		if(!minimap_) {
			ERR_DP << "error creating minimap";
			return;
		}
	}

	// TODO: draw_manager - subclass override in stead of this maybe
	post_draw();
}

bool display::expose(const SDL_Rect& region)
{
	// Note: clipping region is set by draw_manager,
	// and will be contained by <region>.

	if(prevent_draw_) {
		DBG_DP << "draw prevented";
		return false;
	}

	// Blit from the pre-rendered front buffer.
	// TODO: draw_manager - API to get src region in output space
	rect src_region = region;
	src_region *= video::get_pixel_scale();
	//DBG_DP << "  src region " << src_region;
	draw::blit(front_, region, src_region);

	// Render halos.
	// TODO: draw_manager - halo render region not rely on clip?
	halo_man_.render();

	// These all check for clip region overlap before drawing
	draw_all_panels();
	draw_reports();
	draw_minimap();
	draw_buttons();

	// TODO: draw_manager - hmm... buttons redraw over tooltips, because they are TLDs
	font::draw_floating_labels();

	// If there's a fade, apply it over everything
	if(fade_color_.a) {
		draw::fill(map_outside_area().intersect(region), fade_color_);
	}

	DBG_DP << "display::expose " << region;

	return true; // TODO: draw_manager - maybe don't flip yeah?
}

rect display::screen_location()
{
	assert(!map_screenshot_);
	//return map_outside_area();
	// well actually it also has to draw the panels, so
	return video::game_canvas();
	// TODO: draw_manager - get this from theme perhaps?
}

void display::update_render_textures()
{
	if(video::headless()) {
		return;
	}

	// TODO: highdpi - is this correct when there is a logical offset?
	rect darea = video::game_canvas();
	rect oarea = darea * video::get_pixel_scale();

	// Check that the front buffer size is correct.
	// Buffers are always resized together, so we only need to check one.
	point size = front_.get_raw_size();
	point dsize = front_.draw_size();
	bool raw_size_changed = size.x != oarea.w || size.y != oarea.h;
	bool draw_size_changed = dsize.x != darea.w || dsize.y != darea.h;
	if (!raw_size_changed && !draw_size_changed) {
		// buffers are fine
		return;
	}

	// TODO: draw_manager - these need to account for the logical offset
	if(raw_size_changed) {
		LOG_DP << "regenerating render buffers as " << oarea;
		front_ = texture(oarea.w, oarea.h, SDL_TEXTUREACCESS_TARGET);
		back_ = texture(oarea.w, oarea.h, SDL_TEXTUREACCESS_TARGET);
	}
	if(raw_size_changed || draw_size_changed) {
		LOG_DP << "updating render buffer draw size to " << darea;
		front_.set_draw_size(darea.w, darea.h);
		back_.set_draw_size(darea.w, darea.h);
	}

	// Fill entire texture with black, just in case
	for(int i = 0; i < 2; ++i) {
		auto setter = draw::set_render_target(i ? back_ : front_);
		draw::fill(0,0,0);
	}

	// Fill in the background area on both textures.
	render_map_outside_area();

	queue_rerender();
}

void display::render_map_outside_area()
{
	// This could be optimized to avoid the map area,
	// but it's only called on game creation or zoom anyway.
	const SDL_Rect clip_rect = map_outside_area();
	texture bgtex = image::get_texture(theme_.border().background_image);
	for(int i = 0; i < 2; ++i) {
		auto setter = draw::set_render_target(i ? back_ : front_);
		if(bgtex) {
			draw::tiled(bgtex, clip_rect);
		} else {
			draw::fill(clip_rect, 0, 0, 0);
		}
	}
}

map_labels& display::labels()
{
	return *map_labels_;
}

const map_labels& display::labels() const
{
	return *map_labels_;
}

rect display::get_clip_rect() const
{
	return map_area();
}

void display::draw_invalidated() {
//	log_scope("display::draw_invalidated");
	SDL_Rect clip_rect = get_clip_rect();
	auto clipper = draw::reduce_clip(clip_rect);
	DBG_DP << "drawing " << invalidated_.size() << " invalidated hexes"
		<< " with clip " << clip_rect;
	for (const map_location& loc : invalidated_) {
		int xpos = get_location_x(loc);
		int ypos = get_location_y(loc);

		//const bool on_map = get_map().on_board(loc);
		rect hex_rect(xpos, ypos, zoom_, zoom_);
		if(!hex_rect.overlaps(clip_rect)) {
			continue;
		}
		draw_hex(loc);
		drawn_hexes_+=1;
		draw_manager::invalidate_region(hex_rect.intersect(clip_rect));
	}
	invalidated_hexes_ += invalidated_.size();

	// The unit drawer can't function without teams
	if(dc_->teams().empty()) {
		return;
	}

	unit_drawer drawer = unit_drawer(*this);

	for(const map_location& loc : invalidated_) {
		unit_map::const_iterator u_it = dc_->units().find(loc);
		exclusive_unit_draw_requests_t::iterator request = exclusive_unit_draw_requests_.find(loc);
		if(u_it != dc_->units().end()
			&& (request == exclusive_unit_draw_requests_.end() || request->second == u_it->id())) {
			drawer.redraw_unit(*u_it);
		}
	}
}

void display::draw_hex(const map_location& loc)
{
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	const bool on_map = get_map().on_board(loc);
	const time_of_day& tod = get_time_of_day(loc);
	const int zoom = int(zoom_);
	SDL_Rect dest{xpos, ypos, zoom, zoom};

	int num_images_fg = 0;
	int num_images_bg = 0;

	if(!shrouded(loc)) {
		// unshrouded terrain (the normal case)
		get_terrain_images(loc, tod.id, BACKGROUND); // updates terrain_image_vector_
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, dest, terrain_image_vector_);
		num_images_bg = terrain_image_vector_.size();

		get_terrain_images(loc, tod.id, FOREGROUND); // updates terrain_image_vector_
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, dest, terrain_image_vector_);
		num_images_fg = terrain_image_vector_.size();

		// Draw the grid, if that's been enabled
		if(preferences::grid()) {
			static const image::locator grid_top(game_config::images::grid_top);
			drawing_buffer_add(LAYER_GRID_TOP, loc, dest,
				image::get_texture(grid_top, image::TOD_COLORED));
			static const image::locator grid_bottom(game_config::images::grid_bottom);
			drawing_buffer_add(LAYER_GRID_BOTTOM, loc, dest,
				image::get_texture(grid_bottom, image::TOD_COLORED));
		}
	}

	if(!shrouded(loc)) {
		auto it = get_overlays().find(loc);
		if(it != get_overlays().end()) {
			std::vector<overlay>& overlays = it->second;
			if(overlays.size() != 0) {
				tod_color tod_col = tod.color + color_adjust_;
				image::light_string lt = image::get_light_string(-1, tod_col.r, tod_col.g, tod_col.b);

				for(const overlay& ov : overlays) {
					bool item_visible_for_team = true;
					if(dont_show_all_ && !ov.team_name.empty()) {
						//dont_show_all_ imples that viewing_team()is a valid index to get_teams()
						const std::string& current_team_name = get_teams()[viewing_team()].team_name();
						const std::vector<std::string>& current_team_names = utils::split(current_team_name);
						const std::vector<std::string>& team_names = utils::split(ov.team_name);

						item_visible_for_team = std::find_first_of(team_names.begin(), team_names.end(),
							current_team_names.begin(), current_team_names.end()) != team_names.end();
					}
					if(item_visible_for_team && !(fogged(loc) && !ov.visible_in_fog))
					{
						const texture tex = ov.image.find("~NO_TOD_SHIFT()") == std::string::npos ?
							image::get_lighted_texture(ov.image, lt) : image::get_texture(ov.image, image::HEXED);
						drawing_buffer_add(LAYER_TERRAIN_BG, loc, dest, tex);
					}
				}
			}
		}
	}

	if(!shrouded(loc)) {
		// village-control flags.
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, dest, get_flag(loc));
	}

	// Draw the time-of-day mask on top of the terrain in the hex.
	// tod may differ from tod if hex is illuminated.
	const std::string& tod_hex_mask = tod.image_mask;
	if(tod_hex_mask1 || tod_hex_mask2) {
		auto& a = drawing_buffer_add(LAYER_TERRAIN_FG, loc, dest, tod_hex_mask1);
		a.alpha_mod = tod_hex_alpha1;
		auto& b = drawing_buffer_add(LAYER_TERRAIN_FG, loc, dest, tod_hex_mask2);
		b.alpha_mod = tod_hex_alpha2;
	} else if(!tod_hex_mask.empty()) {
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, dest,
			image::get_texture(tod_hex_mask,image::HEXED));
	}

	// Paint mouseover overlays
	if(loc == mouseoverHex_
		&& (on_map || (in_editor() && get_map().on_board_with_border(loc)))
		&& !map_screenshot_
		&& bool(mouseover_hex_overlay_))
	{
		const uint8_t alpha = 196;
		blit_helper& bh = drawing_buffer_add(LAYER_MOUSEOVER_OVERLAY,
			loc, dest, mouseover_hex_overlay_);
		bh.alpha_mod = alpha;
	}

	// Paint arrows
	arrows_map_t::const_iterator arrows_in_hex = arrows_map_.find(loc);
	if(arrows_in_hex != arrows_map_.end()) {
		for (arrow* const a : arrows_in_hex->second) {
			a->draw_hex(loc);
		}
	}

	// Apply shroud, fog and linger overlay

	if(shrouded(loc)) {
		// We apply void also on off-map tiles
		// to shroud the half-hexes too
		const std::string& shroud_image = get_variant(shroud_images_, loc);
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, dest,
			image::get_texture(shroud_image, image::TOD_COLORED));
	} else if(fogged(loc)) {
		const std::string& fog_image = get_variant(fog_images_, loc);
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, dest,
			image::get_texture(fog_image, image::TOD_COLORED));
	}

	if(!shrouded(loc)) {
		drawing_buffer_add(LAYER_FOG_SHROUD, loc, dest, get_fog_shroud_images(loc, image::TOD_COLORED));
	}

	if (on_map) {
		texture bg = image::get_texture("misc/single-pixel.png");
		color_t bg_col = {0, 0, 0, 0xaa};
		if (draw_coordinates_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			texture text = font::pango_render_text(lexical_cast<std::string>(loc), font::SIZE_SMALL, font::NORMAL_COLOR);
			off_x -= text.w() / 2;
			off_y -= text.h() / 2;
			if (draw_terrain_codes_) {
				off_y -= text.h() / 2;
			}
			if (draw_num_of_bitmaps_) {
				off_y -= text.h() / 2;
			}
			rect tdest {off_x, off_y, text.w(), text.h()};
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, bg)
				.set_color_and_alpha(bg_col);
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, text);
		}
		if (draw_terrain_codes_ && (game_config::debug || !shrouded(loc))) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			texture text = font::pango_render_text(lexical_cast<std::string>(get_map().get_terrain(loc)), font::SIZE_SMALL, font::NORMAL_COLOR);
			off_x -= text.w() / 2;
			off_y -= text.h() / 2;
			if (draw_coordinates_ && !draw_num_of_bitmaps_) {
				off_y += text.h() / 2;
			} else if (draw_num_of_bitmaps_ && !draw_coordinates_) {
				off_y -= text.h() / 2;
			}
			rect tdest {off_x, off_y, text.w(), text.h()};
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, bg)
				.set_color_and_alpha(bg_col);
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, text);
		}
		if (draw_num_of_bitmaps_) {
			int off_x = xpos + hex_size()/2;
			int off_y = ypos + hex_size()/2;
			texture text = font::pango_render_text(std::to_string(num_images_bg + num_images_fg), font::SIZE_SMALL, font::NORMAL_COLOR);
			off_x -= text.w() / 2;
			off_y -= text.h() / 2;
			if (draw_coordinates_) {
				off_y += text.h() / 2;
			}
			if (draw_terrain_codes_) {
				off_y += text.h() / 2;
			}
			rect tdest {off_x, off_y, text.w(), text.h()};
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, bg)
				.set_color_and_alpha(bg_col);
			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tdest, text);
		}
	}

	if(debug_foreground) {
		drawing_buffer_add(LAYER_UNIT_DEFAULT, loc, dest,
			image::get_texture("terrain/foreground.png", image::TOD_COLORED));
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
		// TODO: draw_manager omfg why are there unused reports here
		//WRN_DP << "no report '" << report_name << "' in theme";
		return;
	}

	// Now we will need the config. Generate one if needed.

	utils::optional_reference<events::mouse_handler> mhb = std::nullopt;

	if (resources::controller) {
		mhb = resources::controller->get_mouse_handler_base();
	}

	reports::context temp_context = reports::context(*dc_, *this, *resources::tod_manager, wb_.lock(), mhb);

	const config generated_cfg = new_cfg ? config() : reports_object_->generate_report(report_name, temp_context);
	if ( new_cfg == nullptr )
		new_cfg = &generated_cfg;

	rect& loc = reportLocations_[report_name];
	// TODO: draw_manager - rect
	const SDL_Rect& new_loc = item->location(video::game_canvas());
	config &report = reports_[report_name];

	// Report and its location is unchanged since last time. Do nothing.
	if (loc == new_loc && report == *new_cfg) {
		return;
	}

	DBG_DP << "updating report: " << report_name;

	// Mark both old and new locations for redraw.
	draw_manager::invalidate_region(loc);
	draw_manager::invalidate_region(new_loc);

	// Update the config and current location.
	report = *new_cfg;
	loc = new_loc;

	// TODO: draw_manager - verify this is okay here
	tooltips::clear_tooltips(loc);

	if (report.empty()) return;

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

	// Do a fake run of drawing the report, so tooltips can be determined.
	// TODO: this is horrible, refactor reports to actually make sense
	draw_report(report_name, true);
}

void display::draw_report(const std::string& report_name, bool tooltip_test)
{
	const theme::status_item *item = theme_.get_status_item(report_name);
	if (!item) {
		// TODO: draw_manager unused report_clock report_battery WTF
		//WRN_DP << "no report '" << report_name << "' in theme";
		return;
	}

	const rect& loc = reportLocations_[report_name];
	const config& report = reports_[report_name];

	int x = loc.x, y = loc.y;

	// Loop through and display each report element.
	int tallest = 0;
	int image_count = 0;
	bool used_ellipsis = false;
	std::ostringstream ellipsis_tooltip;
	SDL_Rect ellipsis_area = loc;

	for (config::const_child_itors elements = report.child_range("element");
		 elements.begin() != elements.end(); elements.pop_front())
	{
		SDL_Rect area {x, y, loc.w + loc.x - x, loc.h + loc.y - y};
		if (area.h <= 0) break;

		std::string t = elements.front()["text"];
		if (!t.empty())
		{
			if (used_ellipsis) goto skip_element;

			// TODO: draw_manager - don't render if faking
			// Draw a text element.
			font::pango_text& text = font::get_text_renderer();
			bool eol = false;
			if (t[t.size() - 1] == '\n') {
				eol = true;
				t = t.substr(0, t.size() - 1);
			}
			text.set_link_aware(false)
				.set_text(t, true);
			text.set_family_class(font::FONT_SANS_SERIF)
				.set_font_size(item->font_size())
				.set_font_style(font::pango_text::STYLE_NORMAL)
				.set_alignment(PANGO_ALIGN_LEFT)
				.set_foreground_color(item->font_rgb_set() ? item->font_rgb() : font::NORMAL_COLOR)
				.set_maximum_width(area.w)
				.set_maximum_height(area.h, false)
				.set_ellipse_mode(PANGO_ELLIPSIZE_END)
				.set_characters_per_line(0);

			texture s = text.render_and_get_texture();

			// check if next element is text with almost no space to show it
			const int minimal_text = 12; // width in pixels
			config::const_child_iterator ee = elements.begin();
			if (!eol && loc.w - (x - loc.x + s.w()) < minimal_text &&
				++ee != elements.end() && !(*ee)["text"].empty())
			{
				// make this element longer to trigger rendering of ellipsis
				// (to indicate that next elements have not enough space)
				//NOTE this space should be longer than minimal_text pixels
				t = t + "    ";
				text.set_text(t, true);
				s = text.render_and_get_texture();
				// use the area of this element for next tooltips
				used_ellipsis = true;
				ellipsis_area.x = x;
				ellipsis_area.y = y;
				ellipsis_area.w = s.w();
				ellipsis_area.h = s.h();
			}

			area.w = s.w();
			area.h = s.h();
			if (!tooltip_test) {
				draw::blit(s, area);
			}
			if (area.h > tallest) {
				tallest = area.h;
			}
			if (eol) {
				x = loc.x;
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
			texture img(image::get_texture(t));

			if (!img) {
				ERR_DP << "could not find image for report: '" << t << "'";
				continue;
			}

			if (area.w < img.w() && image_count) {
				// We have more than one image, and this one doesn't fit.
				img = image::get_texture(game_config::images::ellipsis);
				used_ellipsis = true;
			}

			if (img.w() < area.w) area.w = img.w();
			if (img.h() < area.h) area.h = img.h();
			if (!tooltip_test) {
				draw::blit(img, area);
			}

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
		t = elements.front()["tooltip"].t_str().c_str();
		if (!t.empty()) {
			if (tooltip_test && !used_ellipsis) {
				tooltips::add_tooltip(area, t, elements.front()["help"].t_str().c_str());
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

	if (tooltip_test && used_ellipsis) {
		tooltips::add_tooltip(ellipsis_area, ellipsis_tooltip.str());
	}
}

// TODO: draw_manager - pass in bounds in stead of using clip region
// TODO: draw_manager - return whether anything was drawn
void display::draw_reports()
{
	for(const auto& it : reports_) {
		const std::string& name = it.first;
		const rect& loc = reportLocations_[name];
		if(loc.overlaps(draw::get_clip())) {
			draw_report(name);
		}
	}
}

void display::invalidate_all()
{
	DBG_DP << "invalidate_all()";
	invalidateAll_ = true;
	invalidated_.clear();
}

bool display::invalidate(const map_location& loc)
{
	if(invalidateAll_)
		return false;

	bool tmp;
	tmp = invalidated_.insert(loc).second;
	return tmp;
}

bool display::invalidate(const std::set<map_location>& locs)
{
	if(invalidateAll_)
		return false;
	bool ret = false;
	for (const map_location& loc : locs) {
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
	return invalidate_locations_in_rect(map_area().intersect(rect));
}

bool display::invalidate_locations_in_rect(const SDL_Rect& rect)
{
	if(invalidateAll_)
		return false;

	DBG_DP << "invalidating locations in " << rect;

	bool result = false;
	for(const map_location& loc : hexes_under_rect(rect)) {
		//DBG_DP << "invalidating " << loc.x << ',' << loc.y;
		result |= invalidate(loc);
	}
	return result;
}

void display::invalidate_animations_location(const map_location& loc)
{
	if(get_map().is_village(loc)) {
		const int owner = dc_->village_owner(loc) - 1;
		if(owner >= 0 && flags_[owner].need_update()
			&& (!fogged(loc) || !dc_->teams()[currentTeam_].is_enemy(owner + 1))) {
			invalidate(loc);
		}
	}
}

void display::invalidate_animations()
{
	// TODO: draw_manager - WTF... how does this timing even work?
	new_animation_frame();
	animate_map_ = preferences::animate_map();
	if(animate_map_) {
		for(const map_location& loc : get_visible_hexes()) {
			if(shrouded(loc))
				continue;
			if(builder_->update_animation(loc)) {
				invalidate(loc);
			} else {
				invalidate_animations_location(loc);
			}
		}
	}

	for(const unit& u : dc_->units()) {
		u.anim_comp().refresh();
	}
	for(const unit* u : *fake_unit_man_) {
		u->anim_comp().refresh();
	}

	bool new_inval;
	do {
		new_inval = false;
		for(const unit& u : dc_->units()) {
			new_inval |= u.anim_comp().invalidate(*this);
		}
		for(const unit* u : *fake_unit_man_) {
			new_inval |= u->anim_comp().invalidate(*this);
		}
	} while(new_inval);

	halo_man_.update();
}

void display::reset_standing_animations()
{
	for(const unit & u : dc_->units()) {
		u.anim_comp().set_standing();
	}
}

void display::add_arrow(arrow& arrow)
{
	for(const map_location& loc : arrow.get_path()) {
		arrows_map_[loc].push_back(&arrow);
	}
}

void display::remove_arrow(arrow& arrow)
{
	for(const map_location& loc : arrow.get_path()) {
		arrows_map_[loc].remove(&arrow);
	}
}

void display::update_arrow(arrow & arrow)
{
	for(const map_location& loc : arrow.get_previous_path()) {
		arrows_map_[loc].remove(&arrow);
	}

	for(const map_location& loc : arrow.get_path()) {
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

void display::process_reachmap_changes()
{
	if (!reach_map_changed_) return;
	if (reach_map_.empty() != reach_map_old_.empty()) {
		// Invalidate everything except the non-darkened tiles
		reach_map &full = reach_map_.empty() ? reach_map_old_ : reach_map_;

		for (const auto& hex : get_visible_hexes()) {
			reach_map::iterator reach = full.find(hex);
			if (reach == full.end()) {
				// Location needs to be darkened or brightened
				invalidate(hex);
			} else if (reach->second != 1) {
				// Number needs to be displayed or cleared
				invalidate(hex);
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

display *display::singleton_ = nullptr;
