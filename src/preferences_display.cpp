/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

/**
 *  @file
 *  Manage display-related preferences, e.g. screen-size, etc.
 */

#include "global.hpp"
#include "preferences_display.hpp"

#include "construct_dialog.hpp"
#include "display.hpp"
#include "formatter.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

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
	set_scroll_to_action(scroll_to_action());
	set_color_cursors(preferences::get("color_cursors", false));
}

display_manager::~display_manager()
{
	disp = NULL;
}

bool detect_video_settings(CVideo& video, std::pair<int,int>& resolution, int& bpp, int& video_flags)
{
	video_flags = fullscreen() ? FULL_SCREEN : 0;
	resolution = preferences::resolution();

	int DefaultBPP = 24;
	const SDL_VideoInfo* const video_info = SDL_GetVideoInfo();
	if(video_info != NULL && video_info->vfmt != NULL) {
		DefaultBPP = video_info->vfmt->BitsPerPixel;
	}

	std::cerr << "Checking video mode: " << resolution.first << 'x'
		<< resolution.second << 'x' << DefaultBPP << "...\n";

	typedef std::pair<int, int> res_t;
	std::vector<res_t> res_list;
	res_list.push_back(res_t(1024, 768));
	res_list.push_back(res_t(1024, 600));
	res_list.push_back(res_t(800, 600));
	res_list.push_back(res_t(800, 480));

	bpp = video.modePossible(resolution.first, resolution.second,
		DefaultBPP, video_flags, true);

	BOOST_FOREACH(const res_t &res, res_list)
	{
		if (bpp != 0) break;
		std::cerr << "Video mode " << resolution.first << 'x'
			<< resolution.second << 'x' << DefaultBPP
			<< " is not supported; attempting " << res.first
			<< 'x' << res.second << 'x' << DefaultBPP << "...\n";
		resolution = res;
		bpp = video.modePossible(resolution.first, resolution.second,
			DefaultBPP, video_flags);
	}

	return bpp != 0;
}

