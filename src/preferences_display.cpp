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

//! @file preferences_display.cpp 
//! Manage display-related preferences, e.g. screen-size, gamma etc.

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "cursor.hpp"
#include "display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "preferences_display.hpp"
#include "construct_dialog.hpp"
#include "show_dialog.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "widgets/button.hpp"
#include "widgets/label.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
#include "widgets/textbox.hpp"
#include "theme.hpp"

#include <vector>
#include <string>

namespace preferences {

display* disp = NULL;

display_manager::display_manager(display* d)
{
	disp = d;

	load_hotkeys();

	set_grid(grid());
	set_turbo(turbo());
	set_turbo_speed(turbo_speed());
	set_fullscreen(fullscreen());
	set_gamma(gamma());
	set_colour_cursors(preferences::get("colour_cursors") == "yes");
}

display_manager::~display_manager()
{
	disp = NULL;
}

void set_fullscreen(bool ison)
{
	_set_fullscreen(ison);

	if(disp != NULL) {
		const std::pair<int,int>& res = resolution();
		CVideo& video = disp->video();
		if(video.isFullScreen() != ison) {
			const int flags = ison ? FULL_SCREEN : 0;
			const int bpp = video.modePossible(res.first,res.second,16,flags);
			if(bpp > 0) {
				video.setMode(res.first,res.second,bpp,flags);
				disp->redraw_everything();
			} else if(video.modePossible(1024,768,16,flags)) {
				set_resolution(std::pair<int,int>(1024,768));
			} else {
				gui::message_dialog(*disp,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen.")).show();
			}
			// We reinit color cursors, because SDL on Mac seems to forget the SDL_Cursor
			set_colour_cursors(preferences::get("colour_cursors") == "yes");
		}
	}
}

void set_resolution(const std::pair<int,int>& resolution)
{
	std::pair<int,int> res = resolution;

	// - Ayin: disabled the following code. Why would one want to enforce that?
	// Some 16:9, or laptop screens, may have resolutions which do not
	// comply to this rule (see bug 10630). 
	// I'm commenting this until it proves absolutely necessary.
	//
	// Make sure resolutions are always divisible by 4
	//res.first &= ~3;
	//res.second &= ~3;

	bool write_resolution = true;

	if(disp != NULL) {
		CVideo& video = disp->video();
		const int flags = fullscreen() ? FULL_SCREEN : 0;
		const int bpp = video.modePossible(res.first,res.second,16,flags);
		if(bpp != 0) {
			video.setMode(res.first,res.second,bpp,flags);
			disp->redraw_everything();

		} else {
			write_resolution = false;
			gui::message_dialog(*disp,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen.")).show();
		}
	}

	if(write_resolution) {
		const std::string postfix = fullscreen() ? "resolution" : "windowsize";
		preferences::set('x' + postfix, lexical_cast<std::string>(res.first));
		preferences::set('y' + postfix, lexical_cast<std::string>(res.second));
	}
}

void set_turbo(bool ison)
{
	_set_turbo(ison);

	if(disp != NULL) {
		disp->set_turbo(ison);
	}
}

void set_turbo_speed(double speed)
{
	save_turbo_speed(speed);

	if(disp != NULL) {
		disp->set_turbo_speed(speed);
	}
}

void set_adjust_gamma(bool val)
{
	// If we are turning gamma adjustment off, then set it to '1.0'
	if(val == false && adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma(1.0);
	}

	_set_adjust_gamma(val);
}

void set_gamma(int gamma)
{
	_set_gamma(gamma);

	if(adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma(static_cast<float>(gamma) / 100);
	}
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void set_colour_cursors(bool value)
{
	_set_colour_cursors(value);

	cursor::set();
}

void set_idle_anim(bool ison) {
	_set_idle_anim(ison);
	if(disp != NULL) {
		disp->set_idle_anim(ison);
	}
}

void set_idle_anim_rate(int rate) {
	_set_idle_anim_rate(rate);
	if(disp != NULL) {
		disp->set_idle_anim_rate(rate);
	}
}

namespace {
class escape_handler : public events::handler {
public:
	escape_handler() : escape_pressed_(false) {}
	bool escape_pressed() { return escape_pressed_; }
	void handle_event(const SDL_Event &event) { escape_pressed_ |= (event.type == SDL_KEYDOWN)
		&& (reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym.sym == SDLK_ESCAPE); }
private:
	bool escape_pressed_;
};
} // end anonymous namespace

void show_hotkeys_dialog (display & disp, config *save_config)
{
	log_scope ("show_hotkeys_dialog");

	const events::event_context dialog_events_context;

	const int centerx = disp.w()/2;
	const int centery = disp.h()/2;
#ifdef USE_TINY_GUI
	const int width  = 300;			  //! @todo FIXME: We should compute this, but using what data ? 
		//! @todo FIXME:  suokko: window width and height could be usefull. min(300,disp.w()*0.9)  So it would be either 300 or max 90% of width
	const int height = 220;
#else
	const int width  = 700;
	const int height = 500;
#endif
	const int xpos = centerx  - width/2;
	const int ypos = centery  - height/2;

	gui::button close_button (disp.video(), _("Close Window"));
	std::vector<gui::button*> buttons;
	buttons.push_back(&close_button);

	gui::dialog_frame f(disp.video(),_("Hotkey Settings"),gui::dialog_frame::default_style,true,&buttons);
	f.layout(xpos,ypos,width,height);
	f.draw();

	SDL_Rect clip_rect = { 0, 0, disp.w (), disp.h () };
	SDL_Rect text_size = font::draw_text(NULL, clip_rect, font::SIZE_PLUS,
					     font::NORMAL_COLOUR,_("Press desired Hotkey"),
					     0, 0);

	std::vector<std::string> menu_items;

	std::vector<hotkey::hotkey_item>& hotkeys = hotkey::get_hotkeys();
	for(std::vector<hotkey::hotkey_item>::iterator i = hotkeys.begin(); i != hotkeys.end(); ++i) {
		if(i->hidden())
			continue;
		std::stringstream str,name;
		name << i->get_description();
		str << name.str();
		str << COLUMN_SEPARATOR;
		// This trick allows to display chars identical to markup characters
		str << font::NULL_MARKUP << i->get_name();
		menu_items.push_back(str.str());
	}

	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Action") << COLUMN_SEPARATOR << _("Binding");
	menu_items.push_back(heading.str());

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_alpha_sort(1);

	gui::menu menu_(disp.video(), menu_items, false, height, -1, &sorter, &gui::menu::bluebg_style);
	menu_.sort_by(0);
	menu_.reset_selection();
	menu_.set_width(font::relative_size(400));
	menu_.set_location(xpos + font::relative_size(20), ypos);

	gui::button change_button (disp.video(), _("Change Hotkey"));
	change_button.set_location(xpos + width - change_button.width () - font::relative_size(30),ypos + font::relative_size(30));

	//! @todo  FIXME: TODO This have to be added after string freeze
	//! @todo Remember to make Clear Hotkey translateable
#if 0
	gui::button clear_button (disp.video(), ("Clear Hotkey"));
	clear_button.set_location(xpos + width - clear_button.width () - font::relative_size(30),ypos + font::relative_size(80));
#endif
	gui::button save_button (disp.video(), _("Save Hotkeys"));
	save_button.set_location(xpos + width - save_button.width () - font::relative_size(30),ypos + font::relative_size(130));

	escape_handler esc_hand;

	for(;;) {

		if (close_button.pressed() || esc_hand.escape_pressed())
			break;

		if (change_button.pressed () || menu_.double_clicked()) {
			// Lets change this hotkey......
			SDL_Rect dlgr = {centerx-text_size.w/2 - 30,
								centery-text_size.h/2 - 16,
									text_size.w+60,
									text_size.h+32};
			surface_restorer restorer(&disp.video(),dlgr);
			gui::dialog_frame mini_frame(disp.video());
			mini_frame.layout(centerx-text_size.w/2 - 20,
									centery-text_size.h/2 - 6,
									text_size.w+40,
									text_size.h+12);
			mini_frame.draw_background();
			mini_frame.draw_border();
			//! @todo FIXME: add to text mention about esc clearing hotkey after string freeze is over!
			font::draw_text (&disp.video(), clip_rect, font::SIZE_LARGE,font::NORMAL_COLOUR,
				 _("Press desired Hotkey"),centerx-text_size.w/2-10,
				 centery-text_size.h/2-3);
			disp.update_display();
			SDL_Event event;
			event.type = 0;
			int character=0,keycode=0; // Just to avoid warning
			int mod=0;
			while (event.type!=SDL_KEYDOWN) SDL_PollEvent(&event);
			do {
				if (event.type==SDL_KEYDOWN)
				{
					keycode=event.key.keysym.sym;
					character=event.key.keysym.unicode;
					mod=event.key.keysym.mod;
				};
				SDL_PollEvent(&event);
				disp.flip();
				disp.delay(10);
			} while (event.type!=SDL_KEYUP);
			restorer.restore();
			disp.update_display();

			if (keycode == SDLK_ESCAPE && mod == 0)
			{
				// clear hotkey
				hotkey::hotkey_item& newhk = hotkey::get_visible_hotkey(menu_.selection());
				newhk.clear_hotkey();
				menu_.change_item(menu_.selection(), 1, font::NULL_MARKUP + newhk.get_name());

			}
			else
			{

				const hotkey::hotkey_item& oldhk = hotkey::get_hotkey(character, keycode, (mod & KMOD_SHIFT) != 0,
						(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);

				hotkey::hotkey_item& newhk = hotkey::get_visible_hotkey(menu_.selection());

				if(oldhk.get_id() != newhk.get_id() && !oldhk.null()) {
					gui::message_dialog(disp,"",_("This Hotkey is already in use.")).show();
				} else {

					newhk.set_key(character, keycode, (mod & KMOD_SHIFT) != 0,
							(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);
		
					menu_.change_item(menu_.selection(), 1, font::NULL_MARKUP + newhk.get_name());
				}
			}
		}
		if (save_button.pressed()) {
			if (save_config == NULL) {
				save_hotkeys();
			} else {
				hotkey::save_hotkeys(*save_config);
			}
		}
//! FIXME: remember to uncomment this also after string freeze
#if 0
		if (clear_button.pressed()) {
			// clear hotkey
			hotkey::hotkey_item& newhk = hotkey::clear_hotkey(menu_.selection());
			newhk.clear_hotkey();
			menu_.change_item(menu_.selection(), 1, font::NULL_MARKUP + newhk.get_name());
		}
#endif

		menu_.process();

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		disp.update_display();

		disp.delay(10);
	}
}

bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs)
{
	return lhs.first*lhs.second < rhs.first*rhs.second;
}

bool show_video_mode_dialog(display& disp)
{
	const resize_lock prevent_resizing;
	const events::event_context dialog_events_context;

	CVideo& video = disp.video();

	SDL_PixelFormat format = *video.getSurface()->format;
	format.BitsPerPixel = video.getBpp();

	const SDL_Rect* const * modes = SDL_ListModes(&format,FULL_SCREEN);

	// The SDL documentation says that a return value of -1 
	// means that all dimensions are supported/possible.
	if(modes == reinterpret_cast<SDL_Rect**>(-1)) {
		std::cerr << "Can support any video mode\n";
		// SDL says that all modes are possible, 
		// so it's OK to use a hardcoded list here.
		static const SDL_Rect scr_modes[] = {
			{ 0, 0,  800, 600 },
			{ 0, 0, 1024, 768 },
			{ 0, 0, 1280, 960 },
			{ 0, 0, 1280, 1024 },
		};
		static const SDL_Rect * const scr_modes_list[] = {
			&scr_modes[0],
			&scr_modes[1],
			&scr_modes[2],
			&scr_modes[3],
			NULL
		};

		modes = scr_modes_list;
	} else if(modes == NULL) {
		std::cerr << "No modes supported\n";
		gui::message_dialog(disp,"",_("There are no alternative video modes available")).show();
		return false;
	}

	std::vector<std::pair<int,int> > resolutions;

	for(int i = 0; modes[i] != NULL; ++i) {
		if(modes[i]->w >= min_allowed_width && modes[i]->h >= min_allowed_height) {
			resolutions.push_back(std::pair<int,int>(modes[i]->w,modes[i]->h));
		}
	}

	const std::pair<int,int> current_res(video.getSurface()->w,video.getSurface()->h);
	resolutions.push_back(current_res);

	std::sort(resolutions.begin(),resolutions.end(),compare_resolutions);
	resolutions.erase(std::unique(resolutions.begin(),resolutions.end()),resolutions.end());

	std::vector<std::string> options;
	for(std::vector<std::pair<int,int> >::const_iterator j = resolutions.begin(); j != resolutions.end(); ++j) {
		std::ostringstream option;
		if (*j == current_res)
			option << DEFAULT_ITEM;

		option << j->first << "x" << j->second;
		options.push_back(option.str());
	}

	const int result = gui::show_dialog(disp,NULL,"",
	                                    _("Choose Resolution"),
	                                    gui::OK_CANCEL,&options);
	if(size_t(result) < resolutions.size() && resolutions[result] != current_res) {
		set_resolution(resolutions[result]);
		return true;
	} else {
		return false;
	}
}

} // end namespace preferences

