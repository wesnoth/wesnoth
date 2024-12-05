/*
	Copyright (C) 2003 - 2024
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

#include "draw.hpp"
#include "draw_manager.hpp"
#include "font/standard_colors.hpp"
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

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace
{
typedef std::map<int, font::floating_label> label_map;
label_map labels;
int label_id = 1;

std::stack<std::set<int>> label_contexts;

}

using namespace std::chrono_literals;

namespace font
{
floating_label::floating_label(const std::string& text)
	: tex_()
	, screen_loc_()
	, alpha_(0)
	, fadeout_(0)
	, time_start_()
	, text_(text)
	, font_size_(SIZE_SMALL)
	, color_(NORMAL_COLOR)
	, bgcolor_(0, 0, 0, SDL_ALPHA_TRANSPARENT)
	, xpos_(0)
	, ypos_(0)
	, xmove_(0)
	, ymove_(0)
	, lifetime_(-1)
	, width_(-1)
	, height_(-1)
	, clip_rect_(video::game_canvas())
	, visible_(true)
	, align_(CENTER_ALIGN)
	, border_(0)
	, scroll_(ANCHOR_LABEL_SCREEN)
	, use_markup_(true)
{
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

rect floating_label::get_bg_rect(const rect& text_rect) const
{
	return text_rect.padded_by(border_);
}

void floating_label::clear_texture()
{
	tex_.reset();
}

bool floating_label::create_texture()
{
	if(video::headless()) {
		return false;
	}

	if(tex_ != nullptr) {
		// Already have a texture
		return true;
	}

	if(text_.empty()) {
		// Empty labels are unfortunately still used sometimes
		return false;
	}

	DBG_FT << "creating floating label texture, text: " << text_.substr(0,15);
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
		.set_characters_per_line(0)
		.set_add_outline(bgcolor_.a == 0);

	// ignore last '\n'
	if(!text_.empty() && *(text_.rbegin()) == '\n') {
		text.set_text(std::string(text_.begin(), text_.end() - 1), use_markup_);
	} else {
		text.set_text(text_, use_markup_);
	}

	tex_ = text.render_and_get_texture();
	if(!tex_) {
		ERR_FT << "could not create floating label's text";
		return false;
	}

	return true;
}

void floating_label::undraw()
{
	DBG_FT << "undrawing floating label from " << screen_loc_;
	draw_manager::invalidate_region(get_bg_rect(screen_loc_));
	screen_loc_ = {};
}

void floating_label::update(const clock::time_point& time)
{
	if(video::headless() || text_.empty()) {
		return;
	}

	if(!create_texture()) {
		ERR_FT << "failed to create texture for floating label";
		return;
	}

	point new_pos = get_pos(time);
	rect draw_loc {new_pos.x, new_pos.y, tex_.w(), tex_.h()};

	uint8_t new_alpha = get_alpha(time);

	// Invalidate former draw loc
	draw_manager::invalidate_region(get_bg_rect(screen_loc_));

	// Invalidate new draw loc in preparation
	draw_manager::invalidate_region(get_bg_rect(draw_loc));

	DBG_FT << "updating floating label from " << screen_loc_ << " to " << draw_loc;

	screen_loc_ = draw_loc;
	alpha_ = new_alpha;
}

void floating_label::draw()
{
	if(!visible_) {
		screen_loc_ = {};
		return;
	}

	if(screen_loc_.empty()) {
		return;
	}

	if(!tex_) {
		ERR_DP << "trying to draw floating label with no texture!";
		return;
	}

	if(!screen_loc_.overlaps(draw::get_clip().intersect(clip_rect_))) {
		return;
	}

	DBG_FT << "drawing floating label to " << screen_loc_;

	// Clip if appropriate.
	auto clipper = draw::reduce_clip(clip_rect_);

	// Draw background, if appropriate
	if(bgcolor_.a != 0) {
		draw::fill(get_bg_rect(screen_loc_), bgcolor_);
	}

	// Apply the label texture to the screen.
	tex_.set_alpha_mod(alpha_);
	draw::blit(tex_, screen_loc_);
}

void floating_label::set_lifetime(const std::chrono::milliseconds& lifetime, const std::chrono::milliseconds& fadeout)
{
	lifetime_ = lifetime;
	fadeout_ = fadeout;
	time_start_	= std::chrono::steady_clock::now();
}

std::chrono::milliseconds floating_label::get_time_alive(const clock::time_point& current_time) const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - time_start_);
}

point floating_label::get_pos(const clock::time_point& time)
{
	auto time_alive = get_time_alive(time);
	return {
		static_cast<int>(time_alive.count() * xmove_ + xpos(tex_.w())),
		static_cast<int>(time_alive.count() * ymove_ + ypos_)
	};
}

uint8_t floating_label::get_alpha(const clock::time_point& time)
{
	if(lifetime_ >= 0ms && fadeout_ > 0ms) {
		auto time_alive = get_time_alive(time);
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

void remove_floating_label(int handle, const std::chrono::milliseconds& fadeout)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		if(fadeout > 0ms) {
			i->second.set_lifetime(0ms, fadeout);
			return;
		} else if(fadeout < 0ms) {
			i->second.set_lifetime(0ms, i->second.get_fade_time());
			return;
		}
		// Queue a redraw of where the label was.
		i->second.undraw();
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
	// hacky but the whole floating label system needs to be redesigned...
	for(auto& [id, label] : labels) {
		if(label_contexts.top().count(id) > 0) {
			label.undraw();
		}
	}

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

	const std::set<int>& context = label_contexts.top();

	// draw the labels in the order they were added, so later added labels (likely to be tooltips)
	// are displayed over earlier added labels.
	for(auto& [id, label] : labels) {
		if(context.count(id) > 0) {
			label.draw();
		}
	}
}

void update_floating_labels()
{
	if(label_contexts.empty()) {
		return;
	}
	auto time = std::chrono::steady_clock::now();

	std::set<int>& context = label_contexts.top();

	for(auto& [id, label] : labels) {
		if(context.count(id) > 0) {
			label.update(time);
		}
	}

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(context.count(j->first) > 0 && j->second.expired(time)) {
			DBG_FT << "removing expired floating label " << j->first;
			context.erase(j->first);
			labels.erase(j++);
		} else {
			++j;
		}
	}
}

}
