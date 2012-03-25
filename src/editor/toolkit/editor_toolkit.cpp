/* $Id$ */
/*
   Copyright (C) 2012 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_toolkit.hpp"
#include "foreach.hpp"
#include "config.hpp"

#include "editor/action/mouse/mouse_action.hpp"
#include "editor/action/mouse/mouse_action_map_label.hpp"
#include "editor/action/mouse/mouse_action_unit.hpp"


namespace editor {

editor_toolkit::editor_toolkit(editor_display& gui, const CKey& key, const config& game_config)
	: gui_(gui)
	, key_(key)
	, palette_manager_()
	, size_specs_()
	, toolbar_dirty_(true)
	, mouse_action_(NULL)
	, mouse_actions_()
	, mouse_action_hints_()
	, brush_(NULL)
	, brushes_()
	, brush_bar_(NULL)
{
	init_brushes(game_config);
	init_sidebar(game_config);
	init_mouse_actions(game_config);
	palette_manager_->adjust_size();
}

editor_toolkit::~editor_toolkit()
{
	//TODO ask someone about that
//	foreach (const mouse_action_map::value_type a, mouse_actions_) {
//		delete a.second;
//	}
	//delete palette_manager_.get();
	//delete brush_bar_.get();
//	delete brush_bar_.
}

void editor_toolkit::init_brushes(const config& game_config)
{
	foreach (const config &i, game_config.child_range("brush")) {
		brushes_.push_back(brush(i));
	}
	if (brushes_.empty()) {
		ERR_ED << "No brushes defined!";
		brushes_.push_back(brush());
		brushes_[0].add_relative_location(0, 0);
	}
	brush_ = &brushes_[0];
}

void editor_toolkit::init_sidebar(const config& game_config)
{
	size_specs_.reset(new size_specs());
	adjust_sizes(gui_, *size_specs_);
	brush_bar_.reset(new brush_bar(gui_, *size_specs_, brushes_, &brush_));
	palette_manager_.reset(new palette_manager(gui_, *size_specs_, game_config, &mouse_action_));
}

void editor_toolkit::hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command)
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::iterator i = mouse_actions_.find(command);
	if (i != mouse_actions_.end()) {
		mouse_action_ = i->second;
		//TODO is the draw call needed?
		palette_manager_->adjust_size();
		palette_manager_->draw(true);
		set_mouseover_overlay();
		redraw_toolbar();
		gui_.invalidate_game_status();
	} else {
		ERR_ED << "Invalid hotkey command ("
			<< static_cast<int>(command) << ") passed to set_mouse_action\n";
	}

}

void editor_toolkit::fill_selection()
{
	//TODO
	/*
	perform_refresh(editor_action_paint_area(get_map().selection(),
			toolkit_->terrain_palette_->selected_fg_item()));
			*/
}

bool editor_toolkit::is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::const_iterator i = mouse_actions_.find(command);
	return (i != mouse_actions_.end()) && (i->second == mouse_action_);
}

void editor_toolkit::update_mouse_action_highlights()
{
	DBG_ED << __func__ << "\n";
	int x, y;
	SDL_GetMouseState(&x, &y);
	map_location hex_clicked = gui_.hex_clicked_on(x,y);
	get_mouse_action()->update_brush_highlights(gui_, hex_clicked);
}

void editor_toolkit::set_mouseover_overlay()
{
	mouse_action_->set_mouse_overlay(gui_);
}

void editor_toolkit::clear_mouseover_overlay()
{
	gui_.clear_mouseover_hex_overlay();
}

void editor_toolkit::cycle_brush()
{
	if (brush_ == &brushes_.back()) {
		brush_ = &brushes_.front();
	} else {
		++brush_;
	}

	update_mouse_action_highlights();
}

void editor_toolkit::adjust_size()
{
	adjust_sizes(gui_, *size_specs_);

	palette_manager_->adjust_size();
	palette_manager_->draw(true);

	brush_bar_->adjust_size();
	brush_bar_->draw(true);

	redraw_toolbar();
}

void editor_toolkit::redraw_toolbar()
{
	foreach (mouse_action_map::value_type a, mouse_actions_) {
		if (a.second->toolbar_button() != NULL) {
			SDL_Rect r = a.second->toolbar_button()->location(gui_.screen_area());
			SDL_Rect outline = create_rect(r.x - 2, r.y - 2, r.h + 4, r.w + 4);
			//TODO comment or remove
			//outline = intersect_rects(r, gui().screen_area());
			surface screen = gui_.video().getSurface();
			Uint32 color;
			if (a.second == mouse_action_) {
				color = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
			} else {
				color = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			}
			draw_rectangle(outline.x, outline.y, outline.w, outline.h, color, screen);
			update_rect(outline);
		}
	}
	toolbar_dirty_ = false;
}

void editor_toolkit::init_mouse_actions(const config& game_config)
{
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_PAINT,
		new mouse_action_paint(&brush_, key_, *palette_manager_->terrain_palette_.get())));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_FILL,
		new mouse_action_fill(key_, *palette_manager_->terrain_palette_.get())));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_SELECT,
		new mouse_action_select(&brush_, key_, *palette_manager_->empty_palette_.get())));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION,
		new mouse_action_starting_position(key_, *palette_manager_->empty_palette_.get())));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_LABEL,
		new mouse_action_map_label(key_, *palette_manager_->empty_palette_.get())));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_UNIT,
		new mouse_action_unit(key_, *palette_manager_->unit_palette_.get())));

//	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_PASTE,
//		new mouse_action_paste(clipboard_, key_, *palette_manager_->empty_palette_.get())));

	foreach (const theme::menu& menu, gui_.get_theme().menus()) {
		if (menu.items().size() == 1) {
			hotkey::HOTKEY_COMMAND hk = hotkey::get_hotkey(menu.items().front()).get_id();
			mouse_action_map::iterator i = mouse_actions_.find(hk);
			if (i != mouse_actions_.end()) {
				i->second->set_toolbar_button(&menu);
			}
		}
	}
	foreach (const config &c, game_config.child_range("editor_tool_hint")) {
		mouse_action_map::iterator i =
			mouse_actions_.find(hotkey::get_hotkey(c["id"]).get_id());
		if (i != mouse_actions_.end()) {
			mouse_action_hints_.insert(std::make_pair(i->first, c["text"]));
		}
	}

	hotkey_set_mouse_action(hotkey::HOTKEY_EDITOR_TOOL_PAINT);
}


} //Namespace editor
