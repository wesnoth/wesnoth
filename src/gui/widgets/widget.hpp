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

	virtual void mouse_enter(tevent_info&) {}
	virtual void mouse_move(tevent_info&) {}
	virtual void mouse_hover(tevent_info&) {}
	virtual void mouse_leave(tevent_info&) {}

	virtual void mouse_left_button_down(tevent_info&) {}
	virtual void mouse_left_button_up(tevent_info&) {}
	virtual void mouse_left_button_click(tevent_info&) {}
	virtual void mouse_left_button_double_click(tevent_info&) {}

	virtual void mouse_middle_button_down(tevent_info&) {}
	virtual void mouse_middle_button_up(tevent_info&) {}
	virtual void mouse_middle_button_click(tevent_info&) {}
	virtual void mouse_middle_button_double_click(tevent_info&) {}

	virtual void mouse_right_button_down(tevent_info&) {}
	virtual void mouse_right_button_up(tevent_info&) {}
	virtual void mouse_right_button_click(tevent_info&) {}
	virtual void mouse_right_button_double_click(tevent_info&) {}

	virtual void window_resize(tevent_info&, const unsigned /* new_width */, 
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
		w_(-1),
		h_(-1),
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

	// draw event does nothing by default, maybe make pure virtual later
	// fixme add force as parameter
	virtual void draw(surface& /*canvas*/) {}

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

#if 0
//! Base classs to hold widgets. Will be used for lists
class tcontainer 
{
public:
	tcontainer() {}

	~tcontainer();

	//! finds a child
	twidget* child(const std::string& id);

	void add_child(twidget *child);

	bool remove_child(const std::string& id);

	// NOTE this baseclass has no sizes so hit also fails it would be nice to 
	// find a good pure virtual function to avoid usage or scrap the entire
	// class and merge it with the panel class... but we still want a spacer type...

protected:
	std::multimap<std::string, twidget *>& children() { return children_; }

private:

	std::multimap<std::string, twidget *> children_;

};
#endif


//! Base class for all visible items.
class tcontrol : public virtual twidget
{

public:

	tcontrol(/*const int x = -1, const int y = -1, const int w = -1, const int h = -1*/);
	virtual ~tcontrol() {}

	void set_label(const std::string& label) { label_ = label; }

	const std::string& label() const { return label_; }
/*
	// moving an object doesn't dirty it, it should dirty the parent container...
	virtual void set_x(const int x) { x_ = x; }
	int get_x() const { return x_; }

	virtual void set_y(const int y) { y_ = y; }
	int get_y() const { return y_; }

	// resizing an object dirties it
	// note most items should not be resized manually but with a sizer
	virtual void set_width(const int width) { w_ = width; set_dirty(); }
	int get_width() const { return w_; }

	virtual void set_height(const int height) { h_ = height; set_dirty(); }
	int get_height() const { return h_; }
*/	
/*
	bool does_hit(const int x, const int y) const 
		{ return visible_ && x => x_ && x < (x_ + w_) && y >= y_ && y < (y_ + h_); }
*/
//	void handle_event(const SDL_Event& event);
protected:
	bool canvas_load_image();

	void canvas_draw_text();

//	void draw(surface& parent_canvas);
/*
	virtual void set_dirty(const bool dirty = true) { dirty_ = dirty; }

	SDL_Rect get_rect() const 
		{ return create_rect( x_, y_, w_, h_ ); }
*/
private:
//	int x_, y_, w_, h_;

//	bool dirty_;
	bool visible_;

	std::string label_;
	std::string tooltip_;
	std::string help_message_;

	surface canvas_;
};

#if 0
/**
 *  tscroll_panel is a panel which handels scrolling for 
 *  clients which have more data then can be shown
 *  eg the text on a message box.
 */
class tscrollpanel : public tpanel /*etc*/
{
};
#endif
/**
 * The slider has a few extra option
 * min, max minimum maximum of the slider
 *
 * min_Value if min is reached a special value can be returned
 * max_value idem for max
 *
 * min_text if min is reached a special caption can be shown
 * max_text idem for max
 */
class tslider
{
};

/**
 * Invisible container holding children
 */
/*
class tgrid : public twidget, public tcontainer
{

};
*/


/**
 * A widget has a mouse over which can either popup directly or after a fixed delay (this is a flag)
 * A widget has a help after pressing F1 it shows a larger tooltip with more info
 */

// base widget definition
#if 0

[widget_defaults]
	font_height = 8
	height = 12

	tooltip_popup_time = 300 		# ms before tooltip pops up automatically
	tooltip_mouse_move_delta = 2    # number of pixels the mouse can be moved to still be recorded as the same place

[/widget_defaults]

// basic stuff about buttons
[widget_definition]
	class = tbutton               # defines a button widget
	# height inherited
	# font_height inherited
	width = 80

	tooltip_delay = true          # should the tooltip pop up directly or wait for the timeout

[/widget_definition]

#endif


//dialog definition
#if 0

[dialog]
	id = dialog_mapgen            # this id is used internally as well so we know
	                              # which elements to expect

    [widget]
		id = widget_width         # this id is also known in the code, only widgets 
		                          # used in the code need an id. The code knows it is
								  # a widget which has the following methods
								  # void set_minimum(int)
								  # void set_maximum(int)
								  # void set_value(int)
								  # int get_value()
								  #
								  # suggest_step(int) proposes a step if applicable
								  # set_set(int) forces a step

		type = slider             # this tells we want to use a slider widget but 
		                          # could have been an int_box as well.

		# 
		label_position = left

		tooltip_delay = false     # override the value in the base class
		tooltip_message = _"foo"  # message on the tooltip
		help_message = _"agasdg"  # the help message

	[/widget]

	[widget]
		type = slider
		id = height

		# 
		label_position = bottom

[/dialog]


#endif


// examples of the draw language
#if 0

positive numbers are from 0,0 negative from width, height

[line]
	x, y =
	w, h =
	colour = r, g, b, a
	thickness =  
[/line]

// note a circle bending outwards is impossible do to the - parameter
[circle]
	x, y = centre
	border_colour = r, g, b, a
	border_thickness = 

	# fill colour only used if start_angle == 0 && end_angle == 360
	fill_colour = r, g, b, a

	# angle draw, twelve oclock is 0° and the angles increas clockwise
	start_angle = 0 # starts at
	end_abgle = 360 # ends at including
[/circle]

[rectangle]

	x, y =
	w, h =

	border_colour = r, g, b, a
	border_thickness = 

	fill_colour = r, g, b, a
[/rectangle]

// support for polygons may follow later

[image]
	x, y  = location to insert the top left corner of the image

	filename = file to load
[/image]

#endif

} // namespace gui2

#endif
