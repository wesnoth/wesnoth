/* $Id$ */
/*
   copyright (c) 2007 - 2008 by mark de wever <koraq@xs4all.nl>
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
		tcontrol()
		{
			canvas_up_.set_cfg(tbutton::default_enabled_draw_);
		}

	virtual void set_width(const int width);

	virtual void set_height(const int height);

	void mouse_down(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse down\n"; }
	void mouse_up(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse up\n"; }
	void mouse_click(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse click\n"; }
	void mouse_double_click(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse double click\n"; }
	void mouse_enter(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse enter\n"; }
	void mouse_leave(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "mouse leave\n"; }

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
};


} // namespace gui2

#endif

