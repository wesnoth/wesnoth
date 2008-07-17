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

#ifndef GUI_WIDGETS_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_WIDGET_HPP_INCLUDED

#include "gui/widgets/helper.hpp"
#include "sdl_utils.hpp"

#include "SDL.h"

#include <string>

namespace gui2 {

class tdialog;
class tevent_handler;
class twindow;

/**
 * Event execution calls.
 *
 * Base class with all possible events, most widgets can ignore most of these,
 * but they are available. In order to use an event simply override the
 * execution function and implement the wanted behaviour. The default behaviour
 * defined here is to do nothing.
 *
 * For more info about the event handling have a look at the tevent_handler
 * class which 'translates' sdl events into 'widget' events.
 */
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

	/***** ***** ***** ***** mouse movement ***** ***** ***** *****/

	/** 
	 * The mouse 'enters' the widget. 
	 *
	 * Entering happens when the mouse moves on a widget it wasn't on before.
	 * When the mouse is captured by another widget this event does not occur.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_enter(tevent_handler& /*event_handler*/) {}

	/** 
	 * The mouse moves 'over' the widget. 
	 *
	 * The mouse either moves over the widget or it has the mouse captured in
	 * which case every move causes a move event for the capturing widget.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_move(tevent_handler& /*event_handler*/) {}

	/**
	 * The mouse 'leaves' the widget.
	 *
	 * If the mouse is moves off the widget this event is called. When the leave
	 * occurs while the mouse is captured the event will be send when still of
	 * the widget if the capture is released. This event is only triggered when
	 * wants_mouse_hover_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_hover(tevent_handler& /*event_handler*/) {}

	/**
	 * The mouse 'hovers' over a widget.
	 *
	 * If the mouse remains a while without moving over a widget this event can
	 * be send. This event can be used to show a tooltip.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_leave(tevent_handler& /*event_handler*/) {}

	/***** ***** ***** ***** mouse left button ***** ***** ***** *****/

	/** 
	 * The left mouse button is pressed down. 
	 *
	 * This is a rather low level event, most of the time you want to have a
	 * look at mouse_left_button_click instead.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_down(tevent_handler& /*event_handler*/) {}

	/** 
	 * The left mouse button is released down.
	 *
	 * This is a rather low level event, most of the time you want to have a
	 * look at mouse_left_button_click instead.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_up(tevent_handler& /*event_handler*/) {}

	/**
	 * The left button is clicked.
	 *
	 * This event happens when the left mouse button is pressed and released on
	 * the same widget. It's execution can be a little delayed when
	 * wants_mouse_left_double_click_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_click(tevent_handler& /*event_handler*/) {}

	/**
	 * The left button is double clicked.
	 *
	 * This event happens when the left mouse button is pressed and released on
	 * the same widget twice within a short time. This event will only occur if
	 * wants_mouse_left_double_click_ is set.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void mouse_left_button_double_click(tevent_handler& /*event_handler*/) {}

	/***** ***** ***** ***** mouse middle button ***** ***** ***** *****/

	/** See mouse_left_button_down. */
	virtual void mouse_middle_button_down(tevent_handler&) {}

	/** See mouse_left_button_up. */
	virtual void mouse_middle_button_up(tevent_handler&) {}

	/** See mouse_left_button_click. */
	virtual void mouse_middle_button_click(tevent_handler&) {}

	/** See mouse_left_button_double_click. */
	virtual void mouse_middle_button_double_click(tevent_handler&) {}

	/***** ***** ***** ***** mouse right button ***** ***** ***** *****/

	/** See mouse_left_button_down. */
	virtual void mouse_right_button_down(tevent_handler&) {}

	/** See mouse_left_button_up. */
	virtual void mouse_right_button_up(tevent_handler&) {}

	/** See mouse_left_button_click. */
	virtual void mouse_right_button_click(tevent_handler&) {}

	/** See mouse_left_button_double_click. */
	virtual void mouse_right_button_double_click(tevent_handler&) {}

	/***** ***** ***** ***** mouse right button ***** ***** ***** *****/

	/**
	 * A key is pressed.
	 *
	 * When a key is pressed it's send to the widget that has the focus, if this
	 * widget doesn't handle the key it is send to the next handler. Some keys
	 * might get captured before send to the widget (eg F1).
	 *
	 * @param event_handler       The event handler that send the event.
	 * @param handled             Do we handle the event.
	 * @param key                 The SDL key code, needed for special keys.
	 * @param modifier            The keyboard modifiers when the key was
	 *                            pressed.
	 * @param unicode             The unicode for the pressed key.
	 */
	virtual void key_press(tevent_handler& /*event_handler*/, bool& /*handled*/, 
		SDLKey /*key*/, SDLMod /*modifier*/, Uint16 /*unicode*/) {} 

	/**
	 * The F1 key was pressed.
	 *
	 * This event is special since we normally want a help when this key is
	 * pressed.
	 *
	 * @param event_handler       The event handler that send the event.
	 */
	virtual void help_key(tevent_handler&) {}

	/***** ***** ***** ***** window management ***** ***** ***** *****/

	/**
	 * The window is resized.
	 *
	 * @param event_handler       The event handler that send the event.
	 * @param new_width           Width of the application window after resizing.
	 * @param new_height          Height of the application window after
	 *                            resizing.
	 */
	virtual void window_resize(tevent_handler&, const unsigned /* new_width */, 
		const unsigned /* new_height */) {}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_wants_mouse_hover(const bool hover = true) 
		{ wants_mouse_hover_ = hover; }
	bool wants_mouse_hover() const { return wants_mouse_hover_; }

	void set_wants_mouse_left_double_click(const bool click = true) 
		{ wants_mouse_left_double_click_ = click; }
	bool wants_mouse_left_double_click() const 
		{ return wants_mouse_left_double_click_; }

	void set_wants_mouse_middle_double_click(const bool click = true) 
		{ wants_mouse_middle_double_click_ = click; }
	bool wants_mouse_middle_double_click() const 
		{ return wants_mouse_middle_double_click_; }

	tevent_executor& set_wants_mouse_right_double_click(const bool click = true) 
		{ wants_mouse_right_double_click_ = click; return *this; }
	bool wants_mouse_right_double_click() const 
		{ return wants_mouse_right_double_click_; }

