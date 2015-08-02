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

#include "display.hpp"
#include "font.hpp"
#include "log.hpp"
#include "text.hpp"

#include <map>
#include <set>
#include <stack>

#if SDL_VERSION_ATLEAST(2,0,0)
#include "sdl/alpha.hpp"
#endif

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace {

typedef std::map<int, font::floating_label> label_map;
label_map labels;
int label_id = 1;

std::stack<std::set<int> > label_contexts;
}

namespace font {

floating_label::floating_label(const std::string& text, const surface& surf)
#if 0
		: img_(),
#else
		: surf_(surf), buf_(NULL),
#endif
		text_(text),
		font_size_(SIZE_NORMAL),
		color_(NORMAL_COLOR),	bgcolor_(), bgalpha_(0),
		xpos_(0), ypos_(0),
		xmove_(0), ymove_(0), lifetime_(-1),
		width_(-1), height_(-1),
		clip_rect_(screen_area()),
		alpha_change_(0), visible_(true), align_(CENTER_ALIGN),
		border_(0), scroll_(ANCHOR_LABEL_SCREEN), use_markup_(true)
{}

void floating_label::move(double xmove, double ymove)
{
	xpos_ += xmove;
	ypos_ += ymove;
}

int floating_label::xpos(size_t width) const
{
	int xpos = int(xpos_);
	if(align_ == font::CENTER_ALIGN) {
		xpos -= width/2;
	} else if(align_ == font::RIGHT_ALIGN) {
		xpos -= width;
	}

	return xpos;
}

#if 0
sdl::timage floating_label::create_image()
{
	if (img_.null()) {
		font::ttext text;
		text.set_foreground_color((color_.r << 24) | (color_.g << 16) | (color_.b << 8) | 255);
		text.set_font_size(font_size_);
		text.set_maximum_width(width_ < 0 ? clip_rect_.w : width_);
		text.set_maximum_height(height_ < 0 ? clip_rect_.h : height_, true);

		//ignore last '\n'
		if(!text_.empty() && *(text_.rbegin()) == '\n'){
			text.set_text(std::string(text_.begin(), text_.end()-1), use_markup_);
		} else {
			text.set_text(text_, use_markup_);
		}

		surface foreground = text.render();

		if(foreground == NULL) {
			ERR_FT << "could not create floating label's text" << std::endl;
			return sdl::timage();
		}

		// combine foreground text with its background
		if(bgalpha_ != 0) {
			// background is a dark tooltip box
			surface background = create_neutral_surface(foreground->w + border_*2, foreground->h + border_*2);

			if (background == NULL) {
				ERR_FT << "could not create tooltip box" << std::endl;
				img_ = sdl::timage(foreground);
				return img_;
			}

			Uint32 color = SDL_MapRGBA(foreground->format, bgcolor_.r,bgcolor_.g, bgcolor_.b, bgalpha_);
			sdl::fill_rect(background,NULL, color);

			// we make the text less transparent, because the blitting on the
			// dark background will darken the anti-aliased part.
			// This 1.13 value seems to restore the brightness of version 1.4
			// (where the text was blitted directly on screen)
			foreground = adjust_surface_alpha(foreground, ftofxp(1.13), false);

			SDL_Rect r = sdl::create_rect( border_, border_, 0, 0);
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);

			img_ = sdl::timage(background);
		}
		else {
			// background is blurred shadow of the text
			surface background = create_neutral_surface
				(foreground->w + 4, foreground->h + 4);
			sdl::fill_rect(background, NULL, 0);
			SDL_Rect r = { 2, 2, 0, 0 };
			blit_surface(foreground, NULL, background, &r);
			background = shadow_image(background, false);

			if (background == NULL) {
				ERR_FT << "could not create floating label's shadow" << std::endl;
				img_ = sdl::timage(foreground);
				return img_;
			}
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);
			img_ = sdl::timage(background);
		}
	}

	return img_;
}

