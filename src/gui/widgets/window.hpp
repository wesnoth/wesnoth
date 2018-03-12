/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  This file contains the window object, this object is a top level container
 *  which has the event management as well.
 */

#pragma once

#include "cursor.hpp"
#include "formula/callable.hpp"
#include "formula/function.hpp"
#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/event/handler.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/widgets/panel.hpp"
#include "gui/widgets/retval.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class CVideo;
class surface;
struct point;

namespace gui2
{

class widget;
namespace event { struct message; }

// ------------ WIDGET -----------{

namespace dialogs { class modal_dialog; }
class debug_layout_graph;
class pane;

namespace event
{
class distributor;
} // namespace event

/**
 * base class of top level items, the only item
 * which needs to store the final canvases to draw on
 */
class window : public panel, public cursor::setter
{
	friend class debug_layout_graph;
	friend window* build(const builder_window::window_resolution*);
	friend struct window_implementation;
	friend class invalidate_layout_blocker;
	friend class pane;

public:
	explicit window(const builder_window::window_resolution* definition);

	~window();

	/**
	 * Returns the instance of a window.
	 *
	 * @param handle              The instance id of the window.
	 *
	 * @returns                   The window or nullptr.
	 */
	static window* window_instance(const unsigned handle);

	/** Gets the retval for the default buttons. */
	static retval get_retval_by_id(const std::string& id);

	/**
	 * @todo Clean up the show functions.
	 *
	 * the show functions are a bit messy and can use a proper cleanup.
	 */

	/**
	 * Shows the window.
	 *
	 * @param restore             Restore the screenarea the window was on
	 *                            after closing it?
	 * @param auto_close_timeout  The time in ms after which the window will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no guarantee about how fast it closes
	 *                            after the minimum.
	 *
	 * @returns                   The close code of the window, predefined
	 *                            values are listed in retval.
	 */
	int show(const bool restore = true, const unsigned auto_close_timeout = 0);

	/**
	 * Shows the window as a tooltip.
	 *
	 * A tooltip can't be interacted with and is just shown.
	 *
	 * @todo implement @p auto_close_timeout.
	 *
	 * @p     auto_close_timeout  The time in ms after which the window will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no guarantee about how fast it closes
	 *                            after the minimum.
	 */
	void show_tooltip(/*const unsigned auto_close_timeout = 0*/);

	/**
	 * Shows the window non modal.
	 *
	 * A tooltip can be interacted with unlike the tooltip.
	 *
	 * @todo implement @p auto_close_timeout.
	 *
	 * @p     auto_close_timeout  The time in ms after which the window will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no guarantee about how fast it closes
	 *                            after the minimum.
	 */
	void show_non_modal(/*const unsigned auto_close_timeout = 0*/);

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
	 * Undraws the window.
	 */
	void undraw();

	/**
	 * Adds an item to the dirty_list_.
	 *
	 * @param call_stack          The list of widgets traversed to get to the
	 *                            dirty widget.
	 */
	void add_to_dirty_list(const std::vector<widget*>& call_stack)
	{
		dirty_list_.push_back(call_stack);
	}

	/** The status of the window. */
	enum status {
		NEW,		   /**< The window is new and not yet shown. */
		SHOWING,	   /**< The window is being shown. */
		REQUEST_CLOSE, /**< The window has been requested to be
						*   closed but still needs to evaluate the
						*   request.
						*/
		CLOSED /**< The window has been closed. */
	};

	/**
	 * Requests to close the window.
	 *
	 * At the moment the request is always honored but that might change in the
	 * future.
	 */
	void close()
	{
		status_ = REQUEST_CLOSE;
	}

	/**
	 * Helper class to block invalidate_layout.
	 *
	 * Some widgets can handling certain layout aspects without help. For
	 * example a listbox can handle hiding and showing rows without help but
	 * setting the visibility calls invalidate_layout(). When this blocker is
	 * Instantiated the call to invalidate_layout() becomes a nop.
	 *
	 * @note The class can't be used recursively.
	 */
	class invalidate_layout_blocker
	{
	public:
		invalidate_layout_blocker(window& window);
		~invalidate_layout_blocker();

	private:
		window& window_;
	};

	/** Is invalidate_layout blocked, see invalidate_layout_blocker. */
	bool invalidate_layout_blocked() const
	{
		return invalidate_layout_blocked_;
	}

	/**
	 * Updates the size of the window.
	 *
	 * If the window has automatic placement set this function recalculates the
	 * window. To be used after creation and after modification or items which
	 * can have different sizes eg listboxes.
	 */
	void invalidate_layout();

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** Inherited from widget. */
	dialogs::modal_dialog* dialog()
	{
		return owner_;
	}

