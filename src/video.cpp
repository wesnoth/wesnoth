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

#include "video.hpp"

#include "display.hpp"
#include "draw_manager.hpp"
#include "floating_label.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "log.hpp"
#include "picture.hpp"
#include "preferences/general.hpp"
#include "sdl/input.hpp"
#include "sdl/point.hpp"
#include "sdl/texture.hpp"
#include "sdl/userevent.hpp"
#include "sdl/utils.hpp"
#include "sdl/window.hpp"

#ifdef TARGET_OS_OSX
#include "desktop/apple_video.hpp"
#include "game_version.hpp"
#endif

#include <SDL2/SDL_render.h> // SDL_Texture

#include <cassert>
#include <vector>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)
#define DBG_DP LOG_STREAM(debug, log_display)

namespace
{
/** The SDL window object. Will be null only if headless_. */
std::unique_ptr<sdl::window> window;

/** The main offscreen render target. */
texture render_texture_ = {};

/** The current offscreen render target. */
texture current_render_target_ = {};

bool headless_ = false; /**< running with no window at all */
bool testing_ = false; /**< running unit tests */
point test_resolution_ = {1024, 768}; /**< resolution for unit tests */
int refresh_rate_ = 0;
point game_canvas_size_ = {0, 0};
int pixel_scale_ = 1;
rect input_area_ = {};

} // anon namespace

namespace video
{

// Non-public interface
void render_screen(); // exposed and used only in draw_manager.cpp

// Internal functions
static void init_window();
static void init_test_window();
static void init_fake();
static void init_test();
static bool update_framebuffer();
static bool update_test_framebuffer();
static point draw_offset();


void init(fake type)
{
	if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		ERR_DP << "Could not initialize SDL_video: " << SDL_GetError();
		throw error("Video initialization failed");
	}

	switch(type) {
	case fake::none:
		init_window();
		break;
	case fake::window:
		init_fake();
		break;
	case fake::draw:
		init_test();
		break;
	default:
		throw error("unrecognized fake type passed to video::init");
	}
}

bool headless()
{
	return headless_;
}

bool testing()
{
	return testing_;
}

void init_fake()
{
	headless_ = true;
	refresh_rate_ = 1;
	game_canvas_size_ = {800,600};
}

void init_test()
{
	testing_ = true;
	refresh_rate_ = 1;
	init_test_window();
}

/** Returns true if the buffer was changed */
bool update_test_framebuffer()
{
	if (!window) {
		throw("trying to update test framebuffer with no window");
	}

	bool changed = false;

	// TODO: code unduplication
	// Build or update the current render texture.
	if (render_texture_) {
		int w, h;
		SDL_QueryTexture(render_texture_, nullptr, nullptr, &w, &h);
		if (w != test_resolution_.x || h != test_resolution_.y) {
			// Delete it and let it be recreated.
			LOG_DP << "destroying old render texture";
			render_texture_.reset();
		}
	}
	if (!render_texture_) {
		LOG_DP << "creating offscreen render texture";
		render_texture_.assign(SDL_CreateTexture(
			*window,
			window->pixel_format(),
			SDL_TEXTUREACCESS_TARGET,
			test_resolution_.x, test_resolution_.y
		));
		LOG_DP << "updated render target to " << test_resolution_.x
			<< "x" << test_resolution_.y;
		changed = true;
	}

	pixel_scale_ = 1;
	game_canvas_size_ = test_resolution_;
	input_area_ = {{}, test_resolution_};

	// The render texture is always the render target in this case.
	force_render_target(render_texture_);

	return changed;
}

