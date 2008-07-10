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
#include "mouse_action.hpp"
#include "../config_adapter.hpp"
#include "../foreach.hpp"
#include "../hotkeys.hpp"
#include "../preferences.hpp"


#include "SDL.h"

namespace editor2 {

const int editor_controller::max_action_stack_size_ = 100;

editor_controller::editor_controller(const config &game_config, CVideo& video)
: controller_base(SDL_GetTicks(), game_config, video)
, mouse_handler_base(map_)
, map_(editor_map::new_map(game_config, 44, 33, t_translation::GRASS_LAND))
, gui_(NULL)
{
	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
	init(video);
	set_mouse_action(new mouse_action_paint(*this));
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

events::mouse_handler_base& editor_controller::get_mouse_handler_base()
{
	return *this;
}

editor_display& editor_controller::get_display()
{
	return *gui_;
}



void editor_controller::perform_action(const editor_action& action)
{
	SCOPE_ED;
	editor_action* undo = action.perform(map_);
	LOG_ED << "Performing action " << action.get_id() << ", actions count is " << action.get_instance_count() << "\n";
	undo_stack_.push_back(undo);
	trim_stack(undo_stack_);
	clear_stack(redo_stack_);
	//TODO rebuild and ivalidate only what's really needed
	gui().rebuild_all();
	gui().invalidate_all();
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

void editor_controller::mouse_motion(int x, int y, const bool browse, bool update)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	if (dragging_) {
		if (get_mouse_action() != NULL) {
			editor_action* a = get_mouse_action()->drag(*gui_, x, y);
			if (a != NULL) {
				perform_action(*a);
				delete a;
			}
		} else {
			LOG_ED << __FUNCTION__ << ": There is no mouse action active!\n";
		}		
	}
	const gamemap::location new_hex = gui().hex_clicked_on(x,y);
	gui().highlight_hex(new_hex);
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	if (mouse_handler_base::left_click(x, y, browse)) return true;
	LOG_ED << "Left click, after generic handling\n";
	if (get_mouse_action() != NULL) {
		editor_action* a = get_mouse_action()->click(*gui_, x, y);
		if (a != NULL) {
			perform_action(*a);
			delete a;
		}
		return true;
	} else {
		LOG_ED << __FUNCTION__ << ": There is no mouse action active!\n";
		return false;
	}
}

void editor_controller::left_drag_end(int x, int y, const bool browse)
{
	if (get_mouse_action() != NULL) {
		editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
		if (a != NULL) {
			perform_action(*a);
			delete a;
		}
	} else {
		LOG_ED << __FUNCTION__ << ": There is no mouse action active!\n";
	}	
}



} //end namespace editor2