	/** See @ref widget::find. */
	widget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string& id,
						const bool must_be_active) const override;

#if 0
	/** @todo Implement these functions. */
	/**
	 * Register a widget that prevents easy closing.
	 *
	 * Duplicate registration are ignored. See click_dismiss_ for more info.
	 *
	 * @param id                  The id of the widget to register.
	 */
	void add_click_dismiss_blocker(const std::string& id);

	/**
	 * Unregister a widget the prevents easy closing.
	 *
	 * Removing a non registered id is allowed but will do nothing. See
	 * click_dismiss_ for more info.
	 *
	 * @param id                  The id of the widget to register.
	 */
	void remove_click_dismiss_blocker(const std::string& id);
#endif

	/**
	 * Does the window close easily?
	 *
	 * The behavior can change at run-time, but that might cause oddities
	 * with the easy close button (when one is needed).
	 *
	 * @returns                   Whether or not the window closes easily.
	 */
	bool does_click_dismiss() const
	{
		return click_dismiss_ && !disable_click_dismiss();
	}

	/**
	 * Disable the enter key.
	 *
	 * This is added to block dialogs from being closed automatically.
	 *
	 * @todo this function should be merged with the hotkey support once
	 * that has been added.
	 */
	void set_enter_disabled(const bool enter_disabled)
	{
		enter_disabled_ = enter_disabled;
	}

	/**
	 * Disable the escape key.
	 *
	 * This is added to block dialogs from being closed automatically.
	 *
	 * @todo this function should be merged with the hotkey support once
	 * that has been added.
	 */
	void set_escape_disabled(const bool escape_disabled)
	{
		escape_disabled_ = escape_disabled;
	}

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
								const bool fixed_width,
								const bool fixed_height);

	/**
	 * Is the linked size group defined for this window?
	 *
	 * @param id                  The id of the group.
	 *
	 * @returns                   True if defined, false otherwise.
	 */
	bool has_linked_size_group(const std::string& id);

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
	void add_linked_widget(const std::string& id, widget* widget);

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
	void remove_linked_widget(const std::string& id, const widget* widget);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	CVideo& video()
	{
		return video_;
	}

	/**
	 * Sets there return value of the window.
	 *
	 * @param retval              The return value for the window.
	 * @param close_window        Close the window after setting the value.
	 */
	void set_retval(const int retval, const bool close_window = true)
	{
		retval_ = retval;
		if(close_window)
			close();
	}

	int get_retval()
	{
		return retval_;
	}

	void set_owner(dialogs::modal_dialog* owner)
	{
		owner_ = owner;
	}

	void set_click_dismiss(const bool click_dismiss)
	{
		click_dismiss_ = click_dismiss;
	}

	bool get_need_layout() const
	{
		return need_layout_;
	}

	void set_variable(const std::string& key, const wfl::variant& value)
	{
		variables_.add(key, value);
		set_is_dirty(true);
	}
	point get_linked_size(const std::string& linked_group_id) const
	{
		std::map<std::string, linked_size>::const_iterator it = linked_size_.find(linked_group_id);
		if(it != linked_size_.end()) {
			return point(it->second.width, it->second.height);
		}

		return point(-1, -1);
	}

	/**
	 * Sets the window's exit hook.
	 *
	 * A window will only close if this function returns true.
	 *
	 * @param func A function taking a window reference and returning a boolean result.
	 */
	void set_exit_hook(std::function<bool(window&)> func)
	{
		exit_hook_ = func;
	}

	void set_exit_hook_ok_only(std::function<bool(window&)> func)
	{
		exit_hook_ = [func](window& w)->bool { return w.get_retval() != OK || func(w); };
	}

	/**
	 * Sets a callback that will be called after the window is drawn next time.
	 * The callback is automatically removed after calling it once.
	 * Useful if you need to do something after the window is drawn for the first time
	 * and it's timing-sensitive (i.e. pre_show is too early).
	 */
	void set_callback_next_draw(std::function<void()> func)
	{
		callback_next_draw_ = func;
	}

	enum show_mode {
		none,
		modal,
		modeless,
		tooltip
	};

