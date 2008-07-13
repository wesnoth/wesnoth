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
#include "../cursor.hpp"
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
	cursor::set(cursor::NORMAL);
	gui_->invalidate_game_status();
	gui_->invalidate_all();
	gui_->draw();
	events::raise_draw_event();
	brushes_.push_back(brush());
	brushes_[0].add_relative_location(0,0);
	brushes_[0].add_relative_location(1,0);
	brushes_[0].add_relative_location(-1,0);
	brushes_[0].add_relative_location(-2,0);
	set_brush(&brushes_[0]);
	mouse_actions_.push_back(new mouse_action_paint(*this));
	mouse_actions_.push_back(new mouse_action_fill(*this));
	set_mouse_action(mouse_actions_[0]);
	
}

void editor_controller::init(CVideo& video)
{
	config dummy;
	const config* theme_cfg = get_theme(game_config_, "editor2");
	theme_cfg = theme_cfg ? theme_cfg : &dummy;
	gui_ = new editor_display(video, map_, *theme_cfg, game_config_, config());
	gui_->set_grid(preferences::grid());
}

editor_controller::~editor_controller()
{
    delete gui_;
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
	foreach (mouse_action* a, mouse_actions_) {
		delete a;
	}
}

void editor_controller::main_loop()
{
	for(;;) {
		play_slice();
	}
}

bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int /*index*/) const
{
	using namespace hotkey; //reduce hotkey:: clutter
	switch (command) {
		case HOTKEY_ZOOM_IN:
		case HOTKEY_ZOOM_OUT:
		case HOTKEY_ZOOM_DEFAULT:
		case HOTKEY_FULLSCREEN:
		case HOTKEY_SCREENSHOT:
		case HOTKEY_MAP_SCREENSHOT:
		case HOTKEY_TOGGLE_GRID:
		case HOTKEY_MOUSE_SCROLL:
		case HOTKEY_MUTE:
		case HOTKEY_PREFERENCES:
		case HOTKEY_HELP:
			return true; //general hotkeys we can always do
		case HOTKEY_EDITOR_QUIT:
		case HOTKEY_EDITOR_MAP_NEW:
		case HOTKEY_EDITOR_MAP_LOAD:
		case HOTKEY_EDITOR_MAP_SAVE_AS:
			return true; //editor hotkeys we can always do
		case HOTKEY_EDITOR_MAP_SAVE:
		case HOTKEY_EDITOR_MAP_REVERT:
			return true; //TODO only when the map was modified
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return true; //tool selection always possible
		case HOTKEY_EDITOR_CUT:
		case HOTKEY_EDITOR_COPY:
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return true; //TODO require nonempty selection
		case HOTKEY_EDITOR_PASTE:
			return true; //TODO requre nonempty clipboard
		case HOTKEY_EDITOR_SELECT_ALL:		
		case HOTKEY_EDITOR_MAP_RESIZE:
		case HOTKEY_EDITOR_MAP_ROTATE:
		case HOTKEY_EDITOR_MAP_FLIP:
		case HOTKEY_EDITOR_MAP_GENERATE:
		case HOTKEY_EDITOR_REFRESH:
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			return true;
		default:
			return false;
	}
}

bool editor_controller::execute_command(hotkey::HOTKEY_COMMAND command, int index)
{
	using namespace hotkey;
	switch (command) {
		case HOTKEY_EDITOR_TOOL_PAINT:
			set_mouse_action(mouse_actions_[0]);
			return true;
		case HOTKEY_EDITOR_TOOL_FILL:
			set_mouse_action(mouse_actions_[1]);
			return true;
		default:
			return controller_base::execute_command(command, index);
	}
}

void editor_controller::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
	gui_->set_grid(preferences::grid());
	gui_->invalidate_all();
}

void editor_controller::preferences()
{
	preferences::show_preferences_dialog(*gui_, game_config_);
	gui_->redraw_everything();
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
	gui().recalculate_minimap();
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
	} else {
		if (get_mouse_action() != NULL) {
			get_mouse_action()->move(*gui_, x, y);
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
