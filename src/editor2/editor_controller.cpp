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
: controller_base(SDL_GetTicks(), game_config, video)
, map_(editor_map::new_map(game_config, 44, 33, t_translation::GRASS_LAND))
, gui_(NULL)
, mouse_handler_(gui_, map_)
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
	mouse_handler_.set_gui(gui_);
}

editor_controller::~editor_controller()
{
    delete gui_;
}

void editor_controller::main_loop()
{
	for(;;) {
		play_slice();
	}
}

bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND, int /*index*/) const
{
	return false;
}

editor_mouse_handler& editor_controller::get_mouse_handler_base()
{
	return mouse_handler_;
}

editor_display& editor_controller::get_display()
{
	return *gui_;
}

} //end namespace editor2
