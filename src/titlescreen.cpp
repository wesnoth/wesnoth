/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "hotkeys.hpp"
#include "key.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "preferences_display.hpp"
#include "sdl_utils.hpp"
#include "show_dialog.hpp"
#include "titlescreen.hpp"
#include "util.hpp"
#include "video.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include "sdl_ttf/SDL_ttf.h"

#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

namespace {

bool fade_logo(display& screen, int xpos, int ypos)
{
	const surface logo(image::get_image(game_config::game_logo,image::UNSCALED));
	if(logo == NULL) {
		ERR_DP << "Could not find game logo\n";
		return true;
	}

	surface const fb = screen.video().getSurface();

	if(fb == NULL || xpos < 0 || ypos < 0 || xpos + logo->w > fb->w || ypos + logo->h > fb->h) {
		return true;
	}

	//only once, when the game is first started, the logo fades in
	static bool faded_in = false;

	CKey key;
	bool last_button = key[SDLK_ESCAPE] || key[SDLK_SPACE];

	LOG_DP << "fading logo in....\n";

	LOG_DP << "logo size: " << logo->w << "," << logo->h << "\n";

	for(int x = 0; x != logo->w; ++x) {
		SDL_Rect srcrect = {x,0,1,logo->h};
		SDL_Rect dstrect = {xpos+x,ypos,1,logo->h};

		SDL_BlitSurface(logo,&srcrect,fb,&dstrect);

		update_rect(dstrect);

		if(!faded_in && (x%5) == 0) {

			const bool new_button = key[SDLK_ESCAPE] || key[SDLK_SPACE] || key[SDLK_RETURN];
			if(new_button && !last_button) {
				faded_in = true;
			}

			last_button = new_button;

			screen.update_display();

			SDL_Delay(10);

			events::pump();
			if(screen.video().modeChanged()) {
				faded_in = true;
				return false;
			}
		}

	}

	LOG_DP << "logo faded in\n";

	faded_in = true;
	return true;
}

const std::string& get_tip_of_day(const config& tips,int* ntip)
{
	static const std::string empty_string;
	string_map::const_iterator it;

	if(preferences::show_tip_of_day() == false) {
		return empty_string;
	}

	int ntips = tips.values.size();
	if(ntips == 0) {
		return empty_string;
	}

	if(ntip != NULL && *ntip > 0) {
		if(*ntip >= ntips) {
			*ntip -= ntips;
		}

		it = tips.values.begin();
		for(int i = 0; i < *ntip; i++,it++);
		return it->second;
	}

	const int tip = (rand()%ntips);
	if(ntip != NULL) {
		*ntip = tip;
	}

	it = tips.values.begin();
	for(int i = 0; i < tip; i++,it++);
	return it->second;
}

const config get_tips_of_day()
{
	config cfg;

	std::cerr << "Loading tips of day\n";
	try {
		scoped_istream stream = preprocess_file("data/tips.cfg");
		read(cfg, *stream);
	} catch(config::error&) {
		std::cerr << "Could not read tips.cfg\n";
	}

	return cfg;
}

} //end anonymous namespace

