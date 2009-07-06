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

/**
 *  @file window.hpp
 *  This file contains the window object, this object is a top level container
 *  which has the event management as well.
 */

#ifndef GUI_WIDGETS_WINDOW_HPP_INCLUDED
#define GUI_WIDGETS_WINDOW_HPP_INCLUDED

#include "cursor.hpp"
#include "gui/auxiliary/formula.hpp"
#include "gui/widgets/event_handler.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/panel.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tooltip.hpp"

#include "events.hpp"
#include "SDL.h"

#include <string>
#include <boost/function.hpp>

class CVideo;

namespace gui2{

class tdialog;
class tdebug_layout_graph;

/**
 * base class of top level items, the only item
 * which needs to store the final canvase to draw on
 */
class twindow
	: public tpanel
	, public tevent_handler
	, public cursor::setter
{
	friend class tdebug_layout_graph;
	friend struct twindow_implementation;

	// Wants to use layout().
	friend class tmessage;
	friend class twml_message_;

public:

	twindow(CVideo& video,
		tformula<unsigned>x,
		tformula<unsigned>y,
		tformula<unsigned>w,
		tformula<unsigned>h,
		const bool automatic_placement,
		const unsigned horizontal_placement,
		const unsigned vertical_placement,
		const unsigned maximum_width,
		const unsigned maximum_height,
		const std::string& definition);

	~twindow();

	/**
	 * Update the size of the screen variables in settings.
	 *
	 * Before a window gets build the screen sizes need to be updates. This
	 * function does that. It's only done when no other window is active, if
	 * another window is active it already updates the sizes with it's resize
	 * event.
	 */
	static void update_screen_size();

	/**
	 * Returns the intance of a window.
	 *
	 * @param handle              The instance id of the window.
	 *
	 * @returns                   The window or NULL.
	 */
	static twindow* window_instance(const unsigned handle);

	/**
	 * Default return values.
	 *
	 * These values are named return values and most are assigned to a widget
	 * automatically when using a certain id for that widget. The automatic
	 * return values are always a negative number.
	 *
	 * Note this might be moved somewhere else since it will force people to
	 * include the button, while it should be and implementation detail for most
	 * callers.
	 */
	enum tretval {
		NONE = 0,                      /**<
										* Dialog is closed with no return
										* value, should be rare but eg a
										* message popup can do it.
										*/
		OK = -1,                       /**< Dialog is closed with ok button. */
		CANCEL = -2,                   /**<
										* Dialog is closed with the cancel
										* button.
										*/
		AUTO_CLOSE = -3                /**<
		                                * The dialog is closed automatically
		                                * since it's timeout has been
		                                * triggered.
		                                */
		};

	/** Gets the retval for the default buttons. */
	static tretval get_retval_by_id(const std::string& id);

	/**
	 * Shows the window.
	 *
	 * @param restore             Restore the screenarea the window was on
	 *                            after closing it?
	 * @param auto_close_time     The time in ms after which the window will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no quarantee about how fast it closes
	 *                            after the minimum.
	 *
	 * @returns                   The close code of the window, predefined
	 *                            values are listed in tretval.
	 */
	int show(const bool restore = true,
			const unsigned auto_close_timeout = 0);

	/**
	 * Draws the window.
	 *
	 * This routine draws the window if needed, it's called from the event
	 * handler. This is done by a drawing event. When a window is shown it
	 * manages an SDL timer which fires a drawing event every X milliseconds,
	 * that event calls this routine. Don't call it manually.
	 */
	void draw();

	/**
	 * Adds an item to the dirty_list_.
	 *
	 * @param call_stack          The list of widgets traversed to get to the
	 *                            dirty wiget.
	 */
	void add_to_dirty_list(const std::vector<twidget*>& call_stack)
	{
		dirty_list_.push_back(call_stack);
	}

	/** The status of the window. */
	enum tstatus{
		NEW,                      /**< The window is new and not yet shown. */
		SHOWING,                  /**< The window is being shown. */
		REQUEST_CLOSE,            /**< The window has been requested to be
		                           *   closed but still needs to evalueate the
								   *   request.
								   */
		CLOSED                    /**< The window has been closed. */
		};

	/**
	 * Requests to close the window.
	 *
	 * At the moment the request is always honoured but that might change in the
	 * future.
	 */
	void close() { status_ = REQUEST_CLOSE; }

	/**
	 * Resize event for the window.
	 *
	 * @param event_handler       The handler sending the event.
	 * @param new_width           The new width for the window.
	 * @param new_height          The new height for the window.
	 */
	void window_resize(tevent_handler& event_handler,
		const unsigned new_width, const unsigned new_height);

	/**
	 * Updates the size of the window.
	 *
	 * If the window has automatic placement set this function recacluates the
	 * window. To be used after creation and after modification or items which
	 * can have different sizes eg listboxes.
	 */
	void invalidate_layout() { need_layout_ = true; }

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event_handler, bool& handled,
		SDLKey key, SDLMod modifier, Uint16 unicode);

