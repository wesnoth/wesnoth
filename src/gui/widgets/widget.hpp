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

#include "gui/widgets/event_executor.hpp"
#include "gui/widgets/helper.hpp"
#include "sdl_utils.hpp"
#include "wml_exception.hpp"

#include <string>
#include <cassert>

namespace gui2 {

class tdialog;
class twindow;

/**
 * Base class for all widgets.
 *
 * From this abstract all other items should inherit. It contains the minimal
 * info needed for a real widget and some pure abstract functions which need to
 * be implemented by classes inheriting from this class.
 */
class twidget : public virtual tevent_executor
{
	friend class tdebug_layout_graph;

public:
	twidget()
		: id_("")
		, definition_("default")
		, parent_(0)
		, x_(-1)
		, y_(-1)
		, w_(0)
		, h_(0)
		, dirty_(true)
		, layout_size_(tpoint(0,0))
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
		, last_best_size_(tpoint(0,0))
#endif			
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

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/**
	 * How the layout engine works.
	 *
	 * Every widget has a member layout_size_ which holds the best size in
	 * the current layout phase. When the windows starts the layout phase it
	 * calls layout_init() which resets this value.
	 *
	 * Every widget has two function to get the best size. get_best_size() tests
	 * whether layout_size_ is set and if so returns that value otherwise it
	 * calls calculate_best_size() so the size can be updated.
	 *
	 * During the layout phase some functions can modify layout_size_ so the
	 * next call to get_best_size() returns the currently best size. This means
	 * that after the layout phase get_best_size() still returns this value.
	 */

	/** Initializes the layout phase. */
	virtual void layout_init() { layout_size_ = tpoint(0,0); }

	/**
	 * Gets the best size for the widget.
	 *
	 * During the layout phase a best size will be determined, several stages
	 * might change the best size. This function will return the currently best
	 * size as determined during the layout phase.
	 *
	 * @returns                      The best size for the widget.
	 * @retval 0,0                   The best size is 0,0.
	 */
	tpoint get_best_size() const;

private:
	/**
	 * Calculates the best size.
	 *
	 * This function calculates the best size and ignores the current values in
	 * the layout phase. Note containers can call the get_best_size() of their
	 * children since it's meant to update itself.
	 *
	 * @returns                      The best size for the widget.
	 * @retval 0,0                   The best size is 0,0.
	 */
	virtual tpoint calculate_best_size() const = 0;
public:

	/**
	 * Can the widget wrap.
	 *
	 * When a widget can wrap it can reduce it's width by increasing it's
	 * height. When a layout is too wide it should first try to wrap and if
	 * that fails it should check the vertical scrollbar status. After wrapping
	 * the height might (probably will) change so the layout engine needs to
	 * recalculate.
	 */
	virtual bool can_wrap() const { return false; }

	/**
	 * Wraps the contents of the widget.
	 *
	 * @todo implement this function properly.
	 *
	 * @param maximum_width       The wanted maximum width of the widget.
	 *
	 * @pre                       can_wrap() == true.
	 */
	virtual void layout_wrap(const unsigned /*maximum_width*/) 
		{ assert(can_wrap()); }

	/**
	 * Does the widget have a horizontal scrollbar.
	 *
	 * See has_vertical_scrollbar for more info.
	 * @returns                   Whether or not the widget has a horizontal
	 *                            scrollbar.
	 */
	virtual bool has_horizontal_scrollbar() const { return false; }

	/**
	 * Tries to use a horizontal scrollbar with the widget.
	 *
	 * @param maximum_width       The wanted maximum width of the widget.
	 *
	 * @pre                       has_horizontal_scrollbar() == true.
	 */
	virtual void layout_use_horizontal_scrollbar(const unsigned /*maximum_width*/) 
		{ assert(has_horizontal_scrollbar()); }

	/**
	 * Tries to shrink a widget so it fits in the wanted width.
	 *
	 * @todo implement this function properly.
	 *
	 * @param maximum_width       The wanted maximum width of the widget.
	 */
	virtual void layout_shrink_width(const unsigned /*maximum_width*/) { }

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
	 * Tries to use a vertical scrollbar with the widget.
	 *
	 * @todo implement this function properly.
	 *
	 * @param maximum_height      The wanted maximum height of the widget.
	 *
	 * @pre                       has_vertical_scrollbar() == true.
	 */
	virtual void layout_use_vertical_scrollbar(const unsigned /*maximum_height*/) 
		{ assert(has_vertical_scrollbar()); }

	/**
	 * Tries to shrink a widget so it fits in the wanted height.
	 *
	 * @todo implement this function properly.
	 *
	 * @param maximum_height      The wanted maximum height of the widget.
	 *
	 */
	virtual void layout_shrink_height(const unsigned /*maximum_height*/) { }

