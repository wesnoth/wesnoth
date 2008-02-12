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

#include "SDL.h"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "../config.hpp"
#include "../construct_dialog.hpp"
#include "../display.hpp"
#include "../events.hpp"
#include "../filesystem.hpp"
#include "../game_config.hpp"
#include "../gettext.hpp"
#include "../language.hpp"
#include "../map.hpp"
#include "../mapgen.hpp"
#include "../map_create.hpp"
#include "../marked-up_text.hpp"
#include "../util.hpp"
#include "../preferences_display.hpp"
#include "../video.hpp"
#include "../widgets/slider.hpp"

#include "editor_dialogs.hpp"

namespace {
	const int map_min_height = 1;
	const int map_min_width = 1;
	const int map_max_height = 200;
	const int map_max_width = 200;
}

namespace map_editor {

bool confirm_modification_disposal(display& disp) {
	const int res = gui::dialog(disp, "",
					 _("Your modifications to the map will be lost. Continue?"),
					 gui::OK_CANCEL).show();
	return res == 0;
}


std::string new_map_dialog(display& disp, const t_translation::t_terrain fill_terrain,
	const bool confirmation_needed, const config &game_config)
{
	const resize_lock prevent_resizing;
	const events::event_context dialog_events_context;
	const gui::dialog_manager dialog_mgr;

	int map_width(40), map_height(40);
	const int width = 600;
	const int height = 400;
	const int xpos = disp.w()/2 - width/2;
	const int ypos = disp.h()/2 - height/2;
	const int horz_margin = 5;
	const int vertical_margin = 20;

	SDL_Rect dialog_rect = {xpos-10,ypos-10,width+20,height+20};
	surface_restorer restorer(&disp.video(),dialog_rect);

	gui::dialog_frame frame(disp.video());
	frame.layout(xpos,ypos,width,height);
	frame.draw_background();
	frame.draw_border();

	SDL_Rect title_rect = font::draw_text(NULL,screen_area(),24,font::NORMAL_COLOUR,
					      _("Create New Map"),0,0);

	const std::string& width_label = _("Width:");
	const std::string& height_label = _("Height:");

	SDL_Rect width_rect = font::draw_text(NULL, screen_area(), 14, font::NORMAL_COLOUR,
										  width_label, 0, 0);
	SDL_Rect height_rect = font::draw_text(NULL, screen_area(), 14, font::NORMAL_COLOUR,
										   height_label, 0, 0);

	const int text_right = xpos + horz_margin +
	        maximum<int>(width_rect.w,height_rect.w);

	width_rect.x = text_right - width_rect.w;
	height_rect.x = text_right - height_rect.w;

	width_rect.y = ypos + title_rect.h + vertical_margin*2;
	height_rect.y = width_rect.y + width_rect.h + vertical_margin;

	gui::button new_map_button(disp.video(), _("Generate New Map"));
	gui::button random_map_button(disp.video(), _("Generate Random Map"));
	gui::button random_map_setting_button(disp.video(), _("Random Generator Settings"));
	gui::button cancel_button(disp.video(), _("Cancel"));

	new_map_button.set_location(xpos + horz_margin,height_rect.y + height_rect.h + vertical_margin);
	random_map_button.set_location(xpos + horz_margin,ypos + height - random_map_button.height()-14*2-vertical_margin);
	random_map_setting_button.set_location(random_map_button.location().x + random_map_button.width() + horz_margin,
	                                       ypos + height - random_map_setting_button.height()
									       - 14*2 - vertical_margin);
	cancel_button.set_location(xpos + width - cancel_button.width() - horz_margin,
	                           ypos + height - cancel_button.height()-14);

	const int right_space = 100;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - horz_margin - right_space;
	SDL_Rect slider_rect = { slider_left,width_rect.y,slider_right-slider_left,width_rect.h};

	slider_rect.y = width_rect.y;
	gui::slider width_slider(disp.video());
	width_slider.set_location(slider_rect);
	width_slider.set_min(map_min_width);
	width_slider.set_max(map_max_width);
	width_slider.set_value(map_width);

	slider_rect.y = height_rect.y;
	gui::slider height_slider(disp.video());
	height_slider.set_location(slider_rect);
	height_slider.set_min(map_min_height);
	height_slider.set_max(map_max_height);
	height_slider.set_value(map_height);

	static util::scoped_ptr<map_generator> random_map_generator(NULL);
	if (random_map_generator == NULL) {
		// Initialize the map generator if this is the first call,
		// otherwise keep the settings and such.
		const config* const toplevel_cfg = game_config.find_child("multiplayer","id","multiplayer_Random_Map");
		const config* const cfg = toplevel_cfg == NULL ? NULL : toplevel_cfg->child("generator");
		if (cfg == NULL) {
			config dummy_cfg;
			random_map_generator.assign(create_map_generator("", &dummy_cfg));
		}
		else {
			random_map_generator.assign(create_map_generator("", cfg));
		}
	}

	for(bool draw = true;; draw = false) {
		if(cancel_button.pressed()) {
			return "";
		}

		if(new_map_button.pressed()) {
			draw = true;
			if ((confirmation_needed &&
				 confirm_modification_disposal(disp))
				|| !confirmation_needed) {

				return map_editor::new_map(width_slider.value() + 2 * gamemap::default_border, 
					height_slider.value() + 2 * gamemap::default_border, fill_terrain);
			}
		}
		if(random_map_setting_button.pressed()) {
			draw = true;
			if (random_map_generator.get()->allow_user_config()) {
				random_map_generator.get()->user_config(disp);
			}
		}

		if(random_map_button.pressed()) {
			draw = true;
			if ((confirmation_needed
				 && confirm_modification_disposal(disp))
				|| !confirmation_needed) {

				const std::string map =
					random_map_generator.get()->create_map(std::vector<std::string>());
				if (map == "") {
					gui::message_dialog(disp, "",
									 _("Map creation failed.")).show();
				}
				return map;
			}
		}
		if (width_slider.value() != map_width
			|| height_slider.value() != map_height) {
			draw = true;
		}
		if (draw) {
			map_width = width_slider.value();
			map_height = height_slider.value();
			frame.draw_background();
			frame.draw_border();
			title_rect = font::draw_text(&disp.video(),screen_area(),24,font::NORMAL_COLOUR,
										 _("Create New Map"),
										 xpos+(width-title_rect.w)/2,ypos+10);

			font::draw_text(&disp.video(),screen_area(),14,font::NORMAL_COLOUR,
							width_label,width_rect.x,width_rect.y);
			font::draw_text(&disp.video(),screen_area(),14,font::NORMAL_COLOUR,
							height_label,height_rect.x,height_rect.y);

			std::stringstream width_str;
			width_str << map_width;
			font::draw_text(&disp.video(),screen_area(),14,font::NORMAL_COLOUR,width_str.str(),
							slider_right+horz_margin,width_rect.y);

			std::stringstream height_str;
			height_str << map_height;
			font::draw_text(&disp.video(),screen_area(),14,font::NORMAL_COLOUR,height_str.str(),
							slider_right+horz_margin,height_rect.y);

		}

		new_map_button.set_dirty();
		random_map_button.set_dirty();
		random_map_setting_button.set_dirty();
		cancel_button.set_dirty();

		width_slider.set_dirty();
		height_slider.set_dirty();

		events::raise_process_event();
		events::raise_draw_event();

		if (draw) {
			update_rect(xpos,ypos,width,height);
		}
		disp.update_display();
		SDL_Delay(20);
		events::pump();
	}
}


void preferences_dialog(display &disp, config &prefs) {
	const events::event_context dialog_events_context;
	const gui::dialog_manager dialog_mgr;

	const int width = 600;
	const int height = 200;
	const int xpos = disp.w()/2 - width/2;
	const int ypos = disp.h()/2 - height/2;

	SDL_Rect clip_rect = disp.screen_area();

	gui::button close_button(disp.video(),_("Close Window"));

	std::vector<gui::button*> buttons;
	buttons.push_back(&close_button);

	gui::dialog_frame frame(disp.video(),_("Preferences"),gui::dialog_frame::default_style,true,&buttons);
	frame.layout(xpos,ypos,width,height);
	frame.draw();

	const std::string& scroll_label = _("Scroll Speed:");

	SDL_Rect scroll_rect = {0,0,0,0};
	scroll_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                              scroll_label,0,0);

