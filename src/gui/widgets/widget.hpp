/* $Id$ */
/*
   copyright (C) 2007 - 2008 by mark de wever <koraq@xs4all.nl>
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

#include "gui/widgets/helper.hpp"
#include "sdl_utils.hpp"

#include "SDL.h"

#include <string>

namespace gui2 {

class tevent_handler;

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

	//! When F1 is pressed this event is triggered.
	virtual void help_key(tevent_handler&) {}

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
	virtual void set_definition(const std::string& definition) 
		{ definition_ = definition; }

	//! Draws a widget.
	// FIXME add force as parameter
	virtual void draw(surface& /*surface*/) = 0;

	// invisible widgets never hit, only items with a size NOT here
	// virtual bool does_hit(const int x, const int y) const { return false; }


	int get_x() const { return x_; }
	int get_y() const { return y_; }
	unsigned get_width() const { return w_; }
	unsigned get_height() const { return h_; }

	//! Is the widget dirty?
	virtual bool dirty() const { return dirty_; }

	//! Gets the minimum size for the object, 0,0 means no size required.
	virtual tpoint get_minimum_size() const = 0;	

	//! Gets the best size for the object, 0,0 means no size required.
	virtual tpoint get_best_size() const = 0;	

	//! Gets the best size for an object, 0,0 means no limits.
	virtual tpoint get_maximum_size() const = 0;	

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
	virtual twidget* get_widget(const tpoint& coordinate) 
	{ 
		return coordinate.x >= x_ && coordinate.x < (x_ + w_) &&
			coordinate.y >= y_ && coordinate.y < (y_ + h_) ? this : 0;
	}

	//! Gets a widget with the wanted id.
	virtual twidget* get_widget_by_id(const std::string& id)
		{ return id_ == id ? this : 0; }

	/** 
	 * Does the widget contain the widget.
	 *
	 * This makes more sence in container classes.
	 */
	virtual bool has_widget(const twidget* widget) const 
		{ return widget == this; }

	//! The toplevel item should always be a window if not null is returned
	twindow* get_window();

	//! loads the configuration of the widget, mainly used for controls.
	virtual void load_config() {} 

	SDL_Rect get_rect() const 
		{ return ::create_rect( x_, y_, w_, h_ ); }


	/** 
	 * If the best size doesn't fit we want to use the best size for normal 
	 * widgets, and resize those who own a scrollbar. 
	 */
	virtual bool has_vertical_scrollbar() const { return false; }
	virtual bool has_horizontal_scrollbar() const { return false; }

protected:	
	virtual void set_dirty(const bool dirty = true) 
	{ 
		dirty_ = dirty; 
		if(parent_ && dirty) parent_->set_dirty(true);
	}

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

/**
 * Small abstract helper class.
 *
 * Parts of the engine inherit this class so we can have generic
 * selectable items.
 */
class tselectable_ 
{
public:
	virtual ~tselectable_() {}

	/** Is the control selected? */
	virtual bool is_selected() const = 0;

	/** Select the control */
	virtual void set_selected(const bool = true) = 0;
};

} // namespace gui2

#endif
