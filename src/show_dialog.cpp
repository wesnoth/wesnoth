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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "show_dialog.hpp"

#include "draw.hpp"
#include "draw_manager.hpp"
#include "floating_label.hpp"
#include "picture.hpp"
#include "gettext.hpp"
#include "gui/core/event/handler.hpp" // is_in_dialog
#include "help/help.hpp"
#include "hotkey/command_executor.hpp"
#include "log.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "font/standard_colors.hpp"
#include "sdl/rect.hpp"
#include "sdl/input.hpp" // get_mouse_state
#include "sdl/utils.hpp" // blur_surface
#include "video.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)
#define DBG_DP LOG_STREAM(debug, log_display)
#define ERR_G  LOG_STREAM(err, lg::general)

namespace {
bool is_in_dialog = false;
}

namespace gui {

//static initialization
const int ButtonHPadding = 10;
const int ButtonVPadding = 10;

//note: style names are directly related to the panel image file names
const dialog_frame::style dialog_frame::default_style("opaque", 0);

const int dialog_frame::title_border_w = 10;
const int dialog_frame::title_border_h = 5;



bool in_dialog()
{
	return is_in_dialog || gui2::is_in_dialog();
}

dialog_manager::dialog_manager() : cursor::setter(cursor::NORMAL), reset_to(is_in_dialog)
{
	is_in_dialog = true;
}

dialog_manager::~dialog_manager()
{
	is_in_dialog = reset_to;
	int mousex, mousey;
	sdl::get_mouse_state(&mousex, &mousey);
	SDL_Event pb_event;
	pb_event.type = SDL_MOUSEMOTION;
	pb_event.motion.state = 0;
	pb_event.motion.x = mousex;
	pb_event.motion.y = mousey;
	pb_event.motion.xrel = 0;
	pb_event.motion.yrel = 0;
	SDL_PushEvent(&pb_event);
}

dialog_frame::dialog_frame(const std::string& title,
		const style& style,
		std::vector<button*>* buttons, button* help_button) :
	title_(title),
	dialog_style_(style),
	buttons_(buttons),
	help_button_(help_button),
	dim_(),
	top_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-top.png")),
	bot_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-bottom.png")),
	left_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-left.png")),
	right_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-right.png")),
	top_left_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-topleft.png")),
	bot_left_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-botleft.png")),
	top_right_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-topright.png")),
	bot_right_(image::get_texture("dialogs/" + dialog_style_.panel + "-border-botright.png")),
	bg_(image::get_texture("dialogs/" + dialog_style_.panel + "-background.png")),
	have_border_(top_ && bot_ && left_ && right_),
	dirty_(true)
{
	// Raise buttons so they are drawn on top.
	// and BTW buttons_ being a pointer to a vector is fucking insane
	for (button* b : *buttons_) {
		draw_manager::raise_drawable(b);
	}
	if (help_button) {
		draw_manager::raise_drawable(help_button_);
	}
}

dialog_frame::~dialog_frame()
{
	draw_manager::invalidate_region(screen_location());
}

dialog_frame::dimension_measurements::dimension_measurements() :
	interior(sdl::empty_rect), exterior(sdl::empty_rect), title(sdl::empty_rect), button_row(sdl::empty_rect)
{}

dialog_frame::dimension_measurements dialog_frame::layout(const SDL_Rect& rect)
{
	return layout(rect.x, rect.y, rect.w, rect.h);
}

int dialog_frame::top_padding() const
{
	int padding = 0;
	if(have_border_) {
		padding += top_.h();
	}
	if(!title_.empty()) {
		padding += font::get_max_height(font::SIZE_TITLE) + 2*dialog_frame::title_border_h;
	}
	return padding;
}

void dialog_frame::set_dirty(bool dirty)
{
	dirty_ = dirty;
}

int dialog_frame::bottom_padding() const {
	int padding = 0;
	if(buttons_ != nullptr) {
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			padding = std::max<int>((**b).height() + ButtonVPadding, padding);
		}
	}
	if(have_border_) {
		padding += bot_.h();
	}
	return padding;
}

