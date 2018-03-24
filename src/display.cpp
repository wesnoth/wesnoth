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

#include "display.hpp"

#include "arrow.hpp"
#include "color.hpp"
#include "cursor.hpp"
#include "fake_unit_manager.hpp"
#include "floating_label.hpp"
#include "font/marked-up_text.hpp"
#include "font/sdl_ttf.hpp"
#include "font/text.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "halo.hpp"
#include "hotkey/command_executor.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "minimap.hpp"
#include "overlay.hpp"
#include "preferences/game.hpp"
#include "resources.hpp"
#include "sdl/render_utils.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "terrain/builder.hpp"
#include "time_of_day.hpp"
#include "tod_manager.hpp"
#include "tooltips.hpp"
#include "units/animation_component.hpp"
#include "units/drawer.hpp"
#include "units/unit.hpp"
#include "whiteboard/manager.hpp"

#include <array>
#include <cmath>
#include <iomanip>
#include <utility>

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

namespace
{
bool benchmark = false;
bool debug_foreground = false;

int prevLabel = 0;

// frametime is in milliseconds
static unsigned calculate_fps(unsigned frametime)
{
	return frametime != 0u ? 1000u / frametime : 999u;
}

} // end anon namespace

unsigned int display::zoom_ = DefaultZoom;
unsigned int display::last_zoom_ = SmallZoom;

display::display(const display_context* dc,
		std::weak_ptr<wb::manager> wb,
		const config& theme_cfg,
		const config& level,
		bool auto_join)
	: events::sdl_handler(auto_join)
	, dc_(dc)
	, halo_man_(new halo::manager())
	, wb_(wb)
	, exclusive_unit_draw_requests_()
	, video_(CVideo::get_singleton())
	, currentTeam_(0)
	, dont_show_all_(false)
	, xpos_(0)
	, ypos_(0)
	, view_locked_(false)
	, theme_(theme_cfg, video().screen_area())
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
	, map_labels_(new map_labels(nullptr))
	, scroll_event_("scrolled")
	, complete_redraw_event_("completely_redrawn")
	, fps_counter_()
	, fps_start_()
	, fps_actual_()
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
	, map_screenshot_(false)
	, reach_map_()
	, overlays_(nullptr)
	, fps_handle_(0)
	, drawn_hexes_(0)
	, idle_anim_(preferences::idle_anim())
	, idle_anim_rate_(1.0)
	, draw_coordinates_(false)
	, draw_terrain_codes_(false)
	, draw_num_of_bitmaps_(false)
	, arrows_map_()
	, color_adjust_()
{
	assert(singleton_ == nullptr);
	singleton_ = this;

	resources::fake_units = fake_unit_man_.get();

	blindfold_ctr_ = 0;

	read(level.child_or_empty("display"));

	if(video_.non_interactive() && video_.faked()) {
		video_.lock_updates(true);
	}

	fill_images_list(game_config::fog_prefix, fog_images_);
	fill_images_list(game_config::shroud_prefix, shroud_images_);

	set_idle_anim_rate(preferences::idle_anim_rate());

	zoom_index_ = std::distance(zoom_levels.begin(), std::find(zoom_levels.begin(), zoom_levels.end(), zoom_));

	image::set_zoom(zoom_);

	init_flags();

	rebuild_all();
	assert(builder_);
	// builder_->rebuild_cache_all();
}

display::~display()
{
	singleton_ = nullptr;
	resources::fake_units = nullptr;
}

void display::set_theme(config theme_cfg)
{
	theme_ = theme(theme_cfg, video_.screen_area());
}

void display::init_flags()
{
	flags_.clear();

	if(!dc_) {
		return;
	}

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

void display::reinit_flags_for_side(std::size_t side)
{
	if(!dc_ || side >= dc_->teams().size()) {
		ERR_DP << "Cannot rebuild flags for nonexistent or unconfigured side " << side << '\n';
		return;
	}

	init_flags_for_side_internal(side, dc_->teams()[side].color());
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
		temp << str << "~RC(" << old_rgb << ">" << new_rgb << ")";
		image::locator flag_image(temp.str());

		temp_anim.add_frame(time, flag_image);
	}

	animated<image::locator>& f = flags_[n];
	f = temp_anim;
	auto time = f.get_end_time();
	if(time > 0) {
		f.start_animation(randomness::rng::default_instance().get_random_int(0, time - 1), true);
	} else {
		// this can happen if both flag and game_config::images::flag are empty.
		ERR_DP << "missing flag for team" << n << "\n";
	}
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
}