	const int text_right = xpos + scroll_rect.w + 5;

	const int scroll_pos = ypos + 20;

	scroll_rect.x = text_right - scroll_rect.w;
	scroll_rect.y = scroll_pos;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - 5;
	if(slider_left >= slider_right)
		return;

	SDL_Rect slider_rect = { slider_left, scroll_pos, slider_right - slider_left, 10  };

	slider_rect.y = scroll_pos;
	gui::slider scroll_slider(disp.video());
	scroll_slider.set_location(slider_rect);
	scroll_slider.set_min(1);
	scroll_slider.set_max(100);
	scroll_slider.set_value(preferences::scroll_speed());

	gui::button fullscreen_button(disp.video(),_("Toggle Full Screen"),
	                              gui::button::TYPE_CHECK);

	fullscreen_button.set_check(preferences::fullscreen());

	fullscreen_button.set_location(slider_left,scroll_pos + 80);

	gui::button grid_button(disp.video(),_("Show Grid"),
	                        gui::button::TYPE_CHECK);
	grid_button.set_check(preferences::grid());

	grid_button.set_location(slider_left + fullscreen_button.width() + 100,
							 scroll_pos + 80);

	gui::button resolution_button(disp.video(),_("Video Mode"));
	resolution_button.set_location(slider_left,scroll_pos + 80 + 50);

