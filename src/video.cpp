/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file video.cpp
 *  Video-testprogram, standalone
 */

#include "global.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "image.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "video.hpp"

#include <vector>
#include <map>
#include <algorithm>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

namespace {
	bool fullScreen = false;
	int disallow_resize = 0;
}
void resize_monitor::process(events::pump_info &info) {
	if(info.resize_dimensions.first >= preferences::min_allowed_width()
	&& info.resize_dimensions.second >= preferences::min_allowed_height()
	&& disallow_resize == 0) {
		preferences::set_resolution(info.resize_dimensions);
	}
}

resize_lock::resize_lock()
{
	++disallow_resize;
}

resize_lock::~resize_lock()
{
	--disallow_resize;
}

static unsigned int get_flags(unsigned int flags)
{
	// SDL under Windows doesn't seem to like hardware surfaces
	// for some reason.
#if !(defined(_WIN32) || defined(__APPLE__) || defined(__AMIGAOS4__))
		flags |= SDL_HWSURFACE;
#endif
	if((flags&SDL_FULLSCREEN) == 0)
		flags |= SDL_RESIZABLE;

	return flags;
}

namespace {
struct event {
	int x, y, w, h;
	bool in;
	event(const SDL_Rect& rect, bool i) : x(i ? rect.x : rect.x + rect.w), y(rect.y), w(rect.w), h(rect.h), in(i) { }
};
bool operator<(const event& a, const event& b) {
	if (a.x != b.x) return a.x < b.x;
	if (a.in != b.in) return a.in;
	if (a.y != b.y) return a.y < b.y;
	if (a.h != b.h) return a.h < b.h;
	if (a.w != b.w) return a.w < b.w;
	return false;
}
bool operator==(const event& a, const event& b) {
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h && a.in == b.in;
}

struct segment {
	int x, count;
	segment() : x(0), count(0) { }
	segment(int x, int count) : x(x), count(count) { }
};


std::vector<SDL_Rect> update_rects;
std::vector<event> events;
std::map<int, segment> segments;

static void calc_rects()
{
	events.clear();

	BOOST_FOREACH (SDL_Rect const &rect, update_rects) {
		events.push_back(event(rect, true));
		events.push_back(event(rect, false));
	}

	std::sort(events.begin(), events.end());
	std::vector<event>::iterator events_end = std::unique(events.begin(), events.end());

	segments.clear();
	update_rects.clear();

	for (std::vector<event>::iterator iter = events.begin(); iter != events_end; ++iter) {
		std::map<int, segment>::iterator lower = segments.find(iter->y);
		if (lower == segments.end()) {
			lower = segments.insert(std::make_pair(iter->y, segment())).first;
			if (lower != segments.begin()) {
				std::map<int, segment>::iterator prev = lower;
				--prev;
				lower->second = prev->second;
			}
		}

		if (lower->second.count == 0) {
			lower->second.x = iter->x;
		}

		std::map<int, segment>::iterator upper = segments.find(iter->y + iter->h);
		if (upper == segments.end()) {
			upper = segments.insert(std::make_pair(iter->y + iter->h, segment())).first;
			std::map<int, segment>::iterator prev = upper;
			--prev;
			upper->second = prev->second;
		}

		if (iter->in) {
			while (lower != upper) {
				++lower->second.count;
				++lower;
			}
		} else {
			while (lower != upper) {
				lower->second.count--;
				if (lower->second.count == 0) {
					std::map<int, segment>::iterator next = lower;
					++next;

					int x = lower->second.x, y = lower->first;
					unsigned w = iter->x - x;
					unsigned h = next->first - y;
					SDL_Rect a = {x, y, w, h};

					if (update_rects.empty()) {
						update_rects.push_back(a);
					} else {
						SDL_Rect& p = update_rects.back(), n;
						int pa = p.w * p.h, aa = w * h, s = pa + aa;
						int thresh = 51;

						n.w = std::max<int>(x + w, p.x + p.w);
						n.x = std::min<int>(p.x, x);
						n.w -= n.x;
						n.h = std::max<int>(y + h, p.y + p.h);
						n.y = std::min<int>(p.y, y);
						n.h -= n.y;

						if (s * 100 < thresh * n.w * n.h) {
							update_rects.push_back(a);
						} else {
							p = n;
						}
					}

					if (lower == segments.begin()) {
						segments.erase(lower);
					} else {
						std::map<int, segment>::iterator prev = lower;
						--prev;
						if (prev->second.count == 0) segments.erase(lower);
					}

					lower = next;
				} else {
					++lower;
				}
			}
		}
	}
}


bool update_all = false;
}