dialog_frame::dimension_measurements dialog_frame::layout(int x, int y, int w, int h) {
	dim_ = dimension_measurements();
	if(!title_.empty()) {
		dim_.title = draw_title(false);
		dim_.title.w += title_border_w;
	}
	if(buttons_ != nullptr) {
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			dim_.button_row.w += (**b).width() + ButtonHPadding;
			dim_.button_row.h = std::max<int>((**b).height() + ButtonVPadding,dim_.button_row.h);
		}

		dim_.button_row.x = -dim_.button_row.w;
		dim_.button_row.y = y + h;

		dim_.button_row.w += ButtonHPadding;
	}

	std::size_t buttons_width = dim_.button_row.w;

	if(help_button_ != nullptr) {
		buttons_width += help_button_->width() + ButtonHPadding*2;
		dim_.button_row.y = y + h;
	}

	y -= dim_.title.h;
	w = std::max(w, std::max(dim_.title.w, static_cast<int>(buttons_width)));
	h += dim_.title.h + dim_.button_row.h;
	dim_.button_row.x += x + w;

	rect bounds = video::game_canvas();
	if(have_border_) {
		bounds.x += left_.w();
		bounds.y += top_.h();
		bounds.w -= left_.w();
		bounds.h -= top_.h();
	}
	if(x < bounds.x) {
		w += x;
		x = bounds.x;
	}
	if(y < bounds.y) {
		h += y;
		y = bounds.y;
	}
	if(x > bounds.w) {
		w = 0;
	} else if(x + w > bounds.w) {
		w = bounds.w - x;
	}
	if(y > bounds.h) {
		h = 0;
	} else if(y + h > bounds.h) {
		h = bounds.h - y;
	}
	dim_.interior.x = x;
	dim_.interior.y = y;
	dim_.interior.w = w;
	dim_.interior.h = h;
	if(have_border_) {
		dim_.exterior.x = dim_.interior.x - left_.w();
		dim_.exterior.y = dim_.interior.y - top_.h();
		dim_.exterior.w = dim_.interior.w + left_.w() + right_.w();
		dim_.exterior.h = dim_.interior.h + top_.h() + bot_.h();
	} else {
		dim_.exterior = dim_.interior;
	}
	dim_.title.x = dim_.interior.x + title_border_w;
	dim_.title.y = dim_.interior.y + title_border_h;

	draw_manager::invalidate_region(dim_.exterior);

	return dim_;
}

void dialog_frame::draw_border()
{
	if(have_border_ == false) {
		return;
	}

	// Too much typing is bad for you.
	const SDL_Rect& i = dim_.interior;
	const SDL_Rect& e = dim_.exterior;

	if(top_) {
		draw::blit(top_, {i.x, e.y, i.w, top_.h()});
	}

	if(bot_) {
		draw::blit(bot_, {i.x, i.y + i.h, i.w, bot_.h()});
	}

	if(left_) {
		draw::blit(left_, {e.x, i.y, left_.w(), i.h});
	}

	if(right_) {
		draw::blit(right_, {i.x + i.w, i.y, right_.w(), i.h});
	}

	if(!top_left_ || !bot_left_ || !top_right_ || !bot_right_) {
		return;
	}

	draw::blit(top_left_,
		{i.x - left_.w(), i.y - top_.h(), top_left_.w(), top_left_.h()});

	draw::blit(bot_left_, {
		i.x - left_.w(),
		i.y + i.h + bot_.h() - bot_left_.h(),
		bot_left_.w(),
		bot_left_.h()
	});

	draw::blit(top_right_, {
		i.x + i.w + right_.w() - top_right_.w(),
		i.y - top_.h(),
		top_right_.w(),
		top_right_.h(),
	});

	draw::blit(bot_right_, {
		i.x + i.w + right_.w() - bot_right_.w(),
		i.y + i.h + bot_.h() - bot_right_.h(),
		bot_right_.w(),
		bot_right_.h()
	});
}

void dialog_frame::draw_background()
{
	if (dialog_style_.blur_radius) {
		// This is no longer used by anything.
		// The only thing that uses dialog_frame is help/help.cpp,
		// and it uses the default style with no blur.
		ERR_DP << "GUI1 dialog_frame blur has been removed";
	}

	if (!bg_) {
		ERR_DP << "could not find dialog background '" << dialog_style_.panel << "'";
		return;
	}

	auto clipper = draw::reduce_clip(dim_.interior);
	for(int i = 0; i < dim_.interior.w; i += bg_.w()) {
		for(int j = 0; j < dim_.interior.h; j += bg_.h()) {
			SDL_Rect src {0,0,0,0};
			src.w = std::min(dim_.interior.w - i, bg_.w());
			src.h = std::min(dim_.interior.h - j, bg_.h());
			SDL_Rect dst = src;
			dst.x = dim_.interior.x + i;
			dst.y = dim_.interior.y + j;
			draw::blit(bg_, dst);
		}
	}
}

rect dialog_frame::draw_title(bool actually_draw)
{
	rect r = video::game_canvas();
	return font::pango_draw_text(
		actually_draw, r, font::SIZE_TITLE, font::TITLE_COLOR, title_,
		dim_.title.x, dim_.title.y, false, font::pango_text::STYLE_NORMAL
	);
}

void dialog_frame::draw()
{
	//draw background
	draw_background();

	//draw frame border
	draw_border();

	//draw title
	if (!title_.empty()) {
		draw_title(true);
	}
}

void dialog_frame::layout()
{
	if (!dirty_) {
		return;
	}

	// Layout buttons
	SDL_Rect buttons_area = dim_.button_row;
	if(buttons_ != nullptr) {
#ifdef OK_BUTTON_ON_RIGHT
		std::reverse(buttons_->begin(),buttons_->end());
#endif
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			(**b).set_location(buttons_area.x, buttons_area.y);
			buttons_area.x += (**b).width() + ButtonHPadding;
		}
	}

	// Layout help button, if any
	if(help_button_ != nullptr) {
		help_button_->set_location(dim_.interior.x+ButtonHPadding, buttons_area.y);
	}

	dirty_ = false;
}

bool dialog_frame::expose(const rect& region)
{
	DBG_DP << "dialog_frame::expose " << region;
	// Just draw everthing.
	draw();
	return true;
}

rect dialog_frame::screen_location()
{
	return dim_.exterior;
}

}
