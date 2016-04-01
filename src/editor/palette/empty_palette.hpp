/*
   Copyright (C) 2012 - 2016 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Manage the empty-palette in the editor.
 */

#ifndef EMPTY_PALETTE_H_INCLUDED
#define EMPTY_PALETTE_H_INCLUDED

#include "editor_palettes.hpp"

namespace editor {

/** Empty palette */
class empty_palette : public common_palette {

public:

	empty_palette(display& gui) :
		common_palette(gui),
		gui_(gui), empty_() {}

	//event handling
	virtual bool mouse_click() { return false;}

	virtual bool scroll_up() { return false;}
	virtual bool can_scroll_up() { return false;}
	virtual bool scroll_down() { return false;}
	virtual bool can_scroll_down() { return false;}

	virtual void select_fg_item(const std::string& /*item_id*/) {}
	virtual void select_bg_item(const std::string& /*item_id*/) {}

	//drawing
	virtual void adjust_size(const SDL_Rect& /*target*/) {}
	virtual void draw() {}

	void hide(bool hidden) {
		if (!hidden) {
			gui::button* upscroll_button = gui_.find_action_button("upscroll-button-editor");
			upscroll_button->enable(false);
			gui::button* downscroll_button = gui_.find_action_button("downscroll-button-editor");
			downscroll_button->enable(false);
			gui::button* palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
			palette_menu_button->set_overlay("");
			palette_menu_button->enable(false);
		} else {
			gui::button* upscroll_button = gui_.find_action_button("upscroll-button-editor");
			upscroll_button->enable(true);
			gui::button* downscroll_button = gui_.find_action_button("downscroll-button-editor");
			downscroll_button->enable(true);
			gui::button* palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
			palette_menu_button->enable(true);
		}
	}

	std::vector<gui::widget>* get_widgets() { return nullptr; }

	//group
	virtual void set_group(size_t /*index*/) {}
	virtual void next_group() {}
	virtual void prev_group() {}
	virtual const config active_group_report() { return config();}
	virtual const std::vector<item_group>& get_groups() const { return empty_; }

	/** Menu expanding for palette group list */
	virtual void expand_palette_groups_menu(std::vector< std::pair< std::string, std::string> >& /*items*/) {}
	virtual void expand_palette_groups_menu(std::vector< std::string> & /*items*/) {}

    //item
	virtual size_t num_items() {return 0;}
	virtual size_t start_num() {return 0;}
	virtual void set_start_item(size_t /*index*/) {}
	virtual bool supports_swap() { return false; }
	virtual void swap() {}

private:
	display& gui_;
	std::vector<item_group> empty_;
};


}
#endif
