/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 *  @file
 *  Video-testprogram, standalone
 */

#include "global.hpp"

#include "font.hpp"
#include "floating_label.hpp"
#include "image.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sdl/utils.hpp"
#include "sdl/rect.hpp"
#include "sdl/window.hpp"
#include "video.hpp"
#include "sdl/gpu.hpp"
#include "display.hpp"

#include <boost/foreach.hpp>

#include <vector>
#include <map>
#include <algorithm>

#include <assert.h>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

CVideo* CVideo::singleton_ = NULL;

namespace {
#ifdef SDL_GPU
	GPU_Target *render_target_;
#endif
}

void resize_monitor::process(events::pump_info &info) {
	UNUSED(info);
}

resize_lock::resize_lock()
{

}

resize_lock::~resize_lock()
{
}

static unsigned int get_flags(unsigned int flags)
{
	/* The wanted flags for the render need to be evaluated for SDL2. */
#ifdef SDL_GPU
	flags |= SDL_OPENGLBLIT;
#endif

	flags |= SDL_WINDOW_RESIZABLE;

	return flags;
}


namespace {

surface frameBuffer = NULL;
bool fake_interactive = false;
}

namespace video2 {

std::list<events::sdl_handler *> draw_layers;

draw_layering::draw_layering(const bool auto_join) :
		sdl_handler(auto_join)
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

	for(std::list<events::sdl_handler*>::iterator it = draw_layers.begin(); it != draw_layers.end(); ++it) {
		(*it)->handle_window_event(event);
	}

	SDL_Event drawEvent;
	SDL_UserEvent data;

	data.type = DRAW_ALL_EVENT;
	data.code = 0;
	data.data1 = NULL;
	data.data2 = NULL;

	drawEvent.type = DRAW_ALL_EVENT;
	drawEvent.user = data;
	SDL_FlushEvent(DRAW_ALL_EVENT);
	SDL_PushEvent(&drawEvent);
}
}


bool CVideo::non_interactive()
{
	if (fake_interactive)
		return false;
	return window == NULL;
}


#ifdef SDL_GPU
GPU_Target *get_render_target()
{
	return render_target_;
}
#endif

surface display_format_alpha(surface surf)
{
		UNUSED(surf);
		return NULL;
}

surface& get_video_surface()
{
	return frameBuffer;
}

SDL_Rect screen_area()
{
	return sdl::create_rect(0, 0, frameBuffer->w, frameBuffer->h);
}

void update_rect(size_t x, size_t y, size_t w, size_t h)
{
	update_rect(sdl::create_rect(x, y, w, h));
}

void update_rect(const SDL_Rect& rect_value)
{

	SDL_Rect rect = rect_value;

	surface const fb = NULL;
	if(fb != NULL) {
		if(rect.x < 0) {
			if(rect.x*-1 >= int(rect.w))
				return;

			rect.w += rect.x;
			rect.x = 0;
		}

		if(rect.y < 0) {
			if(rect.y*-1 >= int(rect.h))
				return;

			rect.h += rect.y;
			rect.y = 0;
		}

		if(rect.x + rect.w > fb->w) {
			rect.w = fb->w - rect.x;
		}

		if(rect.y + rect.h > fb->h) {
			rect.h = fb->h - rect.y;
		}

		if(rect.x >= fb->w) {
			return;
		}

		if(rect.y >= fb->h) {
			return;
		}
	}
}

void CVideo::video_event_handler::handle_window_event(const SDL_Event &event)
{
	if (event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_EXPOSED:
				//if (display::get_singleton())
					//display::get_singleton()->redraw_everything();
				SDL_Event drawEvent;
				SDL_UserEvent data;

				data.type = DRAW_ALL_EVENT;
				data.code = 0;
				data.data1 = NULL;
				data.data2 = NULL;

				drawEvent.type = DRAW_ALL_EVENT;
				drawEvent.user = data;

				SDL_FlushEvent(DRAW_ALL_EVENT);
				SDL_PushEvent(&drawEvent);
				break;
		}
	}
}


CVideo::CVideo(FAKE_TYPES type) :
	window(),
#ifdef SDL_GPU
	shader_(),