static void clear_updates()
{
	update_all = false;
	update_rects.clear();
}

namespace {

surface frameBuffer = NULL;
bool fake_interactive = false;
}

bool non_interactive()
{
	if (fake_interactive)
		return false;
	return SDL_GetVideoSurface() == NULL;
}

surface display_format_alpha(surface surf)
{
	if(SDL_GetVideoSurface() != NULL)
		return SDL_DisplayFormatAlpha(surf);
	else if(frameBuffer != NULL)
		return SDL_ConvertSurface(surf,frameBuffer->format,0);
	else
		return NULL;
}

surface get_video_surface()
{
	return frameBuffer;
}

SDL_Rect screen_area()
{
	const SDL_Rect res = {0,0,frameBuffer->w,frameBuffer->h};
	return res;
}

void update_rect(size_t x, size_t y, size_t w, size_t h)
{
	const SDL_Rect rect = {x,y,w,h};
	update_rect(rect);
}

void update_rect(const SDL_Rect& rect_value)
{
	if(update_all)
		return;

	SDL_Rect rect = rect_value;

	surface const fb = SDL_GetVideoSurface();
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

	update_rects.push_back(rect);
}

void update_whole_screen()
{
	update_all = true;
}
CVideo::CVideo(FAKE_TYPES type) : mode_changed_(false), bpp_(0), fake_screen_(false), help_string_(0), updatesLocked_(0)
{
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
	const int res = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL_video: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}
}

CVideo::~CVideo()
{
	LOG_DP << "calling SDL_Quit()\n";
	SDL_Quit();
	LOG_DP << "called SDL_Quit()\n";
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	const surface target(getSurface());
	SDL_Rect dst = {x,y,0,0};

	if(clip_rect != NULL) {
		const clip_rect_setter clip_setter(target,*clip_rect);
		SDL_BlitSurface(surf,srcrect,target,&dst);
	} else {
		SDL_BlitSurface(surf,srcrect,target,&dst);
	}
}

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

int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, get_flags(flags) );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	update_rects.clear();
	if (fake_screen_) return 0;
	mode_changed_ = true;

	flags = get_flags(flags);
	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

	if( res == 0 )
		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags );

	if( frameBuffer != NULL ) {
		image::set_pixel_format(frameBuffer->format);
		return bits_per_pixel;
	} else	return 0;
}

bool CVideo::modeChanged()
{
	bool ret = mode_changed_;
	mode_changed_ = false;
	return ret;
}

int CVideo::getx() const
{
	return frameBuffer->w;
}

int CVideo::gety() const
{
	return frameBuffer->h;
}

void CVideo::flip()
{
	if(fake_screen_)
		return;

	if(update_all) {
		::SDL_Flip(frameBuffer);
	} else if(update_rects.empty() == false) {
		calc_rects();
		if(!update_rects.empty()) {
			SDL_UpdateRects(frameBuffer, update_rects.size(), &update_rects[0]);
		}
	}

	clear_updates();
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

surface CVideo::getSurface()
{
	return frameBuffer;
}

bool CVideo::isFullScreen() const { return fullScreen; }

void CVideo::setBpp( int bpp )
{
	bpp_ = bpp;
}

int CVideo::getBpp()
{
	return bpp_;
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const SDL_Color colour = { 0, 0, 0, 0xbb };

#ifdef USE_TINY_GUI
	int size = font::SIZE_NORMAL;
#else
	int size = font::SIZE_LARGE;
#endif

	while(size > 0) {
		if(font::line_width(str, size) > getx()) {
			size--;
		} else {
			break;
		}
	}

#ifdef USE_TINY_GUI
	const int border = 2;
#else
	const int border = 5;
#endif

	help_string_ = font::add_floating_label(str,size,font::NORMAL_COLOUR,getx()/2,gety(),0.0,0.0,-1,screen_area(),font::CENTER_ALIGN,&colour,border);
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