	/** Inherited from tevent_handler. */
	twindow& get_window() { return *this; }

	/** Inherited from tevent_handler. */
	const twindow& get_window() const { return *this; }

	/** Inherited from tevent_handler. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active)
		{ return tpanel::find_widget(coordinate, must_be_active); }

	/** Inherited from tevent_handler. */
	const twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) const
		{ return tpanel::find_widget(coordinate, must_be_active); }

	/** Inherited from twidget. */
	tdialog* dialog() { return owner_; }

	/** Inherited from tcontainer_. */
	twidget* find_widget(const std::string& id, const bool must_be_active)
		{ return tcontainer_::find_widget(id, must_be_active); }

	/** Inherited from tcontainer_. */
	const twidget* find_widget(const std::string& id,
			const bool must_be_active) const
		{ return tcontainer_::find_widget(id, must_be_active); }

	/**
	 * Register a widget that prevents easy closing.
	 *
	 * Duplicate registration are ignored. See easy_close_ for more info.
	 *
	 * @param id                  The id of the widget to register.
	 */
	void add_easy_close_blocker(const std::string& id);

	/**
	 * Unregister a widget the prevents easy closing.
	 *
	 * Removing a non registered id is allowed but will do nothing. See
	 * easy_close_ for more info.
	 *
	 * @param id                  The id of the widget to register.
	 */
	void remove_easy_close_blocker(const std::string& id);

	/**
	 * Does the window close easily?
	 *
	 * @returns                   Whether or not the window closes easily.
	 */
	bool does_easy_close() const
		{ return easy_close_ && !easy_close_disabled_; }

	/**
	 * Disable the escape key.
	 *
	 * This is added to block dialogs from being closed automatically.
	 *
	 * @todo this function should be merged with the hotkey support once
	 * that has been added.
	 */
	void set_escape_disabled(const bool escape_disabled)
		{ escape_disabled_ = escape_disabled; }

	/**
	 * Initializes a linked size group.
	 *
	 * Note at least one of fixed_width or fixed_height must be true.
	 *
	 * @param id                  The id of the group.
	 * @param fixed_width         Does the group have a fixed width?
	 * @param fixed_height        Does the group have a fixed height?
	 */
	void init_linked_size_group(const std::string& id,
			const bool fixed_width, const bool fixed_height);

	/**
	 * Adds a widget to a linked size group.
	 *
	 * The group needs to exist, which is done by calling
	 * init_linked_size_group. A widget may only be member of one group.
	 * @todo Untested if a new widget is added after showing the widgets.
	 *
	 * @param id                  The id of the group.
	 * @param widget              The widget to add to the group.
	 */
	void add_linked_widget(const std::string& id, twidget* widget);

	/**
	 * Removes a widget from a linked size group.
	 *
	 * The group needs to exist, which is done by calling
	 * init_linked_size_group. If the widget is no member of the group the
	 * function does nothing.
	 *
	 * @param id                  The id of the group.
	 * @param widget              The widget to remove from the group.
	 */
	void remove_linked_widget(const std::string& id, const twidget* widget);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/**
	 * Sets there return value of the window.
	 *
	 * @param retval              The return value for the window.
	 * @param close               Close the window after setting the value.
	 */
	void set_retval(const int retval, const bool close_window = true)
		{ retval_ = retval; if(close_window) close(); }

	void set_owner(tdialog* owner) { owner_ = owner; }

	void set_easy_close(const bool easy_close) { easy_close_ = easy_close; }

	static void set_sunset(const unsigned interval)
		{ sunset_ = interval ? interval : 5; }

	/**
	 * Set the callback that will be called in the event processing loop of
	 * show() right before the actual event processing.
	 */
	void set_event_loop_pre_callback(boost::function<void ()> cb) {
		event_loop_pre_cb_ = cb;
	}
