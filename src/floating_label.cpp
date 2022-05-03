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

#include "floating_label.hpp"

#include "display.hpp"
#include "font/text.hpp"
#include "log.hpp"
#include "video.hpp"

#include <map>
#include <set>
#include <stack>

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace
{
typedef std::map<int, font::floating_label> label_map;
label_map labels;
int label_id = 1;

std::stack<std::set<int>> label_contexts;
}

namespace font
{
floating_label::floating_label(const std::string& text, const surface& surf)
#if 0
	: img_(),
#else
	: tex_()
	, buf_()
	, buf_pos_()
#endif
	, draw_size_()
	, fadeout_(0)
	, time_start_(0)
	, text_(text)
	, font_size_(SIZE_SMALL)
	, color_(NORMAL_COLOR)
	, bgcolor_()
	, bgalpha_(0)
	, xpos_(0)
	, ypos_(0)
	, xmove_(0)
	, ymove_(0)
	, lifetime_(-1)
	, width_(-1)
	, height_(-1)
	, clip_rect_(CVideo::get_singleton().draw_area())
	, visible_(true)
	, align_(CENTER_ALIGN)
	, border_(0)
	, scroll_(ANCHOR_LABEL_SCREEN)
	, use_markup_(true)
{
	if (surf.get()) {
		tex_ = texture(surf);
		draw_size_ = {surf->w, surf->h};
	}
}

void floating_label::move(double xmove, double ymove)
{
	xpos_ += xmove;
	ypos_ += ymove;
}

int floating_label::xpos(std::size_t width) const
{
	int xpos = int(xpos_);
	if(align_ == font::CENTER_ALIGN) {
		xpos -= width / 2;
	} else if(align_ == font::RIGHT_ALIGN) {
		xpos -= width;
	}

	return xpos;
}

bool floating_label::create_texture()
{
	if(tex_ == nullptr) {
		DBG_FT << "creating floating label texture" << std::endl;
		font::pango_text& text = font::get_text_renderer();

		text.set_link_aware(false)
			.set_family_class(font::FONT_SANS_SERIF)
			.set_font_size(font_size_)
			.set_font_style(font::pango_text::STYLE_NORMAL)
			.set_alignment(PANGO_ALIGN_LEFT)
			.set_foreground_color(color_)
			.set_maximum_width(width_ < 0 ? clip_rect_.w : width_)
			.set_maximum_height(height_ < 0 ? clip_rect_.h : height_, true)
			.set_ellipse_mode(PANGO_ELLIPSIZE_END)
			.set_characters_per_line(0);

		// ignore last '\n'
		if(!text_.empty() && *(text_.rbegin()) == '\n') {
			text.set_text(std::string(text_.begin(), text_.end() - 1), use_markup_);
		} else {
			text.set_text(text_, use_markup_);
		}

		surface foreground = text.render();

		if(foreground == nullptr) {
			ERR_FT << "could not create floating label's text" << std::endl;
			return false;
		}

		// combine foreground text with its background
		if(bgalpha_ != 0) {
			// background is a dark tooltip box
			surface background(foreground->w + border_ * 2, foreground->h + border_ * 2);

			if(background == nullptr) {
				ERR_FT << "could not create tooltip box" << std::endl;
				tex_ = texture(foreground);
				draw_size_ = {foreground->w, foreground->h};
				return tex_ != nullptr;
			}

			uint32_t color = SDL_MapRGBA(foreground->format, bgcolor_.r, bgcolor_.g, bgcolor_.b, bgalpha_);
			sdl::fill_surface_rect(background, nullptr, color);

			// we make the text less transparent, because the blitting on the
			// dark background will darken the anti-aliased part.
			// This 1.13 value seems to restore the brightness of version 1.4
			// (where the text was blitted directly on screen)
			adjust_surface_alpha(foreground, floating_to_fixed_point(1.13));

			SDL_Rect r{border_, border_, 0, 0};
			adjust_surface_alpha(foreground, SDL_ALPHA_OPAQUE);
			sdl_blit(foreground, nullptr, background, &r);

			tex_ = texture(background);
			draw_size_ = {background->w, background->h};
		} else {
			// background is blurred shadow of the text
			surface background(foreground->w + 4, foreground->h + 4);
			sdl::fill_surface_rect(background, nullptr, 0);
			SDL_Rect r{2, 2, 0, 0};
			sdl_blit(foreground, nullptr, background, &r);
			background = shadow_image(background);

			if(background == nullptr) {
				ERR_FT << "could not create floating label's shadow" << std::endl;
				tex_ = texture(foreground);
				draw_size_ = {foreground->w, foreground->h};
				return tex_ != nullptr;
			}
			sdl_blit(foreground, nullptr, background, &r);
			tex_ = texture(background);
			draw_size_ = {background->w, background->h};
		}
	}

	return tex_ != nullptr;
}

void floating_label::draw(int time)
{
	if(!visible_) {
		buf_.reset();
		return;
	}

	if (!create_texture()) {
		return;
	}

	SDL_Point pos = get_loc(time);
	SDL_Rect draw_rect = {pos.x, pos.y, draw_size_.x, draw_size_.y};
	buf_pos_ = draw_rect;

	CVideo& video = CVideo::get_singleton();
	auto clipper = video.set_clip(clip_rect_);

	// Read buf_ back from the screen.
	// buf_pos_ will be intersected with the drawing area,
	// so might not match draw_rect after this.
	buf_ = video.read_texture(&buf_pos_);

	// Fade the label out according to the time.
	tex_.set_alpha_mod(get_alpha(time));

	// Apply the label texture to the screen.
	video.blit_texture(tex_, &draw_rect);
}

void floating_label::set_lifetime(int lifetime, int fadeout)
{
	lifetime_ = lifetime;
	fadeout_ = fadeout;
	time_start_	= SDL_GetTicks();
}


SDL_Point floating_label::get_loc(int time)
{
	int time_alive = get_time_alive(time);
	return {
		static_cast<int>(time_alive * xmove_ + xpos(draw_size_.x)),
		static_cast<int>(time_alive * ymove_ + ypos_)
	};
}

uint8_t floating_label::get_alpha(int time)
{
	if(lifetime_ >= 0 && fadeout_ > 0) {
		int time_alive = get_time_alive(time);
		if(time_alive >= lifetime_ && tex_ != nullptr) {
			// fade out moving floating labels
			int alpha_sub = 255 * (time_alive - lifetime_) / fadeout_;
			if (alpha_sub >= 255) {
				return 0;
			} else {
				return 255 - alpha_sub;
			}
		}
	}
	return 255;
}

void floating_label::undraw()
{
	if(buf_ == nullptr) {
		return;
	}

	CVideo& video = CVideo::get_singleton();
	auto clipper = video.set_clip(clip_rect_);
	video.blit_texture(buf_, &buf_pos_);
}

int add_floating_label(const floating_label& flabel)
{
	if(label_contexts.empty()) {
		return 0;
	}

	++label_id;
	labels.emplace(label_id, flabel);
	label_contexts.top().insert(label_id);
	return label_id;
}

void move_floating_label(int handle, double xmove, double ymove)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.move(xmove, ymove);
	}
}

