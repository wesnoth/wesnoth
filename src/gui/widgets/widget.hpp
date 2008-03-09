/* $id: boilerplate-header.cpp 20001 2007-08-31 19:09:40z soliton $ */
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
#include "gui/widgets/event_info.hpp"

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
	tevent_executor(const bool send_double_click = true) :
		send_double_click_(send_double_click)
		{}
	virtual ~tevent_executor() {}

	//! Happens when a mouse goes down on the widget.
	virtual void mouse_down(const tevent_info&, bool&) {}

	//! Happens when a mouse down focussed this widget and is released
	//! again.
	virtual void mouse_up(const tevent_info&, bool&) {}

	//! Happens when a mouse down and up happen on the same widget.
	virtual void mouse_click(const tevent_info&, bool&) {}
	
	//! Happens when a mouse down and up happen twice on the same widget.
	virtual void mouse_double_click(const tevent_info& event, bool& handled) 
		{ if(!send_double_click_) mouse_click(event, handled); }

#if 0
	virtual void mouse_enter();
	virtual void mouse_move();
	virtual void mouse_leave();

	virtual void hover(); // send when mouse is on the widget for about 1 sec to show a short help

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



private:
	//! If a widget doesn't want a double click we need to send a second
	//! click instead of double click.
	bool send_double_click_;
};

//! Base class for all widgets.
//! This is a non visible widget but it does have dimentions and size hints.
class twidget : public virtual tevent_executor
{
public:
	twidget(const std::string& id = "") : 
		id_(id), 
		parent_(0),
		x_(-1),
		y_(-1),
		w_(-1),
		h_(-1),
		dirty_(true)
		{}
	virtual ~twidget() {}

	twidget* parent() { return parent_; }
	const std::string id() const { return id_; }

	// draw event does nothing by default, maybe make pure virtual later
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
	virtual void set_width(const int width) { w_ = width; set_dirty(); }
	int get_width() const { return w_; }

	virtual void set_height(const int height) { h_ = height; set_dirty(); }
	int get_height() const { return h_; }



protected:	
	virtual void set_dirty(const bool dirty = true) { dirty_ = dirty; }

	SDL_Rect get_rect() const 
		{ return create_rect( x_, y_, w_, h_ ); }

private:
	const std::string id_;
	twidget* parent_;
	int x_, y_, w_, h_;
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

//! Base container class which needs to size children
class tsizer : /*public tcontainer,*/ public virtual twidget
{

public:
	// ***** ***** FLAGS ***** *****
	static const unsigned VERTICAL_RESIZE_GROW           = 1 << 1;
	static const unsigned VERTICAL_GROW_SEND_TO_CLIENT   = 1 << 2;

	static const unsigned VERTICAL_ALIGN_TOP             = 1 << 4;   
	static const unsigned VERTICAL_ALIGN_CENTER          = 1 << 5;   
	static const unsigned VERTICAL_ALIGN_BOTTOM          = 1 << 6;   
	static const unsigned VERTICAL_ALIGN_LANGUAGE        = 1 << 7;   


	static const unsigned HORIZONTAL_RESIZE_GROW         = 1 << 16;
	static const unsigned HORIZONTAL_GROW_SEND_TO_CLIENT = 1 << 17;

	static const unsigned HORIZONTAL_ALIGN_TOP           = 1 << 18;   
	static const unsigned HORIZONTAL_ALIGN_CENTER        = 1 << 19;   
	static const unsigned HORIZONTAL_ALIGN_BOTTOM        = 1 << 20;   
	static const unsigned HORIZONTAL_ALIGN_LANGUAGE      = 1 << 21;   


	static const unsigned BORDER_TOP                     = 1 << 24;
	static const unsigned BORDER_BOTTOM                  = 1 << 25;
	static const unsigned BORDER_LEFT                    = 1 << 26;
	static const unsigned BORDER_RIGHT                   = 1 << 27;

	
	tsizer(const unsigned rows, const unsigned cols, 
		const unsigned default_flags, const unsigned default_border_size);

	virtual ~tsizer();


	void add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size);
	
	void add_child(twidget* widget, const unsigned row, const unsigned col)
		{ add_child(widget, row, col, default_flags_, default_border_size_); }

	void add_child(twidget* widget, const unsigned row, const unsigned col, const unsigned flags)
		{ add_child(widget, row, col, flags, default_border_size_); }
	 
	
	void set_rows(const unsigned rows);
	unsigned int get_rows() const { return rows_; }

	void set_cols(const unsigned cols);
	unsigned int get_cols() const { return cols_; }

	void remove_child(const unsigned row, const unsigned col);
	void removed_child(const std::string& id, const bool find_all = false);

	void layout();
	
private:
	struct tchild {
		tchild() : 
			flags(0),
			border_size(0),
			widget(0) 
			{}

		std::string id;
		unsigned flags;
		unsigned border_size;
		twidget* widget;
	};
public:
	class iterator 
	{

	public:

		iterator(std::vector<tchild>::iterator itor) :
			itor_(itor) 
			{}

		iterator operator++() { return iterator(++itor_); }
		iterator operator--() { return iterator(--itor_); }
		twidget* operator->() { return itor_->widget; }
		twidget* operator*() { return itor_->widget; }

		bool operator!=(const iterator& i) const
			{ return i.itor_ != this->itor_; }

	private:
		std::vector<tchild>::iterator itor_;

	};

	iterator begin() { return iterator(children_.begin()); }
	iterator end() { return iterator(children_.end()); }

private:
	unsigned rows_;
	unsigned cols_;

	const unsigned default_flags_;

	const unsigned default_border_size_;

	


	std::vector<tchild> children_;
	tchild& child(const unsigned row, const unsigned col)
		{ return children_[rows_ * col + row]; }

};


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

//! Visible container to hold children.
class tpanel : public tsizer, public tcontrol
{

public:
	tpanel() : 
		tsizer(0, 0, 0, 0),
		tcontrol()
		{}

private:

}; 

/**
 *  tscroll_panel is a panel which handels scrolling for 
 *  clients which have more data then can be shown
 *  eg the text on a message box.
 */
class tscrollpanel : public tpanel /*etc*/
{
};

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

	void mouse_down(const tevent_info& /*event*/, bool& /*handled*/) { std::cerr << "Hit me again\n"; }

	void draw(surface& canvas);

	// note we should check whether the label fits in the button
	tpoint get_best_size() const { return tpoint(default_width_, default_height_); }
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
}

#endif
