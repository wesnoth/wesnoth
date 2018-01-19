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

/**
 * Manage the empty-palette in the editor.
 */

#pragma once

#include "editor/palette/editor_palettes.hpp"

namespace editor {

/** Empty palette */
class empty_palette : public common_palette {

public:

	empty_palette(display& gui) :
		common_palette(gui.video()),
		gui_(gui) {}

	//event handling
	virtual bool mouse_click() { return false;}

	virtual bool scroll_up() override { return false;}
	virtual bool can_scroll_up() override { return false;}
	virtual bool scroll_down() override { return false;}
	virtual bool can_scroll_down() override { return false;}

	//drawing
	virtual void adjust_size(const SDL_Rect& /*target*/) override {}
	virtual void draw() override {}

	void hide(bool /*hidden*/) override
	{
		std::shared_ptr<gui::button> upscroll_button = gui_.find_action_button("upscroll-button-editor");
		upscroll_button->enable(false);
		std::shared_ptr<gui::button> downscroll_button = gui_.find_action_button("downscroll-button-editor");
		downscroll_button->enable(false);
		std::shared_ptr<gui::button> palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
		palette_menu_button->set_overlay("");
		palette_menu_button->enable(false);
	}

	std::vector<gui::widget>* get_widgets() { return nullptr; }

	//group
	virtual void set_group(size_t /*index*/) override {}
	virtual void next_group() override {}
	virtual void prev_group() override {}
	virtual const std::vector<item_group>& get_groups() const override { static const std::vector<item_group> empty; return empty; }

	/** Menu expanding for palette group list */
	virtual void expand_palette_groups_menu(std::vector<config>& items, int i) override
	{
		items.erase(items.begin() + i);
	}

    //item
	virtual int num_items() override {return 0;}
	virtual size_t start_num() override {return 0;}
	virtual void set_start_item(size_t /*index*/) override {}
	virtual bool supports_swap() override { return false; }
	virtual void swap() override {}

private:
	display& gui_;
};

}