void scroll_floating_labels(double xmove, double ymove)
{
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(i->second.scroll() == ANCHOR_LABEL_MAP) {
			i->second.move(xmove, ymove);
		}
	}
}

void remove_floating_label(int handle, int fadeout)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		if(fadeout > 0) {
			i->second.set_lifetime(0, fadeout);
			return;
		} else if(fadeout < 0) {
			i->second.set_lifetime(0, i->second.get_fade_time());
			return;
		}
		labels.erase(i);
	}

	if(!label_contexts.empty()) {
		label_contexts.top().erase(handle);
	}
}

void show_floating_label(int handle, bool value)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.show(value);
	}
}

SDL_Rect get_floating_label_rect(int handle)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		if (i->second.create_texture()) {
			SDL_Point size = i->second.get_draw_size();
			return {0, 0, size.x, size.y};
		}
	}
	return sdl::empty_rect;
}

floating_label_context::floating_label_context()
{
	//TODO: 'pause' floating labels in other contexrs
	label_contexts.emplace();
}

floating_label_context::~floating_label_context()
{
	//TODO: 'pause' floating labels in other contexrs
	const std::set<int>& context = label_contexts.top();

	while(!context.empty()) {
		// Remove_floating_label removes the passed label from the context.
		// This loop removes a different label in every iteration.
		remove_floating_label(*context.begin());
	}

	label_contexts.pop();
}

void draw_floating_labels()
{
	if(label_contexts.empty()) {
		return;
	}
	int time = SDL_GetTicks();

	const std::set<int>& context = label_contexts.top();

	// draw the labels in the order they were added, so later added labels (likely to be tooltips)
	// are displayed over earlier added labels.
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.draw(time);
		}
	}
}

void undraw_floating_labels()
{
	if(label_contexts.empty()) {
		return;
	}
	int time = SDL_GetTicks();

	std::set<int>& context = label_contexts.top();

	//undraw labels in reverse order, so that a LIFO process occurs, and the screen is restored
	//into the exact state it started in.
	for(label_map::reverse_iterator i = labels.rbegin(); i != labels.rend(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.undraw();
		}
	}

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(context.count(j->first) > 0 && j->second.expired(time)) {
			context.erase(j->first);
			labels.erase(j++);
		} else {
			++j;
		}
	}
}
}