	gui::button hotkeys_button (disp.video(),_("Hotkeys"));
	hotkeys_button.set_location(slider_left + fullscreen_button.width() + 100,
								scroll_pos + 80 + 50);

	bool redraw_all = true;

	for(;;) {
		if(close_button.pressed()) {
			break;
		}

		if(fullscreen_button.pressed()) {
			preferences::set_fullscreen(fullscreen_button.checked());
			redraw_all = true;
		}

		if(redraw_all) {
			frame.draw();
			fullscreen_button.set_dirty();
			close_button.set_dirty();
			resolution_button.set_dirty();
			grid_button.set_dirty();
			hotkeys_button.set_dirty();
			scroll_slider.set_dirty();

			font::draw_text(&disp.video(),clip_rect,14,font::NORMAL_COLOUR,scroll_label,
		                scroll_rect.x,scroll_rect.y);

			update_rect(screen_area());

			redraw_all = false;
		}


		if(grid_button.pressed()) {
			preferences::set_grid(grid_button.checked());
		}

		if(resolution_button.pressed()) {
			preferences::show_video_mode_dialog(disp);
			break;
		}

		if(hotkeys_button.pressed()) {
			preferences::show_hotkeys_dialog(disp, &prefs);
			break;
		}

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		preferences::set_scroll_speed(scroll_slider.value());

		disp.update_display();

		SDL_Delay(20);
	}
}


