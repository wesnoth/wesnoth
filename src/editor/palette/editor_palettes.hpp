/* $Id$ */
/*
  Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#ifndef EDITOR_PALETTES_H_INCLUDED
#define EDITOR_PALETTES_H_INCLUDED

#include "../editor_display.hpp"
#include "../editor_layout.hpp"
#include "common_palette.hpp"

namespace editor {

template<class Item>
class editor_palette: public gui::widget, public common_palette{
public:
	editor_palette(editor_display &gui, const size_specs &sizes, const config& /*cfg*/,
			Item& fore, Item& back)
	: gui::widget(gui.video())
	, size_specs_(sizes)
	, gui_(gui)
	, palette_start_(0)
	, item_map_()
	, selected_fg_item_(fore)
	, selected_bg_item_(back)
{
}
	virtual void set_group(size_t index);

	std::vector<item_group>& get_groups() { return groups_; };

	virtual size_t active_group_index();

	/** Scroll the editor-palette up one step if possible. */
	virtual void scroll_up();

	/** Scroll the editor-palette down one step if possible. */
	virtual void scroll_down();

	/**
	 * Draw the palette.
	 *
	 * If force is true everything will be redrawn,
	 * even though it is not invalidated.
	 */
	virtual void draw(bool force=false);
	virtual void draw() { draw(false); };
	virtual void handle_event(const SDL_Event& event);

	/**
	 * Update the size of this widget.
	 *
	 * Use if the size_specs have changed.
	 */
	virtual void adjust_size();

private:

	virtual void draw_item(SDL_Rect& dstrect, const Item& item, std::stringstream& tooltip) = 0;

	virtual const std::string& get_id(const Item& item) = 0;

	/** Setup the internal data structure. */
	virtual void setup(const config& cfg) = 0;


	/** Scroll the editor-palette to the top. */
	void scroll_top();

	/** Scroll the editor-palette to the bottom. */
	void scroll_bottom();



	virtual const std::string& active_group_id() {return active_group_;};


	virtual const config active_group_report();

protected:
	/**
	 * Sets a group active id
	 *
	 * This can result in no visible
	 * selected items.
	 */
	virtual void set_group(const std::string& id);
	const std::vector<std::string>& active_group() { return group_map_[active_group_]; };

	/** Return the currently selected foreground item. */
	Item selected_fg_item() const { return selected_fg_item_; };

	/**
	 * The editor_groups as defined in editor-groups.cfg.
	 *
	 * Note the user must make sure the id's here are the same as the
	 * editor_group in terrain.cfg.
	 */
	std::vector<item_group> groups_;



private:
	/** Return the currently selected background item. */
	Item selected_bg_item() const { return selected_bg_item_; };

	virtual void swap();

	/** Select a foreground item. */
	virtual void select_fg_item(Item);
	virtual void select_bg_item(Item);

	/** Return the number of terrains in the palette. */
	size_t num_items();

	void draw_old(bool);

	int item_width_;
	int item_size_;

	/**
	 * To be called when a mouse click occurs.
	 *
	 * Check if the coordinates is a terrain that may be chosen,
	 * and select the terrain if that is the case.
	 */
	void left_mouse_click(const int mousex, const int mousey);
	void right_mouse_click(const int mousex, const int mousey);

	/** Return the number of the tile that is at coordinates (x, y) in the panel. */
	int tile_selected(const int x, const int y) const;

	/** Update the report with the currently selected items. */
	virtual void update_report() = 0;

protected:
	const size_specs &size_specs_;
	editor_display &gui_;

private:
	unsigned int palette_start_;

protected:
	std::map<std::string, std::vector<std::string> > group_map_;

	std::map<std::string, Item> item_map_;
	size_t nitems_, nmax_items_, items_start_;
    std::set<std::string> non_core_items_;

private:
	std::string active_group_;
	Item& selected_fg_item_;
	Item& selected_bg_item_;
};


} //end namespace editor
#endif // EDITOR_PALETTES_H_INCLUDED

