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
#include "floating_label.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "sdl/point.hpp"
#include "sdl/userevent.hpp"
#include "sdl/utils.hpp"
#include "sdl/window.hpp"
#include "sdl/input.hpp"

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

CVideo* CVideo::singleton_ = nullptr;

namespace
{
surface drawingSurface = nullptr;
bool fake_interactive = false;

const unsigned MAGIC_DPI_SCALE_NUMBER = 96;
}

namespace video2
{
std::list<events::sdl_handler*> draw_layers;

draw_layering::draw_layering(const bool auto_join)
	: sdl_handler(auto_join)
{
	draw_layers.push_back(this);
}

draw_layering::~draw_layering()
{
	draw_layers.remove(this);

	video2::trigger_full_redraw();
}

void trigger_full_redraw()
{
	SDL_Event event;
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.data1 = (*drawingSurface).h;
	event.window.data2 = (*drawingSurface).w;

	for(const auto& layer : draw_layers) {
		layer->handle_window_event(event);
	}

	SDL_Event drawEvent;
	sdl::UserEvent data(DRAW_ALL_EVENT);

	drawEvent.type = DRAW_ALL_EVENT;
	drawEvent.user = data;
	SDL_FlushEvent(DRAW_ALL_EVENT);
	SDL_PushEvent(&drawEvent);
}

} // video2

CVideo::CVideo(FAKE_TYPES type)
	: window()
	, drawing_texture_(nullptr)
	, fake_screen_(false)
	, help_string_(0)
	, updated_locked_(0)
	, flip_locked_(0)
	, refresh_rate_(0)
{
	assert(!singleton_);
	singleton_ = this;

	initSDL();

	switch(type) {
	case NO_FAKE:
		break;
	case FAKE:
		make_fake();
		break;
	case FAKE_TEST:
		make_test_fake();
		break;
	}
}

void CVideo::initSDL()
{
	const int res = SDL_InitSubSystem(SDL_INIT_VIDEO);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL_video: " << SDL_GetError() << std::endl;
		throw CVideo::error();
	}
}

CVideo::~CVideo()
{
	LOG_DP << "calling SDL_Quit()\n";
	SDL_Quit();
	assert(singleton_);
	singleton_ = nullptr;
	LOG_DP << "called SDL_Quit()\n";
}

bool CVideo::non_interactive() const
{
	return fake_interactive ? false : (window == nullptr);
}

void CVideo::video_event_handler::handle_window_event(const SDL_Event& event)
{
	if(event.type == SDL_WINDOWEVENT) {
		switch(event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
			// if(display::get_singleton())
			// display::get_singleton()->redraw_everything();
			SDL_Event drawEvent;
			sdl::UserEvent data(DRAW_ALL_EVENT);

			drawEvent.type = DRAW_ALL_EVENT;
			drawEvent.user = data;

			SDL_FlushEvent(DRAW_ALL_EVENT);
			SDL_PushEvent(&drawEvent);
			break;
		}
	}
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	surface& target(getDrawingSurface());
	SDL_Rect dst{x, y, 0, 0};

	const clip_rect_setter clip_setter(target, clip_rect, clip_rect != nullptr);
	sdl_blit(surf, srcrect, target, &dst);
}

void CVideo::make_fake()
{
	fake_screen_ = true;
	refresh_rate_ = 1;

	drawingSurface = SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 24, SDL_PIXELFORMAT_BGR888);
}

void CVideo::make_test_fake(const unsigned width, const unsigned height)
{
	drawingSurface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_BGR888);

	fake_interactive = true;
	refresh_rate_ = 1;
}

