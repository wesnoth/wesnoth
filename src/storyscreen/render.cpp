/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/render.cpp
 * Storyscreen pages renderer.
 * @todo Translate relevant parts to GUI2.
 */

#include "global.hpp"
#include "asserts.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "storyscreen/page.hpp"
#include "storyscreen/render.hpp"

#include "display.hpp"
#include "image.hpp"
#include "image_function.hpp"
#include "language.hpp"
#include "marked-up_text.hpp" // TODO: replace with Pango
#include "sound.hpp"
#include "text.hpp"
#include "video.hpp"

// TODO: remove when completed
#include "stub.hpp"

#define ERR_NG LOG_STREAM(err, engine)

#ifndef LOW_MEM
namespace {
	void blur_area(CVideo& video, int y, int h)
	{
		SDL_Rect blur_rect = { 0, y, screen_area().w, h };
		surface blur = get_surface_portion(video.getSurface(), blur_rect);
		blur = blur_surface(blur, 1, false);
		video.blit_surface(0, y, blur);
	}
}
#endif /* ! LOW_MEM */

namespace storyscreen {

page_ui::page_ui(page& p, display& disp, gui::button& next_button, gui::button& skip_button)
	: p_(p)
	, disp_(disp)
	, video_(disp.video())
	, keys_()
	, next_button_(next_button)
	, skip_button_(skip_button)
	, ret_(NEXT)
	, scale_factor_(1.0)
	, base_rect_()
	, background_(NULL)
	, imgs_()
	, has_background_(false)
	, text_x_(200)
	, text_y_(400)
	, buttons_x_()
	, buttons_y_()
{
	// Build background surface
	if(p_.background_file_.empty() != true) {
		background_.assign( image::get_image(p.background_file_) );
	}
	has_background_ = !background_.null();
	if(background_.null() || background_->w * background_-> h == 0) {
		background_.assign( create_neutral_surface(video_.getx(), video_.gety()) );
	}

	const double xscale = 1.0 * video_.getx() / background_->w;
	const double yscale = 1.0 * video_.gety() / background_->h;
	scale_factor_ = p_.scale_background_ ? std::min<double>(xscale,yscale) : 1.0;

	background_ =
		scale_surface(background_, static_cast<int>(background_->w*scale_factor_), static_cast<int>(background_->h*scale_factor_));

	ASSERT_LOG(background_.null() == false, "Ouch: storyscreen page background got NULL");

	base_rect_.x = (video_.getx() - background_->w) / 2;
	base_rect_.y = (video_.gety() - background_->h) / 2;
	base_rect_.w = background_->w;
	base_rect_.h = background_->h;

#ifdef USE_TINY_GUI
	// Use the whole screen for text on tinygui
	text_x_ = 10;
	text_y_ = 0;
	buttons_x_ = video_.getx() - 50;
	buttons_y_ = base_rect_.y + base_rect_.h - 20;

	next_button_.set_location(buttons_x_, buttons_y_ - 20);
	skip_button_.set_location(buttons_x_, buttons_y_);
#else
	text_x_ = 200;
	text_y_ = video_.gety() - 200;
	buttons_x_ = video_.getx() - 200 - 40;
	buttons_y_ = video_.gety() - 40;

	next_button_.set_location(buttons_x_, buttons_y_ - 30);
	skip_button_.set_location(buttons_x_, buttons_y_);
#endif

	// Build floating image surfaces
	foreach(const floating_image& fi, p_.floating_images_) {
		imgs_.push_back( fi.get_render_input(scale_factor_, base_rect_) );
	}
}

void page_ui::render_text_box()
{
	const bool rtl = current_language_rtl();

	const int max_width = next_button_.location().x - 10 - text_x_;
	const std::string storytxt =
		font::word_wrap_text(p_.text_, font::SIZE_PLUS, max_width);

	utils::utf8_iterator itor(storytxt);

	bool skip = false, last_key = true;
	int update_y = 0, update_h = 0;

	// Draw the text box
	if(storytxt.empty() != true)
	{
		// this should kill the tiniest flickering caused
		// by the buttons being hidden and unhidden in this scope.
		update_locker locker(video_);

		const SDL_Rect total_size = font::draw_text(
			NULL, screen_area(), font::SIZE_PLUS,
			font::NORMAL_COLOUR, storytxt, 0, 0
		);

		next_button_.hide();
		skip_button_.hide();

		if(text_y_ + 20 + total_size.h > screen_area().h) {
			text_y_ = screen_area().h > total_size.h + 1 ? screen_area().h - total_size.h - 21 : 0;
		}

		update_y = text_y_;
		update_h = screen_area().h - text_y_;

#ifndef LOW_MEM
		blur_area(video_, update_y, update_h);
#endif

		draw_solid_tinted_rectangle(
			0, text_y_, screen_area().w, screen_area().h-text_y_,
			0, 0, 0, 0.5, video_.getSurface()
		);

		// Draw a nice border
		if(has_background_) {
			// FIXME: perhaps hard-coding the image path isn't a really
			// good idea - it must not be forgotten if someone decides to switch
			// the image directories around.
			surface top_border = image::get_image("dialogs/translucent54-border-top.png");
			top_border = scale_surface_blended(top_border, screen_area().w, top_border->h);
			update_y = text_y_ - top_border->h;
			update_h += top_border->h;
#ifndef LOW_MEM
			blur_area(video_, update_y, top_border->h);
#endif
			video_.blit_surface(0, text_y_ - top_border->h, top_border);
		}

		// Make buttons aware of the changes in the background
		next_button_.set_location(next_button_.location());
		next_button_.hide(false);
		skip_button_.set_location(skip_button_.location());
		skip_button_.hide(false);
	}

	if(imgs_.empty()) {
		update_whole_screen();
	} else if(update_h > 0) {
		update_rect(0,update_y,screen_area().w,update_h);
	}

	if(rtl) {
		text_x_ += max_width;
	}

#ifdef USE_TINY_GUI
	int xpos = text_x_, ypos = text_y_ + 10;
#else
	int xpos = text_x_, ypos = text_y_ + 20;
#endif

	// The maximum position that text can reach before wrapping
	size_t height = 0;

	while(true) {
		if(itor != utils::utf8_iterator::end(storytxt)) {
			if(*itor == '\n') {
				xpos = text_x_;
				ypos += height;
				++itor;
			}
			// Output the character
			/** @todo  FIXME: this is broken: it does not take kerning into account. */
			std::string tmp;
			tmp.append(itor.substr().first, itor.substr().second);
			if(rtl) {
				xpos -= font::line_width(tmp, font::SIZE_PLUS);
			}
			const SDL_Rect rect = font::draw_text(
				&video_, screen_area(), font::SIZE_PLUS,
				font::NORMAL_COLOUR, tmp, xpos, ypos,
				false
			);

			if(rect.h > height)
				height = rect.h;
			if(!rtl)
				xpos += rect.w;
			update_rect(rect);

			++itor;
			if(itor == utils::utf8_iterator::end(storytxt)) {
				skip = true;
			}
		}

		const bool keydown = keys_[SDLK_SPACE] || keys_[SDLK_RETURN] || keys_[SDLK_KP_ENTER];

		if((keydown && !last_key) || next_button_.pressed()) {
			if(skip == true || itor == utils::utf8_iterator::end(storytxt)) {
				ret_ = NEXT;
				break;
			} else {
				skip = true;
			}
		}

		last_key = keydown;

		if(keys_[SDLK_ESCAPE] || skip_button_.pressed()) {
			ret_ = SKIP;
			return;
		}

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp_.flip();

		if(!skip || itor == utils::utf8_iterator::end(storytxt)) {
			disp_.delay(20);
		}
	}

	draw_solid_tinted_rectangle(
		0, 0, video_.getx(), video_.gety(), 0, 0, 0,
		1.0, video_.getSurface()
	);
}

void page_ui::render_title_box()
{
	// Text color
	const int r = 0, g = 0, b = 0;

	const SDL_Rect area = { 0, 0, video_.getx(), video_.gety() };
	const SDL_Rect text_shadow_rect = font::line_size(p_.text_title_, font::SIZE_XLARGE);

	draw_solid_tinted_rectangle(
		base_rect_.x + 15, base_rect_.y + 15,
		text_shadow_rect.w + 10, text_shadow_rect.h + 10, r, g, b, 0.5, video_.getSurface()
	);

	update_rect(font::draw_text(
		&video_, area, font::SIZE_XLARGE, font::BIGMAP_COLOUR,
		p_.text_title_, base_rect_.x + 20, base_rect_.y + 20
	));
}

void page_ui::render_background()
{
	draw_solid_tinted_rectangle(
		0, 0, video_.getx(), video_.gety(), 0, 0, 0, 1.0,
		video_.getSurface()
	);
	SDL_BlitSurface(background_, NULL, video_.getSurface(), &base_rect_);
}

bool page_ui::render_floating_images()
{
	events::raise_draw_event();
	update_whole_screen();

	bool skip = false;

	size_t fi_n = 0;
	foreach(floating_image::render_input& ri, imgs_) {
		const floating_image& fi = p_.floating_images_[fi_n];

		if(!ri.image.null()) {
			SDL_BlitSurface(ri.image, NULL, video_.getSurface(), &ri.rect);
			update_rect(ri.rect);
		}

		if(skip == false) {
			for(unsigned i = 0; i != 50; ++i) {
				if(keys_[SDLK_ESCAPE] || skip_button_.pressed()) {
					ret_ = SKIP;
					return false;
				}
				else if(next_button_.pressed()) {
					ret_ = NEXT;
					return false;
				}

				disp_.delay(fi.display_delay() / 50);

				events::pump();
				events::raise_process_event();
				events::raise_draw_event();

				int mouse_x, mouse_y;
				const int mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
				if(keys_[SDLK_RETURN] || keys_[SDLK_KP_ENTER] || keys_[SDLK_SPACE] || mouse_state) {
					skip = true;
					++fi_n;
					continue;
				}

				// Update display only if there's a slideshow going on.
				// This prevents the textbox from flickering in the most
				// common scenario.
				if(p_.get_floating_images().size() > 1 && fi.display_delay() > 0) {
					disp_.flip();
				}
			}
		}

		if(keys_[SDLK_ESCAPE] || next_button_.pressed() || skip_button_.pressed()) {
			skip = true;
			++fi_n;
			continue;
		}
		++fi_n;
	}

	return true;
}

page_ui::RESULT page_ui::show()
{
	if(p_.music_.empty() != true) {
		sound::play_music_repeatedly(p_.music_);
	}

	render_background();

	if(p_.show_title_) {
		render_title_box();
	}

	if(!imgs_.empty()) {
		if(!render_floating_images()) {
			return ret_;
		}
	}

	try {
		render_text_box();
	}
	catch(utils::invalid_utf8_exception const&) {
		ERR_NG << "invalid UTF-8 sequence in story text, skipping page...\n";
	}

	return ret_;
}

} // end namespace storyscreen
