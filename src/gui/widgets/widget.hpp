/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/event/dispatcher.hpp"
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
	, public virtual event::tdispatcher
{
	friend class tdebug_layout_graph;
	friend class twindow; // needed for modifying the layout_size.

public:
	twidget();

	virtual ~twidget();

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
		 * * find_at always 'sees' the widget (the active flag is tested later).
		 * * widget (if active) handles events (and sends events to children)
		 * * widget is drawn (and sends populate_dirty_list to children)
		 */
		VISIBLE,
		/**
		 * The user set the widget hidden, that means:
		 * * item is invisible but keeps its size.
		 * * find_at 'sees' the widget if active is false.
		 * * item doesn't handle events (and doesn't send events to children).
		 * * item doesn't populate_dirty_list (nor does it send the request
		 *   to its children).
		 */
		HIDDEN,
		/**
		 * The user set the widget invisible, that means:
		 * * item is invisible and gridcell has size 0,0.
		 * * find_at never 'sees' the widget.
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
	 * Clears the initial best size for the widgets.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param full_initialization For widgets with scrollbars it hides them
	 *                            unless the mode is
	 *                            tscrollbar_mode::always_visible. For other
	 *                            widgets this flag is a NOP.
	 */
	virtual void layout_init(const bool full_initialization);

	/**
	 * Tries to reduce the width of a widget.
	 *
	 * This function tries to do it 'friendly' and only use scrollbars or
	 * wrapping of the widget.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	virtual void request_reduce_width(const unsigned maximum_width) = 0;

	/**
	 * Tries to reduce the width of a widget.
	 *
	 * This function does it more agressive and should only be used when
	 * using scrollbars and wrapping failed.
	 *
	 * @todo Make pure virtual.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_width       The wanted maximum width.
	 */
	virtual void demand_reduce_width(const unsigned /*maximum_width*/) {}

	/**
	 * Tries to reduce the height of a widget.
	 *
	 * This function tries to do it 'friendly' and only use scrollbars.
	 *
	 * @todo Make pure virtual.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	virtual void request_reduce_height(const unsigned /*maximum_height*/) {}

	/**
	 * Tries to reduce the height of a widget.
	 *
	 * This function does it more agressive and should only be used when
	 * using scrollbars failed.
	 *
	 * @todo Make pure virtual.
	 *
	 * @see @ref layout_algorihm for more information.
	 *
	 * @param maximum_height      The wanted maximum height.
	 */
	virtual void demand_reduce_height(const unsigned /*maximum_height*/) {}

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
	 * Places the widget.
	 *
	 * This function is normally called by a layout function to do the
	 * placement of a widget.
	 *
	 * @param origin              The position of top left of the widget.
	 * @param size                The size of the widget.
	 */
	virtual void place(const tpoint& origin, const tpoint& size);

	/**
	 * Sets the size of the widget.
	 *
	 * This version is meant to resize a widget, since the origin isn't
	 * modified. This can be used if a widget needs to change its size and the
	 * layout will be fixed later.
	 *
	 * @param size                The size of the widget.
	 */
	virtual void set_size(const tpoint& size);

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
	virtual twidget* find_at(const tpoint& coordinate,
			const bool must_be_active);

	/** The const version of find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
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
	virtual twidget* find(const std::string& id,
			const bool /*must_be_active*/)
		{ return id_ == id ? this : 0; }

	/** The const version of find. */
	virtual const twidget* find(const std::string& id,
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

	/***** ***** ***** Misc. ***** ****** *****/

	/** Does the widget disable easy close? */
	virtual bool disable_click_dismiss() const = 0;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	twidget* parent() { return parent_; }
	void set_parent(twidget* parent) { parent_ = parent; }

	const std::string& id() const { return id_; }
	void set_id(const std::string& id);

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

	tdrawing_action get_drawing_action() const;

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

#ifndef LOW_MEM
	void set_debug_border_mode(const unsigned debug_border_mode)
	{
		debug_border_mode_ = debug_border_mode;
	}

	void set_debug_border_color(const unsigned debug_border_color)
	{
		debug_border_color_ = debug_border_color;
	}
#endif

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
	 * Allows a widget to update its children.
	 *
	 * Before the window is populating the dirty list the widgets can update
	 * their content, which allows delayed initialization. This delayed
	 * initialization is only allowed if the widget resizes itself, not when
	 * being placed.
	 */
	virtual void layout_children() {}

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

private:

	/**
	 * Tries to add all children of a container to the dirty list.
	 *
	 * @note The function is private since everybody should call
	 * populate_dirty_list instead.
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

public:

	void set_linked_group(const std::string& linked_group)
	{
		linked_group_ = linked_group;
	}

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

	/**
	 * The linked group the widget belongs to.
	 *
	 * @todo For now the linked group is initialized when the layout of the
	 * widget is initialized. The best time to set it would be upon adding the
	 * widget in the window. Need to look whether it's possible in a clean way.
	 * Maybe a signal just prior to showing a window where the widget can do
	 * some of it's on things, would also be nice for widgets that need a
	 * finalizer function.
	 */
	std::string linked_group_;

#ifndef LOW_MEM
	/**
	 * Mode for drawing the debug border.
	 *
	 * The debug border is a helper border to determine where a widget is
	 * placed. It's only intended for debugging purposes.
	 *
	 * Possible values:
	 * - 0 no border
	 * - 1 single pixel border
	 * - 2 floodfilled rectangle
	 */
	unsigned debug_border_mode_;

	/** The color for the debug border. */
	unsigned debug_border_color_;

	void draw_debug_border(surface& frame_buffer);
#else
	void draw_debug_border(surface&) {}
#endif

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

	/** (Will be) inherited from event::tdispatcher. */
	virtual bool is_at(const tpoint& coordinate) const
	{
		return is_at(coordinate, true);
	}

	/**
	 * Is the coordinate inside our area.
	 *
	 * Helper for find_at so also looks at our visibility.
	 *
	 * @param coordinate          The coordinate which should be inside the
	 *                            widget.
	 * @param must_be_active      The widget should be active, not all widgets
	 *                            have an active flag, those who don't ignore
	 *                            flag.
	 *
	 * @returns                   Status.
	 */
	bool is_at(const tpoint& coordinate, const bool must_be_active) const;
};