void CVideo::update_framebuffer()
{
	if(!window) {
		return;
	}

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

	// Update logical size if it doesn't match the current resolution and scale.
	point lsize(window->get_logical_size());
	point wsize(window->get_size());
	if (lsize.x != osize.x / scale || lsize.y != osize.y / scale) {
		if (!preferences::auto_pixel_scale() && scale < preferences::pixel_scale()) {
			LOG_DP << "reducing pixel scale from desired "
				<< preferences::pixel_scale() << " to maximum allowable "
				<< scale << std::endl;
		}
		LOG_DP << "pixel scale: " << scale << std::endl;
		LOG_DP << "overriding logical size" << std::endl;
		LOG_DP << "  old lsize: " << lsize << std::endl;
		LOG_DP << "  old wsize: " << wsize << std::endl;
		LOG_DP << "  old osize: " << osize << std::endl;
		window->set_logical_size(osize.x / scale, osize.y / scale);
		lsize = window->get_logical_size();
		wsize = window->get_size();
		osize = window->get_output_size();
		LOG_DP << "  new lsize: " << lsize << std::endl;
		LOG_DP << "  new wsize: " << wsize << std::endl;
		LOG_DP << "  new osize: " << osize << std::endl;
	}

	// Update the drawing surface if required.
	if (!drawingSurface
		|| drawingSurface->w != lsize.x
		|| drawingSurface->h != lsize.y)
	{
		uint32_t format = window->pixel_format();
		int bpp = SDL_BITSPERPIXEL(format);

		// This should match the old system, and so shouldn't cause any
		// problems that weren't there already.
		LOG_DP << "creating " << bpp << "bpp drawing surface with format "
			<< SDL_GetPixelFormatName(format) << std::endl;
		// Note: "surface" destructor automatically frees the old surface
		drawingSurface = SDL_CreateRGBSurfaceWithFormat(
			0,
			lsize.x,
			lsize.y,
			bpp,
			format
		);

		// Also update the drawing texture, with matching format and size.
		if (drawing_texture_) {
			LOG_DP << "destroying old drawing texture" << std::endl;
			SDL_DestroyTexture(drawing_texture_);
		}
		LOG_DP << "creating drawing texture" << std::endl;
		drawing_texture_ = SDL_CreateTexture(
			*window.get(),
			drawingSurface->format->format,
			SDL_TEXTUREACCESS_STREAMING,
			drawingSurface->w,
			drawingSurface->h
		);
	}

	// Update sizes for input conversion.
	sdl::update_input_dimensions(lsize.x, lsize.y, wsize.x, wsize.y);
}

void CVideo::init_window()
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
#ifdef __APPLE__
	window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

	if(preferences::fullscreen()) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if(preferences::maximized()) {
		window_flags |= SDL_WINDOW_MAXIMIZED;
	}

	uint32_t renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

	if(supports_vsync() && preferences::vsync()) {
		LOG_DP << "VSYNC on\n";
		renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
	}

	// Initialize window
	window.reset(new sdl::window("", x, y, w, h, window_flags, renderer_flags));

	std::cerr << "Setting mode to " << w << "x" << h << std::endl;

	window->set_minimum_size(preferences::min_window_width, preferences::min_window_height);

	SDL_DisplayMode currentDisplayMode;
	SDL_GetCurrentDisplayMode(window->get_display_index(), &currentDisplayMode);
	refresh_rate_ = currentDisplayMode.refresh_rate != 0 ? currentDisplayMode.refresh_rate : 60;

	event_handler_.join_global();

	update_framebuffer();
}

void CVideo::set_window_mode(const MODE_EVENT mode, const point& size)
{
	assert(window);
	if(fake_screen_) {
		return;
	}

	switch(mode) {
	case TO_FULLSCREEN:
		window->full_screen();
		break;

	case TO_WINDOWED:
		window->to_window();
		window->restore();
		break;

	case TO_MAXIMIZED_WINDOW:
		window->to_window();
		window->maximize();
		break;

	case TO_RES:
		window->restore();
		window->set_size(size.x, size.y);
		window->center();
		break;
	}

	update_framebuffer();
}

SDL_Point CVideo::output_size() const
{
	// As we are rendering to the drawingSurface, we should never need this.
	return window->get_output_size();
}

SDL_Point CVideo::window_size() const
{
	return window->get_size();
}

SDL_Rect CVideo::draw_area() const
{
	return {0, 0, drawingSurface->w, drawingSurface->h};
}

SDL_Rect CVideo::input_area() const
{
	// This should always match draw_area.
	SDL_Point p(window->get_logical_size());
	return {0, 0, p.x, p.y};
}

int CVideo::get_width() const
{
	return drawingSurface->w;
}