void display::add_overlay(const map_location& loc,
		const std::string& img,
		const std::string& halo,
		const std::string& team_name,
		const std::string& item_id,
		bool visible_under_fog)
{
	if(halo_man_) {
		const halo::handle halo_handle = halo_man_->add(
			get_location_x(loc) + hex_size() / 2,
			get_location_y(loc) + hex_size() / 2, halo, loc
		);

		overlays_->emplace(loc, overlay(img, halo, halo_handle, team_name, item_id, visible_under_fog));
	}
}

void display::remove_overlay(const map_location& loc)
{
	overlays_->erase(loc);
}

void display::remove_single_overlay(const map_location& loc, const std::string& toDelete)
{
	// Iterate through the values with key of loc
	auto itors = overlays_->equal_range(loc);

	while(itors.first != itors.second) {
		const overlay& o = itors.first->second;

		if(o.image == toDelete || o.halo == toDelete || o.id == toDelete) {
			overlays_->erase(itors.first++);
		} else {
			++itors.first;
		}
	}
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
	if(loc.valid()) {
		id = exclusive_unit_draw_requests_[loc];
		// id will be set to the default "" string by the [] operator if the map doesn't have anything for that loc.
		exclusive_unit_draw_requests_.erase(loc);
	}

	return id;
}

const time_of_day& display::get_time_of_day(const map_location& /*loc*/) const
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
	if(prefix == "") {
		return;
	}

	// search prefix.png, prefix1.png, prefix2.png ...
	for(int i = 0;; ++i) {
		std::ostringstream s;
		s << prefix;

		if(i != 0) {
			s << i;
		}

		s << ".png";

		if(image::exists(s.str())) {
			images.push_back(s.str());
		} else if(i > 0) {
			break;
		}
	}

	if(images.empty()) {
		images.emplace_back();
	}
}

