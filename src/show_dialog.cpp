/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "display.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/event/handler.hpp"
#include "help.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"


static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
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
const dialog_frame::style dialog_frame::message_style("translucent65", 3);
const dialog_frame::style dialog_frame::preview_style("../dialogs/selection", 0);
const dialog_frame::style dialog_frame::titlescreen_style("translucent54", 1);

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
	SDL_GetMouseState(&mousex, &mousey);
	SDL_Event pb_event;
	pb_event.type = SDL_MOUSEMOTION;
	pb_event.motion.state = 0;
	pb_event.motion.x = mousex;
	pb_event.motion.y = mousey;
	pb_event.motion.xrel = 0;
	pb_event.motion.yrel = 0;
	SDL_PushEvent(&pb_event);
}

dialog_frame::dialog_frame(CVideo &video, const std::string& title,
		const style& style, bool auto_restore,
		std::vector<button*>* buttons, button* help_button) :
	title_(title),
	video_(video),
	dialog_style_(style),
	buttons_(buttons),
	help_button_(help_button),
	restorer_(NULL),
	auto_restore_(auto_restore),
	dim_(),
	top_(image::get_image("dialogs/" + dialog_style_.panel + "-border-top.png")),
	bot_(image::get_image("dialogs/" + dialog_style_.panel + "-border-bottom.png")),
	left_(image::get_image("dialogs/" + dialog_style_.panel + "-border-left.png")),
	right_(image::get_image("dialogs/" + dialog_style_.panel + "-border-right.png")),
	top_left_(image::get_image("dialogs/" + dialog_style_.panel + "-border-topleft.png")),
	bot_left_(image::get_image("dialogs/" + dialog_style_.panel + "-border-botleft.png")),
	top_right_(image::get_image("dialogs/" + dialog_style_.panel + "-border-topright.png")),
	bot_right_(image::get_image("dialogs/" + dialog_style_.panel + "-border-botright.png")),
	bg_(image::get_image("dialogs/" + dialog_style_.panel + "-background.png")),
	have_border_(top_ != NULL && bot_ != NULL && left_ != NULL && right_ != NULL)
{
}

dialog_frame::~dialog_frame()
{
	delete restorer_;
}

dialog_frame::dimension_measurements::dimension_measurements() :
	interior(empty_rect), exterior(empty_rect), title(empty_rect), button_row(empty_rect)
{}

dialog_frame::dimension_measurements dialog_frame::layout(SDL_Rect const& rect) {
	return layout(rect.x, rect.y, rect.w, rect.h);
}

int dialog_frame::top_padding() const {
	int padding = 0;
	if(have_border_) {
		padding += top_->h;
	}
	if(!title_.empty()) {
		padding += font::get_max_height(font::SIZE_LARGE) + 2*dialog_frame::title_border_h;
	}
	return padding;
}

int dialog_frame::bottom_padding() const {
	int padding = 0;
	if(buttons_ != NULL) {
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			padding = std::max<int>((**b).height() + ButtonVPadding, padding);
		}
	}
	if(have_border_) {
		padding += bot_->h;
	}
	return padding;
}

