/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "construct_dialog.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "key.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "thread.hpp"
#include "language.hpp"
#include "sdl_utils.hpp"
#include "tooltips.hpp"
#include "util.hpp"
#include "video.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "widgets/progressbar.hpp"
#include "widgets/textbox.hpp"
#include "wassert.hpp"

#include "sdl_ttf/SDL_ttf.h"

#include <iostream>
#include <numeric>

#define ERR_DP LOG_STREAM(err, display)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_G  LOG_STREAM(err, general)

namespace {
bool is_in_dialog = false;
}

namespace gui {

//static initialization
const int ButtonHPadding = 10;
const int ButtonVPadding = 10;

static const struct style &default_style = dialog::default_style; //{"opaque", false}

const int dialog_frame::title_border_w = 10;
const int dialog_frame::title_border_h = 5;



bool in_dialog() { return is_in_dialog; }

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
	 const struct style *style, std::vector<button*>* buttons,
	 surface_restorer* restorer, button* help_button) : title_(title), 
	 video_(video), dialog_style_(style ? style : &default_style),
	 buttons_(buttons), help_button_(help_button), restorer_(restorer),
	 top_(image::get_image("dialogs/" + dialog_style_->panel + "-border-top.png",image::UNSCALED)),
	 bot_(image::get_image("dialogs/" + dialog_style_->panel + "-border-bottom.png",image::UNSCALED)),
	 left_(image::get_image("dialogs/" + dialog_style_->panel + "-border-left.png",image::UNSCALED)),
	 right_(image::get_image("dialogs/" + dialog_style_->panel + "-border-right.png",image::UNSCALED)),
	 top_left_(image::get_image("dialogs/" + dialog_style_->panel + "-border-topleft.png",image::UNSCALED)),
	 bot_left_(image::get_image("dialogs/" + dialog_style_->panel + "-border-botleft.png",image::UNSCALED)),
	 top_right_(image::get_image("dialogs/" + dialog_style_->panel + "-border-topright.png",image::UNSCALED)),
	 bot_right_(image::get_image("dialogs/" + dialog_style_->panel + "-border-botright.png",image::UNSCALED)),
	 bg_(image::get_image("dialogs/" + dialog_style_->panel + "-background.png",image::UNSCALED))
{
	have_border_ = top_ != NULL && bot_ != NULL && left_ != NULL && right_ != NULL;
}

dialog_frame::~dialog_frame()
{}

dialog_frame::dimension_measurements::dimension_measurements() :
	interior(empty_rect), exterior(empty_rect), title(empty_rect), button_row(empty_rect)
{}

dialog_frame::dimension_measurements dialog_frame::layout(SDL_Rect const& rect) {
	return layout(rect.x, rect.y, rect.w, rect.h);
}