	/**
	 * Sets the size of the widget.
	 *
	 * @param origin              The position of top left of the widget.
	 * @param size                The size of the widget.
	 */
	virtual void set_size(const tpoint& origin, const tpoint& size);

	/***** ***** ***** ***** drawing ***** ***** ***** *****/

	/**
	 *  Draws a widget.
	 *
	 *  The widget is (rather should) only (be) drawn if dirty. Note when a
	 *  widget draws itself it shouldn't clear the dirty flag. This should be
	 *  done by the toplevel window that issued the draw. This to avoid
	 *  inherited functions to clear the dirty flag too early.
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
	 * Gets a widget with the wanted id.
	 *
	 * This template function doesn't return a pointer to a generic widget but
	 * returns the wanted type and tests for its existence if required.
	 *
	 * @param id                  The id of the widget to find.
	 * @param must_be_active      The widget should be active, not all widgets
	 *                            have an active flag, those who don't ignore
	 *                            flag.
	 * @param must_exist          The widget should be exist, the function will
	 *                            fail if the widget doesn't exist or is
	 *                            inactive and must be active. Upon failure a
	 *                            wml_error is thrown.
	 * 
	 * @returns                   The widget with the id.
	 */
	template<class T>
	T* find_widget(const std::string& id, 
			const bool must_be_active, const bool must_exist)
	{
		T* widget = 
			dynamic_cast<T*>(find_widget(id, must_be_active));
		VALIDATE(!must_exist || widget, missing_widget(id));
	
		return widget;
	}

	/** The const version of find_widget. */
	template<class T>
	const T* find_widget(const std::string& id, 
			const bool must_be_active, const bool must_exist) const
	{
		const T* widget = 
			dynamic_cast<const T*>(find_widget(id, must_be_active));
		VALIDATE(!must_exist || widget, missing_widget(id));
	
		return widget;
	}

	/** 
	 * Gets a widget with the wanted id.
	 *
	 * This template function doesn't return a pointer to a generic widget but
	 * returns the wanted type and tests for its existence.
	 *
	 * @param id                  The id of the widget to find.
	 * @param must_be_active      The widget should be active, not all widgets
	 *                            have an active flag, those who don't ignore
	 *                            flag.
	 * 
	 * @returns                   The widget with the id.
	 */
	template <class T>
	T& get_widget(const std::string& id, const bool must_be_active)
	{
		T* result = dynamic_cast<T*>(find_widget(id, must_be_active));
		VALIDATE(result, missing_widget(id));
		return *result;
	}

	/** The const version of get_widget. */
	template <class T>
	const T& get_widget(const std::string& id, const bool must_be_active) const
	{
		const T* result = dynamic_cast<const T*>(find_widget(id, must_be_active));
		VALIDATE(result, missing_widget(id));
		return *result;
	}
	
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

	/** Gets the sizes in one rect structure. */
	SDL_Rect get_rect() const 
		{ return ::create_rect( x_, y_, w_, h_ ); }
	int get_x() const { return x_; }
	int get_y() const { return y_; }
	unsigned get_width() const { return w_; }
	unsigned get_height() const { return h_; }

	/**
	 * Sets the widgets dirty state.
	 *
	 * When set to dirty it should also mark it's parents as dirty so that the
	 * window easily test for it's dirty state.
	 * When set to not dirty it should also mark it's childeren as not dirty.
	 * (Obviously only for container classes).
	 */
	virtual void set_dirty(const bool dirty = true) 
	{ 
		dirty_ = dirty; 
		if(parent_ && dirty) parent_->set_dirty(true);
	}

	virtual bool is_dirty() const { return dirty_; }

protected:	
	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_layout_size(const tpoint& size) { layout_size_ = size; }
	const tpoint& layout_size() const { return layout_size_; }

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

	/**
	 * The best size for the control.
	 *
	 * When 0,0 the real best size is returned, but in the layout phase a
	 * wrapping or a scrollbar might change the best size for that widget. 
	 * This variable holds that best value.
	 */
	tpoint layout_size_;

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	/** 
	 * Debug helper to store last value of get_best_size().
	 *
	 * We're mutable so calls can stay const and this is disabled in
	 * production code.
	 */
	mutable tpoint last_best_size_;
#endif
};

/**
 * Returns the first parent of a widget with a certain type.
 *
 * @param widget                  The widget to get the parent from,
 * @param T                       The class of the widget to return.
 *
 * @returns                       The parent widget.
 */
template<class T> T* get_parent(twidget* widget)
{
	T* result;
	do {
		widget = widget->parent();
		result = dynamic_cast<T*>(widget);

	} while (widget && !result);
	
	assert(result);
	return result;
}

} // namespace gui2

#endif