#else
surface floating_label::create_surface()
{
	if (surf_.null()) {
		font::ttext text;
		text.set_foreground_color((color_.r << 24) | (color_.g << 16) | (color_.b << 8) | 255);
		text.set_font_size(font_size_);
		text.set_maximum_width(width_ < 0 ? clip_rect_.w : width_);
		text.set_maximum_height(height_ < 0 ? clip_rect_.h : height_, true);

		//ignore last '\n'
		if(!text_.empty() && *(text_.rbegin()) == '\n'){
			text.set_text(std::string(text_.begin(), text_.end()-1), use_markup_);
		} else {
			text.set_text(text_, use_markup_);
		}

		surface foreground = text.render();

		if(foreground == NULL) {
			ERR_FT << "could not create floating label's text" << std::endl;
			return NULL;
		}

		// combine foreground text with its background
		if(bgalpha_ != 0) {
			// background is a dark tooltip box
			surface background = create_neutral_surface(foreground->w + border_*2, foreground->h + border_*2);

			if (background == NULL) {
				ERR_FT << "could not create tooltip box" << std::endl;
				surf_ = create_optimized_surface(foreground);
				return surf_;
			}

			Uint32 color = SDL_MapRGBA(foreground->format, bgcolor_.r,bgcolor_.g, bgcolor_.b, bgalpha_);
			sdl::fill_rect(background,NULL, color);

			// we make the text less transparent, because the blitting on the
			// dark background will darken the anti-aliased part.
			// This 1.13 value seems to restore the brightness of version 1.4
			// (where the text was blitted directly on screen)
			foreground = adjust_surface_alpha(foreground, ftofxp(1.13), false);

			SDL_Rect r = sdl::create_rect( border_, border_, 0, 0);
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);

			surf_ = create_optimized_surface(background);
			// RLE compression seems less efficient for big semi-transparent area
			// so, remove it for this case, but keep the optimized display format
			SDL_SetAlpha(surf_,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
		}
		else {
			// background is blurred shadow of the text
			surface background = create_neutral_surface
				(foreground->w + 4, foreground->h + 4);
			sdl::fill_rect(background, NULL, 0);
			SDL_Rect r = { 2, 2, 0, 0 };
			blit_surface(foreground, NULL, background, &r);
			background = shadow_image(background, false);

			if (background == NULL) {
				ERR_FT << "could not create floating label's shadow" << std::endl;
				surf_ = create_optimized_surface(foreground);
				return surf_;
			}
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);
			surf_ = create_optimized_surface(background);
		}
	}

	return surf_;
}
#endif

#ifdef SDL_GPU
void floating_label::draw(CVideo &video)
{
	if (!visible_) {
		return;
	}
#if 0
	create_image();
	if (img_.null()) {
		return;
	}

	video.draw_texture(img_, xpos(img_.width()), int(ypos_));
#else
	create_surface();
	if (surf_.null()) {
		return;
	}

	video.blit_to_overlay(surf_, xpos(surf_->w), int(ypos_));
#endif
}

#else
void floating_label::draw(surface screen)
{
	if(!visible_) {
		buf_.assign(NULL);
		return;
	}

	create_surface();
	if(surf_ == NULL) {
		return;
	}

	if(buf_ == NULL) {
		buf_.assign(create_compatible_surface(screen, surf_->w, surf_->h));
		if(buf_ == NULL) {
			return;
		}
	}

	if(screen == NULL) {
		return;
	}

	SDL_Rect rect = sdl::create_rect(xpos(surf_->w), ypos_, surf_->w, surf_->h);
	const clip_rect_setter clip_setter(screen, &clip_rect_);
	sdl_blit(screen,&rect,buf_,NULL);
	sdl_blit(surf_,NULL,screen,&rect);

	update_rect(rect);
}
#endif

#ifdef SDL_GPU
void floating_label::undraw(CVideo &video)
{
	SDL_Rect r = sdl::create_rect(xpos(surf_->w), ypos_, surf_->w, surf_->h);
	video.clear_overlay_area(r);
}
#else
void floating_label::undraw(surface screen)
{
	if(screen == NULL || buf_ == NULL) {
		return;
	}
	SDL_Rect rect = sdl::create_rect(xpos(surf_->w), ypos_, surf_->w, surf_->h);
	const clip_rect_setter clip_setter(screen, &clip_rect_);
	sdl_blit(buf_,NULL,screen,&rect);

	update_rect(rect);

	move(xmove_,ymove_);
	if(lifetime_ > 0) {
		--lifetime_;
		if(alpha_change_ != 0 && (xmove_ != 0.0 || ymove_ != 0.0) && surf_ != NULL) {
			// fade out moving floating labels
			// note that we don't optimize these surfaces since they will always change
			surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_,false));
		}
	}
}
#endif

