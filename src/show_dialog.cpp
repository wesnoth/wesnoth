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
#include "construct_dialog.hpp"
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

const int ButtonHPadding = 10;
const int ButtonVPadding = 10;

bool in_dialog() { return is_in_dialog; }

dialog_manager::dialog_manager() : cursor::setter(cursor::NORMAL), reset_to(is_in_dialog)
{
	is_in_dialog = true;
}

dialog_manager::~dialog_manager()
{
	is_in_dialog = reset_to;
	int mousex, mousey;
	int mouse_flags = SDL_GetMouseState(&mousex, &mousey);
	SDL_Event pb_event;
	pb_event.type = SDL_MOUSEMOTION;
	pb_event.motion.state = 0;
	pb_event.motion.x = mousex;
	pb_event.motion.y = mousey;
	pb_event.motion.xrel = 0;
	pb_event.motion.yrel = 0;
	SDL_PushEvent(&pb_event);
	if (!(mouse_flags & SDL_BUTTON_RMASK) || is_in_dialog)
		return;
	// based on krom's idea; remove if you don't like the "responsiveness"
	pb_event.type = SDL_MOUSEBUTTONDOWN;
	pb_event.button.button = SDL_BUTTON_RIGHT;
	pb_event.button.state = SDL_PRESSED;
	pb_event.button.x = mousex;
	pb_event.button.y = mousey;
	SDL_PushEvent(&pb_event);
}

void draw_dialog_frame(int x, int y, int w, int h, CVideo &video, const std::string* dialog_style, surface_restorer* restorer)
{
	if(dialog_style == NULL) {
		static const std::string default_style("menu");
		dialog_style = &default_style;
	}

	const surface top(image::get_image("misc/" + *dialog_style + "-border-top.png",image::UNSCALED));
	const surface bot(image::get_image("misc/" + *dialog_style + "-border-bottom.png",image::UNSCALED));
	const surface left(image::get_image("misc/" + *dialog_style + "-border-left.png",image::UNSCALED));
	const surface right(image::get_image("misc/" + *dialog_style + "-border-right.png",image::UNSCALED));

	const bool have_border = top != NULL && bot != NULL && left != NULL && right != NULL;

	if(have_border && restorer != NULL) {
		const SDL_Rect rect = {x-top->h,y-left->w,w+left->w+right->w,h+top->h+bot->h};
		*restorer = surface_restorer(&video,rect);
	} else if(restorer != NULL) {
		const SDL_Rect rect = {x,y,w,h};
		*restorer = surface_restorer(&video,rect);
	}

	draw_dialog_background(x,y,w,h,video,*dialog_style);

	if(have_border == false) {
		return;
	}

	surface top_image(scale_surface(top,w,top->h));

	if(top_image != NULL) {
		video.blit_surface(x,y-top->h,top_image);
	}

	surface bot_image(scale_surface(bot,w,bot->h));

	if(bot_image != NULL) {
		video.blit_surface(x,y+h,bot_image);
	}

	surface left_image(scale_surface(left,left->w,h));

	if(left_image != NULL) {
		video.blit_surface(x-left->w,y,left_image);
	}

	surface right_image(scale_surface(right,right->w,h));

	if(right_image != NULL) {
		video.blit_surface(x+w,y,right_image);
	}

	update_rect(x-left->w,y-top->h,w+left->w+right->w,h+top->h+bot->h);

	const surface top_left(image::get_image("misc/" + *dialog_style + "-border-topleft.png",image::UNSCALED));
	const surface bot_left(image::get_image("misc/" + *dialog_style + "-border-botleft.png",image::UNSCALED));
	const surface top_right(image::get_image("misc/" + *dialog_style + "-border-topright.png",image::UNSCALED));
	const surface bot_right(image::get_image("misc/" + *dialog_style + "-border-botright.png",image::UNSCALED));
	if(top_left == NULL || bot_left == NULL || top_right == NULL || bot_right == NULL) {
		return;
	}

	video.blit_surface(x-top_left->w,y-top_left->h,top_left);
	video.blit_surface(x-bot_left->w,y+h,bot_left);
	video.blit_surface(x+w,y-top_right->h,top_right);
	video.blit_surface(x+w,y+h,bot_right);
}

