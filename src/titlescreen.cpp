/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file titlescreen.cpp
 *  Shows the titlescreen, with main-menu and tip-of-the-day.
 *
 *  The menu consists of buttons, such als Start-Tutorial, Start-Campaign,
 *  Load-Game, etc.  As decoration, the wesnoth-logo and a landmap in the
 *  background are shown.
 */

#include "global.hpp"

#include <algorithm>
#include <vector>

#include "config.hpp"
#include "construct_dialog.hpp"
#include "cursor.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
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
#include "text.hpp"
#include "titlescreen.hpp"
#include "video.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

/**
 *  Fade-in the wesnoth-logo.
 *
 *  Animation-effect: scroll-in from right. \n
 *  Used only once, after the game is started.
 *
 *  @param	screen	surface to operate on
 *  @param	xcpos	x-position of center of the logo
 *  @param	ycpos	y-position of center of the logo
 *
 *  @return		Result of running the routine
 *  @retval true	operation finished (successful or not)
 *  @retval false	operation failed (because modeChanged), need to retry
 */
static bool fade_logo(game_display& screen, int xcpos, int ycpos)
{
	surface const fb = screen.video().getSurface();
	if (!fb) return true;

	surface logo = image::get_image(game_config::game_logo);
	if (!logo) {
		ERR_DP << "Could not find game logo\n";
		return true;
	}

	int xpos = xcpos - logo->w / 2;
	int ypos = ycpos - logo->h / 2;

	if (xpos < 0 || ypos < 0 || xpos + logo->w > fb->w || ypos + logo->h > fb->h) {
		double scale = 2 * std::min(
			std::min((double)xcpos / logo->w, (double)(fb->w - ycpos) / logo->w),
			std::min((double)ycpos / logo->h, (double)(fb->h - ycpos) / logo->h));
		logo = scale_surface(logo, logo->w * scale, logo->h * scale);
		xpos = xcpos - logo->w / 2;
		ypos = ycpos - logo->h / 2;
	}

	// Only once, when the game is first started, the logo fades in unless
	// it was disabled in adv. preferences
	static bool faded_in = !preferences::startup_effect();
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


/** Read the file with the tips-of-the-day. */
void read_tips_of_day(config& tips_of_day)
{
	tips_of_day.clear();
	LOG_CF << "Loading tips of day\n";
	try {
		scoped_istream stream = preprocess_file(get_wml_location("hardwired/tips.cfg"));
		read(tips_of_day, *stream);
	} catch(config::error&) {
		ERR_CF << "Could not read data/hardwired/tips.cfg\n";
	}

	//We shuffle the tips after each initial loading.
	config::const_child_itors itors = tips_of_day.child_range("tip");
	if (itors.first != itors.second ) {
		std::vector<config> tips(itors.first, itors.second);
		std::random_shuffle(tips.begin(), tips.end());
		tips_of_day.clear();
		BOOST_FOREACH (const config &tip, tips) {
			tips_of_day.add_child("tip", tip);
		}
	}
}

/** Go to the next tips-of-the-day */
void next_tip_of_day(config& tips_of_day, bool reverse)
{
	// we just rotate the tip list, to avoid the need to keep track
	// of the current one, and keep it valid, cycle it, etc...
	config::const_child_itors itors = tips_of_day.child_range("tip");
	if (itors.first != itors.second) {
		std::vector<config> tips(itors.first, itors.second);
		std::vector<config>::iterator direction =
			reverse ? tips.begin() + 1 : tips.end() - 1;
		std::rotate(tips.begin(), direction, tips.end());
		tips_of_day.clear();
		BOOST_FOREACH (const config &tip, tips) {
			tips_of_day.add_child("tip", tip);
		}
	}
}

const config* get_tip_of_day(config& tips_of_day)
{
	if (tips_of_day.empty()) {
		read_tips_of_day(tips_of_day);
	}

	// next_tip_of_day rotate tips, so better stay iterator-safe
	for (int nb_tips = tips_of_day.child_count("tip"); nb_tips > 0;
	     --nb_tips, next_tip_of_day(tips_of_day))
	{
		const config &tip = tips_of_day.child("tip");
		assert(tip);

		const std::vector<std::string> needed_units = utils::split(tip["encountered_units"], ',');
		if (needed_units.empty()) {
			return &tip;
		}
		const std::set<std::string>& seen_units = preferences::encountered_units();

		// test if one of the listed unit types is already encountered
		// if if's a number, test if we have encountered more than this
		for (std::vector<std::string>::const_iterator i = needed_units.begin();
				i != needed_units.end(); ++i) {
			int needed_units_nb = lexical_cast_default<int>(*i,-1);
			if (needed_units_nb !=-1) {
				if (needed_units_nb <= static_cast<int>(seen_units.size())) {
					return &tip;
				}
			} else if (seen_units.find(*i) != seen_units.end()) {
				return &tip;
			}
		}
	}
	// not tip match, someone forget to put an always-match one
	return NULL;
}

/**
 *  Show one tip-of-the-day in a frame on the titlescreen.
 *  This frame has 2 buttons: Next-Tip, and Show-Help.
 */
static void draw_tip_of_day(game_display& screen,
							config& tips_of_day,
							const gui::dialog_frame::style& style,
							gui::button* const previous_tip_button,
							gui::button* const next_tip_button,
							gui::button* const help_tip_button,
							const SDL_Rect* const main_dialog_area,
							surface_restorer& tip_of_day_restorer)
{
	// Restore the previous tip of day area to its old state (section of the title image).
	tip_of_day_restorer.restore();

	// Draw tip of the day
	const config* tip = get_tip_of_day(tips_of_day);
	if (!tip) return;
	int tip_width = game_config::title_tip_width * screen.w() / 1024;

	font::ttext text, source;
	text.set_text((*tip)["text"], false);
	text.set_maximum_width(tip_width);
	text.set_maximum_height(main_dialog_area->h);
	source.set_text((*tip)["source"], false);
	source.set_font_style(font::ttext::STYLE_ITALIC);
	source.set_maximum_width(tip_width);
	source.set_alignment(PANGO_ALIGN_RIGHT);

	int pad = game_config::title_tip_padding;

	SDL_Rect area, source_area;
	area.w = tip_width;
	area.h = text.get_height();
	source_area.w = source.get_width();
	source_area.h = source.get_height();
	area.w = std::max<size_t>(area.w, source_area.w) + 2 * pad;
	area.h += source_area.h + next_tip_button->location().h + 3 * pad;

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

	surface text_s = text.render();
	screen.video().blit_surface(area.x + pad, area.y + pad, text_s);
	surface source_s = source.render();
	screen.video().blit_surface(area.x + pad,
		next_tip_button->location().y - source_area.h - pad, source_s);

	LOG_DP << "drew tip of day\n";
}

/**
 *  Draw the map image background, revision number
 *  and fade the log the first time
 */
static void draw_background(game_display& screen)
{
	bool fade_failed = false;
	do {
		int logo_x = game_config::title_logo_x * screen.w() / 1024,
			logo_y = game_config::title_logo_y * screen.h() / 768;

		/*Select a random game_title*/
		std::vector<std::string> game_title_list =
			utils::split(game_config::game_title, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);

		if(game_title_list.empty()) {
			ERR_CF << "No title image defined\n";
		} else {
			surface const title_surface(scale_opaque_surface(
				image::get_image(game_title_list[rand()%game_title_list.size()]),
				screen.w(), screen.h()));


			if (title_surface.null()) {
				ERR_DP << "Could not find title image\n";
			} else {
				screen.video().blit_surface(0, 0, title_surface);
				update_rect(screen_area());
				LOG_DP << "displayed title image\n";
			}
		}

		fade_failed = !fade_logo(screen, logo_x, logo_y);
	} while (fade_failed);
	LOG_DP << "faded logo\n";

	// Display Wesnoth version and (if possible) revision
	const std::string& version_str = _("Version") +
		std::string(" ") + game_config::revision;

	const SDL_Rect version_area = font::draw_text(NULL, screen_area(),
								  font::SIZE_TINY, font::NORMAL_COLOUR,
								  version_str,0,0);
	const size_t versiony = screen.h() - version_area.h;

	if(versiony < size_t(screen.h())) {
		draw_solid_tinted_rectangle(0, versiony - 2, version_area.w + 3, version_area.h + 2,0,0,0,0.75,screen.video().getSurface());
		font::draw_text(&screen.video(),screen.screen_area(),
				font::SIZE_TINY, font::NORMAL_COLOUR,
				version_str,0,versiony);
	}

	LOG_DP << "drew version number\n";
}

namespace {

/**
 *  Handler for forcing a discrete ESC keypress to quit the game (bug #12747)
 *  This hack is here because the GUI code used here doesn't handle this yet.
 *  Once it does, revert this part of r31758.
 */
class titlescreen_handler : public events::handler
{
public:
	titlescreen_handler(bool ignore_esc = false)
		: handler(), ignore_esc_(ignore_esc)
	{
		if(ignore_esc) {
			LOG_DP << "ignoring held ESCAPE key\n";
		}
	}

	bool get_esc_ignore() const { return ignore_esc_; }

	virtual void handle_event(const SDL_Event& event)
	{
		if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
			ignore_esc_ = false;
			LOG_DP << "ESCAPE key no longer ignored\n";
		}
	}

private:
	bool ignore_esc_;
};

}	// end anon namespace