int add_floating_label(const floating_label& flabel)
{
	if(label_contexts.empty()) {
		return 0;
	}

	++label_id;
	labels.insert(std::pair<int, floating_label>(label_id, flabel));
	label_contexts.top().insert(label_id);
	return label_id;
}

void move_floating_label(int handle, double xmove, double ymove)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.move(xmove,ymove);
	}
}

void scroll_floating_labels(double xmove, double ymove)
{
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(i->second.scroll() == ANCHOR_LABEL_MAP) {
			i->second.move(xmove,ymove);
		}
	}
}

void remove_floating_label(int handle)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		if(label_contexts.empty() == false) {
			label_contexts.top().erase(i->first);
		}

		labels.erase(i);
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
#if 0
	if(i != labels.end()) {
		const sdl::timage img = i->second.create_image();
		if(!img.null()) {
			return sdl::create_rect(0, 0, img.width(), img.height());
		}
	}
#else
	if(i != labels.end()) {
		const surface surf = i->second.create_surface();
		if(surf != NULL) {
			return sdl::create_rect(0, 0, surf->w, surf->h);
		}
	}
#endif
	return sdl::empty_rect;
}

floating_label_context::floating_label_context()
{
#ifdef SDL_GPU

#else
#if SDL_VERSION_ATLEAST(2, 0, 0)
	surface const screen = NULL;
#else
	surface const screen = SDL_GetVideoSurface();
#endif
	if(screen != NULL) {
		draw_floating_labels(screen);
	}
#endif

	label_contexts.push(std::set<int>());
}

floating_label_context::~floating_label_context()
{
	const std::set<int>& labels = label_contexts.top();
	for(std::set<int>::const_iterator i = labels.begin(); i != labels.end(); ) {
		remove_floating_label(*i++);
	}

	label_contexts.pop();

#ifdef SDL_GPU
	//TODO
#else
#if SDL_VERSION_ATLEAST(2, 0, 0)
	surface const screen = NULL;
#else
	surface const screen = SDL_GetVideoSurface();
#endif
	if(screen != NULL) {
		undraw_floating_labels(screen);
	}
#endif
}

#ifdef SDL_GPU
void draw_floating_labels(CVideo &video)
{
	if(label_contexts.empty()) {
		return;
	}

	const std::set<int>& context = label_contexts.top();

	//draw the labels in the order they were added, so later added labels (likely to be tooltips)
	//are displayed over earlier added labels.
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.draw(video);
		}
	}
}

void undraw_floating_labels(CVideo &video)
{
	if(label_contexts.empty()) {
		return;
	}

	std::set<int>& context = label_contexts.top();

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(context.count(j->first) > 0 && j->second.expired()) {
			j->second.undraw(video);
			context.erase(j->first);
			labels.erase(j++);
		} else {
			++j;
		}
	}
}

#else
void draw_floating_labels(surface screen)
{
	if(label_contexts.empty()) {
		return;
	}

	const std::set<int>& context = label_contexts.top();

	//draw the labels in the order they were added, so later added labels (likely to be tooltips)
	//are displayed over earlier added labels.
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.draw(screen);
		}
	}
}

void undraw_floating_labels(surface screen)
{
	if(label_contexts.empty()) {
		return;
	}

	std::set<int>& context = label_contexts.top();

	//undraw labels in reverse order, so that a LIFO process occurs, and the screen is restored
	//into the exact state it started in.
	for(label_map::reverse_iterator i = labels.rbegin(); i != labels.rend(); ++i) {
		if(context.count(i->first) > 0) {			
			i->second.undraw(screen);
		}
	}

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(context.count(j->first) > 0 && j->second.expired()) {
			context.erase(j->first);
			labels.erase(j++);
		} else {
			++j;
		}
	}
}
#endif
}

