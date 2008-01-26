/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file titlescreen.cpp
//! Shows the titlescreen, with main-menu and tip-of-the-day.
//!
//! The menu consists of buttons, such als Start-Tutorial, Start-Campaign, Load-Game, etc. \n
//! As decoration, the wesnoth-logo and a landmap in the background are shown.

#include "global.hpp"

#include "config.hpp"
#include "construct_dialog.hpp"
#include "cursor.hpp"
#include "game_display.hpp"
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
#include "wesconfig.h"
#include <algorithm>

#include "sdl_ttf/SDL_ttf.h"

//! Log info-messages to stdout during the game, mainly for debugging
#define LOG_DP LOG_STREAM(info, display)
//! Log error-messages to stdout during the game, mainly for debugging
#define ERR_DP LOG_STREAM(err, display)
#define LOG_CONFIG LOG_STREAM(info, config)
#define ERR_CONFIG LOG_STREAM(err, config)

//! Fade-in the wesnoth-logo.
//!
//! Animation-effect: scroll-in from right. \n
//! Used only once, after the game is started.
//!
//! @param	screen	surface to operate on
//! @param	xpos	x-position of logo
//! @param	ypos	y-position of logo
//!
//! @return		Result of running the routine
//! @retval true	operation finished (successful or not)
//! @retval false	operation failed (because modeChanged), need to retry
//!
static bool fade_logo(game_display& screen, int xpos, int ypos)
{
	const surface logo(image::get_image(game_config::game_logo));
	if(logo == NULL) {
		ERR_DP << "Could not find game logo\n";
		return true;
	}

	surface const fb = screen.video().getSurface();

	if(fb == NULL || xpos < 0 || ypos < 0 || xpos + logo->w > fb->w || ypos + logo->h > fb->h) {
		return true;
	}

	// Only once, when the game is first started, the logo fades in
	static bool faded_in = false;
//	static bool faded_in = true;	// for faster startup: mark logo as 'has already faded in'

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

			const bool new_button = key[SDLK_ESCAPE] || key[SDLK_SPACE] ||
									key[SDLK_RETURN] || key[SDLK_KP_ENTER] ;
			if(new_button && !last_button) {
				faded_in = true;
			}

			last_button = new_button;

			screen.update_display();
			screen.delay(10);

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


//! Read the file with the tips-of-the-day.
static const void read_tips_of_day(config& tips_of_day)
{
	tips_of_day.clear();
	LOG_CONFIG << "Loading tips of day\n";
	try {
		scoped_istream stream = preprocess_file("data/hardwired/tips.cfg");
		read(tips_of_day, *stream);
	} catch(config::error&) {
		ERR_CONFIG << "Could not read data/hardwired/tips.cfg\n";
	}

	//we shuffle the tips after each initial loading
	config::child_itors tips = tips_of_day.child_range("tip");
	if (tips.first != tips.second) {
		std::random_shuffle(tips.first, tips.second);
	}
}

//! Go to the next tips-of-the-day
static const void next_tip_of_day(config& tips_of_day, bool reverse = false)
{
	// we just rotate the tip list, to avoid the need to keep track
	// of the current one, and keep it valid, cycle it, etc... 
	config::child_itors tips = tips_of_day.child_range("tip");
	if (tips.first != tips.second) {
		config::child_iterator direction = reverse ? tips.first+1 : tips.second-1;
		std::rotate(tips.first, direction, tips.second);
	}
}

//! Return the text for one of the tips-of-the-day.
static const config* get_tip_of_day(config& tips_of_day)
{
	if (tips_of_day.empty()) {
		read_tips_of_day(tips_of_day);
	}

	const config::child_list& tips = tips_of_day.get_children("tip");
	
	// next_tip_of_day rotate tips, so better stay iterator-safe
	for (size_t t=0; t < tips.size(); t++, next_tip_of_day(tips_of_day)) {
		const config* tip = tips.front();
		if (tip == NULL) continue;

		const std::vector<std::string> needed_units = utils::split((*tip)["encountered_units"],',');
		if (needed_units.empty()) {
			return tip;
		}
		const std::set<std::string>& seen_units = preferences::encountered_units();

		// test if one of the listed unit types is already encountered
		// if if's a number, test if we have encountered more than this
		for (std::vector<std::string>::const_iterator i = needed_units.begin();
				i != needed_units.end(); i++) {
			int needed_units_nb = lexical_cast_default<int>(*i,-1);
			if (needed_units_nb !=-1) {
				if (needed_units_nb <= static_cast<int>(seen_units.size())) {
					return tip;
				}
			} else if (seen_units.find(*i) != seen_units.end()) {
				return tip;
			}
		}
	}
	// not tip match, someone forget to put an always-match one
	return NULL;
}

//! Show one tip-of-the-day in a frame on the titlescreen.
//! This frame has 2 buttons: Next-Tip, and Show-Help.
static void draw_tip_of_day(game_display& screen,
							config& tips_of_day,
							const gui::dialog_frame::style& style,
							gui::button* const previous_tip_button,
							gui::button* const next_tip_button,
							gui::button* const help_tip_button,
							const SDL_Rect* const main_dialog_area,
							surface_restorer& tip_of_day_restorer)
{
	if(preferences::show_tip_of_day() == false) {
		return;
	}

    // Restore the previous tip of day area to its old state (section of the title image).
    tip_of_day_restorer.restore();

    // Draw tip of the day
    const config* tip = get_tip_of_day(tips_of_day);
    if(tip != NULL) {
    	int tip_width = game_config::title_tip_width * screen.w() / 1024;

		try {
	        const std::string& text = 
				font::word_wrap_text((*tip)["text"], font::SIZE_NORMAL, tip_width);
			const std::string& source = 
				font::word_wrap_text((*tip)["source"], font::SIZE_NORMAL, tip_width);

			const int pad = game_config::title_tip_padding;

			SDL_Rect area = font::text_area(text,font::SIZE_NORMAL);
			area.w = tip_width;
			SDL_Rect source_area = font::text_area(source, font::SIZE_NORMAL, TTF_STYLE_ITALIC);
			area.w = maximum<size_t>(area.w, source_area.w) + 2*pad;
			area.h += source_area.h + next_tip_button->location().h + 3*pad;

			area.x = main_dialog_area->x - (game_config::title_tip_x * screen.w() / 1024) - area.w;
			area.y = main_dialog_area->y + main_dialog_area->h - area.h;

			// Note: The buttons' locations need to be set before the dialog frame is drawn.
			// Otherwise, when the buttons restore their area, they
			// draw parts of the old dialog frame at their old locations.
			// This way, the buttons draw a part of the title image,
			// because the call to restore above restored the area
			// of the old tip of the day to its initial state (the title image).
			int button_x = area.x + area.w - next_tip_button->location().w - pad;
			int button_y = area.y + area.h - pad - next_tip_button->location().h;
			next_tip_button->set_location(button_x, button_y);
			next_tip_button->set_dirty(); //force redraw even if location did not change.

			button_x -= previous_tip_button->location().w + pad;
			previous_tip_button->set_location(button_x, button_y);
			previous_tip_button->set_dirty();

			button_x = area.x + pad;
			help_tip_button->set_location(button_x, button_y);
			help_tip_button->set_dirty(); 

			gui::dialog_frame f(screen.video(), "", style, false);
			tip_of_day_restorer = surface_restorer(&screen.video(), f.layout(area).exterior);
			f.draw_background();
			f.draw_border();

			font::draw_text(&screen.video(), area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
							 text, area.x + pad, area.y + pad);
			// todo
			font::draw_text(&screen.video(), area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
							 source, area.x + area.w - source_area.w - pad,
							 next_tip_button->location().y - source_area.h - pad,
							 false, TTF_STYLE_ITALIC);
		} catch (utils::invalid_utf8_exception&) {
			LOG_STREAM(err, engine) << "Invalid utf-8 found, tips of day aren't drawn.\n";
			return;
		}

	    LOG_DP << "drew tip of day\n";
	}
}

namespace gui {

//! Show titlepage with logo and background, menu-buttons and tip-of-the-day.
//!
//! After the page is shown, this routine waits
//! for the user to click one of the menu-buttons,
//! or a keypress.
//!
//! @param	screen			surface to write on
//! @param	tips_of_day		list of tips
//! @param	ntip			number of the tip to show
//!
//! @return	the value of the menu-item the user has choosen.
//! @retval	see @ref TITLE_RESULT for possible values
//!
TITLE_RESULT show_title(game_display& screen, config& tips_of_day)
{
	cursor::set(cursor::NORMAL);

	const preferences::display_manager disp_manager(&screen);
	const hotkey::basic_handler key_handler(&screen);

	const font::floating_label_context label_manager;

	screen.video().modeChanged(); // resets modeChanged value

	bool fade_failed = false;
	do {
		int logo_x = game_config::title_logo_x * screen.w() / 1024,
		    logo_y = game_config::title_logo_y * screen.h() / 768;

		surface const title_surface(scale_surface(
			image::get_image(game_config::game_title),
			screen.w(), screen.h()));

		
		if (title_surface.null()) {
			ERR_DP << "Could not find title image\n";
		} else {
			screen.video().blit_surface(0, 0, title_surface);
			update_rect(screen_area());
			LOG_DP << "displayed title image\n";
		}

		fade_failed = !fade_logo(screen, logo_x, logo_y);
	} while (fade_failed);
	LOG_DP << "faded logo\n";

	// Display Wesnoth version and (if configured with --enable-display-revision) the svn-revision
	const std::string& version_str = _("Version") +
		std::string(" ") + game_config::version;

	const SDL_Rect version_area = font::draw_text(NULL, screen_area(),
								  font::SIZE_TINY, font::NORMAL_COLOUR,
								  version_str,0,0);
	const size_t versiony = screen.h() - version_area.h;

	if(versiony < size_t(screen.h())) {
		font::draw_text(&screen.video(),screen.screen_area(),
				font::SIZE_TINY, font::NORMAL_COLOUR,
				version_str,0,versiony);
	}

	LOG_DP << "drew version number\n";

	//- Texts for the menu-buttons.
	//- Members of this array must correspond to the enumeration TITLE_RESULT
	static const char* button_labels[] = {
					       N_("TitleScreen button^Tutorial"),
					       N_("TitleScreen button^Campaign"),
					       N_("TitleScreen button^Multiplayer"),
					       N_("TitleScreen button^Load"),
					       N_("TitleScreen button^Add-ons"),
					       N_("TitleScreen button^Language"),
					       N_("TitleScreen button^Preferences"),
					       N_("TitleScreen button^Credits"),
					       N_("TitleScreen button^Quit"),
								// Only the above buttons go into the menu-frame
								// Next 2 buttons go into frame for the tip-of-the-day:
					       N_("TitleScreen button^Previous"),
					       N_("TitleScreen button^Next"),
					       N_("TitleScreen button^Help"),
								// Next entry is no button, but shown as a mail-icon instead:
					       N_("TitleScreen button^Help Wesnoth") };
	//- Texts for the tooltips of the menu-buttons
	static const char* help_button_labels[] = { N_("Start a tutorial to familiarize yourself with the game"),
						    N_("Start a new single player campaign"),
						    N_("Play multiplayer (hotseat, LAN, or Internet), or a single scenario against the AI"),
						    N_("Load a saved game"),
						    N_("Download usermade campaigns, eras, or map packs"),
						    N_("Change the language"),
						    N_("Configure the game's settings"),
						    N_("View the credits"),
						    N_("Quit the game"),
						    N_("Show next tip of the day"),
						    N_("Show Battle for Wesnoth help"),
						    N_("Upload statistics") };

	static const size_t nbuttons = sizeof(button_labels)/sizeof(*button_labels);
	const int menu_xbase = (game_config::title_buttons_x*screen.w())/1024;
	const int menu_xincr = 0;

#ifdef USE_TINY_GUI
	const int menu_ybase = (game_config::title_buttons_y*screen.h())/768 - 15;
	const int menu_yincr = 15;
#else
	const int menu_ybase = (game_config::title_buttons_y*screen.h())/768;
	const int menu_yincr = 35;
#endif

	const int padding = game_config::title_buttons_padding;

	std::vector<button> buttons;
	size_t b, max_width = 0;
	size_t n_menubuttons = 0;
	for(b = 0; b != nbuttons; ++b) {
		buttons.push_back(button(screen.video(),sgettext(button_labels[b])));
		buttons.back().set_help_string(sgettext(help_button_labels[b]));
		max_width = maximum<size_t>(max_width,buttons.back().width());

		n_menubuttons = b;
		if(b == QUIT_GAME) break;	// Menu-frame ends at the quit-button
	}

	SDL_Rect main_dialog_area = {menu_xbase-padding, menu_ybase-padding, max_width+padding*2,
								 menu_yincr*(n_menubuttons)+buttons.back().height()+padding*2};

	gui::dialog_frame main_frame(screen.video(), "", gui::dialog_frame::titlescreen_style, false);
	main_frame.layout(main_dialog_area);
	main_frame.draw_background();
	main_frame.draw_border();

	for(b = 0; b != nbuttons; ++b) {
		buttons[b].set_width(max_width);
		buttons[b].set_location(menu_xbase + b*menu_xincr, menu_ybase + b*menu_yincr);
		if(b == QUIT_GAME) break;
	}

	b = TIP_PREVIOUS;
	gui::button previous_tip_button(screen.video(),sgettext(button_labels[b]),button::TYPE_PRESS,"lite_small");
	previous_tip_button.set_help_string( sgettext(button_labels[b] ));

	b = TIP_NEXT;
	gui::button next_tip_button(screen.video(),sgettext(button_labels[b]),button::TYPE_PRESS,"lite_small");
	next_tip_button.set_help_string( sgettext(button_labels[b] ));

	b = SHOW_HELP;
	gui::button help_tip_button(screen.video(),sgettext(button_labels[b]),button::TYPE_PRESS,"lite_small");
	help_tip_button.set_help_string( sgettext(button_labels[b] ));

	//! @todo FIXME: Translatable string is here because we WILL put text in before 1.2!
	gui::button beg_button(screen.video(),("Help Wesnoth"),button::TYPE_IMAGE,"menu-button",button::MINIMUM_SPACE);
	beg_button.set_help_string(_("Help Wesnoth by sending us information"));

	next_tip_of_day(tips_of_day);

	surface_restorer tip_of_day_restorer;

	draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
					&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);

	const int pad = game_config::title_tip_padding;
	beg_button.set_location(screen.w() - pad - beg_button.location().w,
		screen.h() - pad - beg_button.location().h);
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

		if(previous_tip_button.pressed()) {
			next_tip_of_day(tips_of_day, true);
			draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
						&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);
		}
		if(next_tip_button.pressed()) {
			next_tip_of_day(tips_of_day, false);
			draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
						&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);
		}

		if(help_tip_button.pressed()) {
			return SHOW_HELP;
		}
		if(beg_button.pressed()) {
			return BEG_FOR_UPLOAD;
		}

		events::raise_process_event();
		events::raise_draw_event();

		screen.flip();

		if(!last_escape && key[SDLK_ESCAPE])
			return QUIT_GAME;

		last_escape = key[SDLK_ESCAPE] != 0;

		events::pump();

		// If the resolution has changed due to the user resizing the screen,
		// or from changing between windowed and fullscreen:
		if(screen.video().modeChanged()) {
			return TIP_NEXT;
		}

		screen.delay(20);
	}

	return QUIT_GAME;
}

} // namespace gui

//.