namespace gui {


static bool background_is_dirty_ = true;
void set_background_dirty() {
	background_is_dirty_ = true;
}


TITLE_RESULT show_title(game_display& screen, config& tips_of_day)
{
	cursor::set(cursor::NORMAL);

	const preferences::display_manager disp_manager(&screen);
	const hotkey::basic_handler key_handler(&screen);

	const font::floating_label_context label_manager;

	screen.video().modeChanged(); // resets modeChanged value

	if (background_is_dirty_) {
		draw_background(screen);
	}

	//- Texts for the menu-buttons.
	//- Members of this array must correspond to the enumeration TITLE_RESULT
	static const char* button_labels[] = {
		N_("TitleScreen button^Tutorial"),
		N_("TitleScreen button^Campaign"),
		N_("TitleScreen button^Multiplayer"),
		N_("TitleScreen button^Load"),
		N_("TitleScreen button^Add-ons"),
#ifndef DISABLE_EDITOR
		N_("TitleScreen button^Map Editor"),
#endif
		N_("TitleScreen button^Language"),
		N_("TitleScreen button^Preferences"),
		N_("TitleScreen button^Credits"),
		N_("TitleScreen button^Quit"),
		// Only the above buttons go into the menu-frame
		// Next 2 buttons go into frame for the tip-of-the-day:
		N_("TitleScreen button^Previous"),
		N_("TitleScreen button^Next"),
		N_("TitleScreen button^Help"),
	};
	//- Texts for the tooltips of the menu-buttons
	static const char* help_button_labels[] = {
		N_("Start a tutorial to familiarize yourself with the game"),
		N_("Start a new single player campaign"),
		N_("Play multiplayer (hotseat, LAN, or Internet), or a single scenario against the AI"),
		N_("Load a saved game"),
		N_("Download usermade campaigns, eras, or map packs"),
#ifndef DISABLE_EDITOR
		N_("Start the map editor"),
#endif
		N_("Change the language"),
		N_("Configure the game's settings"),
		N_("View the credits"),
		N_("Quit the game"),
		N_("Show previous tip of the day"),
		N_("Show next tip of the day"),
		N_("Show Battle for Wesnoth help")
	};

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
	for (b = 0; b != nbuttons; ++b)
	{
		std::string label = sgettext(button_labels[b]);
		if (b + TUTORIAL <= QUIT_GAME) {
			buttons.push_back(button(screen.video(), label));
			max_width = std::max<size_t>(max_width,buttons.back().width	());
			n_menubuttons = b;
		} else {
			buttons.push_back(button(screen.video(), label,
				button::TYPE_PRESS, "lite_small"));
		}
		buttons.back().set_help_string(sgettext(help_button_labels[b]));
	}

	SDL_Rect main_dialog_area = {menu_xbase-padding, menu_ybase-padding, max_width+padding*2,
								 menu_yincr*(n_menubuttons)+buttons.back().height()+padding*2};

	gui::dialog_frame main_frame(screen.video(), "", gui::dialog_frame::titlescreen_style, false);
	main_frame.layout(main_dialog_area);

	// we only redraw transparent parts when asked,
	// to prevent alpha growing
	if (background_is_dirty_) {
		main_frame.draw_background();
		main_frame.draw_border();
	}

	for(b = 0; b != nbuttons; ++b) {
		buttons[b].set_width(max_width);
		buttons[b].set_location(menu_xbase + b*menu_xincr, menu_ybase + b*menu_yincr);
		if(b + TUTORIAL == QUIT_GAME) break;
	}

	gui::button &previous_tip_button = buttons[TIP_PREVIOUS - TUTORIAL];
	gui::button &next_tip_button = buttons[TIP_NEXT - TUTORIAL];
	gui::button &help_tip_button = buttons[SHOW_HELP - TUTORIAL];

	next_tip_of_day(tips_of_day);

	surface_restorer tip_of_day_restorer;

	draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
					&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);

