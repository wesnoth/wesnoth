/*
   Copyright (C) 2012 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "editor/map/context_manager.hpp"
#include "editor/palette/palette_manager.hpp"
#include "editor/toolkit/brush.hpp"
#include "hotkey/hotkey_command.hpp"

class config;

namespace editor {


class editor_toolkit {

public:
	editor_toolkit(editor_display& gui, const CKey& key,
			const config& game_config, context_manager& c_manager);

	~editor_toolkit();

	void adjust_size();

	void update_mouse_action_highlights();

private:
	/** init the sidebar objects */
	void init_sidebar(const config& game_config);

	/** init the brushes */
	void init_brushes(const config& game_config);

	/** init the mouse actions (tools) */
	void init_mouse_actions(context_manager& c_manager);

public:
	void set_mouseover_overlay(editor_display& gui);
	void set_mouseover_overlay() { set_mouseover_overlay(gui_); }
	void clear_mouseover_overlay();

	/**
	 * Set the current mouse action based on a hotkey id
	 */
	void hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command);

	/**
	 * @return true if the mouse action identified by the hotkey is active
	 */
	bool is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const;


	/** Get the current mouse action */
	mouse_action& get_mouse_action() { return *mouse_action_; }
	/** Get the current mouse action */
	const mouse_action& get_mouse_action() const { return *mouse_action_; }
	/** Get the current palette */
	common_palette& get_palette();

// Brush related methods

	/** Cycle to the next brush. */
	void cycle_brush();

	/** TODO */
	void set_brush(std::string id);

	/** TODO */
	bool is_active_brush(std::string id) const { return brush_->id() == id; }

	palette_manager* get_palette_manager() { return palette_manager_.get(); }

private:

	editor_display& gui_;

	const CKey& key_;

	std::unique_ptr<palette_manager> palette_manager_;

//Tools

	/** The current mouse action */
	std::shared_ptr<mouse_action> mouse_action_;  // Never null (outside the constructor).

	/** The mouse actions */
	typedef std::map<hotkey::HOTKEY_COMMAND, std::shared_ptr<mouse_action> > mouse_action_map;
	mouse_action_map mouse_actions_;

//Brush members

	/** The current brush */
	brush* brush_;

	/** All available brushes */
	std::vector<brush> brushes_;

};

}
