/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef __GUI_WIDGETS_TEXT_BOX_HPP_INCLUDED__
#define __GUI_WIDGETS_TEXT_BOX_HPP_INCLUDED__

#include "gui/widgets/widget.hpp"

#include "gui/widgets/settings.hpp"

#include <string>

namespace gui2 {

//! Class for a single line text area.
class ttext_box : public tcontrol
{

public:

	ttext_box() :
		tcontrol(),
		canvas_(),
		definition_(),
		dragging_(false),
		sel_start_(0),
		sel_len_(0),
		max_length_(std::string::npos)
		{
		}


	void set_width(const unsigned width);

	void set_height(const unsigned height);

	void set_label(const t_string& label);

	void mouse_move(tevent_handler&);
	void mouse_hover(tevent_handler&);

	void mouse_left_button_down(tevent_handler& event);
	void mouse_left_button_up(tevent_handler&);
	void mouse_left_button_double_click(tevent_handler&);

	void key_press(tevent_handler& event, bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode);


	void draw(surface& canvas);

	tpoint get_best_size() const;

	void set_best_size(const tpoint& origin);

protected:
	
	// Gets the character position at the wanted place
	// as if the mouse would click there.
//	virtual unsigned get_position_at(tpoint& coordinate);

//	void delete_selection();
//	void copy();
//	void paste();

private:

	tcanvas canvas_;

	std::vector<ttext_box_definition::tresolution>::const_iterator definition_;

	void resolve_definition();

	bool dragging_;
	size_t sel_start_;
	size_t sel_len_;
	size_t max_length_;

};



} //namespace gui2

#endif

