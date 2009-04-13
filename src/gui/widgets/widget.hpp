/* $Id$ */
/*
   Copyright (C) 2007 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_WIDGET_HPP_INCLUDED

#include "gui/widgets/event_executor.hpp"
#include "gui/widgets/helper.hpp"
#include "sdl_utils.hpp"
#include "wml_exception.hpp"

#include <boost/noncopyable.hpp>

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
class twidget
	: private boost::noncopyable
	, public virtual tevent_executor
{
	friend class tdebug_layout_graph;
	friend class twindow; // needed for modifying the layout_size.

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
		, visible_(VISIBLE)
		, drawing_action_(DRAWN)
		, clip_rect_()
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

	/***** ***** ***** ***** flags ***** ***** ***** *****/

	/** Visibility settings done by the user. */
	enum tvisible {
		/**
		 * The user set the widget visible, that means:
		 * * widget is visible.
		 * * find_widget always 'sees' the widget (the active flag is tested
		 *   later).
		 * * widget (if active) handles events (and sends events to children)
		 * * widget is drawn (and sends populate_dirty_list to children)
		 */
		VISIBLE,
		/**
		 * The user set the widget hidden, that means:
		 * * item is invisible but keeps its size.
		 * * find_widget 'sees' the widget if active is false.
		 * * item doesn't handle events (and doesn't send events to children).
		 * * item doesn't populate_dirty_list (nor does it send the request
		 *   to its children).
		 */
		HIDDEN,
		/**
		 * The user set the widget invisible, that means:
		 * * item is invisible and gridcell has size 0,0.
		 * * find_widget never 'sees' the widget.
		 * * item doesn't handle events (and doesn't send events to children).
		 * * item doesn't populate_dirty_list (nor does it send the request
		 *   to its children).
		 */
		INVISIBLE };

	/**
	 * Visibility set by the engine.
	 *
	 * This state only will be used if the widget is visible, depending on
	 * this state the widget might not be visible after all.
	 */
	enum tdrawing_action {
		/** The widget is fully visible and should redrawn when set dirty. */
		DRAWN,
		/**
		 * The widget is partly visible, in order to render it, it's clip
		 * rect needs to be used.
		 */
		PARTLY_DRAWN,
		/** The widget is not visible and should not be drawn if dirty. */
		NOT_DRAWN
	};

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

	/** 
	 * Initializes the layout phase.
	 *
	 * @deprecated Will be removed after the new layout algorithm is
	 * implemented.
	 */
	virtual void layout_init() { layout_size_ = tpoint(0,0); }

	/**
	 * Initializes the layout phase.
	 *
	 * @deprecated Will be removed after the new layout algorithm is
	 * implemented.
	 *
	 * @param full_initialization Reset the widget to its initial state. This
	 *                            flag is used to change the status in the
	 *                            first layout run, but keep the status when a
	 *                            relayout phase happens.
	 */
	virtual void layout_init2(const bool /*full_initialization*/)
		{ layout_size_ = tpoint(0,0); }

	/**
	 * Initializes the layout phase.
	 *
	 * Clears the initial best size for the widgets.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param full_initialization For widgets with scrollbars it hides them
	 *                            unless the mode is tscrollbar_mode::SHOW.
	 *                            For other widgets this flag is a NOP.
	 */
	virtual void NEW_layout_init(const bool full_initialization);

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

	/** Can the widget reduce its width. */
	virtual bool can_shrink_width() const { return false; }

	/**
	 * Shrinks the contents of the widget so it can fit the wanted width
	 *
	 * @todo implement this function properly at the moment it kind of works
	 * for a label but should also work for a button and maybe some other
	 * classes.
	 *
	 * @param maximum_width       The wanted maximum width of the widget.
	 *
	 * @pre                       can_shrink_width() == true.
	 */
	virtual void layout_shrink_width(const unsigned /*maximum_width*/)
		{ assert(can_shrink_width()); }

	/** Flags to decide how to try to fit the widget in the layout. */
	enum tfit_flags {
		  WRAP      = 1 /**< Try to wrap the widget. */
		, SCROLLBAR = 2 /**< Try to use scrollbars. */
		, SHRINK    = 4 /**< Try to shrink the widget. */
		, FORCE     = 7 /**< Try all of the above. */
	};

	/**
	 * Fits the layout in the wanted width.
	 *
	 * @param maximum_width       The maximum width of the window.
	 * @param flags               The flags for the fitting.
	 */
	virtual void layout_fit_width(const unsigned /*maximum_width*/,
			const tfit_flags /*flags*/)
	{
		//FIXME should be pure abstract but gives linker errors.
	}

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
			const bool must_be_active);

	/** The const version of find_widget. */
	virtual const twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) const;

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

	/** The const version of get_window(). */
	const twindow* get_window() const;

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

	unsigned get_width() const { return w_; }
	unsigned get_height() const { return h_; }

	/** Returns the size of the widget. */
	tpoint get_size() const { return tpoint(w_, h_); }

	/** Returns the screen origin of the widget. */
	tpoint get_origin() const { return tpoint(x_, y_); }

	/** Gets the sizes in one rect structure. */
	SDL_Rect get_rect() const
		{ return create_rect(get_origin(), get_size()); }

	/**
	 * Gets the dirty rect of the widget.
	 *
	 * Depending on the drawing action it returns the rect this widget dirties
	 * while redrawing.
	 *
	 * @returns                   The dirty rect.
	 */
	SDL_Rect get_dirty_rect() const;

	/**
	 * Sets the origin of the widget.
	 *
	 * This function can be used to move the widget without dirting it.
	 *
	 * @param origin              The new origin.
	 */
	virtual void set_origin(const tpoint& origin)
	{
		x_ = origin.x;
		y_ = origin.y;
	}

	// Setting the screen locations doesn't dirty the widget.
	void set_x(const int x) { x_ = x; }
	int get_x() const { return x_; }

	void set_y(const int y) { y_ = y; }
	int get_y() const { return y_; }

	void set_visible(const tvisible visible);
	tvisible get_visible() const { return visible_; }

	/** 
	 * Returns true if the widget is visible.
	 *
	 * @deprecated This function name and action is too confusing.
	 */
	bool is_visible() const { return visible_ == VISIBLE; }

	/** 
	 * Returns true if the widget is invisible.
	 *
	 * @deprecated This function name and action is too confusing.
	 */
	bool is_invisible() const { return visible_ == INVISIBLE; }

	tdrawing_action get_drawing_action() const { return drawing_action_; }

	/**
	 * Sets the visible area for a widget.
	 *
	 * This function sets the drawing_state_ and the clip_rect_.
	 *
	 * @param area                The visible area in screen coordinates.
	 */
	virtual void set_visible_area(const SDL_Rect& area);

	/**
	 * Sets the widgets dirty state.
	 *
	 * @todo test whether nobody redefines the function.
	 */
	void set_dirty(const bool dirty = true)
	{
		dirty_ = dirty;
	}

	/** Returns the dirty state for a widget, final function. */
	bool get_dirty() const { return dirty_; }

	/**
	 * Draws the background of a widget.
	 *
	 * Subclasses should override impl_draw_background instead of changing
	 * this function.
	 *
	 * @param frame_buffer        The surface to draw upon.
	 */
	void draw_background(surface& frame_buffer);

	/**
	 * Draws the children of a widget.
	 *
	 * Containers should draw their children when they get this request.
	 *
	 * Subclasses should override impl_draw_children instead of changing
	 * this function.
	 *
	 * @param frame_buffer        The surface to draw upon.
	 */
	void draw_children(surface& frame_buffer);

	/**
	 * Draws the foreground of the widgt.
	 *
	 * Some widgets eg panel and window have a back and foreground layer this
	 * function requests the drawing of the foreground.
	 *
	 * Subclasses should override impl_draw_foreground instead of changing
	 * this function.
	 *
	 * @param frame_buffer        The surface to draw upon.
	 */
	void draw_foreground(surface& frame_buffer);

	/**
	 * Adds a widget to the dirty list if it is dirty.
	 *
	 * See twindow::dirty_list_ for more info on the dirty list.
	 *
	 * If the widget is not dirty and has children it should add itself to the
	 * call_stack and call child_populate_dirty_list with the new call_stack.
	 *
	 * @param caller              The parent window, if dirty it should
	 *                            register itself to this window.
	 * @param call_stack          The callstack of widgets traversed to reach
	 *                            this function.
	 */
	void populate_dirty_list(twindow& caller,
			std::vector<twidget*>& call_stack);

	/**
	 * Tries to add all children of a container to the dirty list.
	 *
	 * @param caller              The parent window, if dirty it should
	 *                            register itself to this window.
	 * @param call_stack          The callstack of widgets traversed to reach
	 *                            this function.
	 */
	virtual void child_populate_dirty_list(twindow& /*caller*/,
			const std::vector<twidget*>& /*call_stack*/) {}
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

	/** The x coordinate of the widget in the screen. */
	int x_;

	/** The y coordinate of the widget in the screen. */
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

	/** Field for the status of the visibility. */
	tvisible visible_;

	/** Field for the action to do on a drawing request. */
	tdrawing_action drawing_action_;

	/** The clip rect is a widget is partly visible. */
	SDL_Rect clip_rect_;

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

	/** See draw_background(). */
	virtual void impl_draw_background(surface& /*frame_buffer*/) {}

	/** See draw_children. */
	virtual void impl_draw_children(surface& /*frame_buffer*/) {}

	/** See draw_foreground. */
	virtual void impl_draw_foreground(surface& /*frame_buffer*/) {}
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