private:
	/** Needed so we can change what's drawn on the screen. */
	CVideo& video_;

	/** The status of the window. */
	status status_;

	/**
	 * The mode in which the window is shown.
	 *
	 * This is used to determine whether or not to remove the tip.
	 */
	show_mode show_mode_;

	// return value of the window, 0 default.
	int retval_;

	/** The dialog that owns the window. */
	dialogs::modal_dialog* owner_;

	/**
	 * When set the form needs a full layout redraw cycle.
	 *
	 * This happens when either a widget changes it's size or visibility or
	 * the window is resized.
	 */
	bool need_layout_;

	/** The variables of the canvas. */
	wfl::map_formula_callable variables_;

	/** Is invalidate_layout blocked, see invalidate_layout_blocker. */
	bool invalidate_layout_blocked_;

	/** Avoid drawing the window.  */
	bool suspend_drawing_;

	/** Whether the window should undraw the window using restorer_ */
	bool restore_;

	/** Whether the window has other windows behind it */
	bool is_toplevel_;

	/** When the window closes this surface is used to undraw the window. */
	surface restorer_;

	/** Do we wish to place the widget automatically? */
	const bool automatic_placement_;

	/**
	 * Sets the horizontal placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a grid placement flag.
	 */
	const unsigned horizontal_placement_;

	/**
	 * Sets the vertical placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a grid placement flag.
	 */
	const unsigned vertical_placement_;

	/** The maximum width if automatic_placement_ is true. */
	unsigned maximum_width_;

	/** The maximum height if automatic_placement_ is true. */
	unsigned maximum_height_;

	/** The formula to calculate the x value of the dialog. */
	typed_formula<unsigned> x_;

	/** The formula to calculate the y value of the dialog. */
	typed_formula<unsigned> y_;

	/** The formula to calculate the width of the dialog. */
	typed_formula<unsigned> w_;

	/** The formula to calculate the height of the dialog. */
	typed_formula<unsigned> h_;

	/** The formula to determine whether the size is good. */
	typed_formula<bool> reevaluate_best_size_;

	/** The formula definitions available for the calculation formulas. */
	wfl::function_symbol_table functions_;

	/** The settings for the tooltip. */
	builder_window::window_resolution::tooltip_info tooltip_;

	/** The settings for the helptip. */
	builder_window::window_resolution::tooltip_info helptip_;

	/**
	 * Do we want to have easy close behavior?
	 *
	 * Easy closing means that whenever a mouse click is done the dialog will
	 * be closed. The widgets in the window may override this behavior by
	 * registering themselves as blockers. This is tested by the function
	 * disable_click_dismiss().
	 *
	 * The handling of easy close is done in the window, in order to do so a
	 * window either needs a click_dismiss or an ok button. Both will be hidden
	 * when not needed and when needed first the ok is tried and then the
	 * click_dismiss button. this allows adding a click_dismiss button to the
	 * window definition and use the ok from the window instance.
	 *
	 * @todo After testing the click dismiss feature it should be documented in
	 * the wiki.
	 */
	bool click_dismiss_;

	/** Disable the enter key see our setter for more info. */
	bool enter_disabled_;

	/** Disable the escape key see our setter for more info. */
	bool escape_disabled_;

	/**
	 * Helper struct to force widgets the have the same size.
	 *
	 * Widget which are linked will get the same width and/or height. This
	 * can especially be useful for listboxes, but can also be used for other
	 * applications.
	 */
	struct linked_size
	{
		linked_size(const bool width = false, const bool height = false)
			: widgets(), width(width ? 0 : -1), height(height ? 0 : -1)
		{
		}

		/** The widgets linked. */
		std::vector<widget*> widgets;

		/** The current width of all widgets in the group, -1 if the width is not linked. */
		int width;

		/** The current height of all widgets in the group, -1 if the height is not linked. */
		int height;
	};

	/** List of the widgets, whose size are linked together. */
	std::map<std::string, linked_size> linked_size_;

	/** List of widgets in the tabbing order. */
	std::vector<widget*> tab_order;

	/**
	 * Layouts the window.
	 *
	 * This part does the pre and post processing for the actual layout
	 * algorithm.
	 *
	 * See @ref layout_algorithm for more information.
	 */
	void layout();

	/**
	 * Layouts the linked widgets.
	 *
	 * See @ref layout_algorithm for more information.
	 */
	void layout_linked_widgets();

	/**
	 * Handles a mouse click event for dismissing the dialog.
	 *
	 * @param mouse_button_mask   The SDL_BUTTON mask for the button used to
	 *                            dismiss the click. If the caller is from the
	 *                            keyboard code the value should be 0.
	 *
	 * @return                    Whether the event should be considered as
	 *                            handled.
	 */
	bool click_dismiss(const int mouse_button_mask);

	/**
	 * The state of the mouse button.
	 *
	 * When click dismissing a dialog in the past the DOWN event was used.
	 * This lead to a bug [1]. The obvious change was to switch to the UP
	 * event, this lead to another bug; the dialog was directly dismissed.
	 * Since the game map code uses the UP and DOWN event to select a unit
	 * there is no simple solution.
	 *
	 * Upon entry this value stores the mouse button state at entry. When a
	 * button is DOWN and goes UP that button does \em not trigger a dismissal
	 * of the dialog, instead that button's down state is removed from this
	 * variable. Therefore the next UP event does dismiss the dialog.
	 *
	 * [1] https://gna.org/bugs/index.php?18970
	 */
	int mouse_button_state_;

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/**
	 * The list with dirty items in the window.
	 *
	 * When drawing only the widgets that are dirty are updated. The draw()
	 * function has more information about the dirty_list_.
	 */
	std::vector<std::vector<widget*>> dirty_list_;

	/**
	 * In how many consecutive frames the window has changed. This is used to
	 * detect the situation where the title screen changes in every frame,
	 * forcing all other windows to redraw everything all the time.
	 */
	unsigned int consecutive_changed_frames_ = 0u;

	/** Schedules windows on top of us (if any) to redraw. */
	void redraw_windows_on_top() const;

	/**
	 * Finishes the initialization of the grid.
	 *
	 * @param content_grid        The new contents for the content grid.
	 */
	void finalize(const std::shared_ptr<builder_grid>& content_grid);

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	debug_layout_graph* debug_layout_;