#endif
	mode_changed_(false),
	fake_screen_(false),
	help_string_(0),
	updatesLocked_(0)
{
	assert(!singleton_);
	singleton_ = this;
	initSDL();
	switch(type)
	{
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
#ifdef SDL_GPU
	//800x600 is a dummy value, the actual resolution is set in setMode
	render_target_ = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);

	if(render_target_ == NULL) {
		ERR_DP << "Could not initialize window: " << SDL_GetError() << std::endl;
		throw CVideo::error();
	}

	const std::string vertex_src = game_config::path + "/data/shaders/default.vert";
	const std::string frag_src = game_config::path + "/data/shaders/default.frag";
	shader_ = sdl::shader_program(vertex_src, frag_src);
	shader_.activate();
#else
	const int res = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL_video: " << SDL_GetError() << std::endl;
		throw CVideo::error();
	}
#endif
}

#ifdef SDL_GPU
void CVideo::update_overlay(SDL_Rect *rect)
{
	sdl::timage img(overlay_);
	shader_.set_overlay(img);

	// Re-render the appropriate screen area so that overlay change is visible
	static sdl::timage empty(image::get_texture("images/misc/blank.png"));
	SDL_Rect whole = sdl::create_rect(0, 0, overlay_->w, overlay_->h);
	SDL_Rect *r = rect == NULL ? &whole : rect;
	empty.set_scale(float(r->w) / empty.base_width(), float(r->h) / empty.base_height());
	draw_texture(empty, r->x, r->y);
}
#endif

CVideo::~CVideo()
{
	LOG_DP << "calling SDL_Quit()\n";
	SDL_Quit();
	assert(singleton_);
	singleton_ = NULL;
	LOG_DP << "called SDL_Quit()\n";
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	surface& target(getSurface());
	SDL_Rect dst = sdl::create_rect(x, y, 0, 0);

	const clip_rect_setter clip_setter(target, clip_rect, clip_rect != NULL);
	sdl_blit(surf,srcrect,target,&dst);
}

#ifdef SDL_GPU
GPU_Target *CVideo::render_target() const
{
	return render_target_;
}

void CVideo::draw_texture(sdl::timage &texture, int x, int y)
{
	texture.draw(*this, x, y);
}

void CVideo::set_texture_color_modulation(int r, int g, int b, int a)
{
	shader_.set_color_mod(r, g, b, a);
}

void CVideo::set_texture_submerge(float f)
{
	shader_.set_submerge(f);
}

void CVideo::set_texture_effects(int effects)
{
	shader_.set_effects(effects);
}

void CVideo::blit_to_overlay(surface surf, int x, int y)
{
	if (x < 0 || y < 0 || x > overlay_->w || y > overlay_->h) {
		return;
	}
	SDL_Rect r = sdl::create_rect(x, y, surf->w, surf->h);
	SDL_BlitSurface(surf, NULL, overlay_, &r);
	update_overlay(&r);
}

void CVideo::clear_overlay_area(SDL_Rect area)
{
	const Uint32 color = SDL_MapRGBA(overlay_->format, 0, 0, 0, 0);
	Uint32 *pixels = static_cast<Uint32*>(overlay_->pixels);
	for (int x = area.x; x<area.x + area.w; ++x) {
		for (int y = area.y; y<area.y +area.h; ++y) {
			const int index = y * (area.w + overlay_->pitch) + x;
			pixels[index] = color;
		}
	}
	update_overlay(&area);
}

void CVideo::clear_overlay()
{
	overlay_ = create_compatible_surface(overlay_, getx(), gety());
	update_overlay();
}
#endif

void CVideo::make_fake()
{
	fake_screen_ = true;
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,16,16,24,0xFF0000,0xFF00,0xFF,0);
	image::set_pixel_format(frameBuffer->format);
}

void CVideo::make_test_fake(const unsigned width,
			const unsigned height, const unsigned bpp)
{
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, bpp, 0xFF0000, 0xFF00, 0xFF, 0);
	image::set_pixel_format(frameBuffer->format);

	fake_interactive = true;

}



void CVideo::update_framebuffer()
{
	if (!window)
		return;

	surface fb = SDL_GetWindowSurface(*window);
	if (!frameBuffer)
		frameBuffer = fb;
	else
		frameBuffer.assign(fb);
}

/**
 * Creates a new window instance.
 */
