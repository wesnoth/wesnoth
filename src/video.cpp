/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "image.hpp"
#include "log.hpp"
#include "ogl/utils.hpp"
#include "preferences/general.hpp"
#include "sdl/render_utils.hpp"
#include "sdl/texture.hpp"
#include "sdl/window.hpp"

#include <cassert>
#include <vector>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

#define MAGIC_DPI_SCALE_NUMBER 96

CVideo* CVideo::singleton_ = nullptr;

namespace {
	surface frameBuffer = nullptr;
	bool fake_interactive = false;
}

namespace video2 {

std::list<events::sdl_handler*> draw_layers;

draw_layering::draw_layering(const bool auto_join)
	: sdl_handler(auto_join)
{
	draw_layers.push_back(this);
}

draw_layering::~draw_layering()
{
	draw_layers.remove(this);
}

} // video2

CVideo::CVideo(FAKE_TYPES type)
	: window()
#ifdef USE_GL_RENDERING
	, gl_context()
#endif
	, fake_screen_(false)
	, help_string_(0)
	, updatesLocked_(0)
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
	return fake_interactive ? false : (window == nullptr);
}

SDL_Rect screen_area()
{
	sdl::window* w = CVideo::get_singleton().get_window();
	if(!w) {
		return sdl::empty_rect;
	}

	SDL_Point size = w->get_size();
	return {0, 0, size.x, size.y};
}

void CVideo::video_event_handler::handle_window_event(const SDL_Event &event)
{
	if(event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_EXPOSED:
				break;
		}
	}
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	texture txt(surf);

	render_clip_rect_setter crs(clip_rect);

	SDL_Rect dst {x, y, surf->w, surf->h};
	render_copy(txt, srcrect, &dst);
}

void CVideo::make_fake()
{
	fake_screen_ = true;
}

void CVideo::make_test_fake()
{
	fake_interactive = true;
}

/**
 * Creates a new window instance.
 */
void CVideo::init_window()
{
	// Position
	const int x = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;
	const int y = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;

	// Dimensions
	const int w = preferences::resolution().first;
	const int h = preferences::resolution().second;

	// Video flags
	int video_flags = 0;

	// Add any more default flags here
	video_flags |= SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef USE_GL_RENDERING
	video_flags |= SDL_WINDOW_OPENGL;
#endif

	if(preferences::fullscreen()) {
		video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if(preferences::maximized()) {
		video_flags |= SDL_WINDOW_MAXIMIZED;
	}

	// Initialize window
	window.reset(new sdl::window("", x, y, w, h, video_flags, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC));

	std::cerr << "Setting mode to " << w << "x" << h << std::endl;

	window->set_minimum_size(
		preferences::min_window_width,
		preferences::min_window_height
	);

#ifdef USE_GL_RENDERING
	// Initialize an OpenGL context for the window.
	gl_context.reset(new gl::context(window.get()));

	gl::clear_screen();
	render_screen();
#endif

	event_handler_.join_global();
}

void CVideo::setMode(int x, int y, const MODE_EVENT mode)
{
	assert(window);
	if(fake_screen_) return;

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
			window->set_size(x, y);
			window->center();
			break;
	}
}

int CVideo::getx() const
{
	if(!window) {
		return 0;
	}

	return window->get_size().x;
}

int CVideo::gety() const
{
	if(!window) {
		return 0;
	}

	return window->get_size().y;
}

SDL_Renderer* CVideo::get_renderer()
{
	if(!window) {
		return nullptr;
	}

	return *window;
}

void CVideo::delay(unsigned int milliseconds)
{
	if(!game_config::no_delay) {
		SDL_Delay(milliseconds);
	}
}