const std::string& display::get_variant(const std::vector<std::string>& variants, const map_location& loc) const
{
	// TODO use better noise function
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

void display::change_display_context(const display_context* dc)
{
	dc_ = dc;
	builder_->change_map(&dc_->map()); // TODO: Should display_context own and initialize the builder object?
}

void display::reset_halo_manager()
{
	halo_man_.reset(new halo::manager());
}

void display::reset_halo_manager(halo::manager& halo_man)
{
	halo_man_.reset(&halo_man);
}

void display::blindfold(bool value)
{
	if(value == true) {
		++blindfold_ctr_;
	} else {
		--blindfold_ctr_;
	}
}

bool display::is_blindfolded() const
{
	return blindfold_ctr_ > 0;
}

const SDL_Rect& display::max_map_area() const
{
	static SDL_Rect max_area {0, 0, 0, 0};

	// hex_size() is always a multiple of 4 and hex_width() a multiple of 3,
	// so there shouldn't be off-by-one-errors due to rounding.
	// To display a hex fully on screen, a little bit extra space is needed.
	// Also added the border two times.
	max_area.w = static_cast<int>((get_map().w() + 2 * theme_.border().size + 1.0 / 3.0) * hex_width());
	max_area.h = static_cast<int>((get_map().h() + 2 * theme_.border().size + 0.5)       * hex_size());

	return max_area;
}

const SDL_Rect& display::map_area() const
{
	static SDL_Rect max_area;
	max_area = max_map_area();

	// if it's for map_screenshot, maximize and don't recenter
	if(map_screenshot_) {
		return max_area;
	}

	static SDL_Rect res;
	res = map_outside_area();

	// map is smaller, center
	if(max_area.w < res.w) {
		res.x += (res.w - max_area.w) / 2;
		res.w = max_area.w;
	}

	// map is smaller, center
	if(max_area.h < res.h) {
		res.y += (res.h - max_area.h) / 2;
		res.h = max_area.h;
	}

	return res;
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
	const SDL_Rect& rect = map_area();
	if(sdl::point_in_rect(xclick, yclick, rect) == false) {
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

	if(y_mod < tesselation_y_size / 2) {
		if((x_mod * 2 + y_mod) < (s / 2)) {
			x_modifier = -1;
			y_modifier = -1;
		} else if((x_mod * 2 - y_mod) < (s * 3 / 2)) {
			x_modifier = 0;
			y_modifier = 0;
		} else {
			x_modifier = 1;
			y_modifier = -1;
		}
	} else {
		if((x_mod * 2 - (y_mod - s / 2)) < 0) {
			x_modifier = -1;
			y_modifier = 0;
		} else if((x_mod * 2 + (y_mod - s / 2)) < s * 2) {
			x_modifier = 0;
			y_modifier = 0;
		} else {
			x_modifier = 1;
			y_modifier = 0;
		}
	}

	return map_location(x_base + x_modifier - offset, y_base + y_modifier - offset);
}

const rect_of_hexes display::hexes_under_rect(const SDL_Rect& r) const
{
	rect_of_hexes res;

	if(r.w <= 0 || r.h <= 0) {
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
	// TODO: don't return location for this,
	// instead directly scroll to the clicked pixel position

	if(!sdl::point_in_rect(x, y, minimap_area())) {
		return map_location();
	}

	// we transform the coordinates from minimap to the full map image
	// probably more adjustments to do (border, minimap shift...)
	// but the mouse and human capacity to evaluate the rectangle center
	// is not pixel precise.
	int px = (x - minimap_location_.x) * get_map().w() * hex_width() / std::max(minimap_location_.w, 1);
	int py = (y - minimap_location_.y) * get_map().h() * hex_size() / std::max(minimap_location_.h, 1);

	map_location loc = pixel_position_to_hex(px, py);
	loc.x = utils::clamp(loc.x, 0, get_map().w() - 1);
	loc.y = utils::clamp(loc.x, 0, get_map().h() - 1);

	return loc;
}

surface display::screenshot(bool map_screenshot)
{
	if(!map_screenshot) {
		// Use make_neutral_surface() to copy surface content
		// TODO: convert to texture handling
		//return make_neutral_surface(video_.getSurface());
		return surface(nullptr);
	}

	if(get_map().empty()) {
		ERR_DP << "No map loaded, cannot create a map screenshot.\n";
		return nullptr;
	}

	const SDL_Rect area = max_map_area();
	surface res = create_neutral_surface(area.w, area.h);

	// Memory problem?
	if(res == nullptr) {
		ERR_DP << "Could not create screenshot surface, try zooming out.\n";
		return nullptr;
	}

	// Back up the current map viewport position and move to top-left.
	const int old_xpos = xpos_;
	const int old_ypos = ypos_;
	xpos_ = 0;
	ypos_ = 0;

	// Reroute render output to a separate texture	.
	texture output_texture(area.w, area.h, SDL_TEXTUREACCESS_TARGET);
	const render_target_setter target_setter(output_texture);

	map_screenshot_ = true;

	DBG_DP << "draw() call for map screenshot\n";
	draw();

	map_screenshot_ = false;

	// Restore map viewport position
	xpos_ = old_xpos;
	ypos_ = old_ypos;

	// Copy the texture data to the output surface.
	SDL_RenderReadPixels(video_.get_renderer(), &area, SDL_PIXELFORMAT_ARGB8888, res->pixels, res->pitch);
	return res;
}

std::shared_ptr<gui::button> display::find_action_button(const std::string& /*id*/)
{
	return nullptr;
}

std::shared_ptr<gui::button> display::find_menu_button(const std::string& /*id*/)
{
	return nullptr;
}

static const std::string& get_direction(std::size_t n)
{
	static const std::array<std::string, 6> dirs {{"-n", "-ne", "-se", "-s", "-sw", "-nw"}};
	return dirs[n >= dirs.size() ? 0 : n];
}

void display::draw_fog_shroud_transition_images(const map_location& loc, image::TYPE /*image_type*/)
{
	std::vector<std::string> names;

	adjacent_loc_array_t adjacent;
	get_adjacent_tiles(loc, adjacent.data());

	enum visibility { FOG = 0, SHROUD = 1, CLEAR = 2 };
	visibility tiles[6];

	const std::string* image_prefix[]{&game_config::fog_prefix, &game_config::shroud_prefix};

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
		for(int i = (start + 1) % 6, cap1 = 0; i != start && cap1 != 6; ++cap1) {
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

	// Now render the images
	for(std::string& name : names) {
		render_scaled_to_zoom(image::get_texture(name), loc); // TODO: image_type
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

void display::draw_background()
{
	const std::string& image = theme_.border().background_image;
	SDL_Rect area = map_outside_area();

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
	video_.render_copy(background, nullptr, &area);
}

int display::draw_text_in_hex(const map_location& loc,
		const std::string& text,
		std::size_t font_size,
		color_t color,
		int fl_label_id,
		double x_in_hex,
		double y_in_hex)
{
	if(text.empty()) {
		return fl_label_id;
	}

	const std::size_t font_sz = static_cast<std::size_t>(font_size * get_zoom_factor());

	const int x = get_location_x(loc) /*- text_surf->w / 2*/ + static_cast<int>(x_in_hex * hex_size());
	const int y = get_location_y(loc) /*- text_surf->h / 2*/ + static_cast<int>(y_in_hex * hex_size());

	// We were given a label id, remove it.
	if(fl_label_id != 0) {
		font::remove_floating_label(fl_label_id);
	}

	font::floating_label flabel(text);
	flabel.set_font_size(font_sz);
	flabel.set_color(color);
	flabel.set_position(x, y);
	flabel.set_alignment(font::CENTER_ALIGN);
	flabel.set_scroll_mode(font::ANCHOR_LABEL_MAP);

	return font::add_floating_label(flabel);
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

void display::announce(const std::string& message, const color_t& color, const announce_options& options)
{
	if(options.discard_previous) {
		font::remove_floating_label(prevLabel);
	}

	font::floating_label flabel(message);
	flabel.set_font_size(font::SIZE_XLARGE);
	flabel.set_color(color);
	flabel.set_position(map_outside_area().w / 2, map_outside_area().h / 3);
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

	// Turn the zoom factor to a double in order to avoid rounding errors.
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
	// draw();

	return true;
}

void display::set_default_zoom()
{
	if(zoom_ != DefaultZoom) {
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
	const SDL_Rect& area = map_area();
	int hw = hex_width(), hs = hex_size();
	return x + hs >= area.x - hw && x < area.x + area.w + hw && y + hs >= area.y - hs && y < area.y + area.h + hs;
}

void display::scroll_to_xy(int screenxpos, int screenypos, SCROLL_TYPE scroll_type, bool force)
{
	if(!force && (view_locked_ || !preferences::scroll_to_action())) {
		return;
	}

	if(video_.update_locked()) {
		return;
	}

	const SDL_Rect area = map_area();
	const int xmove_expected = screenxpos - (area.x + area.w / 2);
	const int ymove_expected = screenypos - (area.y + area.h / 2);

	int xpos = xpos_ + xmove_expected;
	int ypos = ypos_ + ymove_expected;
	bounds_check_position(xpos, ypos);
	int xmove = xpos - xpos_;
	int ymove = ypos - ypos_;

	if(scroll_type == WARP || scroll_type == ONSCREEN_WARP || turbo_speed() > 2.0 || preferences::scroll_speed() > 99) {
		scroll(xmove, ymove, true);
		// draw();
		return;
	}

	// Doing an animated scroll, with acceleration etc.

	int x_old = 0;
	int y_old = 0;

	const double dist_total = std::hypot(xmove, ymove);
	double dist_moved = 0.0;

	int t_prev = SDL_GetTicks();

	double velocity = 0.0;
	while(dist_moved < dist_total) {
		events::run_event_loop();

		int t = SDL_GetTicks();
		double dt = (t - t_prev) / 1000.0;
		if(dt > 0.200) {
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
		double dist_stop = dist_moved + velocity * stop_time - 0.5 * decel * stop_time * stop_time;

		if(dist_stop > dist_total || velocity > velocity_max) {
			velocity -= decel * dt;
			if(velocity < 1.0) {
				velocity = 1.0;
			}
		} else {
			velocity += accel * dt;
			if(velocity > velocity_max) {
				velocity = velocity_max;
			}
		}

		dist_moved += velocity * dt;
		if(dist_moved > dist_total) {
			dist_moved = dist_total;
		}

		int x_new = round_double(xmove * dist_moved / dist_total);
		int y_new = round_double(ymove * dist_moved / dist_total);

		int dx = x_new - x_old;
		int dy = y_new - y_old;

		scroll(dx, dy, true);
		x_old += dx;
		y_old += dy;
		// draw();
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

	scroll_to_tiles(locs, scroll_type, check_fogged, false, 0.0, force);
}

void display::scroll_to_tiles(map_location loc1,
		map_location loc2,
		SCROLL_TYPE scroll_type,
		bool check_fogged,
		double add_spacing,
		bool force)
{
	std::vector<map_location> locs;
	locs.push_back(loc1);
	locs.push_back(loc2);
	scroll_to_tiles(locs, scroll_type, check_fogged, false, add_spacing, force);
}

void display::scroll_to_tiles(const std::vector<map_location>::const_iterator& begin,
		const std::vector<map_location>::const_iterator& end,
		SCROLL_TYPE scroll_type,
		bool check_fogged,
		bool only_if_possible,
		double add_spacing,
		bool force)
{
	// basically we calculate the min/max coordinates we want to have on-screen
	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;
	bool valid = false;

	for(std::vector<map_location>::const_iterator itor = begin; itor != end; ++itor) {
		if(get_map().on_board(*itor) == false)
			continue;
		if(check_fogged && fogged(*itor))
			continue;

		int x = get_location_x(*itor);
		int y = get_location_y(*itor);

		if(!valid) {
			minx = x;
			maxx = x;
			miny = y;
			maxy = y;

			valid = true;
		} else {
			int minx_new = std::min<int>(minx, x);
			int miny_new = std::min<int>(miny, y);
			int maxx_new = std::max<int>(maxx, x);
			int maxy_new = std::max<int>(maxy, y);

			SDL_Rect r = map_area();
			r.x = minx_new;
			r.y = miny_new;

			if(outside_area(r, maxx_new, maxy_new)) {
				// we cannot fit all locations to the screen
				if(only_if_possible) {
					return;
				}

				break;
			}

			minx = minx_new;
			miny = miny_new;
			maxx = maxx_new;
			maxy = maxy_new;
		}
	}

	// if everything is fogged or the location list is empty
	if(!valid) {
		return;
	}

	if(scroll_type == ONSCREEN || scroll_type == ONSCREEN_WARP) {
		SDL_Rect r = map_area();
		int spacing = round_double(add_spacing * hex_size());
		r.x += spacing;
		r.y += spacing;
		r.w -= 2 * spacing;
		r.h -= 2 * spacing;

		if(!outside_area(r, minx, miny) && !outside_area(r, maxx, maxy)) {
			return;
		}
	}

	// let's do "normal" rectangle math from now on
	SDL_Rect locs_bbox {
		minx,
		miny,
		maxx - minx + hex_size(),
		maxy - miny + hex_size()
	};

	// target the center
	int target_x = locs_bbox.x + locs_bbox.w / 2;
	int target_y = locs_bbox.y + locs_bbox.h / 2;

	if(scroll_type == ONSCREEN || scroll_type == ONSCREEN_WARP) {
		// when doing an ONSCREEN scroll we do not center the target unless needed
		SDL_Rect r = map_area();
		int map_center_x = r.x + r.w / 2;
		int map_center_y = r.y + r.h / 2;

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

		r.x = target_x - w / 2;
		r.y = target_y - h / 2;
		r.w = w;
		r.h = h;

		// now any point within r is a possible target to scroll to
		// we take the one with the minimum distance to map_center
		// which will always be at the border of r

		if(map_center_x < r.x) {
			target_x = r.x;
			target_y = utils::clamp(map_center_y, r.y, r.y + r.h - 1);
		} else if(map_center_x > r.x + r.w - 1) {
			target_x = r.x + r.w - 1;
			target_y = utils::clamp(map_center_y, r.y, r.y + r.h - 1);
		} else if(map_center_y < r.y) {
			target_y = r.y;
			target_x = utils::clamp(map_center_x, r.x, r.x + r.w - 1);
		} else if(map_center_y > r.y + r.h - 1) {
			target_y = r.y + r.h - 1;
			target_x = utils::clamp(map_center_x, r.x, r.x + r.w - 1);
		} else {
			ERR_DP << "Bug in the scrolling code? Looks like we would not need to scroll after all..." << std::endl;
			// keep the target at the center
		}
	}

	scroll_to_xy(target_x, target_y, scroll_type, force);
}

void display::bounds_check_position()
{
	const unsigned int orig_zoom = zoom_;

	zoom_ = utils::clamp(zoom_, MinZoom, MaxZoom);

	bounds_check_position(xpos_, ypos_);

	if(zoom_ != orig_zoom) {
		image::set_zoom(zoom_);
	}
}

void display::bounds_check_position(int& xpos, int& ypos) const
{
	const int tile_width = hex_width();

	// Adjust for the border 2 times
	const int xend = static_cast<int>(tile_width * (get_map().w() + 2 * theme_.border().size) + tile_width / 3);
	const int yend = static_cast<int>(zoom_      * (get_map().h() + 2 * theme_.border().size) + zoom_      / 2);

	xpos = utils::clamp(xpos, 0, xend - map_area().w);
	ypos = utils::clamp(ypos, 0, yend - map_area().h);
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
	idle_anim_rate_ = std::pow(2.0, -rate / 10.0);
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

	// openMP can't iterate over std::size_t
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
	for(const unit& u : dc_->units()) {
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

void display::update_arrow(arrow& arrow)
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
	return pixel_position_to_hex(xpos_ + rect.x + rect.w / 2, ypos_ + rect.y + rect.h / 2);
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

//
// NEW RENDERING CODE =========================================================================
//

void display::draw_hex_overlays()
{
	// DO NOTHING
}

void display::draw_visible_hexes(const rect_of_hexes& visible_hexes, TERRAIN_TYPE layer)
{
	assert(builder_);

	drawn_hexes_ = 0;

	terrain_builder::TERRAIN_TYPE builder_terrain_type = layer == FOREGROUND
		? terrain_builder::FOREGROUND
		: terrain_builder::BACKGROUND;

	for(const map_location& loc : visible_hexes) {
		if(shrouded(loc)) {
			continue;
		}

		//image::TYPE image_type = get_image_type(loc);
		const time_of_day& tod = get_time_of_day(loc);

		// Get the image list for this location.
		const terrain_builder::imagelist* const terrains = builder_->get_terrain_at(loc, tod.id, builder_terrain_type);
		if(!terrains) {
			continue;
		}

#if 0
		image::light_string lt;

		const time_of_day& tod = get_time_of_day(loc);

		//get all the light transitions
		map_location adjs[6];
		get_adjacent_tiles(loc,adjs);

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

			const time_of_day& atod1 = get_time_of_day(adjs[d]);
			const time_of_day& atod2 = get_time_of_day(adjs[(d + 1) % 6]);

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

			const time_of_day& atod1 = get_time_of_day(adjs[d]);
			const time_of_day& atod2 = get_time_of_day(adjs[(d + 1) % 6]);

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

			const time_of_day& atod1 = get_time_of_day(adjs[d]);
			const time_of_day& atod2 = get_time_of_day(adjs[(d + 1) % 6]);

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

		// Cache the offmap name.
		// Since it is themeable it can change, so don't make it static.
		//const std::string off_map_name = "terrain/" + theme_.border().tile_image;

		for(const auto& terrain : *terrains) {
			const image::locator& image = animate_map_
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
				surf = image::get_image(image, off_map ? image::SCALED_TO_HEX : image_type);
			} else if(lt.empty()) {
				surf = image::get_image(image, image::SCALED_TO_HEX);
			} else {
				surf = image::get_lighted_image(image, lt, image::SCALED_TO_HEX);
			}
#endif
			if(!tex.null()) {
				render_scaled_to_zoom(tex, loc);
			}
		}

		++drawn_hexes_;
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
	// Real (standing) units
	//
	if(!dc_->teams().empty()) {
		unit_drawer drawer = unit_drawer(*this);

		for(const unit& real_unit : dc_->units()) {
			drawer.redraw_unit(real_unit);
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
	// Fake (moving) units
	//
	if(!dc_->teams().empty()) {
		unit_drawer drawer = unit_drawer(*this); // TODO: don't create this twice per cycle.

		for(const unit* temp_unit : *fake_unit_man_) {
			drawer.redraw_unit(*temp_unit);
		}
	}

	//
	// Draws various overlays, such as reach maps, etc.
	//
	draw_hex_overlays();

	//
	// Hex cursor (TODO: split into layers?)
	//
	draw_hex_cursor(mouseoverHex_);

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
			draw_fog_shroud_transition_images(loc, image_type);
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
	draw_background();

	// Progress animations.
	invalidate_animations();

	// draw_minimap();

	// Draw the gamemap and its contents (units, etc)
	{
		SDL_Rect map_area_rect = map_area();
		render_clip_rect_setter setter(&map_area_rect);

		draw_gamemap();
	}

	// Draw debugging aids such as the FPS counter.
	draw_debugging_aids();

	// Draw floating labels (includes map labels).
	font::draw_floating_labels();

	// TODO: what dis?
	// events::raise_volatile_draw_event();
	// events::raise_volatile_undraw_event();

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

display* display::singleton_ = nullptr;