private:

	/** Does the widget want a hover event? See mouse_hover. */
	bool wants_mouse_hover_;

	/** 
	 * Does the widget want a left button double click? See
	 * mouse_left_button_double_click.
	 */ 
	bool wants_mouse_left_double_click_;

	/** See wants_mouse_left_double_click_ */
	bool wants_mouse_middle_double_click_;

	/** See wants_mouse_left_double_click_ */
	bool wants_mouse_right_double_click_;
};

/**
 * Base class for all widgets.
 *
 * From this abstract all other items should inherit. It contains the minimal
 * info needed for a real widget and some pure abstract functions which need to
 * be implemented by classes inheriting from this class.
 */
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

	/**
	 * Loads the configuration of the widget.
	 *
	 * Controls have their definition stored in a definition object. In order to
	 * determine sizes and drawing the widget this definition needs to be
	 * loaded. The member definition_ contains the name of the definition and
	 * function load the proper configuration.
	 */
	virtual void load_config() {} 

	/***** ***** ***** ***** get optimal sizes ***** ***** ***** *****/

	/**
	 * Gets the minimum size for the widget.
	 *
	 * @returns                      The minimum size for the widget.
	 * @retval 0,0                   The minimum size is 0,0.
	 */
	virtual tpoint get_minimum_size() const = 0;	

	/**
	 * Gets the best size for the widget.
	 *
	 * @returns                      The best size for the widget.
	 * @retval 0,0                   The best size is 0,0.
	 */
	virtual tpoint get_best_size() const = 0;	

	/**
	 * Gets the maximum size for the widget.
	 *
	 * @returns                      The maximum size for the widget.
	 * @retval 0,0                   The widget has no maximum size.
	 */
	virtual tpoint get_maximum_size() const = 0;	

	/**
	 * Does the widget have a vertical scrollbar.
	 *
	 * We want to use the best size for a widget, when the best size for all
	 * widgets doesn't fit inside the window some widgets need to be reduced in
	 * size. Widgets that have a scrollbar in the wanted direction are resized
	 * first. The next option is to look at the minimum size.
	 *
	 * @returns                   Whether or not the widget has a vertical
	 *                            scrollbar.
	 */
	virtual bool has_vertical_scrollbar() const { return false; }

	/**
	 * Does the widget have a vertical scrollbar.
	 *
	 * See has_vertical_scrollbar for more info.
	 * @returns                   Whether or not the widget has a horizontal
	 *                            scrollbar.
	 */
	virtual bool has_horizontal_scrollbar() const { return false; }

	/***** ***** ***** ***** drawing ***** ***** ***** *****/

	/**
	 *  Draws a widget.
	 *
	 *  The widget is (rather should) only (be) drawn if dirty.
	 * 
	 *  @todo add force as parameter.
	 *
	 *  @param surface            The surface to draw the widget upon using the
	 *                            coordinates and size of the widget.
	 *  @param force              Does the widget need to be drawn even if not
	 *                            dirty?
	 *  @param invalidate_background
	 *                            Some widgets can cache the background in order
	 *                            to undraw and redraw themselves if needed. If
	 *                            the background changes this 'feature' will
	 *                            cause glitches. When this parameter is set the
	 *                            widget need to reload the background and use
	 *                            that as new undraw cache. 
	 *                            \n Note if this is set the widget should also
	 *                            be redrawn.
	 */
	virtual void draw(surface& /*surface*/, const bool /*force*/ = false, 
		const bool /*invalidate_background*/ = false) = 0;

	/***** ***** ***** ***** query ***** ***** ***** *****/

	/**
	 * Gets the widget at the wanted coordinates.
	 *
	 * @param coordinate          The coordinate which should be inside the
	 *                            widget.
	 * @param must_be_active      The widget should be active, not all widgets
	 *                            have an active flag, those who don't ignore
	 *                            flag.
	 * 
	 * @returns                   The widget with the id.
	 * @retval 0                  No widget at the wanted coordinate found (or
	 *                            not active if must_be_active was set).
	 */
	virtual twidget* find_widget(const tpoint& coordinate, 
			const bool /*must_be_active*/) 
	{ 
		return coordinate.x >= x_ && coordinate.x < (x_ + static_cast<int>(w_)) &&
			coordinate.y >= y_ && coordinate.y < (y_ + static_cast<int>(h_)) ? this : 0;
	}

	/** The const version of find_widget. */
	virtual const twidget* find_widget(const tpoint& coordinate, 
			const bool /*must_be_active*/) const
	{ 
		return coordinate.x >= x_ && coordinate.x < (x_ + static_cast<int>(w_)) &&
			coordinate.y >= y_ && coordinate.y < (y_ + static_cast<int>(h_)) ? this : 0;
	}

	/**
	 * Gets a widget with the wanted id.
	 *
	 * @param id                  The id of the widget to find.
	 * @param must_be_active      The widget should be active, not all widgets
	 *                            have an active flag, those who don't ignore
	 *                            flag.
	 * 
	 * @returns                   The widget with the id.
	 * @retval 0                  No widget with the id found (or not active if
	 *                            must_be_active was set).
	 */
	virtual twidget* find_widget(const std::string& id, 
			const bool /*must_be_active*/)
		{ return id_ == id ? this : 0; }

	/** The const version of find_widget. */
	virtual const twidget* find_widget(const std::string& id, 
			const bool /*must_be_active*/) const
		{ return id_ == id ? this : 0; }
	/** 
	 * Does the widget contain the widget.
	 *
	 * Widgets can be containers which have more widgets inside them, this
	 * function will traverse in those child widgets and tries to find the
	 * wanted widget.
	 *
	 * @param widget              Pointer to the widget to find.
	 * @returns                   Whether or not the widget was found.
	 */
	virtual bool has_widget(const twidget* widget) const 
		{ return widget == this; }

	/***** ***** ***** ***** query parents ***** ***** ***** *****/

	/**
	 * Get the parent window.
	 *
	 * @returns                   Pointer to parent window.
	 * @retval 0                  No parent window found.
	 */
	twindow* get_window();

	/** 
	 * Returns the toplevel dialog.
	 *
	 * A window is most of the time created by a dialog, this function returns
	 * that dialog.
	 *
	 * @returns                   The toplevel dialog.
	 * @retval 0                  No toplevel window or the toplevel window is
	 *                            not owned by a dialog.
	 */
	tdialog* dialog();

	/***** ***** ***** setters / getters for members ***** ****** *****/

	twidget* parent() { return parent_; }
	void set_parent(twidget* parent) { parent_ = parent; }

	const std::string& id() const { return id_; }
	void set_id(const std::string& id) { id_ = id; }

	const std::string& definition() const { return definition_; }

	/**
	 * Sets the definition.
	 *
	 * This function should be set as soon as possible after creating the widget
	 * and shouldn't be changed after showing the widget. If this is done
	 * undefined things happen and the code doesn't enforce this rule.
	 */
	virtual void set_definition(const std::string& definition) 
		{ definition_ = definition; }

	/**
	 * Sets the size (and location) of the widget.
	 *
	 * There are no separate setters for the size only this function. Most of
	 * the time (or always) all sizes are modified together.
	 */
	virtual void set_size(const SDL_Rect& rect);
	
	/** Gets the sizes in one rect structure. */
	SDL_Rect get_rect() const 
		{ return ::create_rect( x_, y_, w_, h_ ); }
	int get_x() const { return x_; }
	int get_y() const { return y_; }
	unsigned get_width() const { return w_; }
	unsigned get_height() const { return h_; }