bool update_framebuffer()
{
	if (!window) {
		throw error("trying to update framebuffer with no window");
	}

	if (testing_) {
		return update_test_framebuffer();
	}

	bool changed = false;

	// Make sure we're getting values from the native window.
	SDL_SetRenderTarget(*window, nullptr);

	// Non-integer scales are not currently supported.
	// This option makes things neater when window size is not a perfect
	// multiple of logical size, which can happen when manually resizing.
	SDL_RenderSetIntegerScale(*window, SDL_TRUE);

	// Find max valid pixel scale at current output size.
	point osize(window->get_output_size());
	int max_scale = std::min(
		osize.x / preferences::min_window_width,
		osize.y / preferences::min_window_height);
	max_scale = std::min(max_scale, preferences::max_pixel_scale);

	// Determine best pixel scale according to preference and window size
	int scale = 1;
	if (preferences::auto_pixel_scale()) {
		// Try to match the default size (1280x720) but do not reduce below
		int def_scale = std::min(
			osize.x / preferences::def_window_width,
			osize.y / preferences::def_window_height);
		scale = std::min(max_scale, def_scale);
		// Otherwise reduce to keep below the max window size (1920x1080).
		int min_scale = std::min(
			osize.x / (preferences::max_window_width+1) + 1,
			osize.y / (preferences::max_window_height+1) + 1);
		scale = std::max(scale, min_scale);
	} else {
		scale = std::min(max_scale, preferences::pixel_scale());
	}
	// Cache it for easy access.
	if (pixel_scale_ != scale) {
		pixel_scale_ = scale;
		changed = true;
	}

	// Update logical size if it doesn't match the current resolution and scale.
	point lsize(window->get_logical_size());
	point wsize(window->get_size());
	if (lsize.x != osize.x / scale || lsize.y != osize.y / scale) {
		if (!preferences::auto_pixel_scale() && scale < preferences::pixel_scale()) {
			LOG_DP << "reducing pixel scale from desired "
				<< preferences::pixel_scale() << " to maximum allowable "
				<< scale;
		}
		LOG_DP << "pixel scale: " << scale;
		LOG_DP << "overriding logical size";
		LOG_DP << "  old lsize: " << lsize;
		LOG_DP << "  old wsize: " << wsize;
		LOG_DP << "  old osize: " << osize;
		window->set_logical_size(osize.x / scale, osize.y / scale);
		lsize = window->get_logical_size();
		wsize = window->get_size();
		osize = window->get_output_size();
		LOG_DP << "  new lsize: " << lsize;
		LOG_DP << "  new wsize: " << wsize;
		LOG_DP << "  new osize: " << osize;
		float sx, sy;
		SDL_RenderGetScale(*window, &sx, &sy);
		LOG_DP << "  render scale: " << sx << ", " << sy;
	}
	// Cache it for easy access
	game_canvas_size_ = lsize;

	// Build or update the current render texture.
	if (render_texture_) {
		int w, h;
		SDL_QueryTexture(render_texture_, nullptr, nullptr, &w, &h);
		if (w != osize.x || h != osize.y) {
			// Delete it and let it be recreated.
			LOG_DP << "destroying old render texture";
			render_texture_.reset();
		}
	}
	if (!render_texture_) {
		LOG_DP << "creating offscreen render texture";
		render_texture_.assign(SDL_CreateTexture(
			*window,
			window->pixel_format(),
			SDL_TEXTUREACCESS_TARGET,
			osize.x, osize.y
		));
		// This isn't really necessary, but might be nice to have attached
		render_texture_.set_draw_size(lsize);
		changed = true;
	}

	// Assign the render texture now. It will be used for all drawing.
	force_render_target(render_texture_);

	// By default input area is the same as the window area.
	input_area_ = {{}, wsize};

	rect active_area = to_output(draw_area());
	if (active_area.size() != osize) {
		LOG_DP << "render target offset: LT " << active_area.pos() << " RB "
		       << osize - active_area.size() - active_area.pos();
		// Translate active_area into display coordinates as input_area_
		input_area_ = {
			(active_area.pos() * wsize) / osize,
			(active_area.size() * wsize) / osize
		};
		LOG_DP << "input area: " << input_area_;
	}

	return changed;
}

