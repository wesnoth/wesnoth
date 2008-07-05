/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_controller.hpp"
#include "editor_display.hpp"
#include "editor_map.hpp"
#include "../config_adapter.hpp"
#include "../preferences.hpp"


#include "SDL.h"

namespace editor2 {

editor_controller::editor_controller(const config &game_config, CVideo& video)
: game_config_(game_config), map_(editor_map::new_map(game_config, 44, 33, t_translation::GRASS_LAND))
, gui_(NULL)
{
	init(video);
	gui_->invalidate_game_status();
	gui_->invalidate_all();
	gui_->draw();
	events::raise_draw_event();
	//redraw_everything();
}

void editor_controller::init(CVideo& video)
{
	config dummy;
	const config* theme_cfg = get_theme(game_config_, "editor2");
	theme_cfg = theme_cfg ? theme_cfg : &dummy;
	gui_ = new editor_display(video, map_, *theme_cfg, game_config_, config());
}

editor_controller::~editor_controller()
{
    delete gui_;
}

void editor_controller::main_loop()
{
	for(;;) {
		int mousex, mousey;
		const int scroll_speed = preferences::scroll_speed();
		Uint8 mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool l_button_down = (0 != (mouse_flags & SDL_BUTTON_LMASK));
		const bool r_button_down = (0 != (mouse_flags & SDL_BUTTON_RMASK));
		const bool m_button_down = (0 != (mouse_flags & SDL_BUTTON_MMASK));

		const gamemap::location cur_hex = gui_->hex_clicked_on(mousex,mousey);
		const theme::menu* const m = gui_->menu_pressed();
		if (m != NULL) {
			const SDL_Rect& menu_loc = m->location(gui_->screen_area());
			const int x = menu_loc.x + 1;
			const int y = menu_loc.y + menu_loc.h + 1;
//			show_menu(m->items(), x, y, false);
		}

		if(key_[SDLK_UP] || mousey == 0) {
			gui_->scroll(0,-scroll_speed);
		}
		if(key_[SDLK_DOWN] || mousey == gui_->h()-1) {
			gui_->scroll(0,scroll_speed);
		}
		if(key_[SDLK_LEFT] || mousex == 0) {
			gui_->scroll(-scroll_speed,0);
		}
		if(key_[SDLK_RIGHT] || mousex == gui_->w()-1) {
			gui_->scroll(scroll_speed,0);
		}

		if (l_button_down) {
//			left_button_down(mousex, mousey);
		}
		else {
//			if (l_button_held_func_ == MOVE_SELECTION) {
				// When it is detected that the mouse is no longer down
				// and we are in the progress of moving a selection,
				// perform the movement.
//				perform_selection_move();
//			}
		}
		if (r_button_down) {
//			right_button_down(mousex, mousey);
		}
		if (m_button_down) {
//			middle_button_down(mousex, mousey);
		}

		gui_->draw(true, true);
		events::raise_draw_event();

		// When the map has changed, wait until the left mouse button
		// is not held down, and then update the minimap and
		// the starting position labels.
//		if (map_dirty_) {
//			if (!l_button_down && !r_button_down) {
//				if (auto_update_) {
//					gui_.rebuild_all();
//					gui_.invalidate_all();
//					map_dirty_ = false;
//				}
				//gui_.recalculate_minimap();
//				recalculate_starting_pos_labels();
//			}
//		}
		gui_->update_display();
		SDL_Delay(20);
		events::pump();
//		if (everything_dirty_) {
//			redraw_everything();
//			everything_dirty_ = false;
//		}
//		if (abort_ == ABORT_NORMALLY) {
//			if (!confirm_exit_and_save()) {
//				set_abort(DONT_ABORT);
//			}
//		}
//		mouse_moved_ = false;
	}
}

bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND, int) const
{
	return false;
}
	
} //end namespace editor2
