/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2016 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * @file
 * Storyscreen parts renderer.
 * @todo Translate relevant parts to GUI2.
 */

#include "global.hpp"
#include <cassert>
#include "log.hpp"
#include "storyscreen/part.hpp"
#include "storyscreen/render.hpp"

#include "config.hpp"
#include "font/text.hpp"
#include "image.hpp"
#include "language.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG  LOG_STREAM(err,  log_engine)
#define WARN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG  LOG_STREAM(info, log_engine)


namespace {
	const int storybox_padding = 10; // px
	const double storyshadow_opacity = 0.5;
	const int storyshadow_r = 0;
	const int storyshadow_g = 0;
	const int storyshadow_b = 0;

	const int titlebox_padding = 20; // px
	const int titleshadow_padding = 5; // px
	const double titleshadow_opacity = 0.5;
	const int titleshadow_r = 0;
	const int titleshadow_g = 0;
	const int titleshadow_b = 0;

	const int titlebox_font_size = 20; // pt?
	const int storybox_font_size = 17; // pt?

	const Uint32 titlebox_font_color = 0xFFFFFFFF;
	const Uint32 storybox_font_color = 0xDDDDDDFF;

	// Hard-coded path to a suitable (tileable) pic for the storytxt box border.
	const std::string storybox_top_border_path = "dialogs/translucent54-border-top.png";
	const std::string storybox_bottom_border_path = "dialogs/translucent54-border-bottom.png";
	void blur_area(CVideo& video, int y, int h)
	{
		SDL_Rect blur_rect = sdl::create_rect(0, y, screen_area().w, h);
		surface blur = get_surface_portion(video.getSurface(), blur_rect);
		blur = blur_surface(blur, 1, false);
		video.blit_surface(0, y, blur);
	}
}