namespace gui {

TITLE_RESULT show_title(display& screen, config& tips_of_day, int* ntip)
{
	cursor::set(cursor::NORMAL);

	const preferences::display_manager disp_manager(&screen);
	const hotkey::basic_handler key_handler(&screen);

	const font::floating_label_context label_manager;

	// Display Wesnoth logo
	surface const title_surface(scale_surface(
		image::get_image(game_config::game_title,image::UNSCALED),
		screen.x(), screen.y()));
	screen.video().modeChanged(); // resets modeChanged value
	int logo_x = game_config::title_logo_x * screen.x() / 1024,
	    logo_y = game_config::title_logo_y * screen.y() / 768;
	do {
		if (title_surface.null()) {
			ERR_DP << "Could not find title image\n";
		} else {
			screen.video().blit_surface(0, 0, title_surface);
			update_rect(screen_area());
			LOG_DP << "displayed title image\n";
		}
	} while (!fade_logo(screen, logo_x, logo_y));
	LOG_DP << "faded logo\n";

	const std::string& version_str = _("Version") + std::string(" ") +
	                                 game_config::version;

	const SDL_Rect version_area = font::draw_text(NULL,screen_area(),
						      font::SIZE_TINY,
	                                    font::NORMAL_COLOUR,version_str,0,0);
	const size_t versiony = screen.y() - version_area.h;

	if(versiony < size_t(screen.y())) {
		font::draw_text(&screen.video(),screen.screen_area(),
				font::SIZE_TINY,
				font::NORMAL_COLOUR,version_str,0,versiony);
	}

	LOG_DP << "drew version number\n";

	//members of this array must correspond to the enumeration TITLE_RESULT
	static const char* button_labels[] = { N_("TitleScreen button^Tutorial"),
					       N_("TitleScreen button^Campaign"),
					       N_("TitleScreen button^Multiplayer"),
					       N_("TitleScreen button^Load"),
					       N_("TitleScreen button^Language"),
					       N_("TitleScreen button^Preferences"),
						   N_("TitleScreen button^Help Wesnoth"),
					       N_("About"),
						   N_("TitleScreen button^Quit") };
	static const char* help_button_labels[] = { N_("Start a tutorial to familiarize yourself with the game"),
						    N_("Start a new single player campaign"),
						    N_("Play multiplayer (hotseat, LAN, or Internet), or a single scenario against the AI"),
						    N_("Load a single player saved game"),
						    N_("Change the language"),
						    N_("Configure the game's settings"),
							N_("Help Wesnoth by sending us information"),
						    N_("View the credits"),
						    N_("Quit the game") };

	static const size_t nbuttons = sizeof(button_labels)/sizeof(*button_labels);

	const int menu_xbase = (game_config::title_buttons_x*screen.x())/1024;
	const int menu_xincr = 0;
	const int menu_ybase = (game_config::title_buttons_y*screen.y())/768;
#ifdef USE_TINY_GUI
	const int menu_yincr = 15;
#else
	const int menu_yincr = 35;
#endif
	const int padding = game_config::title_buttons_padding;

	std::vector<button> buttons;
	size_t b, max_width = 0;
	for(b = 0; b != nbuttons; ++b) {
		buttons.push_back(button(screen.video(),sgettext(button_labels[b])));
		buttons.back().set_help_string(sgettext(help_button_labels[b]));
		max_width = maximum<size_t>(max_width,buttons.back().width());
	}

	SDL_Rect main_dialog_area = {menu_xbase-padding,menu_ybase-padding,max_width+padding*2,menu_yincr*(nbuttons-1)+buttons.back().height()+padding*2};
	std::string style = "mainmenu";
	draw_dialog_frame(main_dialog_area.x,main_dialog_area.y,main_dialog_area.w,main_dialog_area.h,screen.video(),&style);

	for(b = 0; b != nbuttons; ++b) {
		buttons[b].set_width(max_width);
		buttons[b].set_location(menu_xbase + b*menu_xincr, menu_ybase + b*menu_yincr);
	}

	gui::button next_tip_button(screen.video(),_("More"),button::TYPE_PRESS,"lite_small");

	if(tips_of_day.empty()) {
		tips_of_day = get_tips_of_day();
	}
	std::string tip_of_day = get_tip_of_day(tips_of_day,ntip);
	if(tip_of_day.empty() == false) {
		tip_of_day = font::word_wrap_text(tip_of_day,font::SIZE_NORMAL,
						  (game_config::title_tip_width*screen.x())/1024);

		const std::string& tome = font::word_wrap_text(_("-- The Tome of Wesnoth"),
							       font::SIZE_NORMAL,
							       (game_config::title_tip_width*screen.x())/1024);

		const int pad = game_config::title_tip_padding;

		SDL_Rect area = font::text_area(tip_of_day,font::SIZE_NORMAL);
		SDL_Rect tome_area = font::text_area(tome,font::SIZE_NORMAL,TTF_STYLE_ITALIC);
		area.w = maximum<size_t>(area.w,tome_area.w) + 2*pad;
		area.h += tome_area.h + next_tip_button.location().h + 3*pad;

		area.x = main_dialog_area.x - (game_config::title_tip_x*screen.x())/1024 - area.w;
		area.y = main_dialog_area.y + main_dialog_area.h - area.h;

		draw_dialog_frame(area.x,area.y,area.w,area.h,screen.video(),&style);

		next_tip_button.set_location(area.x + area.w - next_tip_button.location().w - pad,
		                             area.y + area.h - pad - next_tip_button.location().h);

		font::draw_text(&screen.video(), area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
		                tip_of_day, area.x + pad, area.y + pad);
		font::draw_text(&screen.video(), area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
		                tome, area.x + area.w - tome_area.w - pad,
		                next_tip_button.location().y - tome_area.h - pad, false, TTF_STYLE_ITALIC);
	}

	events::raise_draw_event();

	LOG_DP << "drew buttons dialog\n";

	CKey key;

	bool last_escape = key[SDLK_ESCAPE] != 0;

	update_whole_screen();

	LOG_DP << "entering interactive loop...\n";

	for(;;) {
		for(size_t b = 0; b != buttons.size(); ++b) {
			if(buttons[b].pressed()) {
				return TITLE_RESULT(b);
			}
		}

		if(next_tip_button.pressed()) {
			if(ntip != NULL) {
				*ntip = *ntip + 1;
			}

			return TITLE_CONTINUE;
		}

		events::raise_process_event();
		events::raise_draw_event();

		screen.flip();

		if(!last_escape && key[SDLK_ESCAPE])
			return QUIT_GAME;

		last_escape = key[SDLK_ESCAPE] != 0;

		events::pump();

		//if the resolution has changed due to the user resizing the screen,
		//or from changing between windowed and fullscreen
		if(screen.video().modeChanged()) {
			return TITLE_CONTINUE;
		}

		SDL_Delay(20);
	}

	return QUIT_GAME;
}

}
