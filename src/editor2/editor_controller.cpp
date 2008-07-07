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
#include "../foreach.hpp"
#include "../preferences.hpp"


#include "SDL.h"

namespace editor2 {

const int editor_controller::max_action_stack_size_ = 100;

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
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
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



void editor_controller::perform_action(const editor_action& action)
{
	editor_action* undo = action.perform(map_);
	undo_stack_.push_back(undo);
	trim_stack(undo_stack_);
	clear_stack(redo_stack_);
}

void editor_controller::trim_stack(action_stack& stack)
{
	if (stack.size() > max_action_stack_size_) {
		delete stack.front();
		stack.pop_front();
	}
}

void editor_controller::clear_stack(action_stack& stack)
{
	foreach (editor_action* a, stack) {
		delete a;
	}
	stack.clear();
}

bool editor_controller::can_undo() const
{
	return !undo_stack_.empty();
}

bool editor_controller::can_redo() const
{
	return !redo_stack_.empty();
}

void editor_controller::undo()
{
	perform_action_between_stacks(undo_stack_, redo_stack_);
}

void editor_controller::redo()
{
	perform_action_between_stacks(redo_stack_, undo_stack_);
}

void editor_controller::perform_action_between_stacks(action_stack& from, action_stack& to)
{
	assert(!from.empty());
	editor_action* action = from.back();
	from.pop_back();
	editor_action* reverse_action = action->perform(map_);
	to.push_back(reverse_action);
	trim_stack(to);
}



} //end namespace editor2
