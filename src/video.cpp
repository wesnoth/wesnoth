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
#include "preferences/general.hpp"
#include "sdl/window.hpp"

#include <cassert>
#include <vector>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

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

	video2::trigger_full_redraw();
}

void trigger_full_redraw() {
	SDL_Event event;
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.data1 = (*frameBuffer).h;
	event.window.data2 = (*frameBuffer).w;

	for(const auto& layer : draw_layers) {
		layer->handle_window_event(event);
	}

	SDL_Event drawEvent;
	SDL_UserEvent data;

	data.type = DRAW_ALL_EVENT;
	data.code = 0;
	data.data1 = nullptr;
	data.data2 = nullptr;

	drawEvent.type = DRAW_ALL_EVENT;
	drawEvent.user = data;
	SDL_FlushEvent(DRAW_ALL_EVENT);
	SDL_PushEvent(&drawEvent);
}

} // video2

CVideo::CVideo(FAKE_TYPES type)
	: window()
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
	const int res = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

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
	return {0, 0, frameBuffer->w, frameBuffer->h};
}

void CVideo::video_event_handler::handle_window_event(const SDL_Event &event)
{
	if(event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_EXPOSED:
				//if(display::get_singleton())
					//display::get_singleton()->redraw_everything();
				SDL_Event drawEvent;
				SDL_UserEvent data;

				data.type = DRAW_ALL_EVENT;
				data.code = 0;
				data.data1 = nullptr;
				data.data2 = nullptr;

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
	surface& target(getSurface());
	SDL_Rect dst {x, y, 0, 0};

	const clip_rect_setter clip_setter(target, clip_rect, clip_rect != nullptr);
	sdl_blit(surf,srcrect,target,&dst);
}

void CVideo::make_fake()
{
	fake_screen_ = true;
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 16, 24, 0xFF0000, 0xFF00, 0xFF, 0);
	image::set_pixel_format(frameBuffer->format);
}

void CVideo::make_test_fake(const unsigned width, const unsigned height, const unsigned bpp)
{
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bpp, 0xFF0000, 0xFF00, 0xFF, 0);
	image::set_pixel_format(frameBuffer->format);

	fake_interactive = true;
}

void CVideo::update_framebuffer()
{
	if(!window) {
		return;
	}

	surface fb = SDL_GetWindowSurface(*window);
	if(!frameBuffer) {
		frameBuffer = fb;
	} else {
		frameBuffer.assign(fb);
	}
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
	video_flags |= SDL_WINDOW_RESIZABLE;

	if(preferences::fullscreen()) {
		video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if(preferences::maximized()) {
		video_flags |= SDL_WINDOW_MAXIMIZED;
	}

	// Initialize window
	window.reset(new sdl::window("", x, y, w, h, video_flags, SDL_RENDERER_SOFTWARE));

	std::cerr << "Setting mode to " << w << "x" << h << std::endl;

	window->set_minimum_size(
		preferences::min_window_width,
		preferences::min_window_height
	);

	event_handler_.join_global();

	update_framebuffer();
	if(frameBuffer) {
		image::set_pixel_format(frameBuffer->format);
	}
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

	update_framebuffer();
	if(frameBuffer) {
		image::set_pixel_format(frameBuffer->format);
	}
}

int CVideo::getx() const
{
	return frameBuffer->w;
}

int CVideo::gety() const
{
	return frameBuffer->h;
}

void CVideo::delay(unsigned int milliseconds)
{
	if(!game_config::no_delay) {
		SDL_Delay(milliseconds);
	}
}

void CVideo::flip()
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

sdl::window *CVideo::get_window()
{
	return window.get();
}

std::vector<std::pair<int, int>> CVideo::get_available_resolutions(const bool include_current)
{
	std::vector<std::pair<int, int>> result;

	if(!window) {
		return result;
	}

	const int modes = SDL_GetNumDisplayModes(window->get_display_index());
	if(modes <= 0) {
		std::cerr << "No modes supported\n";
		return result;
	}

	const std::pair<int,int> min_res = std::make_pair(preferences::min_window_width, preferences::min_window_height);

	SDL_DisplayMode mode;
	for (int i = 0; i < modes; ++i) {
		if(SDL_GetDisplayMode(0, i, &mode) == 0) {
			if(mode.w >= min_res.first && mode.h >= min_res.second)
				result.emplace_back(mode.w, mode.h);
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

surface& CVideo::getSurface()
{
	return frameBuffer;
}

std::pair<int,int> CVideo::current_resolution()
{
	return std::make_pair(getSurface()->w, getSurface()->h);
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