public:
	/** wrapper for debug_layout_graph::generate_dot_file. */
	void generate_dot_file(const std::string& generator, const unsigned domain);

private:
#else
	void generate_dot_file(const std::string&, const unsigned)
	{
	}
#endif

	std::unique_ptr<event::distributor> event_distributor_;

public:
	/** Gets a reference to the window's distributor to allow some state peeking. */
	const event::distributor& get_distributor() const
	{
		return *event_distributor_;
	}

	/** Returns the dialog mode for this window. */
	show_mode mode() const
	{
		return show_mode_;
	}

	// mouse and keyboard_capture should be renamed and stored in the
	// dispatcher. Chaining probably should remain exclusive to windows.
	void mouse_capture(const bool capture = true);
	void keyboard_capture(widget* widget);

	/**
	 * Adds the widget to the keyboard chain.
	 *
	 * @todo rename to keyboard_add_to_chain.
	 * @param widget              The widget to add to the chain. The widget
	 *                            should be valid widget, which hasn't been
	 *                            added to the chain yet.
	 */
	void add_to_keyboard_chain(widget* widget);

	/**
	 * Remove the widget from the keyboard chain.
	 *
	 * @todo rename to keyboard_remove_from_chain.
	 *
	 * @param widget              The widget to be removed from the chain.
	 */
	void remove_from_keyboard_chain(widget* widget);

	/**
	 * Add the widget to the tabbing order
	 * @param widget              The widget to be added to the tabbing order
	 * @param at                  A hint for where to place the widget in the tabbing order
	 */
	void add_to_tab_order(widget* widget, int at = -1);

private:
	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_sdl_video_resize(const event::ui_event event,
										 bool& handled,
										 const point& new_size);

	/**
	 * The handler for the click dismiss mouse 'event'.
	 *
	 * @param event               See @ref event::dispatcher::fire.
	 * @param handled             See @ref event::dispatcher::fire.
	 * @param halt                See @ref event::dispatcher::fire.
	 * @param mouse_button_mask   Forwared to @ref click_dismiss.
	 */
	void signal_handler_click_dismiss(const event::ui_event event,
									  bool& handled,
									  bool& halt,
									  const int mouse_button_mask);

	void signal_handler_sdl_key_down(const event::ui_event event,
									 bool& handled,
									 const SDL_Keycode key,
									 const SDL_Keymod mod,
									 bool handle_tab);

	void signal_handler_message_show_tooltip(const event::ui_event event,
											 bool& handled,
											 event::message& message);

	void signal_handler_message_show_helptip(const event::ui_event event,
											 bool& handled,
											 event::message& message);

	void signal_handler_request_placement(const event::ui_event event,
										  bool& handled);

	void signal_handler_close_window();

	std::function<bool(window&)> exit_hook_;
	std::function<void()> callback_next_draw_;
};

// }---------- DEFINITION ---------{

struct window_definition : public styled_widget_definition
{
	explicit window_definition(const config& cfg);

	struct resolution : public panel_definition::resolution
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }------------ END --------------

} // namespace gui2