dialog_frame::dimension_measurements dialog_frame::layout(int x, int y, int w, int h) {
	dim_ = dimension_measurements();
	if(!title_.empty()) {
		dim_.title = draw_title(NULL);
		dim_.title.w += title_border_w;
	}
	if(buttons_ != NULL) {
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			dim_.button_row.w += (**b).width() + ButtonHPadding;
			dim_.button_row.h = std::max<int>((**b).height() + ButtonVPadding,dim_.button_row.h);
		}

		dim_.button_row.x = -dim_.button_row.w;
		dim_.button_row.y = y + h;

		dim_.button_row.w += ButtonHPadding;
	}

	size_t buttons_width = dim_.button_row.w;

	if(help_button_ != NULL) {
		buttons_width += help_button_->width() + ButtonHPadding*2;
		dim_.button_row.y = y + h;
	}

	y -= dim_.title.h;
	w = std::max<int>(w,std::max<int>(int(dim_.title.w),int(buttons_width)));
	h += dim_.title.h + dim_.button_row.h;
	dim_.button_row.x += x + w;

	SDL_Rect bounds = screen_area();
	if(have_border_) {
		bounds.x += left_->w;
		bounds.y += top_->h;
		bounds.w -= left_->w;
		bounds.h -= top_->h;
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
		dim_.exterior.x = dim_.interior.x - left_->w;
		dim_.exterior.y = dim_.interior.y - top_->h;
		dim_.exterior.w = dim_.interior.w + left_->w + right_->w;
		dim_.exterior.h = dim_.interior.h + top_->h + bot_->h;
	} else {
		dim_.exterior = dim_.interior;
	}
	dim_.title.x = dim_.interior.x + title_border_w;
	dim_.title.y = dim_.interior.y + title_border_h;
	return dim_;
}

void dialog_frame::draw_border()
{
	if(have_border_ == false) {
		return;
	}

	surface top_image(scale_surface(top_, dim_.interior.w, top_->h));

	if(top_image != NULL) {
		video_.blit_surface(dim_.interior.x, dim_.exterior.y, top_image);
	}

	surface bot_image(scale_surface(bot_, dim_.interior.w, bot_->h));

	if(bot_image != NULL) {
		video_.blit_surface(dim_.interior.x, dim_.interior.y + dim_.interior.h, bot_image);
	}

	surface left_image(scale_surface(left_, left_->w, dim_.interior.h));

	if(left_image != NULL) {
		video_.blit_surface(dim_.exterior.x, dim_.interior.y, left_image);
	}

	surface right_image(scale_surface(right_, right_->w, dim_.interior.h));

	if(right_image != NULL) {
		video_.blit_surface(dim_.interior.x + dim_.interior.w, dim_.interior.y, right_image);
	}

	update_rect(dim_.exterior);

	if(top_left_ == NULL || bot_left_ == NULL || top_right_ == NULL || bot_right_ == NULL) {
		return;
	}

	video_.blit_surface(dim_.interior.x - left_->w, dim_.interior.y - top_->h, top_left_);
	video_.blit_surface(dim_.interior.x - left_->w, dim_.interior.y + dim_.interior.h + bot_->h - bot_left_->h, bot_left_);
	video_.blit_surface(dim_.interior.x + dim_.interior.w + right_->w - top_right_->w, dim_.interior.y - top_->h, top_right_);
	video_.blit_surface(dim_.interior.x + dim_.interior.w + right_->w - bot_right_->w, dim_.interior.y + dim_.interior.h + bot_->h - bot_right_->h, bot_right_);
}

void dialog_frame::clear_background()
{
	delete restorer_;
	restorer_ = NULL;
}

void dialog_frame::draw_background()
{
	if(auto_restore_) {
		clear_background();
		restorer_ = new surface_restorer(&video_, dim_.exterior);
	}

	if (dialog_style_.blur_radius) {
		surface surf = ::get_surface_portion(video_.getSurface(), dim_.exterior);
		surf = blur_surface(surf, dialog_style_.blur_radius, false);
		sdl_blit(surf, NULL, video_.getSurface(), &dim_.exterior);
	}

	if(bg_ == NULL) {
		ERR_DP << "could not find dialog background '" << dialog_style_.panel << "'\n";
		return;
	}
	for(int i = 0; i < dim_.interior.w; i += bg_->w) {
		for(int j = 0; j < dim_.interior.h; j += bg_->h) {
			SDL_Rect src = {0,0,0,0};
			src.w = std::min(dim_.interior.w - i, bg_->w);
			src.h = std::min(dim_.interior.h - j, bg_->h);
			SDL_Rect dst = src;
			dst.x = dim_.interior.x + i;
			dst.y = dim_.interior.y + j;
			sdl_blit(bg_, &src, video_.getSurface(), &dst);
		}
	}
}

