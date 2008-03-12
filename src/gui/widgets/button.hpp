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

namespace gui2 {

// Class for a simple push button
class tbutton : public tcontrol
{
	friend void load_settings();
public:
	tbutton(const std::string& id) : 
		tcontrol(),
		state_(NORMAL)
		{
			canvas_up_.set_cfg(tbutton::default_enabled_draw_);
			canvas_up_mouse_over_.set_cfg(tbutton::default_mouse_over_draw_);
			canvas_down_.set_cfg(tbutton::default_pressed_draw_);
		}

	virtual void set_width(const int width);

	virtual void set_height(const int height);

	void mouse_down(const tevent_info& /*event*/, bool& /*handled*/);
	void mouse_up(const tevent_info& /*event*/, bool& /*handled*/);
	void mouse_click(const tevent_info& /*event*/, bool& /*handled*/);
	void mouse_double_click(const tevent_info& /*event*/, bool& /*handled*/);
	void mouse_enter(const tevent_info& /*event*/, bool& /*handled*/);
	void mouse_leave(const tevent_info& /*event*/, bool& /*handled*/);

	void draw(surface& canvas);

	// note we should check whether the label fits in the button
	tpoint get_best_size() const { return tpoint(default_width_, default_height_); }

	void set_best_size(const tpoint& origin) 
	{
		set_x(origin.x);
		set_y(origin.y);
		set_width(default_width_);
		set_height(default_height_);
	}

protected:
	
private:

	tcanvas 
		canvas_up_,
		canvas_up_mouse_over_,
		canvas_down_;

	static unsigned default_width_;
	static unsigned default_height_;
	static config default_enabled_draw_;
	static config default_mouse_over_draw_;
	static config default_pressed_draw_;

	enum tstate { NORMAL, DOWN, MOUSE_OVER };

	void set_state(tstate state);
	tstate state_;
};


} // namespace gui2

#endif