	events::raise_draw_event();

	LOG_DP << "drew buttons dialog\n";

	CKey key;

	size_t keyboard_button = nbuttons;
	bool key_processed = false;

	update_whole_screen();
	background_is_dirty_ = false;

	titlescreen_handler ts_handler(key[SDLK_ESCAPE] != 0); //!= 0 to avoid a MSVC warning C4800

	LOG_DP << "entering interactive loop...\n";

	for(;;) {
		events::pump();
		for(size_t b = 0; b != buttons.size(); ++b) {
			if (b != TIP_PREVIOUS - TUTORIAL && b != TIP_NEXT - TUTORIAL &&
			    buttons[b].pressed()) {
				return static_cast<TITLE_RESULT>(b + TUTORIAL);
			}
		}

		if(previous_tip_button.pressed() || (key[SDLK_RETURN] && keyboard_button == TIP_PREVIOUS - TUTORIAL)) {
			next_tip_of_day(tips_of_day, true);
			draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
						&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);
		}
		if(next_tip_button.pressed() || (key[SDLK_RETURN] && keyboard_button == TIP_NEXT - TUTORIAL)) {
			next_tip_of_day(tips_of_day, false);
			draw_tip_of_day(screen, tips_of_day, gui::dialog_frame::titlescreen_style,
						&previous_tip_button, &next_tip_button, &help_tip_button, &main_dialog_area, tip_of_day_restorer);
		}