bool CVideo::init_window()
{
	// Position
	const int x = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;
	const int y = preferences::fullscreen() ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED;

	// Dimensions
	const int w = preferences::resolution().first;
	const int h = preferences::resolution().second;

	// Video flags
	int video_flags = 0;

	video_flags = get_flags(video_flags);

	if (preferences::fullscreen()) {
		video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if (preferences::maximized()) {
		video_flags |= SDL_WINDOW_MAXIMIZED;
	}

	// Initialize window
	window.reset(new sdl::twindow("", x, y, w, h, video_flags, SDL_RENDERER_SOFTWARE));

	std::cerr << "Setting mode to " << w << "x" << h << std::endl;

	window->set_minimum_size(
		preferences::min_allowed_width(),
		preferences::min_allowed_height()
	);

	event_handler_.join_global();

	update_framebuffer();
	if(frameBuffer) {
		image::set_pixel_format(frameBuffer->format);
	}

	return true;
}

void CVideo::setMode(int x, int y, const MODE_EVENT mode)
{
	assert(window);
	if (fake_screen_) return;
	mode_changed_ = true;

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

bool CVideo::modeChanged()
{
	bool ret = mode_changed_;
	mode_changed_ = false;
	return ret;
}

int CVideo::getx() const
{
#ifdef SDL_GPU
	return GPU_GetContextTarget()->w;
#else
	return frameBuffer->w;
#endif
}

int CVideo::gety() const
{
#ifdef SDL_GPU
	return GPU_GetContextTarget()->h;
#else
	return frameBuffer->h;
#endif
}

void CVideo::delay(unsigned int milliseconds)
{
	if (!game_config::no_delay)
		SDL_Delay(milliseconds);
}

void CVideo::flip()
{
	if(fake_screen_)
		return;
#ifdef SDL_GPU
	assert(render_target_);
	GPU_Flip(render_target_);
#else
	if (window)
		window->render();
#endif
}

void CVideo::lock_updates(bool value)
{
	if(value == true)
		++updatesLocked_;
	else
		--updatesLocked_;
}

bool CVideo::update_locked() const
{
	return updatesLocked_ > 0;
}

Uint8 CVideo::window_state()
{
	Uint8 state = 0;
	Uint32 flags = 0;

	if(!window) {
		return state;
	}

	flags = SDL_GetWindowFlags(*window);
	if ((flags & SDL_WINDOW_SHOWN) && !(flags & SDL_WINDOW_MINIMIZED)) {
		state |= SDL_APPACTIVE;
	}
	if (flags & SDL_WINDOW_INPUT_FOCUS) {
		state |= SDL_APPINPUTFOCUS;
	}
	if (flags & SDL_WINDOW_MOUSE_FOCUS) {
		state |= SDL_APPMOUSEFOCUS;
	}
	if (flags & SDL_WINDOW_MAXIMIZED) {
		state |= SDL_WINDOW_MAXIMIZED;
	}
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		state |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	return state;
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

sdl::twindow *CVideo::get_window()
{
	return window.get();
}


static int sdl_display_index(sdl::twindow* window)
{
	if(window) {
		return SDL_GetWindowDisplayIndex(*window);
	}
	return 0;
}

std::vector<std::pair<int, int> > CVideo::get_available_resolutions(const bool include_current)
{
	std::vector<std::pair<int, int> > result;

	const int modes = SDL_GetNumDisplayModes(sdl_display_index(window.get()));
	if (modes <= 0) {
		std::cerr << "No modes supported\n";
		return result;
	}

	const std::pair<int,int> min_res = std::make_pair(preferences::min_allowed_width(),preferences::min_allowed_height());

	SDL_DisplayMode mode;
	for (int i = 0; i < modes; ++i) {
		if(SDL_GetDisplayMode(0, i, &mode) == 0) {
			if (mode.w >= min_res.first && mode.h >= min_res.second)
				result.push_back(std::make_pair(mode.w, mode.h));
		}
	}

	if (std::find(result.begin(), result.end(), min_res) == result.end()) {
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

	const SDL_Color color = { 0, 0, 0, 0xbb };

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

	if (window && isFullScreen() != ison) {

		const std::pair<int,int>& res = preferences::resolution();

		MODE_EVENT mode;

		if (ison) {
			mode = TO_FULLSCREEN;
		} else {
			mode = preferences::maximized() ? TO_MAXIMIZED_WINDOW : TO_WINDOWED;
		}

		setMode(res.first, res.second, mode);

		if (display::get_singleton()) {
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
	if (static_cast<unsigned int> (current_resolution().first)  == width &&
		static_cast<unsigned int> (current_resolution().second) == height) {

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
