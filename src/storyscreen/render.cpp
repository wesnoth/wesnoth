/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2011 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * Storyscreen parts renderer.
 * @todo Translate relevant parts to GUI2.
 */

#include "global.hpp"
#include "asserts.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "storyscreen/part.hpp"
#include "storyscreen/render.hpp"

#include "display.hpp"
#include "image.hpp"
#include "language.hpp"
#include "sound.hpp"
#include "text.hpp"
#include "video.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG  LOG_STREAM(err,  log_engine)
#define WARN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG  LOG_STREAM(info, log_engine)


namespace {
	int const storybox_padding = 10; // px
	double const storyshadow_opacity = 0.5;
	int const storyshadow_r = 0;
	int const storyshadow_g = 0;
	int const storyshadow_b = 0;

	int const titlebox_padding = 20; // px
	int const titleshadow_padding = 5; // px
	double const titleshadow_opacity = 0.5;
	int const titleshadow_r = 0;
	int const titleshadow_g = 0;
	int const titleshadow_b = 0;

#ifndef USE_TINY_GUI
	int const titlebox_font_size = 20; // pt?
	int const storybox_font_size = 14; // pt?
#else
	int const titlebox_font_size = 11; // pt?
	int const storybox_font_size = 10; // pt?
#endif

	Uint32 const titlebox_font_color = 0xFFFFFFFF;
	Uint32 const storybox_font_color = 0xDDDDDDFF;

#ifndef LOW_MEM
	// Hard-coded path to a suitable (tileable) pic for the storytxt box border.
	std::string const storybox_top_border_path = "dialogs/translucent54-border-top.png";
	std::string const storybox_bottom_border_path = "dialogs/translucent54-border-bottom.png";

	void blur_area(CVideo& video, int y, int h)
	{
		SDL_Rect blur_rect = { 0, y, screen_area().w, h };
		surface blur = get_surface_portion(video.getSurface(), blur_rect);
		blur = blur_surface(blur, 1, false);
		video.blit_surface(0, y, blur);
	}
#endif
}