		if(help_tip_button.pressed()) {
			return SHOW_HELP;
		}
		if (key[SDLK_UP]) {
			if (!key_processed) {
				if (keyboard_button < nbuttons)
					buttons[keyboard_button].set_active(false);
				if (keyboard_button == 0) {
					keyboard_button = nbuttons - 1;
				} else {
					--keyboard_button;
				}
				key_processed = true;
				buttons[keyboard_button].set_active(true);
			}
		} else if (key[SDLK_DOWN]) {
			if (!key_processed) {
				if (keyboard_button < nbuttons)
					buttons[keyboard_button].set_active(false);
				if (keyboard_button >= nbuttons - 1) {
					keyboard_button = 0;
				} else {
					++keyboard_button;
				}
				key_processed = true;
				buttons[keyboard_button].set_active(true);
			}
		} else {
			key_processed = false;
		}

		events::raise_process_event();
		events::raise_draw_event();

		screen.flip();

		if (key[SDLK_ESCAPE] && !ts_handler.get_esc_ignore())
			return QUIT_GAME;
		if (key[SDLK_F5])
			return RELOAD_GAME_DATA;

		if (key[SDLK_RETURN] && keyboard_button < nbuttons
		&& keyboard_button != TIP_PREVIOUS - TUTORIAL
		&& keyboard_button != TIP_NEXT - TUTORIAL) {
			return static_cast<TITLE_RESULT>(keyboard_button + TUTORIAL);
		}



		// If the resolution has changed due to the user resizing the screen,
		// or from changing between windowed and fullscreen:
		if(screen.video().modeChanged()) {
			return REDRAW_BACKGROUND;
		}

		screen.delay(20);
	}

	return QUIT_GAME;
}

} // namespace gui

//.
