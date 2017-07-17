/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "floating_label.hpp"

#include "font/standard_colors.hpp"
#include "font/text.hpp"
#include "log.hpp"
#include "sdl/render_utils.hpp"
#include "sdl/surface.hpp"
#include "utils/general.hpp"

#include <boost/algorithm/string.hpp>

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
std::map<int, font::floating_label> labels;

int label_id = 1;

std::stack<std::set<int>> label_contexts;
}

namespace font
{
floating_label::floating_label(const std::string& text)
	: texture_(nullptr)
	, text_(text)
	, color_(NORMAL_COLOR)
	, bgcolor_()
	, xpos_(0)
	, ypos_(0)
	, xmove_(0)
	, ymove_(0)
	, width_(-1)
	, height_(-1)
	, font_size_(SIZE_NORMAL)
	, lifetime_(-1)
	, border_(0)
	, alpha_change_(0)
	, current_alpha_(255)
	, clip_rect_(screen_area())
	, align_(CENTER_ALIGN)
	, scroll_(ANCHOR_LABEL_SCREEN)
	, visible_(true)
	, use_markup_(true)
	, fill_background_(false)
{
}

void floating_label::move(double xmove, double ymove)
{
	xpos_ += xmove;
	ypos_ += ymove;
}

int floating_label::xpos(size_t width) const
{
	int xpos = static_cast<int>(xpos_);

	if(align_ == font::CENTER_ALIGN) {
		xpos -= width / 2;
	} else if(align_ == font::RIGHT_ALIGN) {
		xpos -= width;
	}

	return xpos;
}

texture floating_label::create_texture()
{
	if(texture_.null()) {
		//
		// Render text
		//

		// TODO: figure out why the global text renderer object gives too large a size.
		font::pango_text renderer;

		renderer.set_foreground_color(color_);
		renderer.set_font_size(font_size_);
		renderer.set_maximum_width(width_ < 0 ? clip_rect_.w : width_);
		renderer.set_maximum_height(height_ < 0 ? clip_rect_.h : height_, true);

		// Add text outline if we're not drawing the background.
		if(!fill_background_) {
			renderer.set_add_outline(true);
		}

		// Strip trailing newlines.
		boost::trim_right(text_);

		renderer.set_text(text_, use_markup_);

		texture_ = renderer.render_and_get_texture();

		if(texture_.null()) {
			ERR_FT << "could not create floating label's text" << std::endl;
		}
	}

	return texture_;
}

void floating_label::draw()
{
	// No-op if texture is valid
	create_texture();

	if(texture_ == nullptr) {
		return;
	}

	const texture::info info = texture_.get_info();
	SDL_Rect rect = sdl::create_rect(xpos(info.w), ypos_, info.w, info.h);

	move(xmove_, ymove_);

	// Fade out moving floating labels
	if(lifetime_ > 0) {
		--lifetime_;

		if(alpha_change_ != 0 && (xmove_ != 0.0 || ymove_ != 0.0)) {
			current_alpha_ = utils::clamp<unsigned int>(current_alpha_ + alpha_change_, 0, 255);
			set_texture_alpha(texture_, current_alpha_);
		}
	}

	// Draw a semi-transparent background background alpha provided.
	// NOTE: doing this this way instead of embedding it as part of label texture itself does
	// have the side effect of removing the background from the fadeout effect. However, in
	// practical use only the non-background versions are used with the fadeout effect. I can do
	// some alpha fadeout on the background later too if relevant.
	if(fill_background_) {
		SDL_Rect bg_rect {
			rect.x -  border_,
			rect.y -  border_,
			rect.w + (border_ * 2),
			rect.h + (border_ * 2)
		};

		sdl::fill_rectangle(bg_rect, bgcolor_);
	}

	CVideo::get_singleton().render_copy(texture_, nullptr, &rect);
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
	const auto i = labels.find(handle);
	if(i != labels.end()) {
		i->second.move(xmove, ymove);
	}
}

void scroll_floating_labels(double xmove, double ymove)
{
	for(auto& label : labels) {
		if(label.second.scroll() == ANCHOR_LABEL_MAP) {
			label.second.move(xmove, ymove);
		}
	}
}

void remove_floating_label(int handle)
{
	const auto i = labels.find(handle);
	if(i != labels.end()) {
		labels.erase(i);
	}

	if(!label_contexts.empty()) {
		label_contexts.top().erase(handle);
	}
}

void show_floating_label(int handle, bool value)
{
	const auto i = labels.find(handle);
	if(i != labels.end()) {
		i->second.show(value);
	}
}

SDL_Rect get_floating_label_rect(int handle)
{
	const auto i = labels.find(handle);
	if(i != labels.end()) {
		const texture& tex = i->second.create_texture();
		if(tex != nullptr) {
			const texture::info info = tex.get_info();

			return {0, 0, info.w, info.h};
		}
	}

	return sdl::empty_rect;
}

floating_label_context::floating_label_context()
{
	label_contexts.push(std::set<int>());
}

floating_label_context::~floating_label_context()
{
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

	std::set<int>& context = label_contexts.top();

	// Remove expired labels.
	for(auto itor = labels.begin(); itor != labels.end(); /* Handle increment in loop*/) {
		if(context.count(itor->first) > 0 && itor->second.expired()) {
			context.erase(itor->first);
			labels.erase(itor++);
		} else {
			++itor;
		}
	}

	// Draw the labels in the order they were added, so later added labels (likely to be tooltips)
	// are displayed over earlier added labels.
	for(auto& label : labels) {
		if(context.count(label.first) > 0) {
			label.second.draw();
		}
	}
}
}