namespace storyscreen {

part_ui::part_ui(part& p, display& disp, gui::button& next_button, gui::button& skip_button)
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
	, buttons_x_(0)
	, buttons_y_(0)
{
	this->prepare_background();
	this->prepare_geometry();
	this->prepare_floating_images();
}

void part_ui::prepare_background()
{
	// Build background surface
	if(p_.background().empty() != true) {
		background_.assign( image::get_image(p_.background()) );
	}
	has_background_ = !background_.null();
	if(background_.null() || background_->w * background_-> h == 0) {
		background_.assign( create_neutral_surface(video_.getx(), video_.gety()) );
	}

	const double xscale = 1.0 * video_.getx() / background_->w;
	const double yscale = 1.0 * video_.gety() / background_->h;
	scale_factor_ = p_.scale_background() ? std::min<double>(xscale,yscale) : 1.0;

	background_ =
		scale_surface(background_, static_cast<int>(background_->w*scale_factor_), static_cast<int>(background_->h*scale_factor_));

	ASSERT_LOG(background_.null() == false, "Oops: storyscreen part background got NULL");
}

void part_ui::prepare_geometry()
{
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

#else // elif !defined(USE_TINY_GUI)

	// The horizontal locations are always the same
	text_x_ = 200;
	buttons_x_ = video_.getx() - 200 - 40;

	switch(p_.story_text_location())
	{
	case part::BLOCK_TOP:
		text_y_ = 0;
		buttons_y_ = 40;
		break;
	case part::BLOCK_MIDDLE:
		text_y_ = video_.gety() / 3;
		buttons_y_ = video_.gety() / 2 + 15;
		break;
	default: // part::BLOCK_BOTTOM
		text_y_ = video_.gety() - 200;
		buttons_y_ = video_.gety() - 40;
		break;
	}
	next_button_.set_location(buttons_x_, buttons_y_ - 30);
	skip_button_.set_location(buttons_x_, buttons_y_);
#endif

    next_button_.set_volatile(true);
    skip_button_.set_volatile(true);
}

void part_ui::prepare_floating_images()
{
	// Build floating image surfaces
	BOOST_FOREACH(const floating_image& fi, p_.get_floating_images()) {
		imgs_.push_back( fi.get_render_input(scale_factor_, base_rect_) );
	}
}

void part_ui::render_background()
{
	draw_solid_tinted_rectangle(
		0, 0, video_.getx(), video_.gety(), 0, 0, 0, 1.0,
		video_.getSurface()
	);
	SDL_BlitSurface(background_, NULL, video_.getSurface(), &base_rect_);
}

bool part_ui::render_floating_images()
{
	events::raise_draw_event();
	update_whole_screen();

	bool skip = false;

	size_t fi_n = 0;
	BOOST_FOREACH(floating_image::render_input& ri, imgs_) {
		const floating_image& fi = p_.get_floating_images()[fi_n];

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

void part_ui::render_title_box()
{
	const std::string& titletxt = p_.title();
	if(titletxt.empty()) {
		return;
	}

	int titlebox_x, titlebox_y, titlebox_max_w, titlebox_max_h;
	// We later correct these according to the storytext box location.
	// The text box is always aligned according to the base_rect_
	// (effective background area) at the end.
	titlebox_x = titlebox_padding;
	titlebox_max_w = base_rect_.w - 2*titlebox_padding;
	titlebox_y = titlebox_padding;
	titlebox_max_h = base_rect_.h - 2*titlebox_padding;

	font::ttext t;
	if(!t.set_text(titletxt, true)) {
		ERR_NG << "Text: Invalid markup in '"
				<< titletxt << "' rendered as is.\n";
		t.set_text(titletxt, false);
	}

	t.set_font_style(font::ttext::STYLE_NORMAL)
		 .set_font_size(titlebox_font_size)
		 .set_foreground_colour(titlebox_font_color)
		 .set_maximum_width(titlebox_max_w)
		 .set_maximum_height(titlebox_max_h);
	surface txtsurf = t.render();

	if(txtsurf.null()) {
		ERR_NG << "storyscreen titlebox rendering resulted in a null surface\n";
		return;
	}

	const int titlebox_w = txtsurf->w;
	const int titlebox_h = txtsurf->h;

	switch(p_.title_text_alignment()) {
	case part::TEXT_CENTERED:
		titlebox_x = base_rect_.w / 2 - titlebox_w / 2 - titlebox_padding;
		break;
	case part::TEXT_RIGHT:
		titlebox_x = base_rect_.w - titlebox_padding - titlebox_w;
		break;
	default:
		break; // already set before
	}

	draw_solid_tinted_rectangle(
		base_rect_.x + titlebox_x - titleshadow_padding,
		base_rect_.y + titlebox_y - titleshadow_padding,
		titlebox_w + 2*titleshadow_padding,
		titlebox_h + 2*titleshadow_padding,
		titleshadow_r, titleshadow_g, titleshadow_b,
		titleshadow_opacity,
		video_.getSurface()
	);

	video_.blit_surface(base_rect_.x + titlebox_x, base_rect_.y + titlebox_y, txtsurf);

	update_rect(
		static_cast<size_t>(std::max(0, base_rect_.x + titlebox_x)),
		static_cast<size_t>(std::max(0, base_rect_.y + titlebox_y)),
		static_cast<size_t>(std::max(0, titlebox_w)),
		static_cast<size_t>(std::max(0, titlebox_h))
	);
}

#ifdef LOW_MEM
void part_ui::render_story_box_borders(SDL_Rect& /*update_area*/)
{}
#else
void part_ui::render_story_box_borders(SDL_Rect& update_area)
{
	const part::BLOCK_LOCATION tbl = p_.story_text_location();

	if(has_background_) {
		surface border_top = NULL;
		surface border_bottom = NULL;

		if(tbl == part::BLOCK_BOTTOM || tbl == part::BLOCK_MIDDLE) {
			border_top = image::get_image(storybox_top_border_path);
		}

		if(tbl == part::BLOCK_TOP || tbl == part::BLOCK_MIDDLE) {
			border_bottom = image::get_image(storybox_bottom_border_path);
		}

		//
		// If one of those are null at this point, it means that either we
		// don't need that border pic, or it is missing (in such case get_image()
		// would report).
		//

		if(border_top.null() != true) {
			if((border_top = scale_surface_blended(border_top, screen_area().w, border_top->h)).null()) {
				WARN_NG << "storyscreen got a null top border surface after rescaling\n";
			}
			else {
				update_area.y -= border_top->h;
				update_area.h += border_top->h;
				blur_area(video_, update_area.y, border_top->h);
				video_.blit_surface(0, update_area.y, border_top);
			}
		}

		if(border_bottom.null() != true) {
			if((border_bottom = scale_surface_blended(border_bottom, screen_area().w, border_bottom->h)).null()) {
				WARN_NG << "storyscreen got a null bottom border surface after rescaling\n";
			}
			else {
				blur_area(video_, update_area.h, border_bottom->h);
				video_.blit_surface(0, update_area.y+update_area.h, border_bottom);
				update_area.h += border_bottom->h;
			}
		}
	}
}
#endif

void part_ui::render_story_box()
{
	const std::string& storytxt = p_.text();
	if(storytxt.empty()) {
		wait_for_input();
		return;
	}

	const part::BLOCK_LOCATION tbl = p_.story_text_location();
	const int max_width = buttons_x_ - storybox_padding - text_x_;
	const int max_height = screen_area().h - storybox_padding;

	bool skip = false, last_key = true;

	font::ttext t;
	if(!t.set_text(p_.text(), true)) {
		ERR_NG << "Text: Invalid markup in '"
				<< p_.text() << "' rendered as is.\n";
		t.set_text(p_.text(), false);
	}
	t.set_font_style(font::ttext::STYLE_NORMAL)
	     .set_font_size(storybox_font_size)
		 .set_foreground_colour(storybox_font_color)
		 .set_maximum_width(max_width)
		 .set_maximum_height(max_height);
	surface txtsurf = t.render();

	if(txtsurf.null()) {
		ERR_NG << "storyscreen text area rendering resulted in a null surface\n";
		return;
	}

	int fix_text_y = text_y_;
	if(fix_text_y + storybox_padding + txtsurf->h > screen_area().h && tbl != part::BLOCK_TOP) {
		fix_text_y =
			(screen_area().h > txtsurf->h + 1) ?
			(std::max(0, screen_area().h - txtsurf->h - (storybox_padding+1))) :
			(0);
	}
	int fix_text_h;
	switch(tbl) {
	case part::BLOCK_TOP:
		fix_text_h = std::max(txtsurf->h + 2*storybox_padding, screen_area().h/4);
		break;
	case part::BLOCK_MIDDLE:
		fix_text_h = std::max(txtsurf->h + 2*storybox_padding, screen_area().h/3);
		break;
	default:
		fix_text_h = screen_area().h - fix_text_y;
		break;
	}

	SDL_Rect update_area = { 0, fix_text_y, screen_area().w, fix_text_h };

	/* do */ {
		// this should kill the tiniest flickering caused
		// by the buttons being hidden and unhidden in this scope.
		update_locker locker(video_);

		//back_button_.hide();
		next_button_.hide();
		skip_button_.hide();
		//quit_button_.hide();

#ifndef LOW_MEM
		blur_area(video_, fix_text_y, fix_text_h);
#endif

		draw_solid_tinted_rectangle(
			0, fix_text_y, screen_area().w, fix_text_h,
			storyshadow_r, storyshadow_g, storyshadow_b,
			storyshadow_opacity,
			video_.getSurface()
		);

		render_story_box_borders(update_area); // no-op if LOW_MEM is defined

		// Make GUI1 buttons aware of background modifications
		next_button_.set_location(next_button_.location());
		next_button_.hide(false);
		skip_button_.set_location(skip_button_.location());
		skip_button_.hide(false);
	}

	if(imgs_.empty()) {
		update_whole_screen();
	} else if(update_area.h > 0) {
		update_rect(update_area);
	}

	// Time to do some fucking visual effect.
	const int scan_height = 1, scan_width = txtsurf->w;
	SDL_Rect scan = { 0, 0, scan_width, scan_height };
	SDL_Rect dstrect = { text_x_, 0, scan_width, scan_height };
	surface scan_dst = video_.getSurface();
	bool scan_finished = false;
	while(true) {
		if(!(scan_finished = scan.y >= txtsurf->h)) {
			//dstrect.x = text_x_;
			dstrect.y = fix_text_y + scan.y + storybox_padding;
			// NOTE: ::blit_surface() screws up with antialiasing and hinting when
			//       on backgroundless (e.g. black) screens; ttext::draw()
			//       uses it nonetheless, no idea why...
			//       Here we'll use CVideo::blit_surface() instead.
			video_.blit_surface(dstrect.x, dstrect.y, txtsurf, &scan);
			update_rect(dstrect);
			++scan.y;
		}

		const bool keydown = keys_[SDLK_SPACE] || keys_[SDLK_RETURN] || keys_[SDLK_KP_ENTER];

		if((keydown && !last_key) || next_button_.pressed()) {
			if(skip == true || scan_finished) {
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

		if(!skip || scan_finished) {
			disp_.delay(20);
		}
	}

	draw_solid_tinted_rectangle(
		0, 0, video_.getx(), video_.gety(), 0, 0, 0,
		1.0, video_.getSurface()
	);
}

void part_ui::wait_for_input()
{
	bool last_key = true;
	while(true) {
		const bool keydown = keys_[SDLK_SPACE] || keys_[SDLK_RETURN] || keys_[SDLK_KP_ENTER];

		if((keydown && !last_key) || next_button_.pressed()) {
			ret_ = NEXT;
			break;
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
		disp_.delay(20);
	}
}

part_ui::RESULT part_ui::show()
{
	if(p_.music().empty() != true) {
		sound::play_music_repeatedly(p_.music());
	}

	if(p_.sound().empty() != true) {
		sound::play_sound(p_.sound());
	}

	render_background();

	if(p_.show_title()) {
		render_title_box();
	}

	if(!imgs_.empty()) {
		if(!render_floating_images()) {
			return ret_;
		}
	}

	try {
		render_story_box();
	}
	catch(utils::invalid_utf8_exception const&) {
		ERR_NG << "invalid UTF-8 sequence in story text, skipping part...\n";
	}

	return ret_;
}

} // end namespace storyscreen