void CVideo::render_copy(
		const texture& txt, SDL_Rect* src_rect, SDL_Rect* dst_rect, const bool flip_h, const bool flip_v)
{
	if(!window) {
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

void CVideo::render_screen()
{
	if(fake_screen_ || flip_locked_ > 0) {
		return;
	}

	if(window) {
		window->render();
	}
}

void CVideo::lock_updates(bool value)
{
	if(value == true) {
		++updatesLocked_;
	} else {
		--updatesLocked_;
	}
}

bool CVideo::update_locked() const
{
	return updatesLocked_ > 0;
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

sdl::window *CVideo::get_window()
{
	return window.get();
}

std::pair<float, float> CVideo::get_dpi_scale_factor() const
{
	std::pair<float, float> result {1.0f, 1.0f};

	if(!window) {
		return result;
	}

/* This check is here because Travis CI builds with SDL 2.0.2.
 * The code isn't intended to be skipped: end users, distribution packagers etc.
 * should build the game with SDL 2.0.4 or above.
 * - Jyrki, 2017-06-03 */
#if SDL_VERSION_ATLEAST(2, 0, 4)
	float hdpi, vdpi;
	SDL_GetDisplayDPI(window->get_display_index(), nullptr, &hdpi, &vdpi);

	result.first = hdpi / MAGIC_DPI_SCALE_NUMBER;
	result.second = vdpi / MAGIC_DPI_SCALE_NUMBER;
#endif

	return result;
}

std::vector<std::pair<int, int>> CVideo::get_available_resolutions(const bool include_current)
{
	std::vector<std::pair<int, int>> result;

	if(!window) {
		return result;
	}

	const int display_index = window->get_display_index();

	const int modes = SDL_GetNumDisplayModes(display_index);
	if(modes <= 0) {
		std::cerr << "No modes supported\n";
		return result;
	}

	const std::pair<int,int> min_res = std::make_pair(preferences::min_window_width, preferences::min_window_height);
	const std::pair<int,int> current_res = current_resolution();

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

			if(mode.w >= min_res.first && mode.h >= min_res.second) {
				result.emplace_back(mode.w, mode.h);
			}
		}
	}

	if(std::find(result.begin(), result.end(), min_res) == result.end()) {
		result.push_back(min_res);
	}

	if(include_current) {
		result.push_back(current_res);
	}

	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());

	return result;
}

// TODO: REMOVE ASAP
surface& CVideo::getSurface()
{
	return frameBuffer;
}

std::pair<int,int> CVideo::current_resolution()
{
	SDL_Point size = window->get_size();

	return std::make_pair(size.x, size.y);
}

bool CVideo::isFullScreen() const {
	return (window->get_flags() & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const color_t color { 0, 0, 0, 0xbb };

	int size = font::SIZE_LARGE;

	while(size > 0) {
		if(font::line_width(str, size) > getx()) {
			size--;
		} else {
			break;
		}
	}

	const int border = 5;

	font::floating_label flabel(str);
	flabel.set_font_size(size);
	flabel.set_position(getx()/2, gety());
	flabel.set_bg_color(color);
	flabel.set_border_size(border);

	help_string_ = font::add_floating_label(flabel);

	const SDL_Rect& rect = font::get_floating_label_rect(help_string_);
	font::move_floating_label(help_string_,0.0,-double(rect.h));
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

	if(window && isFullScreen() != ison) {

		const std::pair<int,int>& res = preferences::resolution();

		MODE_EVENT mode;

		if(ison) {
			mode = TO_FULLSCREEN;
		} else {
			mode = preferences::maximized() ? TO_MAXIMIZED_WINDOW : TO_WINDOWED;
		}

		setMode(res.first, res.second, mode);

		if(display::get_singleton()) {
			display::get_singleton()->redraw_everything();
		}
	}

	// Change the config value.
	preferences::_set_fullscreen(ison);
}

void CVideo::set_resolution(const std::pair<int,int>& resolution)
{
	set_resolution(resolution.first, resolution.second);
}

void CVideo::set_resolution(const unsigned width, const unsigned height)
{
	if(static_cast<unsigned int>(current_resolution().first)  == width &&
		static_cast<unsigned int>(current_resolution().second) == height) {

		return;
	}

	setMode(width, height, TO_RES);

	if(display::get_singleton()) {
		display::get_singleton()->redraw_everything();
	}

	// Change the config values.
	preferences::_set_resolution(std::make_pair(width, height));
	preferences::_set_maximized(false);
}

void CVideo::lock_flips(bool lock) {
	if(lock) {
		++flip_locked_;
	} else {
		--flip_locked_;
	}
}
