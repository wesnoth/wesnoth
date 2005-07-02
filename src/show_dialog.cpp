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
#include "font.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "key.hpp"
#include "log.hpp"
#include "playlevel.hpp"
#include "show_dialog.hpp"
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

#include "SDL_ttf.h"

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
	SDL_Rect rect = {0,0,10000,10000};
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

	bool can_execute_command(hotkey::HOTKEY_COMMAND cmd) const
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

int show_dialog(display& disp, surface image,
				const std::string& caption, const std::string& msg,
				DIALOG_TYPE type,
				const std::vector<std::string>* menu_items_ptr,
				const std::vector<preview_pane*>* preview_panes,
				const std::string& text_widget_label,
				std::string* text_widget_text,
				int text_widget_max_chars,
				dialog_action* action, std::vector<check_item>* options, int xloc, int yloc,
				const std::string* dialog_style, std::vector<dialog_button>* action_buttons,
				const std::string& help_topic, const menu::sorter* sorter)
{
	if(disp.update_locked())
		return -1;

	LOG_DP << "showing dialog '" << caption << "' '" << msg << "'\n";

	//create the event context, but only activate it if we don't have preview panes.
	//the presence of preview panes indicates that the caller will create the context,
	//so that their preview panes may fall inside it.
	const events::event_context dialog_events_context(preview_panes == NULL);
	const dialog_manager manager;

	const events::resize_lock prevent_resizing;

	help_handler helper(disp,help_topic);
	hotkey::basic_handler help_dispatcher(&disp,&helper);

	const std::vector<std::string>& menu_items =
		(menu_items_ptr == NULL) ? std::vector<std::string>() : *menu_items_ptr;

	static const int message_font_size = font::SIZE_PLUS;
	static const int caption_font_size = font::SIZE_LARGE;

	CVideo& screen = disp.video();
	surface const scr = screen.getSurface();

	SDL_Rect clipRect = disp.screen_area();

	const bool use_textbox = text_widget_text != NULL;
	const bool editable_textbox = use_textbox && std::find(text_widget_text->begin(),text_widget_text->end(),'\n') == text_widget_text->end();
	static const std::string default_text_string = "";
	const unsigned int text_box_width = font::relative_size(350);
	textbox text_widget(screen,text_box_width,
	                    use_textbox ? *text_widget_text : default_text_string, editable_textbox, text_widget_max_chars);

	int text_widget_width = 0;
	int text_widget_height = 0;
	if(use_textbox) {

		text_widget.set_wrap(!editable_textbox);

		const SDL_Rect& area = font::text_area(*text_widget_text,message_font_size);

		text_widget.set_width(minimum<size_t>(screen.getx()/2,maximum<size_t>(area.w,text_widget.location().w)));
		text_widget.set_height(minimum<size_t>(screen.gety()/2,maximum<size_t>(area.h,text_widget.location().h)));
		text_widget_width = font::text_area(text_widget_label,message_font_size).w + text_widget.location().w;;
		text_widget_height = text_widget.location().h + message_font_size;
	}

#ifdef USE_TINY_GUI
	const int max_menu_width = 150;
#else
	const int max_menu_width = -1;
#endif
	menu menu_(screen,menu_items,type == MESSAGE,-1,max_menu_width,sorter);

	menu_.set_numeric_keypress_selection(use_textbox == false);

	const size_t left_padding = font::relative_size(10);
	const size_t right_padding = font::relative_size(10);
	const size_t image_h_padding = font::relative_size(image != NULL ? 10 : 0);
	const size_t top_padding = font::relative_size(10);
	const size_t bottom_padding = font::relative_size(10);

	const std::string message = font::word_wrap_text(msg, message_font_size,
			screen.getx() / 2,
			screen.gety() / 2);

	SDL_Rect text_size = { 0, 0, 0, 0 };
	if(!message.empty()) {
		text_size = font::draw_text(NULL, clipRect, message_font_size,
		                            font::NORMAL_COLOUR, message, 0, 0);
	}

	SDL_Rect caption_size = { 0, 0, 0, 0 };
	if (!caption.empty() && image != NULL) {
		caption_size = font::draw_text(NULL, clipRect, caption_font_size,
		                               font::NORMAL_COLOUR, caption, 0, 0);
	}

	const char** button_list = NULL;
	std::vector<button> buttons;
	switch(type) {
		case MESSAGE:
		default:
			break;

		case OK_ONLY: {
			static const char* thebuttons[] = { N_("OK"), "" };
			button_list = thebuttons;
			break;
		}

		case YES_NO: {
			static const char* thebuttons[] = { N_("Yes"),
							    N_("No"), ""};
			button_list = thebuttons;
			break;
		}

		case OK_CANCEL: {
			static const char* thebuttons[] = { N_("OK"),
							    N_("Cancel"),""};
			button_list = thebuttons;
			break;
		}

		case CANCEL_ONLY: {
			static const char* thebuttons[] = { N_("Cancel"), "" };
			button_list = thebuttons;
			break;
		}

		case CLOSE_ONLY: {
			static const char* thebuttons[] = { N_("Close"), "" };
			button_list = thebuttons;
			break;
		}
	}

	if(button_list != NULL) {
		try {
			while((*button_list)[0] != '\0') {
				buttons.push_back(button(screen,gettext(*button_list)));

				++button_list;
			}

		} catch(button::error&) {
			ERR_DP << "error initializing button!\n";
		}
	}

	int check_button_height = 0;
	int check_button_width = 0;
	const int button_height_padding = 5;

	std::vector<button> check_buttons;
	if(options != NULL) {
		for(std::vector<check_item>::const_iterator i = options->begin(); i != options->end(); ++i) {
			button check_button(screen,i->label,button::TYPE_CHECK);
			check_button_height += check_button.height() + button_height_padding;
			check_button_width = maximum<int>(check_button.width(),check_button_width);

			check_buttons.push_back(check_button);
		}
	}

	if(action_buttons != NULL) {
		for(std::vector<dialog_button>::const_iterator i = action_buttons->begin(); i != action_buttons->end(); ++i) {
			button new_button(screen,i->label);
			check_button_height += new_button.height() + button_height_padding;
			check_button_width = maximum<int>(new_button.width(),check_button_width);

			check_buttons.push_back(new_button);
		}
	}

	size_t above_preview_pane_height = 0, above_left_preview_pane_width = 0, above_right_preview_pane_width = 0;
	size_t preview_pane_height = 0, left_preview_pane_width = 0, right_preview_pane_width = 0;
	if(preview_panes != NULL) {
		for(std::vector<preview_pane*>::const_iterator i = preview_panes->begin(); i != preview_panes->end(); ++i) {
			const SDL_Rect& rect = (**i).location();

			if((**i).show_above() == false) {
				preview_pane_height = maximum<size_t>(rect.h,preview_pane_height);
				if((**i).left_side()) {
					left_preview_pane_width += rect.w;
				} else {
					right_preview_pane_width += rect.w;
				}
			} else {
				above_preview_pane_height = maximum<size_t>(rect.h,above_preview_pane_height);
				if((**i).left_side()) {
					above_left_preview_pane_width += rect.w;
				} else {
					above_right_preview_pane_width += rect.w;
				}
			}
		}
	}

	const int menu_hpadding = font::relative_size(text_size.h > 0 && menu_.height() > 0 ? 10 : 0);
	const size_t padding_width = left_padding + right_padding + image_h_padding;
	const size_t padding_height = top_padding + bottom_padding + menu_hpadding;
	const size_t caption_width = caption_size.w;
	const size_t image_width = image != NULL ? image->w : 0;
	const size_t image_height = image != NULL ? image->h : 0;
	const size_t total_text_height = text_size.h + caption_size.h;

	size_t text_width = text_size.w;
	if(caption_width > text_width)
		text_width = caption_width;
	// Prevent the menu to be larger than the screen
	if(menu_.width() + image_width + padding_width > size_t(scr->w))
		menu_.set_width(scr->w - image_width - padding_width);
	if(menu_.width() > text_width)
		text_width = menu_.width();

	size_t total_width = image_width + text_width + padding_width;

	if(text_widget_width+left_padding+right_padding > total_width)
		total_width = text_widget_width+left_padding+right_padding;

	const size_t text_and_image_height = image_height > total_text_height ? image_height : total_text_height;

	const int total_height = text_and_image_height +
	                         padding_height + menu_.height() +
							 text_widget_height + check_button_height;


	int frame_width = maximum<int>(total_width,above_left_preview_pane_width + above_right_preview_pane_width);
	int frame_height = maximum<int>(total_height,int(preview_pane_height));
	int xframe = maximum<int>(0,xloc >= 0 ? xloc : scr->w/2 - (frame_width + left_preview_pane_width + right_preview_pane_width)/2);
	int yframe = maximum<int>(0,yloc >= 0 ? yloc : scr->h/2 - (frame_height + above_preview_pane_height)/2);

	LOG_DP << "above_preview_pane_height: " << above_preview_pane_height << "; "
		<< "yframe: " << scr->h/2 << " - " << (frame_height + above_preview_pane_height)/2 << " = "
		<< yframe << "; " << "frame_height: " << frame_height << "\n";

	if(xloc <= -1 || yloc <= -1) {
		xloc = xframe + left_preview_pane_width;
		yloc = yframe + above_preview_pane_height;
	}

	if(xloc + frame_width > scr->w) {
		xloc = scr->w - frame_width;
		if(xloc < xframe) {
			xframe = xloc;
		}
	}

	if(yloc + frame_height > scr->h) {
		yloc = scr->h - frame_height;
		if(yloc < yframe) {
			yframe = yloc;
		}
	}

	std::vector<button*> buttons_ptr;
	for(std::vector<button>::iterator bt = buttons.begin(); bt != buttons.end(); ++bt) {
		buttons_ptr.push_back(&*bt);
	}

	frame_width += left_preview_pane_width + right_preview_pane_width;
	frame_height += above_preview_pane_height;

	surface_restorer restorer;

	button help_button(screen,_("Help"));

	const std::string& title = image == NULL ? caption : "";
	draw_dialog(xframe,yframe,frame_width,frame_height,screen,title,dialog_style,&buttons_ptr,&restorer,help_topic.empty() ? NULL : &help_button);

	//calculate the positions of the preview panes to the sides of the dialog
	if(preview_panes != NULL) {

		int left_preview_pane = xframe;
		int right_preview_pane = xframe + total_width + left_preview_pane_width;
		int above_left_preview_pane = xframe + frame_width/2;
		int above_right_preview_pane = above_left_preview_pane;

		for(std::vector<preview_pane*>::const_iterator i = preview_panes->begin(); i != preview_panes->end(); ++i) {
			SDL_Rect area = (**i).location();

			if((**i).show_above() == false) {
				area.y = yloc;
				if((**i).left_side()) {
					area.x = left_preview_pane;
					left_preview_pane += area.w;
				} else {
					area.x = right_preview_pane;
					right_preview_pane += area.w;
				}
			} else {
				area.y = yframe;
				if((**i).left_side()) {
					area.x = above_left_preview_pane - area.w;
					above_left_preview_pane -= area.w;
				} else {
					area.x = above_right_preview_pane;
					above_right_preview_pane += area.w;
				}
			}

			(**i).set_location(area);
		}
	}

	const int text_widget_y = yloc+top_padding+text_and_image_height-6+menu_hpadding;

	if(use_textbox) {
		const int text_widget_y_unpadded = text_widget_y + (text_widget_height - text_widget.location().h)/2;
		text_widget.set_location(xloc + left_padding +
		                         text_widget_width - text_widget.location().w,
		                         text_widget_y_unpadded);
		events::raise_draw_event();
		font::draw_text(&screen, clipRect, message_font_size,
		                font::NORMAL_COLOUR, text_widget_label,
						xloc + left_padding,text_widget_y_unpadded);
	}

	const int menu_xpos = xloc+image_width+left_padding+image_h_padding;
	const int menu_ypos = yloc+top_padding+text_and_image_height+menu_hpadding+ (use_textbox ? text_widget.location().h + top_padding : 0);
	if(menu_.height() > 0) {
		menu_.set_location(menu_xpos,menu_ypos);
	}

	if(image != NULL) {
		const int x = xloc + left_padding;
		const int y = yloc + top_padding;

		screen.blit_surface(x,y,image);

		font::draw_text(&screen, clipRect, caption_font_size,
		                font::NORMAL_COLOUR, caption,
		                xloc+image_width+left_padding+image_h_padding,
		                yloc+top_padding);
	}

	font::draw_text(&screen, clipRect, message_font_size,
	                font::NORMAL_COLOUR, message,
	                xloc+image_width+left_padding+image_h_padding,
	                yloc+top_padding+caption_size.h);

	//set the position of any tick boxes. they go right below the menu, slammed against
	//the right side of the dialog
	if(check_buttons.empty() == false) {
		int options_y = text_widget_y + text_widget_height + menu_.height() + button_height_padding + menu_hpadding;
		for(size_t i = 0; i != check_buttons.size(); ++i) {
			check_buttons[i].set_location(xloc + total_width - check_buttons[i].width() - ButtonHPadding,options_y);

			options_y += check_buttons[i].height() + button_height_padding;

			if(options != NULL && i < options->size()) {
				check_buttons[i].set_check((*options)[i].checked);
			}
		}
	}

	disp.flip();

	CKey key;

	bool left_button = true, right_button = true, key_down = true,
	     up_arrow = false, down_arrow = false,
	     page_up = false, page_down = false;

	disp.invalidate_all();

	int cur_selection = -1;

	SDL_Rect unit_details_rect;
	unit_details_rect.w = 0;

	bool first_time = true;

	for(;;) {
		events::pump();

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool new_right_button = (mouse_flags&SDL_BUTTON_RMASK) != 0;
		const bool new_left_button = (mouse_flags&SDL_BUTTON_LMASK) != 0;
		const bool new_key_down = key[SDLK_SPACE] || key[SDLK_RETURN] ||
		                          key[SDLK_ESCAPE];

		const bool new_up_arrow = key[SDLK_UP] != 0;
		const bool new_down_arrow = key[SDLK_DOWN] != 0;

		const bool new_page_up = key[SDLK_PAGEUP] != 0;
		const bool new_page_down = key[SDLK_PAGEDOWN] != 0;

		if((!key_down && key[SDLK_RETURN] || menu_.double_clicked()) &&
		   (type == YES_NO || type == OK_CANCEL || type == OK_ONLY || type == CLOSE_ONLY)) {

			if(text_widget_text != NULL && use_textbox)
				*text_widget_text = text_widget.text();

			if(menu_.height() == 0) {
				return 0;
			} else {
				return menu_.selection();
			}
		}

		if(!key_down && key[SDLK_ESCAPE] && type == MESSAGE) {
			return ESCAPE_DIALOG;
		}

		//escape quits from the dialog -- unless it's an "ok" dialog with a menu,
		//since such dialogs require a selection of some kind.
		if(!key_down && key[SDLK_ESCAPE] && (type != OK_ONLY || menu_.height() == 0)) {

			if(menu_.height() == 0) {
				return 1;
			} else {
				return -1;
			}
		}

		if(menu_.selection() != cur_selection || first_time) {
			cur_selection = menu_.selection();

			int selection = cur_selection;
			if(selection < 0) {
				selection = 0;
			}

			if(preview_panes != NULL) {
				for(std::vector<preview_pane*>::const_iterator i = preview_panes->begin(); i != preview_panes->end(); ++i) {
					(**i).set_selection(selection);
					if(first_time) {
						(**i).set_dirty();
					}
				}
			}
		}

		first_time = false;

		if(menu_.height() > 0) {
			const int res = menu_.process();
			if(res != -1)
			{
				return res;
			}
		}

		up_arrow = new_up_arrow;
		down_arrow = new_down_arrow;
		page_up = new_page_up;
		page_down = new_page_down;

		events::raise_process_event();
		events::raise_draw_event();

		const SDL_Rect menu_rect = {menu_xpos,menu_ypos,menu_.width(),menu_.height()};
		if(buttons.empty() && (new_left_button && !left_button && !point_in_rect(mousex,mousey,menu_rect) ||
		                       new_right_button && !right_button) ||
		   buttons.size() < 2 && new_key_down && !key_down &&
		   menu_.height() == 0)
			break;

		left_button = new_left_button;
		right_button = new_right_button;
		key_down = new_key_down;

		for(std::vector<button>::iterator button_it = buttons.begin();
		    button_it != buttons.end(); ++button_it) {
			if(button_it->pressed()) {
				if(text_widget_text != NULL && use_textbox)
					*text_widget_text = text_widget.text();

				//if the menu is not used, then return the index of the
				//button pressed, otherwise return the index of the menu
				//item selected if the last button is not pressed, and
				//cancel (-1) otherwise
				if(menu_.height() == 0) {
					return button_it - buttons.begin();
				} else if(buttons.size() <= 1 ||
				       size_t(button_it-buttons.begin()) != buttons.size()-1) {
					return menu_.selection();
				} else {
					return -1;
				}
			}
		}

		if(help_topic.empty() == false && help_button.pressed()) {
			help::show_help(disp,help_topic);
		}

		for(unsigned int n = 0; n != check_buttons.size(); ++n) {
			const bool pressed = check_buttons[n].pressed();

			if(options != NULL && n < options->size()) {
				(*options)[n].checked = check_buttons[n].checked();
			} else if(pressed) {
				const size_t options_size = options == NULL ? 0 : options->size();
				wassert(action_buttons != NULL && action_buttons->size() > n - options_size);

				const dialog_button_action::RESULT res = (*action_buttons)[n - options_size].handler->button_pressed(menu_.selection());
				if(res == dialog_button_action::DELETE_ITEM) {
					first_time = true;
					menu_.erase_item(menu_.selection());
					if(menu_.nitems() == 0) {
						return -1;
					}
				} else if(res == dialog_button_action::CLOSE_DIALOG) {
					return -1;
				}

				//reset button-tracking flags so that if the action displays a dialog, a button-press
				//at the end of the dialog won't be mistaken for a button-press in this dialog.
				//(We should eventually use a proper event-handling system instead of tracking
				//flags to avoid problems like this altogether).
				left_button = true;
				right_button = true;
				key_down = true;
			}
		}

		disp.flip();
		SDL_Delay(10);

		if(action != NULL) {
			const int act = action->do_action();
			if(act != dialog_action::CONTINUE_DIALOG) {
				return act;
			}
		}
	}

	return -1;
}

}