namespace storyscreen {

part_ui::part_ui(part &p, CVideo& video, gui::button &next_button,
	gui::button &back_button, gui::button&play_button)
	: events::sdl_handler(false)
	, p_(p)
	, video_(video)
	, keys_()
	, next_button_(next_button)
	, back_button_(back_button)
	, play_button_(play_button)
	, dirty_(true)
	, ret_(NEXT), skip_(false), last_key_(false)
	, x_scale_factor_(1.0)
	, y_scale_factor_(1.0)
	, base_rect_()
	, background_(nullptr)
	, imgs_()
	, has_background_(false)
	, text_x_(200)
	, text_y_(400)
	, buttons_x_(0)
	, buttons_y_(0)
{

}

void part_ui::prepare_background()
{
	background_.assign( create_neutral_surface(video_.getx(), video_.gety()) );
	base_rect_.w = video_.getx();
	base_rect_.h = video_.gety();
	has_background_ = false;
	bool no_base_yet = true;

	// Build background surface
	for (const background_layer& bl : p_.get_background_layers()) {
		surface layer;

		if(bl.file().empty() != true) {
			layer.assign( image::get_image(bl.file()) );
		}
		has_background_ = has_background_ || !layer.null();
		if(layer.null() || layer->w * layer->h == 0) {
			continue;
		}

		layer = make_neutral_surface(layer);

		const double xscale = 1.0 * video_.getx() / layer->w;
		const double yscale = 1.0 * video_.gety() / layer->h;
		const bool scalev = bl.scale_vertically();
		const bool scaleh = bl.scale_horizontally();
		const bool keep_ratio = bl.keep_aspect_ratio();

		double x_scale_factor = scaleh ? xscale : 1.0;
		double y_scale_factor = scalev ? yscale : 1.0;

		if (scalev && scaleh && keep_ratio) {
			x_scale_factor = y_scale_factor = std::min<double>(xscale, yscale);
		} else if (keep_ratio && scaleh) {
			x_scale_factor = y_scale_factor = xscale;
		} else if (keep_ratio && scalev) {
			x_scale_factor = y_scale_factor = yscale;
		}

		layer = scale_surface(layer, static_cast<int>(layer->w*x_scale_factor), static_cast<int>(layer->h*y_scale_factor), false);

		const int tilew = bl.tile_horizontally() ? video_.getx() : layer->w;
		const int tileh = bl.tile_vertically() ? video_.gety() : layer->h;

		layer = tile_surface(layer, tilew, tileh, false);

		SDL_Rect drect = sdl::create_rect(
				  (background_->w - layer->w) / 2
				, (background_->h - layer->h) / 2
				, layer->w
				, layer->h);
		SDL_Rect srect = sdl::create_rect(
				  0
				, 0
				, layer->w
				, layer->h);
		SDL_Rect base_rect = drect;

		// If we can't see the whole image anyways, we'll want to display the
		// top-middle area.
		if (drect.y < 0) {
			drect.y = 0;
			base_rect.y = 0;
		}

		if (drect.x < 0) {
			srect.x -= drect.x;
			drect.x = 0;
		}

		sdl_blit(layer, &srect, background_, &drect);
		assert(layer.null() == false && "Oops: a storyscreen part background layer got nullptr");

		if (bl.is_base_layer() || no_base_yet) {
			x_scale_factor_ = x_scale_factor;
			y_scale_factor_ = y_scale_factor;
			base_rect_ = base_rect;
			no_base_yet = false;
		}
	}
}

void part_ui::prepare_geometry()
{
	if(video_.getx() <= 800) {
		text_x_ = 10;
		buttons_x_ = video_.getx() - 100 - 20;
	}
	else {
		text_x_ = 100 - 40;
		buttons_x_ = video_.getx() - 100 - 40;
	}

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

	back_button_.set_location(buttons_x_, buttons_y_ - 30);
	next_button_.set_location(buttons_x_ + play_button_.width() - next_button_.width(), buttons_y_ - 30);
	play_button_.set_location(buttons_x_, buttons_y_);
}

void part_ui::prepare_floating_images()
{
	// Build floating image surfaces
	for (const floating_image& fi : p_.get_floating_images()) {
		imgs_.push_back( fi.get_render_input(x_scale_factor_, y_scale_factor_, base_rect_) );
	}
}

void part_ui::render_background()
{
	sdl::draw_solid_tinted_rectangle(
			0, 0, video_.getx(), video_.gety(), 0, 0, 0, 1.0,
			video_.getSurface()
	);
	sdl_blit(background_, nullptr, video_.getSurface(), nullptr);
	// Render the titlebox over the background
	render_title_box();
}

bool part_ui::render_floating_images()
{
	events::raise_draw_event();

	skip_ = false;
	last_key_ = true;

	size_t fi_n = 0;
	for (floating_image::render_input& ri : imgs_) {
		const floating_image& fi = p_.get_floating_images()[fi_n];

		if(!ri.image.null()) {
			render_background();
			for (size_t i = 0; i <= fi_n; i++)
			{
				floating_image::render_input& old_ri = imgs_[i];
				sdl_blit(old_ri.image, nullptr, video_.getSurface(), &old_ri.rect);
				update_rect(old_ri.rect);
			}
		}
		back_button_.set_dirty();
		next_button_.set_dirty();
		play_button_.set_dirty();

		if (!skip_)
		{
			int delay = fi.display_delay(), delay_step = 20;
			for (int i = 0; i != (delay + delay_step - 1) / delay_step; ++i)
			{
				if (handle_interface()) return false;
				if (skip_) break;
				CVideo::delay(std::min<int>(delay_step, delay - i * delay_step));
			}
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

	font::pango_text t;
	if(!t.set_text(titletxt, true)) {
		ERR_NG << "Text: Invalid markup in '"
				<< titletxt << "' rendered as is.\n";
		t.set_text(titletxt, false);
	}

	t.set_font_style(font::pango_text::STYLE_NORMAL)
		 .set_font_size(titlebox_font_size)
		 .set_foreground_color(titlebox_font_color)
		 .set_maximum_width(titlebox_max_w)
		 .set_maximum_height(titlebox_max_h, true);
	surface txtsurf = t.render();

	if(txtsurf.null()) {
		ERR_NG << "storyscreen titlebox rendering resulted in a null surface" << std::endl;
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

	update_locker locker(video_);

	next_button_.hide();
	back_button_.hide();
	play_button_.hide();

	sdl::draw_solid_tinted_rectangle(
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

	next_button_.hide(false);
	back_button_.hide(false);
	play_button_.hide(false);

}

void part_ui::render_story_box_borders(SDL_Rect& update_area)
{
	const part::BLOCK_LOCATION tbl = p_.story_text_location();

	if(has_background_) {
		surface border_top = nullptr;
		surface border_bottom = nullptr;

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
			if((border_top = scale_surface(border_top, screen_area().w, border_top->h)).null()) {
				WARN_NG << "storyscreen got a null top border surface after rescaling" << std::endl;
			}
			else {
				update_area.y -= border_top->h;
				update_area.h += border_top->h;
				blur_area(video_, update_area.y, border_top->h);
				video_.blit_surface(0, update_area.y, border_top);
			}
		}

		if(border_bottom.null() != true) {
			if((border_bottom = scale_surface(border_bottom, screen_area().w, border_bottom->h)).null()) {
				WARN_NG << "storyscreen got a null bottom border surface after rescaling" << std::endl;
			}
			else {
				blur_area(video_, update_area.h, border_bottom->h);
				video_.blit_surface(0, update_area.y+update_area.h, border_bottom);
				update_area.h += border_bottom->h;
			}
		}
	}
}

void part_ui::render_story_box()
{

	LOG_NG<< "ENTER part_ui()::render_story_box()\n";
	bool first = true;

	render_background();

	if(p_.show_title()) {
		render_title_box();
	}

	if(!imgs_.empty()) {
		if(!render_floating_images()) {
			return;
		}
	}


	const std::string& storytxt = p_.text();
	if(storytxt.empty()) {
		wait_for_input();
		return;
	}

	skip_ = false;
	last_key_ = true;
	font::pango_text t;
	bool scan_finished = false;
	SDL_Rect scan = {0,0,0,0};
	SDL_Rect dstrect = {0,0,0,0};


	// Convert the story part text alignment types into the Pango equivalents
	PangoAlignment story_text_alignment = PANGO_ALIGN_LEFT;

	switch(p_.story_text_alignment()) {
	case part::TEXT_CENTERED:
		story_text_alignment = PANGO_ALIGN_CENTER;
		break;
	case part::TEXT_RIGHT:
		story_text_alignment = PANGO_ALIGN_RIGHT;
		break;
	default:
		break; // already set before
	}


	while(true) {

		if (dirty_) {

			render_background();

			if(p_.show_title()) {
				render_title_box();
			}

			if(!imgs_.empty()) {
				if(!render_floating_images()) {
					return;
				}
			}
		}

		part::BLOCK_LOCATION tbl = p_.story_text_location();
		int max_width = buttons_x_ - storybox_padding - text_x_;
		int max_height = screen_area().h - storybox_padding;

		if(!t.set_text(p_.text(), true)) {
			ERR_NG << "Text: Invalid markup in '"
					<< p_.text() << "' rendered as is.\n";
			t.set_text(p_.text(), false);
		}
		t.set_font_style(font::pango_text::STYLE_NORMAL)
				.set_alignment(story_text_alignment)
				.set_font_size(storybox_font_size)
				.set_foreground_color(storybox_font_color)
				.set_maximum_width(max_width)
				.set_maximum_height(max_height, true);

		surface txtsurf = t.render();

		if(txtsurf.null()) {
			ERR_NG << "storyscreen text area rendering resulted in a null surface" << std::endl;
			return;
		}

		int fix_text_y = text_y_;
		if(fix_text_y + 2*(storybox_padding+1) + txtsurf->h > screen_area().h && tbl != part::BLOCK_TOP) {
			fix_text_y =
			(screen_area().h > txtsurf->h + 1) ?
			(std::max(0, screen_area().h - txtsurf->h - 2*(storybox_padding+1))) :
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

		SDL_Rect update_area = sdl::create_rect(0
				, fix_text_y
				, screen_area().w
				, fix_text_h);

		/* do */{
			// this should kill the tiniest flickering caused
			// by the buttons being hidden and unhidden in this scope.
			update_locker locker(video_);

			next_button_.hide();
			back_button_.hide();
			play_button_.hide();

			if (dirty_ || first) {
				blur_area(video_, fix_text_y, fix_text_h);
			}
			if (dirty_ || first) {
				sdl::draw_solid_tinted_rectangle(
						0, fix_text_y, screen_area().w, fix_text_h,
						storyshadow_r, storyshadow_g, storyshadow_b,
						storyshadow_opacity,
						video_.getSurface()
				);
			}

			render_story_box_borders(update_area);

			next_button_.hide(false);
			back_button_.hide(false);
			play_button_.hide(false);
		}

		if (dirty_ || first) {
			if(!imgs_.empty() && update_area.h > 0) {
				update_rect(update_area);
			}
		}

		// Time to do some visual effecta.
		if (dirty_ || first) {
			const int scan_height = 1, scan_width = txtsurf->w;
			scan = sdl::create_rect(0, 0, scan_width, scan_height);
			dstrect = sdl::create_rect(text_x_, 0, scan_width, scan_height);
		}

		/* This needs to happen before poll for events */
		dirty_ = false;
		first = false;

		scan_finished = scan.y >= txtsurf->h;
		if (!scan_finished)
		{
			//dstrect.x = text_x_;
			dstrect.y = fix_text_y + scan.y + storybox_padding;
			// NOTE: ::blit_surface() screws up with antialiasing and hinting when
			//       on backgroundless (e.g. black) screens; pango_text::draw()
			//       uses it nonetheless, no idea why...
			//       Here we'll use CVideo::blit_surface() instead.
			video_.blit_surface(dstrect.x, dstrect.y, txtsurf, &scan);
			update_rect(dstrect);
			++scan.y;
		}
		else {
			skip_ = true;
		}

		if (handle_interface()) break;

		if (!skip_ || scan_finished) {
			CVideo::delay(20);
		}

	}

	sdl::draw_solid_tinted_rectangle(
			0, 0, video_.getx(), video_.gety(), 0, 0, 0,
			1.0, video_.getSurface()
	);
}

void part_ui::wait_for_input()
{
	LOG_NG << "ENTER part_ui()::wait_for_input()\n";

	last_key_ = true;
	skip_ = true;
	while (!handle_interface()) {
		CVideo::delay(20);
	}
}

bool part_ui::handle_interface()
{
	bool result = false;

	bool next_keydown = keys_[SDLK_SPACE] || keys_[SDLK_RETURN] ||
		keys_[SDLK_KP_ENTER] || keys_[SDLK_RIGHT];
	bool back_keydown = keys_[SDLK_BACKSPACE] || keys_[SDLK_LEFT];
	bool play_keydown = keys_[SDLK_ESCAPE];

	if ((next_keydown && !last_key_) || next_button_.pressed())
	{
		next_button_.release();
		if (skip_) {
			ret_ = NEXT;
			result = true;
		} else {
			skip_ = true;
		}
	}

	if ((play_keydown && !last_key_) || play_button_.pressed()) {
		play_button_.release();
		ret_ = QUIT;
		result = true;
	}

	if ((back_keydown && !last_key_) || back_button_.pressed()) {
		back_button_.release();
		ret_ = BACK;
		result = true;
	}

	last_key_ = next_keydown || back_keydown || play_keydown;

	events::pump();
	events::raise_process_event();
	events::raise_draw_event();
	video_.flip();

	return result;
}

part_ui::RESULT part_ui::show()
{
	update_locker locker(video_);

	this->prepare_background();
	this->prepare_geometry();
	this->prepare_floating_images();

	if(p_.music().empty() != true) {
		config music_config;
		music_config["name"] = p_.music();
		music_config["ms_after"] = 2000;
		music_config["immediate"] = true;

		sound::play_music_config(music_config);
	}

	if(p_.sound().empty() != true) {
		sound::play_sound(p_.sound());
	}

	join();
	try {
		render_story_box();
	}
	catch(utf8::invalid_utf8_exception const&) {
		ERR_NG << "invalid UTF-8 sequence in story text, skipping part..." << std::endl;
	}

	leave();

	return ret_;
}

void part_ui::handle_event(const SDL_Event &event)
{
	if (event.type == DRAW_ALL_EVENT) {
		dirty_ = true;
		draw();
	}

}

void part_ui::handle_window_event(const SDL_Event &event)
{

	if (event.type == SDL_WINDOWEVENT &&
			(event.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
					event.window.event == SDL_WINDOWEVENT_RESIZED ||
					event.window.event == SDL_WINDOWEVENT_EXPOSED ||
					event.window.event == SDL_WINDOWEVENT_RESTORED)) {



		this->prepare_background();
		this->prepare_geometry();
		this->prepare_floating_images();
		dirty_ = true;
	}
}


} // end namespace storyscreen
