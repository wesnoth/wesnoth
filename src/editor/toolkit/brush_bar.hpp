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

/**
 * Manage the brush bar in the editor.
 * Note: this is a near-straight rip from the old editor.
*/

#ifndef BRUSH_BAR_H_INCLUDED
#define BRUSH_BAR_H_INCLUDED

#include "../../display.hpp"
#include "brush.hpp"
#include "editor/palette/palette_layout.hpp"

namespace editor {

/** A bar where the brush is drawn */
class brush_bar : public gui::widget {
public:
	brush_bar(display &gui, const size_specs &sizes, std::vector<brush>& brushes, brush** the_brush);

	/** Return the size of currently selected brush. */
	unsigned int selected_brush_size();

	/**
	 * Draw the palette. If force is true, everything
	 * will be redrawn, even though it is not dirty.
	 */
	void draw(bool force=false);
	virtual void draw();
	virtual void handle_event(const SDL_Event& event);

	/**
	 * Update the size of this widget.
	 *
	 * Use if the size_specs have changed.
	 */
	void adjust_size();

private:
	/**
	 * To be called when a mouse click occurs.
	 *
	 * Check if the coordinates is a terrain that may be chosen, and select the
	 * terrain if that is the case.
	 */
	void left_mouse_click(const int mousex, const int mousey);

	/** Return the index of the brush that is at coordinates (x, y) in the panel. */
	int selected_index(const int x, const int y) const;

	const size_specs &size_specs_;
	display &gui_;
	unsigned int selected_;
	std::vector<brush>& brushes_;
	brush** the_brush_;
	const size_t size_;
};

} // end namespace editor
#endif // BRUSH_BAR_H_INCLUDED