namespace gui {

network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	const std::string title = _("Receiving data...");

	const size_t width = 300;
	const size_t height = 80;
	const size_t border = 20;
	const int left = disp.x()/2 - width/2;
	const int top = disp.y()/2 - height/2;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	surface_restorer restorer;
	gui::draw_dialog(left,top,width,height,disp.video(),title,NULL,&buttons_ptr,&restorer);

	const SDL_Rect progress_rect = {left+border,top+border,width-border*2,height-border*2};
	gui::progress_bar progress(disp.video());
	progress.set_location(progress_rect);

	events::raise_draw_event();
	disp.flip();

	std::pair<int,int> old_stats = network::current_transfer_stats();

	cfg.clear();
	for(;;) {
		const network::connection res = network::receive_data(cfg,connection_num,100);

		const std::pair<int,int> stats = network::current_transfer_stats();
		if(stats.first != -1 && stats.second != 0 && stats != old_stats) {
			old_stats = stats;
			progress.set_progress_percent((stats.first*100)/stats.second);
			std::ostringstream stream;
			stream << stats.first/1024 << "/" << stats.second/1024 << _("KB");
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
	const size_t width = 250;
	const size_t height = 20;
	const int left = disp.x()/2 - width/2;
	const int top = disp.y()/2 - height/2;

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
