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

#ifndef __GUI_WIDGETS_WIDGET_HPP_INCLUDED__
#define __GUI_WIDGETS_WIDGET_HPP_INCLUDED__

#include "gui/widgets/canvas.hpp"
#include "gui/widgets/event_handler.hpp"

#include "sdl_utils.hpp"
#include "video.hpp"

#include "tstring.hpp"
#include "config.hpp"
#include "variable.hpp"

#include "events.hpp"
#include "SDL.h"

#include <string>
#include <vector>
#include <map>


namespace gui2 {

// init needs a cfg object later to init the subsystem
bool init();

struct tpoint
{
	tpoint(const int x_, const int y_) : 
		x(x_),
		y(y_) 
		{}

	int x;
	int y;
};

std::ostream &operator<<(std::ostream &stream, const tpoint& point);

SDL_Rect create_rect(const tpoint& origin, const tpoint& size);

struct terror 
{
	terror(const std::string& msg) : message(msg) {}

	const std::string message;
};

//! Base class with all possible events, most widgets can ignore most of
//! these, but they are available.
class tevent_executor
{
public:
	tevent_executor() :
		wants_mouse_hover_(false),
		wants_mouse_left_double_click_(false),
		wants_mouse_middle_double_click_(false),
		wants_mouse_right_double_click_(false)
		{}
	virtual ~tevent_executor() {}

// Description of various event generating scenarios
//
// mouse moves on a widget and the focus isn't stolen:
// - mouse enter
//
// mouse on widget clicked without moving
// - mouse down
// - mouse up
// wait for possible double click if widget wants double click
// - mouse click

	virtual void mouse_enter(tevent_handler&) {}
	virtual void mouse_move(tevent_handler&) {}
	virtual void mouse_hover(tevent_handler&) {}
	virtual void mouse_leave(tevent_handler&) {}

	virtual void mouse_left_button_down(tevent_handler&) {}
	virtual void mouse_left_button_up(tevent_handler&) {}
	virtual void mouse_left_button_click(tevent_handler&) {}
	virtual void mouse_left_button_double_click(tevent_handler&) {}

	virtual void mouse_middle_button_down(tevent_handler&) {}
	virtual void mouse_middle_button_up(tevent_handler&) {}
	virtual void mouse_middle_button_click(tevent_handler&) {}
	virtual void mouse_middle_button_double_click(tevent_handler&) {}

	virtual void mouse_right_button_down(tevent_handler&) {}
	virtual void mouse_right_button_up(tevent_handler&) {}
	virtual void mouse_right_button_click(tevent_handler&) {}
	virtual void mouse_right_button_double_click(tevent_handler&) {}

	//! Handled, if there's a keyboard focus it will get the change to
	//! handle the key first, if not done it's send to the window.
	//! SDLKey the sdl key code needed for special keys
	//! SDLMod the keyboard modifiers at moment of pressing
	//! Unit16 the unicode for the pressed key
	virtual void key_press(tevent_handler&, bool&, SDLKey, SDLMod, Uint16) {} 

	virtual void window_resize(tevent_handler&, const unsigned /* new_width */, 
		const unsigned /* new_height */) {}

#if 0
	virtual void help(); // send when F1 is pressed on widget to get more help
#endif

	//! determines the best size for a widget.
	// x = best width
	// y = best height
	virtual tpoint get_best_size() const { return tpoint(0, 0); }

	//! determines the minimal size for a widget, needed?


	// layout sets up the children, this is only used by container
	// containers should know their own size and optimize their 
	// children.
	virtual void layout() {}

	bool wants_mouse_hover() const { return wants_mouse_hover_; }

	bool wants_mouse_left_double_click() const { return wants_mouse_left_double_click_; }
	bool wants_mouse_middle_double_click() const { return wants_mouse_middle_double_click_; }
	bool wants_mouse_right_double_click() const { return wants_mouse_right_double_click_; }

	tevent_executor& set_wants_mouse_hover(const bool hover = true) 
		{ wants_mouse_hover_ = hover; return *this; }

	tevent_executor& set_wants_mouse_left_double_click(const bool click = true) 
		{ wants_mouse_left_double_click_ = click; return *this; }