private:

	/** Needed so we can change what's drawn on the screen. */
	CVideo& video_;

	/** The status of the window. */
	tstatus status_;

	// return value of the window, 0 default.
	int retval_;

	/** The dialog that owns the window. */
	tdialog* owner_;

	/**
	 * When set the form needs a full layout redraw cycle.
	 *
	 * This happens when either a widget changes it's size or visibility or
	 * the window is resized.
	 */
	bool need_layout_;

	/** Avoid drawing the window.  */
	bool suspend_drawing_;

	/**
	 * The first window shown is the toplevel window.
	 *
	 * The toplevel window is the one that starts and stops the drawing timer.
	 * It's set when the timer is 0 when the window is shown.
	 */
	bool top_level_;

	/** When the window closes this surface is used to undraw the window. */
	surface restorer_;

	/** Widget for the tooltip. */
	ttooltip tooltip_;

	/**
	 * Restorer for the tooltip area.
	 *
	 * @todo the current way of showing a tooltip is a kind of hack which
	 * should be polished post 1.6.
	 */
	surface tooltip_restorer_;

	/** Widget for the help popup FIXME should be thelp_popup. */
	ttooltip help_popup_;

	/** Do we wish to place the widget automatically? */
	const bool automatic_placement_;

	/**
	 * Sets the horizontal placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a tgrid placement flag.
	 */
	const unsigned horizontal_placement_;

	/**
	 * Sets the vertical placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a tgrid placement flag.
	 */
	const unsigned vertical_placement_;

	/** The maximum width if automatic_placement_ is true. */
	unsigned maximum_width_;

	/** The maximum height if automatic_placement_ is true. */
	unsigned maximum_height_;

	/** The formula to calulate the x value of the dialog. */
	tformula<unsigned>x_;

	/** The formula to calulate the y value of the dialog. */
	tformula<unsigned>y_;

	/** The formula to calulate the width of the dialog. */
	tformula<unsigned>w_;

	/** The formula to calulate the height of the dialog. */
	tformula<unsigned>h_;

	/**
	 * Do we want to have easy close behaviour?
	 *
	 * Easy closing means that whenever a mouse click is done the dialog will
	 * be closed. The widgets in the window may override this behaviour by
	 * registering themselves as blockers. This is tested by the function
	 * disable_easy_close().
	 */
	bool easy_close_;

	/** 
	 * Hold the state of the easy close disable state.
	 *
	 * This value is set on showing and remains valid for the entire time
	 * shown. This means the easy close can't be changed on run-time, which
	 * is good since an extra close button might be needed.
	 */
	bool easy_close_disabled_;

	/** Disable the escape key see our setter for more info. */
	bool escape_disabled_;

	/**
	 * Controls the sunset feature.
	 *
	 * If this value is not 0 it will darken the entire screen every
	 * sunset_th drawing request that nothing has been modified. It's a debug
	 * feature.
	 */
	static unsigned sunset_;

	/**
	 * Helper struct to force widgets the have the same size.
	 *
	 * Widget which are linked will get the same width and/or height. This
	 * can especialy be usefull for listboxes, but can also be used for other
	 * applications.
	 */
	struct tlinked_size
	{
		tlinked_size(const bool width = false, const bool height = false)
			: widgets()
			, width(width)
			, height(height)
		{
		}

		/** The widgets linked. */
		std::vector<twidget*> widgets;

		/** Link the widgets in the width? */
		bool width;

		/** Link the widgets in the height? */
		bool height;
	};

	/** List of the widgets, whose size are linked together. */
	std::map<std::string, tlinked_size> linked_size_;

	/**
	 * Layouts the window.
	 *
	 * This part does the pre and post processing for the actual layout
	 * algorithm.
	 *
	 * @see layout_algorihm for more information.
	 */
	void NEW_layout();

	/**
	 * Layouts the linked widgets.
	 *
	 * @see layout_algorihm for more information.
	 */
	void NEW_layout_linked_widgets();

	/** Inherited from tevent_handler. */
	void do_show_tooltip(const tpoint& location, const t_string& tooltip);

	/** Inherited from tevent_handler. */
	void do_remove_tooltip();

	/** Inherited from tevent_handler. */
	void do_show_help_popup(const tpoint& location, const t_string& help_popup);

	/** Inherited from tevent_handler. */
	void do_remove_help_popup()
		{ help_popup_.set_visible(twidget::HIDDEN); }

	/** Inherited from tevent_handler. */
	bool easy_close();

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
		{ static const std::string type = "window"; return type; }

	/**
	 * Inherited from tpanel.
	 *
	 * Don't call this function it's only asserts.
	 */
	void draw(surface& surface, const bool force = false,
			const bool invalidate_background = false);

	/**
	 * The list with dirty items in the window.
	 *
	 * When drawing only the widgets that are dirty are updated. The draw()
	 * function has more information about the dirty_list_.
	 */
	std::vector<std::vector<twidget*> > dirty_list_;

	/**
	 * The callback function called before each iteration in the main event
	 * processing loop.
	 */
	boost::function<void ()> event_loop_pre_cb_;

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	tdebug_layout_graph* debug_layout_;

public:

	/** wrapper for tdebug_layout_graph::generate_dot_file. */
	void generate_dot_file(
			const std::string& generator, const unsigned domain);
#else
	void generate_dot_file(const std::string&,
			const unsigned) {}
#endif
};

} // namespace gui2

#endif