void init_test_window()
{
	LOG_DP << "creating test window " << test_resolution_.x
		<< "x" << test_resolution_.y;

	uint32_t window_flags = 0;
	window_flags |= SDL_WINDOW_HIDDEN;
	// The actual window won't be used, as we'll be rendering to texture.

	uint32_t renderer_flags = 0;
	renderer_flags |= SDL_RENDERER_TARGETTEXTURE;
	// All we need is to be able to render to texture.

	window.reset(new sdl::window(
		"", 0, 0, test_resolution_.x, test_resolution_.y,
		window_flags, renderer_flags
	));

	update_test_framebuffer();
}

void init_window()
{
	// Position
	const int x = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;
	const int y = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;

	// Dimensions
	const point res = preferences::resolution();
	const int w = res.x;
	const int h = res.y;

	uint32_t window_flags = 0;

	// Add any more default flags here
	window_flags |= SDL_WINDOW_RESIZABLE;
	window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

	if(preferences::fullscreen()) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if(preferences::maximized()) {
		window_flags |= SDL_WINDOW_MAXIMIZED;
	}

	uint32_t renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

	if(preferences::vsync()) {
		LOG_DP << "VSYNC on";
		renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
	}

	// Initialize window
	window.reset(new sdl::window("", x, y, w, h, window_flags, renderer_flags));

	// It is assumed that this function is only ever called once.
	// If that is no longer true, then you should clean things up.
	assert(!render_texture_);

	PLAIN_LOG << "Setting mode to " << w << "x" << h;

	window->set_minimum_size(preferences::min_window_width, preferences::min_window_height);

	SDL_DisplayMode currentDisplayMode;
	SDL_GetCurrentDisplayMode(window->get_display_index(), &currentDisplayMode);
	refresh_rate_ = currentDisplayMode.refresh_rate != 0 ? currentDisplayMode.refresh_rate : 60;

	update_framebuffer();
}

point output_size()
{
	if (testing_) {
		return test_resolution_;
	}
	// As we are rendering via an abstraction, we should never need this.
	return window->get_output_size();
}

point window_size()
{
	if (testing_) {
		return test_resolution_;
	}
	return window->get_size();
}

rect game_canvas()
{
	return {0, 0, game_canvas_size_.x, game_canvas_size_.y};
}

point game_canvas_size()
{
	return game_canvas_size_;
}

point draw_size()
{
	return current_render_target_.draw_size();
}

rect draw_area()
{
	return {0, 0, current_render_target_.w(), current_render_target_.h()};
}

point draw_offset()
{
	// As we are using SDL_RenderSetIntegerScale, there may be a slight
	// offset of the drawable area on the render target if the target size
	// is not perfectly divisble by the scale.
	// SDL doesn't provide any way of retrieving this offset,
	// so we just have to base our calculation on the known behaviour.
	point osize = output_size();
	point dsize = draw_size();
	point scale = osize / dsize;
	return (osize - (scale * dsize)) / 2;
}

rect output_area()
{
	point p = output_size();
	return {0, 0, p.x, p.y};
}

rect to_output(const rect& r)
{
	// Multiply r by integer scale, adding draw_offset to the position.
	point dsize = current_render_target_.draw_size();
	point osize = current_render_target_.get_raw_size();
	point pos = (r.pos() * (osize / dsize)) + draw_offset();
	point size = r.size() * (osize / dsize);
	return {pos, size};
}

rect input_area()
{
	return input_area_;
}

int get_pixel_scale()
{
	return pixel_scale_;
}

int current_refresh_rate()
{
	// TODO: this should be more clever, depending on usage
	return refresh_rate_;
}