int CVideo::get_height() const
{
	return drawingSurface->h;
}

void CVideo::delay(unsigned int milliseconds)
{
	if(!game_config::no_delay) {
		SDL_Delay(milliseconds);
	}
}

void CVideo::render_screen()
{
	if(fake_screen_ || flip_locked_ > 0) {
		return;
	}

	if (drawingSurface && drawing_texture_) {
		// Upload the drawing surface to the drawing texture.
		void* pixels_out; // somewhere we can write raw pixel data to
		int pitch; // the length of one row of pixels in bytes
		SDL_LockTexture(drawing_texture_, nullptr, &pixels_out, &pitch);
		if (pitch != drawingSurface->pitch) {
			// If these don't match we are not gonna have a good time.
			throw game::error("drawing surface and texture are incompatible");
		}
		size_t num_bytes = drawingSurface->h * pitch;
		memcpy(pixels_out, drawingSurface->pixels, num_bytes);
		SDL_UnlockTexture(drawing_texture_);

		//SDL_UpdateTexture(drawing_texture_, nullptr, drawingSurface->pixels, drawingSurface->pitch);

		// Copy the drawing texture to the render target.
		SDL_RenderCopy(*window.get(), drawing_texture_, nullptr, nullptr);
	}

	if(window) {
		window->render();
	}
}

void CVideo::lock_updates(bool value)
{
	if(value == true) {
		++updated_locked_;
	} else {
		--updated_locked_;
	}
}

bool CVideo::update_locked() const
{
	return updated_locked_ > 0;
}

void CVideo::set_window_title(const std::string& title)
{
	assert(window);
	window->set_title(title);
}

void CVideo::set_window_icon(surface& icon)
{
	assert(window);
	window->set_icon(icon);
}

void CVideo::clear_screen()
{
	if(!window) {
		return;
	}

	window->fill(0, 0, 0, 255);
}

sdl::window* CVideo::get_window()
{
	return window.get();
}

SDL_Renderer* CVideo::get_renderer()
{
	if(window) {
		return *window;
	} else {
		return nullptr;
	}
}

std::string CVideo::current_driver()
{
	const char* const drvname = SDL_GetCurrentVideoDriver();
	return drvname ? drvname : "<not initialized>";
}

std::vector<std::string> CVideo::enumerate_drivers()
{
	std::vector<std::string> res;
	int num_drivers = SDL_GetNumVideoDrivers();

	for(int n = 0; n < num_drivers; ++n) {
		const char* drvname = SDL_GetVideoDriver(n);
		res.emplace_back(drvname ? drvname : "<invalid driver>");
	}

	return res;
}

bool CVideo::window_has_flags(uint32_t flags) const
{
	if(!window) {
		return false;
	}

	return (window->get_flags() & flags) != 0;
}

std::pair<float, float> CVideo::get_dpi() const
{
	float hdpi, vdpi;
	if(window && SDL_GetDisplayDPI(window->get_display_index(), nullptr, &hdpi, &vdpi) == 0) {
#ifdef TARGET_OS_OSX
		// SDL 2.0.12 changes SDL_GetDisplayDPI. Function now returns DPI
		// multiplied by screen's scale factor. This part of code reverts
		// this multiplication.
		//
		// For more info see issue: https://github.com/wesnoth/wesnoth/issues/5019

		if(sdl_get_version() >= version_info{2, 0, 12}) {
			float scale_factor = desktop::apple::get_scale_factor(window->get_display_index());
			hdpi /= scale_factor;
			vdpi /= scale_factor;
		}
#endif
		return { hdpi, vdpi };
	}
	// SDL doesn't know the screen dpi, there's a configuration issue, or we
	// don't have a window yet.
	return { 0.0f, 0.0f };
}

std::pair<float, float> CVideo::get_dpi_scale_factor() const
{
	auto dpi = get_dpi();
	if(dpi.first != 0.0f && dpi.second != 0.0f) {
		// adjust for pixel scale
		SDL_Point wsize = window_size();
		dpi.first /= wsize.x / get_width();
		dpi.second /= wsize.y / get_height();
		return { dpi.first / MAGIC_DPI_SCALE_NUMBER, dpi.second / MAGIC_DPI_SCALE_NUMBER };
	}
	// Assume a scale factor of 1.0 if the screen dpi is currently unknown.
	return { 1.0f, 1.0f };
}