/**
 * Returns the first parent of a widget with a certain type.
 *
 * @param widget                  The widget to get the parent from,
 * @tparam T                      The class of the widget to return.
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

/**
 * Gets a widget with the wanted id.
 *
 * This template function doesn't return a pointer to a generic widget but
 * returns the wanted type and tests for its existence if required.
 *
 * @param widget              The widget test or find a child with the wanted
 *                            id.
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
T* find_widget(typename tconst_duplicator<T, twidget>::type* widget
		, const std::string& id
		, const bool must_be_active
		, const bool must_exist)
{
	T* result =
		dynamic_cast<T*>(widget->find(id, must_be_active));
	VALIDATE(!must_exist || result, missing_widget(id));

	return result;
}

/**
 * Gets a widget with the wanted id.
 *
 * This template function doesn't return a reference to a generic widget but
 * returns a reference to the wanted type
 *
 * @param widget              The widget test or find a child with the wanted
 *                            id.
 * @param id                  The id of the widget to find.
 * @param must_be_active      The widget should be active, not all widgets
 *                            have an active flag, those who don't ignore
 *                            flag.
 *
 * @returns                   The widget with the id.
 */
template<class T>
T& find_widget(typename tconst_duplicator<T, twidget>::type* widget
		, const std::string& id
		, const bool must_be_active)
{
	return *find_widget<T>(widget, id, must_be_active, true);
}

} // namespace gui2

/**
 * Registers a widget.
 *
 * Call this function to register a widget. Use this macro in the
 * implementation, inside the gui2 namespace.
 *
 * @see @ref load_widget_definitions for more information.
 *
 * @note When the type is tfoo_definition, the id "foo" and no special key best
 * use RESISTER_WIDGET(foo) instead.
 *
 * @param type                    Class type of the window to register.
 * @param id                      Id of the widget
 * @param key                     The id to load if differs from id.
 */
#define REGISTER_WIDGET3(                                                  \
		  type                                                             \
		, id                                                               \
		, key)                                                             \
namespace {                                                                \
                                                                           \
	namespace ns_##type {                                                  \
                                                                           \
		struct tregister_helper {                                          \
			tregister_helper()                                             \
			{                                                              \
				register_widget(#id, boost::bind(                          \
						  load_widget_definitions<type>                    \
						, _1                                               \
						, _2                                               \
						, _3                                               \
						, key));                                           \
                                                                           \
				register_builder_widget(#id, boost::bind(                  \
							  build_widget<implementation::tbuilder_##id>  \
							, _1));                                        \
			}                                                              \
		};                                                                 \
                                                                           \
		static tregister_helper register_helper;                           \
	}                                                                      \
}

/**
 * Wrapper for REGISTER_WIDGET3.
 *
 * "Calls" REGISTER_WINDOW3(tid_definition, id, _4)
 */
#define REGISTER_WIDGET(id) REGISTER_WIDGET3(t##id##_definition, id, _4)

#endif