void force_render_target(const texture& t)
{
	if (SDL_SetRenderTarget(get_renderer(), t)) {
		ERR_DP << "failed to set render target to "
			<< static_cast<void*>(t.get()) << ' '
			<< t.draw_size() << " / " << t.get_raw_size();
		ERR_DP << "last SDL error: " << SDL_GetError();
		throw error("failed to set render target");
	}
	current_render_target_ = t;

	if (testing_) {
		return;
	}

	// The scale factor gets reset when the render target changes,
	// so make sure it gets set back appropriately.
	if (!t) {
		DBG_DP << "rendering to window / screen";
		window->set_logical_size(game_canvas_size_);
	} else if (t == render_texture_) {
		DBG_DP << "rendering to primary buffer";
		window->set_logical_size(game_canvas_size_);
	} else {
		DBG_DP << "rendering to custom target "
			<< static_cast<void*>(t.get()) << ' '
			<< t.draw_size() << " / " << t.get_raw_size();
		window->set_logical_size(t.w(), t.h());
	}
}

void clear_render_target()
{
	force_render_target({});
}

texture get_render_target()
{
	// This should always be up-to-date, but assert for sanity.
	assert(current_render_target_ == SDL_GetRenderTarget(get_renderer()));
	return current_render_target_;
}

// Note: this is not thread-safe.
// Drawing functions should not be called while this is active.
// SDL renderer usage is not thread-safe anyway, so this is fine.
void render_screen()
{
	if(headless_ || testing_) {
		// No need to present anything in this case
		return;
	}

	if(!window) {
		WRN_DP << "trying to render with no window";
		return;
	}

	// This should only ever be called when the main render texture is the
	// current render target. It could be adapted otherwise... but let's not.
	if(SDL_GetRenderTarget(*window) != render_texture_) {
		ERR_DP << "trying to render screen, but current render texture is "
			<< static_cast<void*>(SDL_GetRenderTarget(*window))
			<< " | " << static_cast<void*>(current_render_target_.get())
			<< ". It should be " << static_cast<void*>(render_texture_.get());
		throw error("tried to render screen from wrong render target");
	}

	// Clear the render target so we're drawing to the window.
	clear_render_target();

	// Copy the render texture to the window.
	SDL_RenderCopy(*window, render_texture_, nullptr, nullptr);

	// Finalize and display the frame.
	SDL_RenderPresent(*window);

	// Reset the render target to the render texture.
	force_render_target(render_texture_);
}

surface read_pixels(SDL_Rect* r)
{
	if (!window) {
		WRN_DP << "trying to read pixels with no window";
		return surface();
	}

	// This should be what we want to read from.
	texture& target = current_render_target_;

	// Make doubly sure.
	if (target != SDL_GetRenderTarget(*window)) {
		SDL_Texture* t = SDL_GetRenderTarget(*window);
		ERR_DP << "render target " << static_cast<void*>(target.get())
			<< ' ' << target.draw_size() << " / " << target.get_raw_size()
			<< " doesn't match window render target "
			<< static_cast<void*>(t);
		throw error("unexpected render target while reading pixels");
	}

	// Intersect the draw area with the given rect.
	rect r_clipped = draw_area();
	if (r) {
		r_clipped.clip(*r);
		if (r_clipped != *r) {
			DBG_DP << "modifying pixel read area from " << *r
			       << " to " << r_clipped;
			*r = r_clipped;
		}
	}

	// Convert the rect to output coordinates, if necessary.
	rect o = to_output(r_clipped);

	// Create surface and read pixels
	surface s(o.w, o.h);
	SDL_RenderReadPixels(*window, &o, s->format->format, s->pixels, s->pitch);
	return s;
}

surface read_pixels_low_res(SDL_Rect* r)
{
	if(!window) {
		WRN_DP << "trying to read pixels with no window";
		return surface();
	}
	surface s = read_pixels(r);
	if(r) {
		return scale_surface(s, r->w, r->h);
	} else {
		return scale_surface(s, draw_size().x, draw_size().y);
	}
}

void set_window_title(const std::string& title)
{
	assert(window);
	window->set_title(title);
}

void set_window_icon(surface& icon)
{
	assert(window);
	window->set_icon(icon);
}

SDL_Renderer* get_renderer()
{
	if(window) {
		return *window;
	} else {
		return nullptr;
	}
}

SDL_Window* get_window()
{
	return *window;
}

