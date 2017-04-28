/*
   Copyright (C) 2012 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor/toolkit/editor_toolkit.hpp"
#include "config.hpp"

#include "editor/action/mouse/mouse_action.hpp"
#include "editor/action/mouse/mouse_action_map_label.hpp"
#include "editor/action/mouse/mouse_action_unit.hpp"
#include "editor/action/mouse/mouse_action_village.hpp"
#include "editor/action/mouse/mouse_action_item.hpp"
#include "editor/action/mouse/mouse_action_select.hpp"

namespace editor {

editor_toolkit::editor_toolkit(editor_display& gui, const CKey& key,
		const config& game_config, context_manager& c_manager)
	: gui_(gui)
	, key_(key)
	, palette_manager_()
	, mouse_action_(nullptr)  // Will be set before this constructor ends.
	, mouse_actions_()
	, brush_(nullptr)
	, brushes_()
{
	init_brushes(game_config);
	init_sidebar(game_config);
	init_mouse_actions(c_manager);
}

editor_toolkit::~editor_toolkit()
{
	//TODO ask someone about that
	//delete palette_manager_.get();
}

void editor_toolkit::init_brushes(const config& game_config)
{
	for (const config &i : game_config.child_range("brush")) {
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
	palette_manager_.reset(new palette_manager(gui_, game_config, *this));
}

void editor_toolkit::init_mouse_actions(context_manager& cmanager)
{
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_PAINT,
		std::make_shared<mouse_action_paint>(&brush_, key_, *palette_manager_->terrain_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_FILL,
		std::make_shared<mouse_action_fill>(key_, *palette_manager_->terrain_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_SELECT,
		std::make_shared<mouse_action_select>(&brush_, key_, *palette_manager_->empty_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION,
		std::make_shared<mouse_action_starting_position>(key_, *palette_manager_->location_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_LABEL,
		std::make_shared<mouse_action_map_label>(key_, *palette_manager_->empty_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_UNIT,
		std::make_shared<mouse_action_unit>(key_, *palette_manager_->unit_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_VILLAGE,
		std::make_shared<mouse_action_village>(key_, *palette_manager_->empty_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_CLIPBOARD_PASTE,
		std::make_shared<mouse_action_paste>(cmanager.get_clipboard(), key_, *palette_manager_->empty_palette_.get()));
	mouse_actions_.emplace(hotkey::HOTKEY_EDITOR_TOOL_ITEM,
		std::make_shared<mouse_action_item>(key_, *palette_manager_->item_palette_.get()));

	for (const theme::menu& menu : gui_.get_theme().menus()) {
		if (menu.items().size() == 1) {
			hotkey::HOTKEY_COMMAND hk = hotkey::get_id(menu.items().front()["id"]);
			mouse_action_map::iterator i = mouse_actions_.find(hk);
			if (i != mouse_actions_.end()) {
				i->second->set_toolbar_button(&menu);
			}
		}
	}

	mouse_action_ = (mouse_actions_.find(hotkey::HOTKEY_EDITOR_TOOL_PAINT))->second;
	set_mouseover_overlay();
}


void editor_toolkit::hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command)
{
	mouse_action_map::iterator i = mouse_actions_.find(command);
	if (i != mouse_actions_.end()) {
		palette_manager_->active_palette().hide(true);
		mouse_action_ = i->second;
		palette_manager_->adjust_size();

		set_mouseover_overlay();
		gui_.invalidate_game_status();
		palette_manager_->active_palette().hide(false);
	} else {
		ERR_ED << "Invalid hotkey command ("
			<< static_cast<int>(command) << ") passed to set_mouse_action\n";
	}

}

bool editor_toolkit::is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const
{
	mouse_action_map::const_iterator i = mouse_actions_.find(command);
	return (i != mouse_actions_.end()) && (i->second == mouse_action_);
}

common_palette& editor_toolkit::get_palette()
{
	return get_mouse_action().get_palette();
}

void editor_toolkit::update_mouse_action_highlights()
{
	DBG_ED << __func__ << "\n";
	int x, y;
	SDL_GetMouseState(&x, &y);
	map_location hex_clicked = gui_.hex_clicked_on(x,y);
	get_mouse_action().update_brush_highlights(gui_, hex_clicked);
}

void editor_toolkit::set_mouseover_overlay(editor_display& gui)
{
	get_mouse_action().set_mouse_overlay(gui);
}

void editor_toolkit::clear_mouseover_overlay()
{
	gui_.clear_mouseover_hex_overlay();
}

void editor_toolkit::set_brush(std::string id) {

	for (brush& i : brushes_) {
		if (i.id() == id) {
			brush_ = &i;
		}
	}
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
	palette_manager_->adjust_size();
}


} //Namespace editor