SDL_Rect dialog_frame::draw_title(CVideo* video)
{
	SDL_Rect rect = {0, 0, 10000, 10000};
	rect = screen_area();
	return font::draw_text(video, rect, font::SIZE_LARGE, font::TITLE_COLOR,
	                       title_, dim_.title.x, dim_.title.y, false, TTF_STYLE_BOLD);
}

void dialog_frame::draw()
{
	//draw background
	draw_background();

	//draw frame border
	draw_border();

	//draw title
	if (!title_.empty()) {
		draw_title(&video_);
	}

	//draw buttons
	SDL_Rect buttons_area = dim_.button_row;
	if(buttons_ != NULL) {
#ifdef OK_BUTTON_ON_RIGHT
		std::reverse(buttons_->begin(),buttons_->end());
#endif
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			(**b).set_location(buttons_area.x, buttons_area.y);
			buttons_area.x += (**b).width() + ButtonHPadding;
		}
	}

	if(help_button_ != NULL) {
		help_button_->set_location(dim_.interior.x+ButtonHPadding, buttons_area.y);
	}
}

} //end namespace gui

namespace {

struct help_handler : public hotkey::command_executor
{
	help_handler(display& disp, const std::string& topic) : disp_(disp), topic_(topic)
	{}

private:
	void show_help()
	{
		if(topic_.empty() == false) {
			help::show_help(disp_,topic_);
		}
	}

	bool can_execute_command(const hotkey::hotkey_command& cmd, int /*index*/) const
	{
		hotkey::HOTKEY_COMMAND command = cmd.id;
		return (topic_.empty() == false && command == hotkey::HOTKEY_HELP) || command == hotkey::HOTKEY_SCREENSHOT;
	}

	display& disp_;
	std::string topic_;
};

}

namespace gui
{

int show_dialog(display& screen, surface image,
				const std::string& caption, const std::string& message,
				DIALOG_TYPE type,
				const std::vector<std::string>* menu_items,
				const std::vector<preview_pane*>* preview_panes,
				const std::string& text_widget_label,
				std::string* text_widget_text,
				const int text_widget_max_chars,
				std::vector<check_item>* options,
				int xloc,
				int yloc,
				const dialog_frame::style* dialog_style,
				std::vector<dialog_button_info>* action_buttons,
				const menu::sorter* sorter,
				menu::style* menu_style)
{
	std::string title;
	if (image.null()) title = caption;
	const dialog::style& style = (dialog_style)? *dialog_style : dialog::default_style;
	CVideo &disp = screen.video();

	gui::dialog d(screen, title, message, type, style);

	//add the components
	if(!image.null()) {
		d.set_image(image, caption);
	}
	if(menu_items) {
		d.set_menu( new gui::menu(disp,*menu_items,type == MESSAGE,-1,dialog::max_menu_width,sorter,menu_style,false));
	}
	if(preview_panes) {
		for(unsigned int i=0; i < preview_panes->size(); ++i) {
			d.add_pane((*preview_panes)[i]);
		}
	}
	if(text_widget_text) {
		d.set_textbox(text_widget_label,*text_widget_text, text_widget_max_chars);
	}
	if(options) {
		for(unsigned int i=0; i < options->size(); ++i) {
			check_item& item = (*options)[i];
			d.add_option(item.label, item.checked);
		}
	}
	if(action_buttons) {
		for(unsigned int i=0; i < action_buttons->size(); ++i) {
			d.add_button((*action_buttons)[i]);
		}
	}
	//enter the dialog loop
	d.show(xloc, yloc);

	//send back results
	if(options) {
		for(unsigned int i=0; i < options->size(); ++i)
		{
			(*options)[i].checked = d.option_checked(i);
		}
	}
	if(text_widget_text) {
		*text_widget_text = d.textbox_text();
	}
	return d.result();
}

}

