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
#include "draw.hpp"
#include "draw_manager.hpp"
#include "font/sdl_ttf_compat.hpp" // pango_line_width
#include "font/text.hpp"
#include "log.hpp"
#include "sdl/utils.hpp"
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

/** Curent ID of the help string. */
int help_string_ = 0;
}

// TODO: draw_manager - why tf is this in namespace font?
namespace font
{
floating_label::floating_label(const std::string& text, const surface& surf)
	: tex_()
	, screen_loc_()
	, alpha_(0)
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
	, clip_rect_(video::game_canvas())
	, visible_(true)
	, align_(CENTER_ALIGN)
	, border_(0)
	, scroll_(ANCHOR_LABEL_SCREEN)
	, use_markup_(true)
{
	if (surf.get()) {
		tex_ = texture(surf);
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
	if(video::headless()) {
		return false;
	}

	if(tex_ != nullptr) {
		// Already have a texture
		return true;
	}

	DBG_FT << "creating floating label texture";
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

	surface foreground = text.render_surface();

	// Pixel scaling is necessary as we are manipulating the raw surface
	const int ps = video::get_pixel_scale();
	// For consistent results we must also enlarge according to zoom
	const int sf = ps * display::get_singleton()->get_zoom_factor();

	if(foreground == nullptr) {
		// TODO: draw_manager - find what triggers this and fix it
		//ERR_FT << "could not create floating label's text";
		return false;
	}

	// combine foreground text with its background
	if(bgalpha_ != 0) {
		// background is a dark tooltip box
		surface background(foreground->w + border_ * 2 * sf, foreground->h + border_ * 2 * sf);

		if(background == nullptr) {
			ERR_FT << "could not create tooltip box";
			tex_ = texture(foreground);
			return tex_ != nullptr;
		}

		uint32_t color = SDL_MapRGBA(foreground->format, bgcolor_.r, bgcolor_.g, bgcolor_.b, bgalpha_);
		sdl::fill_surface_rect(background, nullptr, color);

		SDL_Rect r{border_ * sf, border_ * sf, 0, 0};
		adjust_surface_alpha(foreground, SDL_ALPHA_OPAQUE);
		sdl_blit(foreground, nullptr, background, &r);

		tex_ = texture(background);
	} else {
		// background is blurred shadow of the text
		surface background(foreground->w + 4*sf, foreground->h + 4*sf);
		sdl::fill_surface_rect(background, nullptr, 0);
		SDL_Rect r{2*sf, 2*sf, 0, 0};
		sdl_blit(foreground, nullptr, background, &r);
		background = shadow_image(background, sf);

		if(background == nullptr) {
			ERR_FT << "could not create floating label's shadow";
			tex_ = texture(foreground);
			return tex_ != nullptr;
		}
		sdl_blit(foreground, nullptr, background, &r);
		tex_ = texture(background);
	}

	// adjust high-dpi text display scale
	tex_.set_draw_width(tex_.w() / ps);
	tex_.set_draw_height(tex_.h() / ps);

	return true;
}

void floating_label::undraw()
{
	DBG_FT << "undrawing floating label from " << screen_loc_;
	draw_manager::invalidate_region(screen_loc_);
	screen_loc_ = {};
}

void floating_label::update(int time)
{
	if(video::headless()) {
		return;
	}

	if(!create_texture()) {
		// TODO: draw_manager - find what triggers this and fix it
		//ERR_FT << "failed to create texture for floating label";
		return;
	}

	point new_pos = get_pos(time);
	rect draw_loc = {new_pos.x, new_pos.y, tex_.w(), tex_.h()};

	uint8_t new_alpha = get_alpha(time);

	if(screen_loc_ == draw_loc && alpha_ == new_alpha) {
		// nothing has changed
		return;
	}

	draw_manager::invalidate_region(screen_loc_);
	draw_manager::invalidate_region(draw_loc);

	DBG_FT << "updating floating label from " << screen_loc_
		<< " to " << draw_loc;

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

	// TODO: draw_manager - is this actually useful?
	// Clip if appropriate.
	auto clipper = draw::reduce_clip(clip_rect_);

	// Apply the label texture to the screen.
	tex_.set_alpha_mod(alpha_);
	draw::blit(tex_, screen_loc_);
}

void floating_label::set_lifetime(int lifetime, int fadeout)
{
	lifetime_ = lifetime;
	fadeout_ = fadeout;
	time_start_	= SDL_GetTicks();
}


point floating_label::get_pos(int time)
{
	int time_alive = get_time_alive(time);
	return {
		static_cast<int>(time_alive * xmove_ + xpos(tex_.w())),
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
	int time = SDL_GetTicks();

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

void set_help_string(const std::string& str)
{
	remove_floating_label(help_string_);

	const color_t color{0, 0, 0, 0xbb};

	int size = font::SIZE_LARGE;
	point canvas_size = video::game_canvas_size();

	while(size > 0) {
		if(pango_line_width(str, size) > canvas_size.x) {
			size--;
		} else {
			break;
		}
	}

	const int border = 5;

	floating_label flabel(str);
	flabel.set_font_size(size);
	flabel.set_position(canvas_size.x / 2, canvas_size.y);
	flabel.set_bg_color(color);
	flabel.set_border_size(border);

	help_string_ = add_floating_label(flabel);

	const rect& r = get_floating_label_rect(help_string_);
	move_floating_label(help_string_, 0.0, -double(r.h));
}

void clear_help_string()
{
	remove_floating_label(help_string_);
	help_string_ = 0;
}

}
