/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef __GUI_WIDGETS_BUTTON_HPP_INCLUDED__
#define __GUI_WIDGETS_BUTTON_HPP_INCLUDED__

#include "gui/widgets/widget.hpp"

#include "gui/widgets/settings.hpp"
#include "log.hpp"
#include "tstring.hpp"

namespace gui2 {

// Class for a simple push button
class tbutton : public tcontrol
{
	friend void load_settings();
public:
	tbutton() : 
		tcontrol(),
		canvas_enabled_(),
		canvas_disabled_(),
		canvas_pressed_(),
		canvas_focussed_(),
		state_(ENABLED),
		definition_()
		{
		}

	void set_width(const unsigned width);

	void set_height(const unsigned height);

	void set_label(const t_string& label);

	
	void mouse_enter(tevent_handler&);
	void mouse_hover(tevent_handler&);
	void mouse_leave(tevent_handler&);

	void mouse_left_button_down(tevent_handler& event);
	void mouse_left_button_up(tevent_handler&);
	void mouse_left_button_click(tevent_handler&);
	void mouse_left_button_double_click(tevent_handler&); //FIXME remove

	void draw(surface& canvas);

	// note we should check whether the label fits in the button
	tpoint get_best_size() const;

	void set_best_size(const tpoint& origin);

protected:
	
private:

	tcanvas 
		canvas_enabled_,
		canvas_disabled_,
		canvas_pressed_,
		canvas_focussed_;

	enum tstate { ENABLED, DISABLED, PRESSED, FOCUSSED };

	void set_state(tstate state);
	tstate state_;

	std::vector<tbutton_definition::tresolution>::const_iterator definition_;

	void resolve_definition();
};


} // namespace gui2

#endif

