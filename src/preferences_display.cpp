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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "cursor.hpp"
#include "display.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "marked-up_text.hpp"
#include "preferences_display.hpp"
#include "show_dialog.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "widgets/button.hpp"
#include "widgets/label.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
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
				gui::show_dialog(*disp,NULL,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."),
				                 gui::MESSAGE);
			}
		}
	}
}

void set_resolution(const std::pair<int,int>& resolution)
{
	std::pair<int,int> res = resolution;

	// - Ayin: disabled the following code. Why would one want to enforce that?
	// Some 16:9, or laptop screens, may have resolutions which do not
	// comply to this rule (see bug 10630). I'm commenting this until it
	// proves absolutely necessary.
	//
	//make sure resolutions are always divisible by 4
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
			gui::show_dialog(*disp,NULL,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."),gui::MESSAGE);
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

void set_adjust_gamma(bool val)
{
	//if we are turning gamma adjustment off, then set it to '1.0'
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
		video.setGamma((float)gamma / 100);
	}
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void set_lobby_joins(bool ison)
{
	_set_lobby_joins(ison);
}

void set_colour_cursors(bool value)
{
	_set_colour_cursors(value);

	cursor::use_colour(value);
}

namespace {

class preferences_dialog : public gui::preview_pane
{
public:
	preferences_dialog(display& disp, const config& game_cfg);

	struct video_mode_change_exception
	{
		enum TYPE { CHANGE_RESOLUTION, MAKE_FULLSCREEN, MAKE_WINDOWED };

		video_mode_change_exception(TYPE type) : type(type)
		{}

		TYPE type;
	};

private:

	void process_event();
	bool left_side() const { return false; }
	void set_selection(int index);
	void update_location(SDL_Rect const &rect);
	const config* get_advanced_pref() const;
	void set_advanced_menu();

	gui::slider music_slider_, sound_slider_, scroll_slider_, gamma_slider_;
	gui::button fullscreen_button_, turbo_button_, show_ai_moves_button_,
	            show_grid_button_, show_lobby_joins_button_, show_floating_labels_button_, turn_dialog_button_,
	            turn_bell_button_, show_team_colours_button_, show_colour_cursors_button_,
	            show_haloing_button_, video_mode_button_, theme_button_, hotkeys_button_, gamma_button_,
				flip_time_button_, advanced_button_, sound_button_, music_button_;
	gui::label music_label_, sound_label_, scroll_label_, gamma_label_;
	unsigned slider_label_width_;

	gui::menu advanced_;
	int advanced_selection_;

	enum TAB { GENERAL_TAB, DISPLAY_TAB, SOUND_TAB, ADVANCED_TAB };
	TAB tab_;
	display &disp_;
	const config& game_cfg_;
};

preferences_dialog::preferences_dialog(display& disp, const config& game_cfg)
	: gui::preview_pane(disp.video()),
	  music_slider_(disp.video()), sound_slider_(disp.video()),
	  scroll_slider_(disp.video()), gamma_slider_(disp.video()),
	  fullscreen_button_(disp.video(), _("Toggle Full Screen"), gui::button::TYPE_CHECK),
	  turbo_button_(disp.video(), _("Accelerated Speed"), gui::button::TYPE_CHECK),
	  show_ai_moves_button_(disp.video(), _("Skip AI Moves"), gui::button::TYPE_CHECK),
	  show_grid_button_(disp.video(), _("Show Grid"), gui::button::TYPE_CHECK),
	  show_lobby_joins_button_(disp.video(), _("Show lobby joins"), gui::button::TYPE_CHECK),
	  show_floating_labels_button_(disp.video(), _("Show Floating Labels"), gui::button::TYPE_CHECK),
	  turn_dialog_button_(disp.video(), _("Turn Dialog"), gui::button::TYPE_CHECK),
	  turn_bell_button_(disp.video(), _("Turn Bell"), gui::button::TYPE_CHECK),
	  show_team_colours_button_(disp.video(), _("Show Team Colors"), gui::button::TYPE_CHECK),
	  show_colour_cursors_button_(disp.video(), _("Show Color Cursors"), gui::button::TYPE_CHECK),
	  show_haloing_button_(disp.video(), _("Show Haloing Effects"), gui::button::TYPE_CHECK),
	  video_mode_button_(disp.video(), _("Video Mode")),
	  theme_button_(disp.video(), _("Theme")),
	  hotkeys_button_(disp.video(), _("Hotkeys")),
	  gamma_button_(disp.video(), _("Adjust Gamma"), gui::button::TYPE_CHECK),
	  flip_time_button_(disp.video(), _("Reverse Time Graphics"), gui::button::TYPE_CHECK),
	  sound_button_(disp.video(), _("Sound effects"), gui::button::TYPE_CHECK),
	  music_button_(disp.video(), _("Music"), gui::button::TYPE_CHECK),
	  advanced_button_(disp.video(), "", gui::button::TYPE_CHECK),
	  music_label_(disp.video(), _("Music Volume:")), sound_label_(disp.video(), _("SFX Volume:")),
	  scroll_label_(disp.video(), _("Scroll Speed:")), gamma_label_(disp.video(), _("Gamma:")),
	  slider_label_width_(0), advanced_(disp.video(),std::vector<std::string>()), advanced_selection_(-1),
	  tab_(GENERAL_TAB), disp_(disp), game_cfg_(game_cfg)
{
	// FIXME: this box should be vertically centered on the screen, but is not
#if USE_TINY_GUI
	set_measurements(260, 220);		  // FIXME: should compute this, but using what data ?
#else
	set_measurements(400, 430);
#endif


	sound_button_.set_check(sound_on());
	sound_button_.set_help_string(_("Sound effects on/off"));
	sound_slider_.set_min(0);
	sound_slider_.set_max(128);
	sound_slider_.set_value(sound_volume());
	sound_slider_.set_help_string(_("Change the sound effects volume"));

	music_button_.set_check(music_on());
	music_button_.set_help_string(_("Music on/off"));
	music_slider_.set_min(0);
	music_slider_.set_max(128);
	music_slider_.set_value(music_volume());
	music_slider_.set_help_string(_("Change the music volume"));


	scroll_slider_.set_min(1);
	scroll_slider_.set_max(100);
	scroll_slider_.set_value(scroll_speed());
	scroll_slider_.set_help_string(_("Change the speed of scrolling around the map"));

	gamma_button_.set_check(adjust_gamma());
	gamma_button_.set_help_string(_("Change the brightness of the display"));

	gamma_slider_.set_min(50);
	gamma_slider_.set_max(200);
	gamma_slider_.set_value(gamma());
	gamma_slider_.set_help_string(_("Change the brightness of the display"));

	fullscreen_button_.set_check(fullscreen());
	fullscreen_button_.set_help_string(_("Choose whether the game should run full screen or in a window"));

	turbo_button_.set_check(turbo());
	turbo_button_.set_help_string(_("Make units move and fight faster"));

	show_ai_moves_button_.set_check(!show_ai_moves());
	show_ai_moves_button_.set_help_string(_("Do not animate AI units moving"));

	show_grid_button_.set_check(grid());
	show_grid_button_.set_help_string(_("Overlay a grid onto the map"));

	show_lobby_joins_button_.set_check(lobby_joins());
	show_lobby_joins_button_.set_help_string(_("Show messages about players joining the multiplayer lobby"));

	show_floating_labels_button_.set_check(show_floating_labels());
	show_floating_labels_button_.set_help_string(_("Show text above a unit when it is hit to display damage inflicted"));

	video_mode_button_.set_help_string(_("Change the resolution the game runs at"));
	theme_button_.set_help_string(_("Change the theme the game runs with"));

	turn_dialog_button_.set_check(turn_dialog());
	turn_dialog_button_.set_help_string(_("Display a dialog at the beginning of your turn"));

	turn_bell_button_.set_check(turn_bell());
	turn_bell_button_.set_help_string(_("Play a bell sound at the beginning of your turn"));

	show_team_colours_button_.set_check(show_side_colours());
	show_team_colours_button_.set_help_string(_("Show a colored circle around the base of each unit to show which side it is on"));

	flip_time_button_.set_check(flip_time());
	flip_time_button_.set_help_string(_("Choose whether the sun moves left-to-right or right-to-left"));

	show_colour_cursors_button_.set_check(use_colour_cursors());
	show_colour_cursors_button_.set_help_string(_("Use colored mouse cursors (may be slower)"));

	show_haloing_button_.set_check(show_haloes());
	show_haloing_button_.set_help_string(_("Use graphical special effects (may be slower)"));

	hotkeys_button_.set_help_string(_("View and configure keyboard shortcuts"));

	set_advanced_menu();
}

void preferences_dialog::update_location(SDL_Rect const &rect)
{
	bg_register(rect);

	const int border = font::relative_size(10);
#if USE_TINY_GUI
	const int item_interline = 20;
#else
	const int item_interline = 50;
#endif

	// General tab
	int ypos = rect.y;
	scroll_label_.set_location(rect.x, ypos);
	SDL_Rect scroll_rect = { rect.x + scroll_label_.width(), ypos,
	                         rect.w - scroll_label_.width() - border, 0 };
	scroll_slider_.set_location(scroll_rect);
	ypos += item_interline; turbo_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_ai_moves_button_.set_location(rect.x, ypos);
	ypos += item_interline; turn_dialog_button_.set_location(rect.x, ypos);
	ypos += item_interline; turn_bell_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_team_colours_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_grid_button_.set_location(rect.x, ypos);
	ypos += item_interline;	show_lobby_joins_button_.set_location(rect.x, ypos);
	ypos += item_interline; hotkeys_button_.set_location(rect.x, ypos);

	// Display tab
	ypos = rect.y;
	gamma_button_.set_location(rect.x, ypos);
	ypos += item_interline;
	gamma_label_.set_location(rect.x, ypos);
	SDL_Rect gamma_rect = { rect.x + gamma_label_.width(), ypos,
	                        rect.w - gamma_label_.width() - border, 0 };
	gamma_slider_.set_location(gamma_rect);
	ypos += item_interline; flip_time_button_.set_location(rect.x,ypos);
	ypos += item_interline; show_floating_labels_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_colour_cursors_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_haloing_button_.set_location(rect.x, ypos);
	ypos += item_interline; fullscreen_button_.set_location(rect.x, ypos);
	ypos += item_interline; video_mode_button_.set_location(rect.x, ypos);
	theme_button_.set_location(rect.x+video_mode_button_.width()+10, ypos);

	// Sound tab
	slider_label_width_ = maximum<unsigned>(music_label_.width(), sound_label_.width());
	ypos = rect.y;
	sound_button_.set_location(rect.x, ypos);

	ypos += item_interline;
	sound_label_.set_location(rect.x, ypos);
	const SDL_Rect sound_rect = { rect.x + slider_label_width_, ypos,
	                        rect.w - slider_label_width_ - border, 0 };
	sound_slider_.set_location(sound_rect);

	ypos += item_interline;
	music_button_.set_location(rect.x, ypos);

	ypos += item_interline;
	music_label_.set_location(rect.x, ypos);
	const SDL_Rect music_rect = { rect.x + slider_label_width_, ypos,
	                        rect.w - slider_label_width_ - border, 0 };
	music_slider_.set_location(music_rect);

	//Advanced tab
	ypos = rect.y;
	advanced_.set_location(rect.x,ypos);
	advanced_.set_max_height(height()-100);

	ypos += advanced_.height() + border;

	advanced_button_.set_location(rect.x,ypos);

	set_selection(tab_);
}

void preferences_dialog::process_event()
{
	if (turbo_button_.pressed())
		set_turbo(turbo_button_.checked());
	if (show_ai_moves_button_.pressed())
		set_show_ai_moves(!show_ai_moves_button_.checked());
	if (show_grid_button_.pressed())
		set_grid(show_grid_button_.checked());
	if (show_lobby_joins_button_.pressed())
		set_lobby_joins(show_lobby_joins_button_.checked());
	if (show_floating_labels_button_.pressed())
		set_show_floating_labels(show_floating_labels_button_.checked());
	if (video_mode_button_.pressed())
		throw video_mode_change_exception(video_mode_change_exception::CHANGE_RESOLUTION);
	if (theme_button_.pressed())
	        show_theme_dialog(disp_);
	if (fullscreen_button_.pressed())
		throw video_mode_change_exception(fullscreen_button_.checked()
		                                  ? video_mode_change_exception::MAKE_FULLSCREEN
		                                  : video_mode_change_exception::MAKE_WINDOWED);
	if (turn_bell_button_.pressed())
		set_turn_bell(turn_bell_button_.checked());
	if (turn_dialog_button_.pressed())
		set_turn_dialog(turn_dialog_button_.checked());
	if (show_team_colours_button_.pressed())
		set_show_side_colours(show_team_colours_button_.checked());
	if (hotkeys_button_.pressed())
		show_hotkeys_dialog(disp_);
	if (show_colour_cursors_button_.pressed())
		set_colour_cursors(show_colour_cursors_button_.checked());
	if (show_haloing_button_.pressed())
		set_show_haloes(show_haloing_button_.checked());
	if (gamma_button_.pressed()) {
		set_adjust_gamma(gamma_button_.checked());
		bool hide_gamma = !adjust_gamma();
		gamma_slider_.hide(hide_gamma);
		gamma_label_.hide(hide_gamma);
	}

	if (sound_button_.pressed()) {
		if(!set_sound(sound_button_.checked()))
			sound_button_.set_check(false);
	}
	set_sound_volume(sound_slider_.value());

	if (music_button_.pressed()) {
		if(!set_music(music_button_.checked()))
			music_button_.set_check(false);
	}
	set_music_volume(music_slider_.value());

	if (flip_time_button_.pressed())
		set_flip_time(flip_time_button_.checked());

	set_scroll_speed(scroll_slider_.value());
	set_gamma(gamma_slider_.value());

	if(advanced_.selection() != advanced_selection_) {
		advanced_selection_ = advanced_.selection();
		const config* const adv = get_advanced_pref();
		if(adv != NULL) {
			const config& pref = *adv;
			advanced_button_.set_width(0);
			advanced_button_.set_label(pref["name"]);
			std::string value = preferences::get(pref["field"]);
			if(value.empty()) {
				value = pref["default"];
			}

			advanced_button_.set_check(value == "yes");
		}
	}

	if(advanced_button_.pressed()) {
		const config* const adv = get_advanced_pref();
		if(adv != NULL) {
			const config& pref = *adv;
			preferences::set(pref["field"],
					 advanced_button_.checked() ? "yes" : "no");
			set_advanced_menu();
		}
	}
}

const config* preferences_dialog::get_advanced_pref() const
{
	const config::child_list& adv = game_cfg_.get_children("advanced_preference");
	if(advanced_selection_ >= 0 && advanced_selection_ < int(adv.size())) {
		return adv[advanced_selection_];
	} else {
		return NULL;
	}
}

void preferences_dialog::set_advanced_menu()
{
	std::vector<std::string> advanced_items;
	const config::child_list& adv = game_cfg_.get_children("advanced_preference");
	for(config::child_list::const_iterator i = adv.begin(); i != adv.end(); ++i) {
		std::ostringstream str;
		std::string field = preferences::get((**i)["field"]);
		if(field.empty()) {
			field = (**i)["default"];
		}

		if(field == "yes") {
			field = _("yes");
		} else if(field == "no") {
			field = _("no");
		}

		str << (**i)["name"] << COLUMN_SEPARATOR << field;
		advanced_items.push_back(str.str());
	}

	advanced_.set_items(advanced_items,true,true);
}

void preferences_dialog::set_selection(int index)
{
	tab_ = TAB(index);
	set_dirty();
	bg_restore();

	const bool hide_general = tab_ != GENERAL_TAB;
	scroll_label_.hide(hide_general);
	scroll_slider_.hide(hide_general);
	turbo_button_.hide(hide_general);
	show_ai_moves_button_.hide(hide_general);
	turn_dialog_button_.hide(hide_general);
	turn_bell_button_.hide(hide_general);
	hotkeys_button_.hide(hide_general);
	show_team_colours_button_.hide(hide_general);
	show_grid_button_.hide(hide_general);
	show_lobby_joins_button_.hide(hide_general);

	const bool hide_display = tab_ != DISPLAY_TAB, hide_gamma = hide_display || !adjust_gamma();
	gamma_label_.hide(hide_gamma);
	gamma_slider_.hide(hide_gamma);
	gamma_button_.hide(hide_display);
	show_floating_labels_button_.hide(hide_display);
	show_colour_cursors_button_.hide(hide_display);
	show_haloing_button_.hide(hide_display);
	fullscreen_button_.hide(hide_display);
	video_mode_button_.hide(hide_display);
	theme_button_.hide(hide_display);
	flip_time_button_.hide(hide_display);

	const bool hide_sound = tab_ != SOUND_TAB;
	music_button_.hide(hide_sound);
	music_label_.hide(hide_sound);
	music_slider_.hide(hide_sound);
	sound_button_.hide(hide_sound);
	sound_label_.hide(hide_sound);
	sound_slider_.hide(hide_sound);

	const bool hide_advanced = tab_ != ADVANCED_TAB;
	advanced_.hide(hide_advanced);
	advanced_button_.hide(hide_advanced);
}

}

void show_preferences_dialog(display& disp, const config& game_cfg)
{
	std::vector<std::string> items;

	std::string const pre = IMAGE_PREFIX + std::string("icons/icon-");
	char const sep = COLUMN_SEPARATOR;
	items.push_back(pre + "general.png" + sep + sgettext("Prefs section^General"));
	items.push_back(pre + "display.png" + sep + sgettext("Prefs section^Display"));
	items.push_back(pre + "music.png" + sep + sgettext("Prefs section^Sound"));
	items.push_back(pre + "advanced.png" + sep + sgettext("Advanced section^Advanced"));

	for(;;) {
		try {
			const events::event_context dialog_events_context;

			preferences_dialog dialog(disp,game_cfg);
			std::vector<gui::preview_pane*> panes;
			panes.push_back(&dialog);

			gui::show_dialog(disp,NULL,_("Preferences"),"",gui::CLOSE_ONLY,&items,&panes);
			return;
		} catch(preferences_dialog::video_mode_change_exception& e) {
			switch(e.type) {
			case preferences_dialog::video_mode_change_exception::CHANGE_RESOLUTION:
				show_video_mode_dialog(disp);
				break;
			case preferences_dialog::video_mode_change_exception::MAKE_FULLSCREEN:
				set_fullscreen(true);
				break;
			case preferences_dialog::video_mode_change_exception::MAKE_WINDOWED:
				set_fullscreen(false);
				break;
			}

			if(items[1].empty() || items[1][0] != '*') {
				items[1] = "*" + items[1];
			}
		}
	}
}

bool show_video_mode_dialog(display& disp)
{
	const events::resize_lock prevent_resizing;
	const events::event_context dialog_events_context;

	CVideo& video = disp.video();

	SDL_PixelFormat format = *video.getSurface()->format;
	format.BitsPerPixel = video.getBpp();

	const SDL_Rect* const * const modes = SDL_ListModes(&format,FULL_SCREEN);

	//the SDL documentation says that a return value of -1 if no dimension
	//is available.
	if(modes == reinterpret_cast<SDL_Rect**>(-1) || modes == NULL) {
		if(modes != NULL)
			std::cerr << "Can support any video mode\n";
		else
			std::cerr << "No modes supported\n";
		gui::show_dialog(disp,NULL,"",_("There are no alternative video modes available"));
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

void show_hotkeys_dialog (display & disp, config *save_config)
{
	log_scope ("show_hotkeys_dialog");

	const events::event_context dialog_events_context;

	const int centerx = disp.x()/2;
	const int centery = disp.y()/2;
#ifdef USE_TINY_GUI
	const int width = 300;			  // FIXME: should compute this, but using what data ?
	const int height = 220;
#else
	const int width = 700;
	const int height = 500;
#endif
	const int xpos = centerx  - width/2;
	const int ypos = centery  - height/2;

	gui::button close_button (disp.video(), _("Close Window"));
	std::vector<gui::button*> buttons;
	buttons.push_back(&close_button);

	surface_restorer restorer;
	gui::draw_dialog(xpos,ypos,width,height,disp.video(),_("Hotkey Settings"),NULL,&buttons,&restorer);

	SDL_Rect clip_rect = { 0, 0, disp.x (), disp.y () };
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
		str << i->get_name();
		menu_items.push_back(str.str());
	}

	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Action") << COLUMN_SEPARATOR << _("Binding");
	menu_items.push_back(heading.str());

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_alpha_sort(1);

	gui::menu menu_(disp.video(), menu_items, false, height, -1, &sorter);
	menu_.sort_by(0);
	menu_.reset_selection();
	menu_.set_width(font::relative_size(400));
	menu_.set_location(xpos + font::relative_size(20), ypos);

	gui::button change_button (disp.video(), _("Change Hotkey"));
	change_button.set_location(xpos + width - change_button.width () - font::relative_size(30),ypos + font::relative_size(80));

	gui::button save_button (disp.video(), _("Save Hotkeys"));
	save_button.set_location(xpos + width - save_button.width () - font::relative_size(30),ypos + font::relative_size(130));

	for(;;) {

		if (close_button.pressed())
			break;

		if (change_button.pressed ()) {
			// Lets change this hotkey......
			SDL_Rect dlgr = {centerx-text_size.w/2 - 30,
								centery-text_size.h/2 - 16,
									text_size.w+60,
									text_size.h+32};
			surface_restorer restorer(&disp.video(),dlgr);
			gui::draw_dialog_frame (centerx-text_size.w/2 - 20,
									centery-text_size.h/2 - 6,
									text_size.w+40,
									text_size.h+12,disp.video());
			font::draw_text (&disp.video(), clip_rect, font::SIZE_LARGE,font::NORMAL_COLOUR,
				 _("Press desired Hotkey"),centerx-text_size.w/2-10,
				 centery-text_size.h/2-3);
			disp.update_display();
			SDL_Event event;
			event.type = 0;
			int key=0; //just to avoid warning
			int mod=0;
			while (event.type!=SDL_KEYDOWN) SDL_PollEvent(&event);
			do {
				if (event.type==SDL_KEYDOWN)
				{
					key=event.key.keysym.sym;
					mod=event.key.keysym.mod;
				};
				SDL_PollEvent(&event);
				disp.flip();
				SDL_Delay(10);
			} while (event.type!=SDL_KEYUP);
			restorer.restore();
			disp.update_display();

			const hotkey::hotkey_item& oldhk = hotkey::get_hotkey(key, (mod & KMOD_SHIFT) != 0,
					(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);
			hotkey::hotkey_item& newhk = hotkey::get_visible_hotkey(menu_.selection());

			if(oldhk.get_id() != newhk.get_id() && !oldhk.null()) {
				gui::show_dialog(disp,NULL,"",_("This Hotkey is already in use."),gui::MESSAGE);
			} else {
				newhk.set_key(key, (mod & KMOD_SHIFT) != 0,
						(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);

				menu_.change_item(menu_.selection(), 1, newhk.get_name());
			};
		}
		if (save_button.pressed()) {
			if (save_config == NULL) {
				save_hotkeys();
			} else {
				hotkey::save_hotkeys(*save_config);
			}
		}

		menu_.process();

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		disp.update_display();

		SDL_Delay(10);
	}
}

bool show_theme_dialog(display& disp)
{
  int action = 0;
  std::vector<std::string> options = disp.get_theme().get_known_themes();
  if(options.size()){
    std::string current_theme=_("Saved Theme Preference: ")+preferences::theme();
    action = gui::show_dialog(disp,NULL,"",current_theme,gui::OK_CANCEL,&options);
    if(action == 0){
      preferences::set_theme(options[action]);
      //it would be preferable for the new theme to take effect
      //immediately, however, this will have to do for now.
      gui::show_dialog(disp,NULL,"",_("New theme will take effect on next new or loaded game."),gui::MESSAGE);
      return(1);
    }
  }else{
      gui::show_dialog(disp,NULL,"",_("No known themes.  Try changing from within an existing game."),gui::MESSAGE);
  }
  return(0);
}

}