int dialog_frame::vertical_padding() const {
	int padding = 0;
	if(buttons_ != NULL) {
		for(std::vector<button*>::const_iterator b = buttons_->begin(); b != buttons_->end(); ++b) {
			padding = maximum<int>((**b).height() + ButtonVPadding, padding);
		}
	}
	if(have_border_) {
		padding += bot_->h + top_->h;
	}
	if(!title_.empty()) {
		padding += font::get_max_height(font::SIZE_LARGE) + 2*dialog_frame::title_border_h;
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
			dim_.button_row.h = maximum<int>((**b).height() + ButtonVPadding,dim_.button_row.h);
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
	w = maximum<int>(w,maximum<int>(int(dim_.title.w),int(buttons_width)));
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

	video_.blit_surface(dim_.interior.x - top_left_->w, dim_.interior.y - top_left_->h, top_left_);
	video_.blit_surface(dim_.interior.x - bot_left_->w, dim_.interior.y + dim_.interior.h, bot_left_);
	video_.blit_surface(dim_.interior.x + dim_.interior.w, dim_.interior.y - top_right_->h, top_right_);
	video_.blit_surface(dim_.interior.x + dim_.interior.w, dim_.interior.y + dim_.interior.h, bot_right_);
}

void dialog_frame::draw_background()
{
	if(restorer_ != NULL) {
		*restorer_ = surface_restorer(&video_, dim_.exterior);
	}


	if (dialog_style_->blur_radius) {
		surface surf = ::get_surface_portion(video_.getSurface(), dim_.exterior);
		surf = blur_surface(surf, dialog_style_->blur_radius);
		SDL_BlitSurface(surf, NULL, video_.getSurface(), &dim_.exterior);
	}

	if(bg_ == NULL) {
		ERR_DP << "could not find dialog background '" << dialog_style_->panel << "'\n";
		return;
	}
	for(int i = 0; i < dim_.interior.w; i += bg_->w) {
		for(int j = 0; j < dim_.interior.h; j += bg_->h) {
			SDL_Rect src = {0,0,0,0};
			src.w = minimum(dim_.interior.w - i, bg_->w);
			src.h = minimum(dim_.interior.h - j, bg_->h);
			SDL_Rect dst = src;
			dst.x = dim_.interior.x + i;
			dst.y = dim_.interior.y + j;
			SDL_BlitSurface(bg_, &src, video_.getSurface(), &dst);
		}
	}
}

SDL_Rect dialog_frame::draw_title(CVideo* video)
{
	SDL_Rect rect = {0, 0, 10000, 10000};
	rect = screen_area();
	return font::draw_text(video, rect, font::SIZE_LARGE, font::TITLE_COLOUR,
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

	bool can_execute_command(hotkey::HOTKEY_COMMAND cmd, int /*index*/) const
	{
		return (topic_.empty() == false && cmd == hotkey::HOTKEY_HELP) || cmd == hotkey::HOTKEY_SCREENSHOT;
	}

	display& disp_;
	std::string topic_;
};

}

namespace gui
{

void show_error_message(display &disp, std::string const &message)
{
	ERR_G << message << std::endl;
	show_dialog(disp, NULL, _("Error"), message, OK_ONLY);
}

int show_dialog(display& screen, surface image,
				const std::string& caption, const std::string& message,
				DIALOG_TYPE type,
				const std::vector<std::string>* menu_items,
				const std::vector<preview_pane*>* preview_panes,
				const std::string& text_widget_label,
				std::string* text_widget_text,
				int text_widget_max_chars,
				std::vector<check_item>* options, int xloc, int yloc,
				const struct style* dialog_style, std::vector<dialog_button_info>* action_buttons,
				const std::string& help_topic, const menu::sorter* sorter, menu::style* menu_style)
{
	const std::string& title = (image.null())? caption : "";
	const struct style *style = (dialog_style)? dialog_style : &dialog::default_style;
	CVideo &disp = screen.video();

	gui::dialog d(screen, title, message, type, style, help_topic);

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

namespace gui {

network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num, network::statistics (*get_stats)(network::connection handle))
{
#ifdef USE_TINY_GUI
	const size_t width = 200;
	const size_t height = 40;
	const size_t border = 10;
#else
	const size_t width = 300;
	const size_t height = 80;
	const size_t border = 20;
#endif
	const int left = disp.w()/2 - width/2;
	const int top = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	surface_restorer restorer;
	gui::dialog_frame frame(disp.video(),msg,NULL,&buttons_ptr,&restorer);
	frame.layout(left,top,width,height);
	frame.draw();

	const SDL_Rect progress_rect = {left+border,top+border,width-border*2,height-border*2};
	gui::progress_bar progress(disp.video());
	progress.set_location(progress_rect);

	events::raise_draw_event();
	disp.flip();

	network::statistics old_stats = get_stats(connection_num);

	cfg.clear();
	for(;;) {
		const network::connection res = network::receive_data(cfg,connection_num,100);
		const network::statistics stats = get_stats(connection_num);
		if(stats.current_max != 0 && stats != old_stats) {
			old_stats = stats;
			progress.set_progress_percent((stats.current*100)/stats.current_max);
			std::ostringstream stream;
			stream << stats.current/1024 << "/" << stats.current_max/1024 << _("KB");
			progress.set_text(stream.str());
		}

		events::raise_draw_event();
		disp.flip();

		if(res != 0) {
			return res;
		}

		events::pump();
		if(cancel_button.pressed()) {
			return res;
		}
	}
}

network::connection network_send_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_send_stats);
}

network::connection network_receive_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_receive_stats);
}

namespace {

class connect_waiter : public threading::waiter
{
public:
	connect_waiter(display& disp, gui::button& button) : disp_(disp), button_(button)
	{}
	ACTION process();

private:
	display& disp_;
	gui::button& button_;
};

connect_waiter::ACTION connect_waiter::process()
{
	events::raise_draw_event();
	disp_.flip();
	events::pump();
	if(button_.pressed()) {
		return ABORT;
	} else {
		return WAIT;
	}
}

}

network::connection network_connect_dialog(display& disp, const std::string& msg, const std::string& hostname, int port)
{
#ifdef USE_TINY_GUI
	const size_t width = 200;
	const size_t height = 20;
#else
	const size_t width = 250;
	const size_t height = 20;
#endif
	const int left = disp.w()/2 - width/2;
	const int top = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	surface_restorer restorer;
	gui::dialog_frame frame(disp.video(),msg,NULL,&buttons_ptr,&restorer);
	frame.layout(left,top,width,height);
	frame.draw();

	events::raise_draw_event();
	disp.flip();

	connect_waiter waiter(disp,cancel_button);
	return network::connect(hostname,port,waiter);
}

} //end namespace gui