bool resize_dialog(display &disp, unsigned& width, unsigned& height,
	int& x_offset, int& y_offset, bool& do_expand)
{
	const resize_lock prevent_resizing;
	const events::event_context dialog_events_context;
	const gui::dialog_manager dialog_mgr;

	const int dlg_width = 600;
	const int dlg_height = 350;
	const int xpos = disp.w() / 2 - dlg_width / 2;
	const int ypos = disp.h() / 2 - dlg_height / 2;
	const int horz_margin = 5;
	const int vertical_margin = 20;
	const int button_padding = 20;

	SDL_Rect dialog_rect = {xpos - 10,
		ypos - 10, dlg_width + 20, dlg_height + 20};
	surface_restorer restorer(&disp.video(), dialog_rect);

	gui::dialog_frame frame(disp.video());
	frame.layout(xpos,ypos,dlg_width, dlg_height);
	frame.draw_background();
	frame.draw_border();

	SDL_Rect title_rect = font::draw_text(NULL, screen_area(), 24,
		font::NORMAL_COLOUR, _("Resize Map"), 0, 0);

	const std::string& width_label = _("Width:");
	const std::string& height_label = _("Height:");
	const std::string& x_offset_label = _("X offset:");
	const std::string& y_offset_label = _("Y offset:");

	SDL_Rect width_rect = font::draw_text(NULL, screen_area(), 14,
		font::NORMAL_COLOUR, width_label, 0, 0);

	SDL_Rect height_rect = font::draw_text(NULL, screen_area(), 14,
		font::NORMAL_COLOUR, height_label, 0, 0);

	SDL_Rect x_offset_rect = font::draw_text(NULL, screen_area(), 14,
		font::NORMAL_COLOUR, x_offset_label, 0, 0);

	SDL_Rect y_offset_rect = font::draw_text(NULL, screen_area(), 14,
		font::NORMAL_COLOUR, y_offset_label, 0, 0);

	// store the width of all labels in an array to determine the maximum
	const int label_arr_size = 4;
	int label_arr[label_arr_size] =
		{ width_rect.w, height_rect.w, x_offset_rect.w, y_offset_rect.w };

	// use the biggest label to deterimine the right side for the labels
	const int text_right = xpos + horz_margin +
	        *std::max_element(label_arr, label_arr + label_arr_size);

	width_rect.x = text_right - width_rect.w;
	height_rect.x = text_right - height_rect.w;
	x_offset_rect.x = text_right - x_offset_rect.w;
	y_offset_rect.x = text_right - y_offset_rect.w;

	width_rect.y = ypos + title_rect.h + vertical_margin * 2;
	height_rect.y = width_rect.y + width_rect.h + vertical_margin;
	x_offset_rect.y = height_rect.y + height_rect.h + vertical_margin * 2;
	y_offset_rect.y = x_offset_rect.y + x_offset_rect.h + vertical_margin;

	gui::button cancel_button(disp.video(), _("Cancel"));
	gui::button ok_button(disp.video(), _("OK"));

	cancel_button.set_location(
		xpos + dlg_width - cancel_button.width() - horz_margin,
		ypos + dlg_height - cancel_button.height() - 14);

	ok_button.set_location(
		xpos + dlg_width - cancel_button.width() - horz_margin - ok_button.width() - button_padding,
		ypos + dlg_height - ok_button.height()-14);

	const int right_space = 100;
	const int slider_left = text_right + 10;
	const int slider_right = xpos + dlg_width - horz_margin - right_space;
	SDL_Rect slider_rect =
		{ slider_left, width_rect.y, slider_right-slider_left, width_rect.h};

	slider_rect.y = width_rect.y;
	gui::slider width_slider(disp.video());
	width_slider.set_location(slider_rect);
	width_slider.set_min(map_min_width);
	width_slider.set_max(map_max_width);
	width_slider.set_value(width);

	slider_rect.y = height_rect.y;
	gui::slider height_slider(disp.video());
	height_slider.set_location(slider_rect);
	height_slider.set_min(map_min_height);
	height_slider.set_max(map_max_height);
	height_slider.set_value(height);

	slider_rect.y = x_offset_rect.y;
	gui::slider x_offset_slider(disp.video());
	x_offset_slider.set_location(slider_rect);
	x_offset_slider.set_min(-map_max_height);
	x_offset_slider.set_max(map_max_height);
	x_offset_slider.set_value(x_offset);

	slider_rect.y = y_offset_rect.y;
	gui::slider y_offset_slider(disp.video());
	y_offset_slider.set_location(slider_rect);
	y_offset_slider.set_min(-map_max_height);
	y_offset_slider.set_max(map_max_height);
	y_offset_slider.set_value(y_offset);

	slider_rect.y += y_offset_rect.h + vertical_margin * 2;
	gui::button do_expand_button(disp.video(), _("Smart expand"), gui::button::TYPE_CHECK);
	// assume the width will be correct for this widget
	do_expand_button.set_location(slider_rect);
	do_expand_button.set_check(do_expand);

	for(bool draw = true;; draw = false) {
		if(cancel_button.pressed()) {
			return false;
		}
		if (static_cast<unsigned>(width_slider.value()) != width
				|| static_cast<unsigned>(height_slider.value()) != height
				|| x_offset_slider.value() != x_offset
				|| y_offset_slider.value() != y_offset
				|| do_expand_button.checked() != do_expand) {

			draw = true;
		}
		if (draw) {
			width = width_slider.value();
			height = height_slider.value();
			x_offset = x_offset_slider.value();
			y_offset = y_offset_slider.value();
			do_expand = do_expand_button.checked();

			frame.draw_background();
			frame.draw_border();

			title_rect = font::draw_text(&disp.video(), screen_area(), 24,
				font::NORMAL_COLOUR, _("Resize Map"),
				xpos + (dlg_width - title_rect.w) / 2, ypos + 10);

			font::draw_text(&disp.video(), screen_area(), 14, font::NORMAL_COLOUR,
				width_label, width_rect.x, width_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14, font::NORMAL_COLOUR,
				height_label, height_rect.x, height_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14, font::NORMAL_COLOUR,
				x_offset_label, x_offset_rect.x, x_offset_rect.y);

			font::draw_text(&disp.video(),screen_area(),14,font::NORMAL_COLOUR,
				y_offset_label, y_offset_rect.x, y_offset_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14,
				font::NORMAL_COLOUR, lexical_cast<std::string>(width),
				slider_right + horz_margin, width_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14,
				font::NORMAL_COLOUR, lexical_cast<std::string>(height),
				slider_right + horz_margin, height_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14,
				font::NORMAL_COLOUR, lexical_cast<std::string>(x_offset),
				slider_right + horz_margin, x_offset_rect.y);

			font::draw_text(&disp.video(), screen_area(), 14,
				font::NORMAL_COLOUR, lexical_cast<std::string>(y_offset),
				slider_right + horz_margin, y_offset_rect.y);

		}
		if (ok_button.pressed()) {
			return true;
		}
		// make sure the all elements are redrawn
		cancel_button.set_dirty();
		ok_button.set_dirty();

		width_slider.set_dirty();
		height_slider.set_dirty();
		x_offset_slider.set_dirty();
		y_offset_slider.set_dirty();
		do_expand_button.set_dirty();

		events::raise_process_event();
		events::raise_draw_event();

		if (draw) {
			update_rect(xpos, ypos, dlg_width, dlg_height);
		}

		disp.update_display();
		SDL_Delay(20);
		events::pump();
	}

}

FLIP_AXIS flip_dialog(display &disp) {
	std::vector<std::string> items;
	items.push_back(_("X-Axis"));
	items.push_back(_("Y-Axis"));
	const std::string msg = _("Flip around (this may change the dimensions of the map):");
	 gui::dialog flipmenu = gui::dialog(disp, "",
						 font::word_wrap_text(msg, 12, 180),
						 gui::OK_CANCEL);
	flipmenu.set_menu(items);
	switch (flipmenu.show()) {
	case 0:
		return FLIP_X;
	case 1:
		return FLIP_Y;
	default:
		return NO_FLIP;
	}
}

}