	tevent_executor& set_wants_mouse_middle_double_click(const bool click = true) 
		{ wants_mouse_middle_double_click_ = click; return *this; }

	tevent_executor& set_wants_mouse_right_double_click(const bool click = true) 
		{ wants_mouse_right_double_click_ = click; return *this; }

private:
	//! If a widget doesn't want a double click we need to send a second
	//! click instead of double click.
	bool wants_mouse_hover_;
	bool wants_mouse_left_double_click_;
	bool wants_mouse_middle_double_click_;
	bool wants_mouse_right_double_click_;
};

class twindow;

//! Base class for all widgets.
//! This is a non visible widget but it does have dimentions and size hints.
class twidget : public virtual tevent_executor
{
public:
	twidget() : 
		id_(""), 
		definition_("default"),
		parent_(0),
		x_(-1),
		y_(-1),
		w_(0),
		h_(0),
		dirty_(true)
		{}
	virtual ~twidget() {}

	twidget* parent() { return parent_; }
	void set_parent(twidget* parent) { parent_ = parent; }

	const std::string& id() const { return id_; }
	void set_id(const std::string& id) { id_ = id; }

	const std::string& definition() const { return definition_; }

	//! This should not be changed after the widget is shown, strange things
	//! might occur.
	void set_definition(const std::string& definition) 
		{ definition_ = definition; }

	//! Draws a widget.
	// FIXME add force as parameter
	virtual void draw(surface& /*surface*/) = 0;

	// invisible widgets never hit, only items with a size NOT here
	// virtual bool does_hit(const int x, const int y) const { return false; }


//	virtual void handle_event(const SDL_Event& event) { std::cerr << "x\n";}
	// moving an object doesn't dirty it, it should dirty the parent container...
	virtual void set_x(const int x) { x_ = x; }
	int get_x() const { return x_; }

	virtual void set_y(const int y) { y_ = y; }
	int get_y() const { return y_; }

	// resizing an object dirties it
	// note most items should not be resized manually but with a sizer
	virtual void set_width(const unsigned width) { w_ = width; set_dirty(); }
	unsigned get_width() const { return w_; }

	virtual void set_height(const unsigned height) { h_ = height; set_dirty(); }
	unsigned get_height() const { return h_; }

	bool dirty() const { return dirty_; }

	//! Sets the best size for the object.
	virtual void set_best_size(const tpoint& origin)
		{ set_size(create_rect(origin, get_best_size())); }
		

	//! Sets the minumum size for the object.
//	virtual void set_minimum_size();

	//! Sets a predefined size for the object.
	virtual void set_size(const SDL_Rect& rect)
	{
		x_ = rect.x;
		y_ = rect.y;
		w_ = rect.w;
		h_ = rect.h;
		dirty_ = true;
	}

	//! Gets the widget at the wanted coordinates.
	twidget* get_widget(const tpoint& coordinate) 
	{ 
		return coordinate.x >= x_ && coordinate.x < (x_ + w_) &&
			coordinate.y >= y_ && coordinate.y < (y_ + h_) ? this : 0;
	}

	//! Gets a widget with the wanted id.
	twidget* get_widget_by_id(const std::string& id)
		{ return id_ == id ? this : 0; }

	//! The toplevel item should always be a window if not null is returned
	twindow* get_window();

protected:	
	virtual void set_dirty(const bool dirty = true) 
	{ 
		dirty_ = dirty; 
		if(parent_ && dirty) parent_->set_dirty(true);
	}

	SDL_Rect get_rect() const 
		{ return ::create_rect( x_, y_, w_, h_ ); }

private:
	//! The id is the unique name of the widget in a certain context. This is
	//! needed for certain widgets so the engine knows which widget is which. 
	//! Eg it knows which button is pressed and thuswhich engine action is 
	//! connected to the button.
	std::string id_;

	//! The definition is the id of that widget class. Eg for a button it
	//! [button_definition]id. A button can have multiple definitions which all
	//! look different but for the engine still is a button.
	std::string definition_;

	twidget* parent_;
	int x_, y_;
	unsigned w_, h_;
	bool dirty_;
};

} // namespace gui2

#endif