protected:	
	virtual void set_dirty(const bool dirty = true) 
	{ 
		dirty_ = dirty; 
		if(parent_ && dirty) parent_->set_dirty(true);
	}

public:	
	virtual bool is_dirty() const { return dirty_; }

private:
	/**
	 * The id is the unique name of the widget in a certain context. This is
	 * needed for certain widgets so the engine knows which widget is which. 
	 * Eg it knows which button is pressed and thus which engine action is 
	 * connected to the button. This doesn't mean that the id is unique in a
	 * window, eg a listbox can have the same id for every row.
	 */
	std::string id_;

	/**
	 * The definition is the id of that widget class. Eg for a button it
	 * [button_definition]id. A button can have multiple definitions which all
	 * look different but for the engine still is a button.
	 */
	std::string definition_;

	/**
	 * The parent widget, if the widget has a parent it contains a pointer to
	 * the parent, else it's set to 0.
	 */
	twidget* parent_;

	/** The x coordinate of the widget. */
	int x_;

	/** The y coordinate of the widget. */
	int y_;

	/** The width of the widget. */
	unsigned w_;
	
	/** The height of the widget. */
	unsigned h_;

	/** 
	 * Is the widget dirty? When a widget is dirty it needs to be redrawn at
	 * the next drawing cycle, setting it to dirty also need to set it's parent
	 * dirty so at so point the toplevel parent knows which item to redraw.
	 *
	 * NOTE dirtying the parent might be inefficient and this behaviour might be
	 * optimized later on.
	 */
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

	/** Select the control. */
	virtual void set_selected(const bool = true) = 0;
};

/**
 * Small abstract helper class.
 *
 * Parts of the engine inherit this class so we can have generic
 * widgets to select an integer value.
 */
class tinteger_selector_ 
{
public:
	virtual ~tinteger_selector_() {}

	/** Sets the selected value. */
	virtual void set_value(const int value) = 0;

	/** Gets the selected value. */
	virtual int get_value() const = 0;

	/** Sets the minimum value. */
	virtual void set_minimum_value(const int value) = 0;

	/** Gets the minimum value. */
	virtual int get_minimum_value() const = 0;

	/** Sets the maximum value. */
	virtual void set_maximum_value(const int value) = 0;

	/** Gets the maximum value. */
	virtual int get_maximum_value() const = 0;
};

} // namespace gui2

#endif
