/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "game_config.hpp"
#include "log.hpp"
#include "ogl/utils.hpp"
#include "preferences/general.hpp"
#include "sdl/point.hpp"
#include "sdl/render_utils.hpp"
#include "sdl/texture.hpp"
#include "sdl/userevent.hpp"
#include "sdl/utils.hpp"
#include "sdl/window.hpp"

#include <cassert>
#include <vector>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

#define MAGIC_DPI_SCALE_NUMBER 96

CVideo* CVideo::singleton_ = nullptr;

namespace
{
bool interactive = true;
}

CVideo::CVideo(FAKE_TYPES type)
	: window()
#ifdef USE_GL_RENDERING
	, gl_context()
#endif
	, fake_screen_(false)
	, fake_size_(0u, 0u)
	, updated_locked_(0)
	, flip_locked_(0)
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
	return !interactive || window == nullptr;
}

void CVideo::video_event_handler::handle_window_event(const SDL_Event& event)
{
	if(event.type == SDL_WINDOWEVENT) {
		switch(event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
			break;
		}
	}
}

void CVideo::make_fake()
{
	fake_screen_ = true;
	fake_size_ = { 1024, 768 };
}

void CVideo::make_test_fake(const unsigned width, const unsigned height)
{
	interactive = false;
	fake_size_.first = width;
	fake_size_.second = height;
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
#ifdef USE_GL_RENDERING
	video_flags |= SDL_WINDOW_OPENGL;
#endif

	if(preferences::fullscreen()) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if(preferences::maximized()) {
		window_flags |= SDL_WINDOW_MAXIMIZED;
	}

	// Initialize window
	window.reset(new sdl::window("", x, y, w, h, window_flags, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE));

	std::cerr << "Setting mode to " << w << "x" << h << std::endl;

	window->set_minimum_size(preferences::min_window_width, preferences::min_window_height);

#ifdef USE_GL_RENDERING
	// Initialize an OpenGL context for the window.
	gl_context.reset(new gl::context(window.get()));

	gl::clear_screen();
	render_screen();
#endif

	event_handler_.join_global();
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
}

SDL_Rect CVideo::screen_area(bool as_pixels) const
{
	if(!window) {
		return {0, 0,
			static_cast<int>(fake_size_.first),
			static_cast<int>(fake_size_.second)
		};
	}

	// First, get the renderer size in pixels.
	SDL_Point size = window->get_output_size();

	// Then convert the dimensions into screen coordinates, if applicable.
	if(!as_pixels) {
#ifdef __APPLE__
		size = window->get_size();
#else
		float scale_x, scale_y;
		std::tie(scale_x, scale_y) = get_dpi_scale_factor();

		size.x /= scale_x;
		size.y /= scale_y;
#endif
	}

	return {0, 0, size.x, size.y};
}

int CVideo::get_width(bool as_pixels) const
{
	return screen_area(as_pixels).w;
}

int CVideo::get_height(bool as_pixels) const
{
	return screen_area(as_pixels).h;
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

	if(window) {
		window->render();
	}
}

void CVideo::render_copy(
		const texture& txt, SDL_Rect* src_rect, SDL_Rect* dst_rect, const bool flip_h, const bool flip_v)
{
	if(!window || txt.null()) {
		return;
	}

	// If no additional data was provided, render immediately.
	if(!flip_h && !flip_v) {
		SDL_RenderCopy(*window, txt, src_rect, dst_rect);
		return;
	}

	// Calculate flipping mode.
	int fmode = SDL_FLIP_NONE;

	if(flip_h && flip_v) {
		fmode = SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL;
	} else if(flip_h) {
		fmode = SDL_FLIP_HORIZONTAL;
	} else if(flip_v) {
		fmode = SDL_FLIP_VERTICAL;
	}

	SDL_RendererFlip flip_mode = static_cast<SDL_RendererFlip>(fmode);

	// TODO: add handling of rotations.
	static const double rotate_angle = 0;
	static const SDL_Point* center = nullptr;

	SDL_RenderCopyEx(*window, txt, src_rect, dst_rect, rotate_angle, center, flip_mode);
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
	if(!window) {
		return nullptr;
	}

	return *window;
}

bool CVideo::window_has_flags(uint32_t flags) const
{
	if(!window) {
		return false;
	}

	return (window->get_flags() & flags) != 0;
}

std::pair<float, float> CVideo::get_dpi_scale_factor() const
{
	std::pair<float, float> result{1.0f, 1.0f};

	if(!window) {
		return result;
	}

	float hdpi, vdpi;
	SDL_GetDisplayDPI(window->get_display_index(), nullptr, &hdpi, &vdpi);

	result.first = hdpi / MAGIC_DPI_SCALE_NUMBER;
	result.second = vdpi / MAGIC_DPI_SCALE_NUMBER;

	return result;
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
	float scale_h, scale_v;
	std::tie(scale_h, scale_v) = get_dpi_scale_factor();
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

point CVideo::current_resolution()
{
	return point(window->get_size()); // Convert from plain SDL_Point
}

bool CVideo::is_fullscreen() const
{
	return (window->get_flags() & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
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

	// Change the saved values in preferences.
	preferences::_set_resolution(resolution);
	preferences::_set_maximized(false);

	// Push a window-resized event to the queue. This is necessary so various areas
	// of the game (like GUI2) update properly with the new size.
	events::raise_resize_event();

	return true;
}

void CVideo::lock_flips(bool lock)
{
	if(lock) {
		++flip_locked_;
	} else {
		--flip_locked_;
	}
}