std::string current_driver()
{
	const char* const drvname = SDL_GetCurrentVideoDriver();
	return drvname ? drvname : "<not initialized>";
}

std::vector<std::string> enumerate_drivers()
{
	std::vector<std::string> res;
	int num_drivers = SDL_GetNumVideoDrivers();

	for(int n = 0; n < num_drivers; ++n) {
		const char* drvname = SDL_GetVideoDriver(n);
		res.emplace_back(drvname ? drvname : "<invalid driver>");
	}

	return res;
}

/**
 * Tests whether the given flags are currently set on the SDL window.
 *
 * @param flags               The flags to test, OR'd together.
 */
static bool window_has_flags(uint32_t flags)
{
	return window && (window->get_flags() & flags) != 0;
}

bool window_is_visible()
{
	return window_has_flags(SDL_WINDOW_SHOWN);
}

bool window_has_focus()
{
	return window_has_flags(SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS);
}

bool window_has_mouse_focus()
{
	return window_has_flags(SDL_WINDOW_MOUSE_FOCUS);
}

std::vector<point> get_available_resolutions(const bool include_current)
{
	std::vector<point> result;

	if(!window) {
		return result;
	}

	const int display_index = window->get_display_index();

	const int modes = SDL_GetNumDisplayModes(display_index);
	if(modes <= 0) {
		PLAIN_LOG << "No modes supported";
		return result;
	}

	const point min_res(preferences::min_window_width, preferences::min_window_height);

	// The maximum size to which this window can be set. For some reason this won't
	// pop up as a display mode of its own.
	SDL_Rect bounds;
	SDL_GetDisplayBounds(display_index, &bounds);

	SDL_DisplayMode mode;

	for(int i = 0; i < modes; ++i) {
		if(SDL_GetDisplayMode(display_index, i, &mode) == 0) {
			// Exclude any results outside the range of the current DPI.
			if(mode.w > bounds.w && mode.h > bounds.h) {
				continue;
			}

			if(mode.w >= min_res.x && mode.h >= min_res.y) {
				result.emplace_back(mode.w, mode.h);
			}
		}
	}

	if(std::find(result.begin(), result.end(), min_res) == result.end()) {
		result.push_back(min_res);
	}

	if(include_current) {
		result.push_back(current_resolution());
	}

	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());

	return result;
}

point current_resolution()
{
	if (testing_) {
		return test_resolution_;
	}
	return point(window->get_size()); // Convert from plain SDL_Point
}

bool is_fullscreen()
{
	if (testing_) {
		return true;
	}
	return (window->get_flags() & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}

void set_fullscreen(bool fullscreen)
{
	if (headless_ || testing_) {
		return;
	}

	// Only do anything if the current value differs from the desired value
	if (window && is_fullscreen() != fullscreen) {
		if (fullscreen) {
			window->full_screen();
		} else if (preferences::maximized()) {
			window->to_window();
			window->maximize();
		} else {
			window->to_window();
			window->restore();
		}
		update_buffers();
	}

	// Update the config value in any case.
	preferences::_set_fullscreen(fullscreen);
}

void toggle_fullscreen()
{
	set_fullscreen(!preferences::fullscreen());
}

bool set_resolution(const point& resolution)
{
	if(resolution == current_resolution()) {
		return false;
	}

	if(!window) {
		throw error("tried to set resolution with no window");
	}

	if(testing_) {
		LOG_DP << "resizing test resolution to " << resolution;
		test_resolution_ = resolution;
		return update_test_framebuffer();
	}

	window->restore();
	window->set_size(resolution.x, resolution.y);
	window->center();

	update_buffers();

	// Change the saved values in preferences.
	LOG_DP << "updating resolution to " << resolution;
	preferences::_set_resolution(resolution);
	preferences::_set_maximized(false);

	return true;
}

void update_buffers(bool autoupdate)
{
	if(headless_) {
		return;
	}

	LOG_DP << "updating video buffers";
	if(update_framebuffer() && autoupdate) {
		draw_manager::invalidate_all();
	}
}

} // namespace video