void draw_dialog_background(int x, int y, int w, int h, CVideo &video, const std::string& style)
{
	const std::string menu_background = "misc/" + style + "-background.png";

	const surface bg(image::get_image(menu_background,image::UNSCALED));
	if(bg == NULL) {
		ERR_DP << "could not find dialog background '" << style << "'\n";
		return;
	}

	const SDL_Rect& screen_bounds = screen_area();
	if(x < 0) {
		w += x;
		x = 0;
	}

	if(y < 0) {
		h += y;
		y = 0;
	}

	if(x > screen_bounds.w) {
		return;
	}

	if(y > screen_bounds.h) {
		return;
	}

	if(x + w > screen_bounds.w) {
		w = screen_bounds.w - x;
	}

	if(y + h > screen_bounds.h) {
		h = screen_bounds.h - y;
	}

	for(int i = 0; i < w; i += bg->w) {
		for(int j = 0; j < h; j += bg->h) {
			SDL_Rect src = {0,0,0,0};
			src.w = minimum(w - i,bg->w);
			src.h = minimum(h - j,bg->h);
			SDL_Rect dst = src;
			dst.x = x + i;
			dst.y = y + j;
			SDL_BlitSurface(bg,&src,video.getSurface(),&dst);
		}
	}
}

SDL_Rect draw_dialog_title(int x, int y, CVideo* video, const std::string& text)
{
	SDL_Rect rect = {0, 0, 10000, 10000};
	rect = screen_area();

	return font::draw_text(video, rect, font::SIZE_LARGE, font::TITLE_COLOUR,
	                       text, x, y + 5, false, TTF_STYLE_BOLD);
}

void draw_dialog(int x, int y, int w, int h, CVideo &video, const std::string& title,
				 const std::string* style, std::vector<button*>* buttons,
				 surface_restorer* restorer, button* help_button)
{
	int border_size = 10;
	SDL_Rect title_area = {0,0,0,0};

	if (!title.empty()) {
		title_area = draw_dialog_title(0,0,NULL,title);
		title_area.w += border_size;
	}

	SDL_Rect buttons_area = {0,0,0,0};

	if(buttons != NULL) {
		for(std::vector<button*>::const_iterator b = buttons->begin(); b != buttons->end(); ++b) {
			buttons_area.w += (**b).width() + ButtonHPadding;
			buttons_area.h = maximum<int>((**b).height() + ButtonVPadding,buttons_area.h);
		}

		buttons_area.x = -buttons_area.w;
		buttons_area.y = y + h;

		buttons_area.w += ButtonHPadding;
	}

	size_t buttons_width = buttons_area.w;

	if(help_button != NULL) {
		buttons_width += help_button->width() + ButtonHPadding*2;
		buttons_area.y = y + h;
	}

	const int xpos = x;
	const int ypos = y - int(title_area.h);
	const int width = maximum<int>(w,maximum<int>(int(title_area.w),int(buttons_width)));
	const int height = title_area.h + buttons_area.h + h;

	buttons_area.x += xpos + width;

	draw_dialog_frame(xpos,ypos,width,height,video,style,restorer);

	if (!title.empty()) {
		draw_dialog_title(x + border_size, y - title_area.h, &video, title);
	}

	if(buttons != NULL) {
#ifdef OK_BUTTON_ON_RIGHT
		std::reverse(buttons->begin(),buttons->end());
#endif

		for(std::vector<button*>::const_iterator b = buttons->begin(); b != buttons->end(); ++b) {
			(**b).set_location(buttons_area.x,buttons_area.y);
			buttons_area.x += (**b).width() + ButtonHPadding;
		}
	}

	if(help_button != NULL) {
		help_button->set_location(x+ButtonHPadding,buttons_area.y);
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

	bool can_execute_command(hotkey::HOTKEY_COMMAND cmd, int index=-1) const
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
				dialog_action* action, std::vector<check_item>* options, int xloc, int yloc,
				const std::string* dialog_style, std::vector<dialog_button_info>* action_buttons,
				const std::string& help_topic, const menu::sorter* sorter, menu::style* menu_style)
{
	const std::string& title = (image.null())? caption : "";
	const std::string& style = (dialog_style)? *dialog_style : dialog::default_style;
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
	if(action) {
		d.set_action(action);
	}
	if(options) {
		for(unsigned int i=0; i < options->size(); ++i) {
			gui::dialog_button *btn = new gui::dialog_button(disp,(*options)[i].label,gui::button::TYPE_CHECK);
			gui::dialog::BUTTON_LOCATION loc = ((*options)[i].align == LEFT_ALIGN)? gui::dialog::BUTTON_CHECKBOX_LEFT : gui::dialog::BUTTON_CHECKBOX;
			btn->set_check((*options)[i].checked);
			d.add_button(btn, loc);
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
	const int left = disp.x()/2 - width/2;
	const int top = disp.y()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	surface_restorer restorer;
	gui::draw_dialog(left,top,width,height,disp.video(),msg,NULL,&buttons_ptr,&restorer);

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
	const int left = disp.x()/2 - width/2;
	const int top = disp.y()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	surface_restorer restorer;
	gui::draw_dialog(left,top,width,height,disp.video(),msg,NULL,&buttons_ptr,&restorer);

	events::raise_draw_event();
	disp.flip();

	connect_waiter waiter(disp,cancel_button);
	return network::connect(hostname,port,waiter);
}

} //end namespace gui