std::vector<point> CVideo::get_available_resolutions(const bool include_current)
{
	std::vector<point> result;

	if(!window) {
		return result;
	}

	const int display_index = window->get_display_index();

	const int modes = SDL_GetNumDisplayModes(display_index);
	if(modes <= 0) {
		std::cerr << "No modes supported\n";
		return result;
	}

	const point min_res(preferences::min_window_width, preferences::min_window_height);

#if 0
	// DPI scale factor.
	auto [scale_h, scale_v] = get_dpi_scale_factor();
#endif

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

surface& CVideo::getDrawingSurface()
{
	return drawingSurface;
}

point CVideo::current_resolution()
{
	return point(window->get_size()); // Convert from plain SDL_Point
}

bool CVideo::is_fullscreen() const
{
	return (window->get_flags() & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}

bool CVideo::supports_vsync() const
{
	return sdl_get_version() >= version_info{2, 0, 17};
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const color_t color{0, 0, 0, 0xbb};

	int size = font::SIZE_LARGE;

	while(size > 0) {
		if(font::pango_line_width(str, size) > get_width()) {
			size--;
		} else {
			break;
		}
	}

	const int border = 5;

	font::floating_label flabel(str);
	flabel.set_font_size(size);
	flabel.set_position(get_width() / 2, get_height());
	flabel.set_bg_color(color);
	flabel.set_border_size(border);

	help_string_ = font::add_floating_label(flabel);

	const SDL_Rect& rect = font::get_floating_label_rect(help_string_);
	font::move_floating_label(help_string_, 0.0, -double(rect.h));

	return help_string_;
}

void CVideo::clear_help_string(int handle)
{
	if(handle == help_string_) {
		font::remove_floating_label(handle);
		help_string_ = 0;
	}
}

void CVideo::clear_all_help_strings()
{
	clear_help_string(help_string_);
}

void CVideo::set_fullscreen(bool ison)
{
	if(window && is_fullscreen() != ison) {
		const point& res = preferences::resolution();

		MODE_EVENT mode;

		if(ison) {
			mode = TO_FULLSCREEN;
		} else {
			mode = preferences::maximized() ? TO_MAXIMIZED_WINDOW : TO_WINDOWED;
		}

		set_window_mode(mode, res);

		if(display* d = display::get_singleton()) {
			d->redraw_everything();
		}
	}

	// Change the config value.
	preferences::_set_fullscreen(ison);
}

void CVideo::toggle_fullscreen()
{
	set_fullscreen(!preferences::fullscreen());
}

bool CVideo::set_resolution(const unsigned width, const unsigned height)
{
	return set_resolution(point(width, height));
}

bool CVideo::set_resolution(const point& resolution)
{
	if(resolution == current_resolution()) {
		return false;
	}

	set_window_mode(TO_RES, resolution);

	if(display* d = display::get_singleton()) {
		d->redraw_everything();
	}

	// Change the saved values in preferences.
	LOG_DP << "updating resolution to " << resolution << std::endl;
	preferences::_set_resolution(resolution);
	preferences::_set_maximized(false);

	// Push a window-resized event to the queue. This is necessary so various areas
	// of the game (like GUI2) update properly with the new size.
	events::raise_resize_event();

	return true;
}

void CVideo::update_buffers()
{
	LOG_DP << "updating buffers" << std::endl;
	// We could also double-check the resolution here.
	/*if (preferences::resolution() != current_resolution()) {
		LOG_DP << "updating resolution from " << current_resolution()
			<< " to " << preferences::resolution() << std::endl;
		set_window_mode(TO_RES, preferences::resolution());
	}*/

	update_framebuffer();

	if(display* d = display::get_singleton()) {
		d->redraw_everything();
	}

	// Push a window-resized event to the queue. This is necessary so various areas
	// of the game (like GUI2) update properly with the new size.
	events::raise_resize_event();
}

void CVideo::lock_flips(bool lock)
{
	if(lock) {
		++flip_locked_;
	} else {
		--flip_locked_;
	}
}