void set_fullscreen(CVideo& video, const bool ison)
{
	_set_fullscreen(ison);

	const std::pair<int,int>& res = resolution();
	if(video.isFullScreen() != ison) {
		const int flags = ison ? FULL_SCREEN : 0;
		int bpp = video.bppForMode(res.first, res.second, flags);

		if(bpp > 0) {
			video.setMode(res.first,res.second,bpp,flags);
			if(disp) {
				disp->redraw_everything();
			}
		} else {
			int tmp_flags = flags;
			std::pair<int,int> tmp_res;
			if(detect_video_settings(video, tmp_res, bpp, tmp_flags)) {
				set_resolution(video, tmp_res.first, tmp_res.second);
			// TODO: see if below line is actually needed, possibly for displays that only support 16 bbp
			} else if(video.modePossible(1024,768,16,flags)) {
				set_resolution(video, 1024, 768);
			} else {
				gui2::show_transient_message(video,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."));
			}
			// We reinit color cursors, because SDL on Mac seems to forget the SDL_Cursor
			set_color_cursors(preferences::get("color_cursors", false));
		}
	}
}

void set_fullscreen(bool ison)
{
	_set_fullscreen(ison);

	if(disp != NULL) {
		set_fullscreen(disp->video(), ison);
	}
}

void set_scroll_to_action(bool ison)
{
	_set_scroll_to_action(ison);
}
void set_resolution(const std::pair<int,int>& resolution)
{
	if(disp) {
		set_resolution(disp->video(), resolution.first, resolution.second);
	} else {
		/* This part is needed when wesnoth is started with the -r parameter. */
		const std::string postfix = fullscreen() ? "resolution" : "windowsize";
		preferences::set(
				'x' + postfix, lexical_cast<std::string>(resolution.first));
		preferences::set(
				'y' + postfix, lexical_cast<std::string>(resolution.second));
	}
}

bool set_resolution(CVideo& video
		, const unsigned width, const unsigned height)
{
	SDL_Rect rect;
	SDL_GetClipRect(video.getSurface(), &rect);
	if(rect.w == width && rect.h == height) {
		return true;
	}

	const int flags = fullscreen() ? FULL_SCREEN : 0;
	int bpp = video.bppForMode(width, height, flags);

	if(bpp != 0) {
		video.setMode(width, height, bpp, flags);

		if(disp) {
			disp->redraw_everything();
		}

	} else {
        // grzywacz: is this even true?
		gui2::show_transient_message(video,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."));
		return false;
	}

	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	preferences::set('x' + postfix, lexical_cast<std::string>(width));
	preferences::set('y' + postfix, lexical_cast<std::string>(height));

	return true;
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

void set_ellipses(bool ison)
{
	_set_ellipses(ison);
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void set_color_cursors(bool value)
{
	_set_color_cursors(value);

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
	bool escape_pressed() const { return escape_pressed_; }
	void handle_event(const SDL_Event &event) { escape_pressed_ |= (event.type == SDL_KEYDOWN)
		&& (reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym.sym == SDLK_ESCAPE); }
private:
	bool escape_pressed_;
};

void repopulate_hotkeys_menu(std::vector<std::string>& menu_items,
		std::vector<std::string>& item_commands)
{
	menu_items.clear();
	item_commands.clear();

	const hotkey::hotkey_command* list = hotkey::get_hotkey_commands();

	for (size_t i = 0; list[i].id != hotkey::HOTKEY_NULL; ++i) {

		const hotkey::scope& scope = list[i].scope;
		if (list[i].hidden || !(hotkey::is_scope_active(scope)) )
				continue;

		const std::string& description = hotkey::get_description(list[i].command);
		const std::string& name = hotkey::get_names(list[i].id);

		menu_items.push_back((formatter() << description << COLUMN_SEPARATOR << font::NULL_MARKUP << name).str());
		item_commands.push_back(list[i].command);
	}
	menu_items.push_back((formatter() << HEADING_PREFIX << _("Action") << COLUMN_SEPARATOR << _("Binding")).str());
}

} // end anonymous namespace

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4701)
#endif
void show_hotkeys_dialog(display & disp)
{
	log_scope ("show_hotkeys_dialog");

	const events::event_context dialog_events_context;

	const int centerx = disp.w()/2;
	const int centery = disp.h()/2;
	const int width  = 750;
	const int height = disp.video().gety() < 600 ? 380 : 500;
	const int xpos = centerx  - width/2;
	const int ypos = centery  - height/2;

	gui::button close_button (disp.video(), _("Close"));
	std::vector<gui::button*> buttons;
	buttons.push_back(&close_button);

	gui::dialog_frame f(disp.video(),_("Hotkey Settings"),gui::dialog_frame::default_style,true,&buttons);
	f.layout(xpos,ypos,width,height);
	f.draw();

	SDL_Rect clip_rect = create_rect(0, 0, disp.w (), disp.h ());
	SDL_Rect text_size = font::draw_text(NULL, clip_rect, font::SIZE_LARGE,
					     font::NORMAL_COLOR,_("Press desired hotkey (Esc cancels)"),
					     0, 0);

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_alpha_sort(1);

	std::vector<std::string> menu_items;
	std::vector<std::string> item_commands;
	repopulate_hotkeys_menu(menu_items, item_commands);

	gui::menu menu_(disp.video(), menu_items, false, height - font::relative_size(10), -1, &sorter, &gui::menu::bluebg_style);
	menu_.sort_by(0);
	menu_.reset_selection();
	menu_.set_width(font::relative_size(550));
	menu_.set_location(xpos + font::relative_size(10), ypos + font::relative_size(10));

	gui::button change_button(disp.video(), _("Add Hotkey"));
	change_button.set_location(xpos + width - change_button.width () - font::relative_size(30), ypos + font::relative_size(30));

	gui::button clear_button(disp.video(), _("Clear Hotkey"));
	clear_button.set_location(xpos + width - clear_button.width () - font::relative_size(30), ypos + font::relative_size(60));

	gui::button reset_button(disp.video(), _("Reset All"));
	reset_button.set_location(xpos + width - reset_button.width() - font::relative_size(30), ypos + font::relative_size(90));

	escape_handler esc_hand;

	for(;;) {

		if (close_button.pressed() || esc_hand.escape_pressed())
		{
			save_hotkeys();
			break;
		}

		bool repopulate = false;
		const int selected = menu_.selection();
		const std::string id = item_commands[selected];
		if (change_button.pressed () || menu_.double_clicked()) {
			// Lets change this hotkey......
			SDL_Rect dlgr = create_rect(centerx - text_size.w / 2 - 30
					, centery - text_size.h / 2 - 16
					, text_size.w + 60
					, text_size.h + 32);

			surface_restorer restorer(&disp.video(),dlgr);
			gui::dialog_frame mini_frame(disp.video());
			mini_frame.layout(centerx-text_size.w/2 - 20,
									centery-text_size.h/2 - 6,
									text_size.w+40,
									text_size.h+12);
			mini_frame.draw_background();
			mini_frame.draw_border();
			font::draw_text (&disp.video(), clip_rect, font::SIZE_LARGE,font::NORMAL_COLOR,
				 _("Press desired hotkey (Esc cancels)"),centerx-text_size.w/2,
				 centery-text_size.h/2);
			disp.update_display();
			SDL_Event event;
			event.type = 0;
			int character = 0, keycode = 0, mod = 0; // Just to avoid warning
			int device = 0, button = 0, hat = 0, value = 0;
			const int any_mod = KMOD_CTRL | KMOD_ALT | KMOD_LMETA;

			while (event.type!=SDL_KEYDOWN
					&& event.type!=SDL_JOYBUTTONDOWN && event.type!= SDL_JOYHATMOTION
					&& (event.type!=SDL_MOUSEBUTTONDOWN || event.button.button < 8) )
				SDL_PollEvent(&event);

			do {
				switch (event.type) {

					case SDL_KEYDOWN:
						keycode=event.key.keysym.sym;
						character=event.key.keysym.unicode;
						mod=event.key.keysym.mod;
						break;
					case SDL_JOYBUTTONDOWN:
						device = event.jbutton.which;
						button = event.jbutton.button;
						break;
					case SDL_JOYHATMOTION:
						device = event.jhat.which;
						hat = event.jhat.hat;
						value = event.jhat.value;
						break;
					case SDL_MOUSEBUTTONDOWN:
						device = event.button.which;
						button = event.button.button;
						break;
				}

				SDL_PollEvent(&event);
				disp.flip();
				disp.delay(10);
			} while (event.type!=SDL_KEYUP
					&& event.type!=SDL_JOYBUTTONUP && event.type!=SDL_JOYHATMOTION
					&& event.type!=SDL_MOUSEBUTTONUP);

			restorer.restore();
			disp.update_display();

			// only if not canceled.
			if (!(keycode == SDLK_ESCAPE && (mod & any_mod) == 0)) {

				hotkey::hotkey_item newhk(id);
				hotkey::hotkey_item* oldhk = NULL;

				Uint8 *keystate = SDL_GetKeyState(NULL);
				bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];
				bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
				bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
				bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];

				switch (event.type) {

				case SDL_JOYHATMOTION:
					oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::JHAT, device, hat, value, shift, ctrl, alt, cmd);
					newhk.set_jhat(device, hat, value, shift, ctrl, alt, cmd);
					break;
				case SDL_JOYBUTTONUP:
					oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::JBUTTON, device, button, 0, shift, ctrl, alt, cmd);
					newhk.set_jbutton(device, button, shift, ctrl, alt, cmd);
					break;
				case SDL_MOUSEBUTTONUP:
					oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::MBUTTON, device, button, 0, shift, ctrl, alt, cmd);
					newhk.set_mbutton(device, button, shift, ctrl, alt, cmd);
					break;
				case SDL_KEYUP:
					oldhk = &hotkey::get_hotkey(character, keycode, (mod & KMOD_SHIFT) != 0,
					 	(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);
					newhk.set_key(character, keycode, (mod & KMOD_SHIFT) != 0,
							(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);

					if ( (newhk.get_id() == hotkey::HOTKEY_SCREENSHOT
							|| newhk.get_id() == hotkey::HOTKEY_MAP_SCREENSHOT)
							&& (mod & any_mod) == 0 ) {
						gui2::show_transient_message(disp.video(), _("Warning"),
								_("Screenshot hotkeys should be combined with the Control, Alt or Meta modifiers to avoid problems."));
					}
					break;
				}

				if ((oldhk && (!(oldhk->null())) )) {
					if (oldhk->get_command() != id) {

						utils::string_map symbols;
						symbols["hotkey_sequence"]   = oldhk->get_name();
						symbols["old_hotkey_action"] = oldhk->get_description();
						symbols["new_hotkey_action"] = newhk.get_description();

						std::string text = vgettext("\"$hotkey_sequence|\" is in use by \"$old_hotkey_action|\".\nDo you wish to reassign it to \"$new_hotkey_action|\"?"
								, symbols);
						text += "\n\n";

						const int res = gui2::show_message(disp.video(), _("Hotkey is already in use.")
								, text, gui2::tmessage::yes_no_buttons);
						if(res == gui2::twindow::OK) {
							oldhk->set_command(id);
							repopulate = true;
						}
					}
				} else {
					hotkey::add_hotkey(newhk);
					repopulate = true;
				}
			}
		}

		if (clear_button.pressed()) {
			// clear hotkey
			hotkey::clear_hotkeys(id);
			repopulate = true;
		}

		if (reset_button.pressed()) {
			const int res = gui2::show_message(
					disp.video(), _("Reset Hotkeys"),
					_("This will reset all hotkeys to their default values. Do you wish to continue?"), gui2::tmessage::yes_no_buttons);

			if(res != gui2::twindow::CANCEL) {
				clear_hotkeys();
				repopulate = true;
				gui2::show_transient_message(disp.video(), _("Hotkeys Reset"), _("All hotkeys have been reset to their default values."));
			}
		}

		if (repopulate) {
			repopulate_hotkeys_menu(menu_items, item_commands);
			menu_.set_items(menu_items,true,true);
			menu_.move_selection_keeping_viewport(selected);
		}

		menu_.process();

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		disp.update_display();
		disp.delay(10);
	}
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

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
		// SDL says that all modes are possible, so it's OK to use a
		// hardcoded list here. Include tiny and small gui since they
		// will be filtered out later if not needed.
		static const SDL_Rect scr_modes[] = {
			{ 0, 0,  320, 240  },
			{ 0, 0,  640, 480  },
			{ 0, 0,  800, 480  },	// small-gui (EeePC resolution)
			{ 0, 0,  800, 600  },
			{ 0, 0, 1024, 600  },	// used on many netbooks
			{ 0, 0, 1024, 768  },
			{ 0, 0, 1280, 960  },
			{ 0, 0, 1280, 1024 },
			{ 0, 0, 1366, 768  },	// 16:9 notebooks
			{ 0, 0, 1440, 900  },
			{ 0, 0, 1440, 1200 },
			{ 0, 0, 1600, 1200 },
			{ 0, 0, 1680, 1050 },
			{ 0, 0, 1920, 1080 },
			{ 0, 0, 1920, 1200 },
			{ 0, 0, 2560, 1600 }
		};
		static const SDL_Rect * const scr_modes_list[] = {
			&scr_modes[0],
			&scr_modes[1],
			&scr_modes[2],
			&scr_modes[3],
			&scr_modes[4],
			&scr_modes[5],
			&scr_modes[6],
			&scr_modes[7],
			&scr_modes[8],
			&scr_modes[9],
			&scr_modes[10],
			&scr_modes[11],
			&scr_modes[12],
			&scr_modes[13],
			&scr_modes[14],
			&scr_modes[15],
			NULL
		};

		modes = scr_modes_list;
	} else if(modes == NULL) {
		std::cerr << "No modes supported\n";
		gui2::show_transient_message(disp.video(),"",_("There are no alternative video modes available"));
		return false;
	}

	std::vector<std::pair<int,int> > resolutions;

	for(int i = 0; modes[i] != NULL; ++i) {
		if(modes[i]->w >= min_allowed_width() && modes[i]->h >= min_allowed_height()) {
			resolutions.push_back(std::pair<int,int>(modes[i]->w,modes[i]->h));
		}
	}

	const std::pair<int,int> current_res(video.getSurface()->w,video.getSurface()->h);
	resolutions.push_back(current_res);

	std::sort(resolutions.begin(),resolutions.end(),compare_resolutions);
	resolutions.erase(std::unique(resolutions.begin(),resolutions.end()),resolutions.end());

	std::vector<std::string> options;
	unsigned current_choice = 0;

	for(size_t k = 0; k < resolutions.size(); ++k) {
		std::pair<int, int> const& res = resolutions[k];
		std::ostringstream option;

		if (res == current_res)
			current_choice = static_cast<unsigned>(k);

		option << res.first << utils::unicode_multiplication_sign << res.second;
		/*widescreen threshold is 16:10*/
		if (static_cast<double>(res.first)/res.second >= 16.0/10.0)
		  option << _(" (widescreen)");
		options.push_back(option.str());
	}

	gui2::tsimple_item_selector dlg(_("Choose Resolution"), "", options);
	dlg.set_selected_index(current_choice);
	dlg.show(disp.video());

	int choice = dlg.selected_index();

	if(choice == -1 || resolutions[static_cast<size_t>(choice)] == current_res) {
		return false;
	}

	set_resolution(resolutions[static_cast<size_t>(choice)]);
	return true;
}

} // end namespace preferences

