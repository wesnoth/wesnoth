/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SCROLLBAR_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_SCROLLBAR_CONTAINER_HPP_INCLUDED

#include "gui/auxiliary/notifiee.hpp"
#include "gui/widgets/container.hpp"
#include "gui/widgets/scrollbar.hpp"

namespace gui2
{

class tspacer;

namespace implementation
{
struct tbuilder_scroll_label;
struct tbuilder_scrollbar_panel;
}

/**
 * Base class for creating containers with one or two scrollbar(s).
 *
 * For now users can't instanciate this class directly and needs to use small
 * wrapper classes. Maybe in the future users can use the class directly.
 *
 * @todo events are not yet send to the content grid.
 */
class tscrollbar_container : public tcontainer_
{
	friend class tdebug_layout_graph;

	friend struct implementation::tbuilder_scroll_label;
	friend struct implementation::tbuilder_scrollbar_panel;
#ifndef GUI2_EXPERIMENTAL_LISTBOX
	friend class tlistbox;
#endif
	friend class ttree_view;
	friend struct tscrollbar_container_implementation;

public:
	explicit tscrollbar_container(const unsigned canvas_count);

	~tscrollbar_container()
	{
		delete content_grid_;
	}

	/** The way to handle the showing or hiding of the scrollbar. */
	enum tscrollbar_mode {
		always_visible, /**<
						 * The scrollbar is always shown, whether
						 * needed or not.
						 */
		always_invisible, /**<
						   * The scrollbar is never shown even not
						   * when needed. There's also no space
						   * reserved for the scrollbar.
						   */
		auto_visible, /**<
					   * The scrollbar is shown when the number of
					   * items is larger as the visible items. The
					   * space for the scrollbar is always
					   * reserved, just in case it's needed after
					   * the initial sizing (due to adding items).
					   */
		auto_visible_first_run /**<
								* Like auto_visible, but when not needed
								* upon the initial layout phase, the bars
								* are not shown and no space is reserved
								* for them. (The algorithm hides them by
								* default.
								*/
	};

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) OVERRIDE;

	/** See @ref twidget::request_reduce_height. */
	virtual void request_reduce_height(const unsigned maximum_height) OVERRIDE;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE;

	/**
	 * See @ref twidget::can_wrap.
	 *
	 * @note This function is called before the object is finalised.
	 */
	virtual bool can_wrap() const OVERRIDE;

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

public:
	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) OVERRIDE;

	/***** ***** ***** inherited ****** *****/

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** @note shouldn't be called after being shown in a dialog. */
	void set_vertical_scrollbar_mode(const tscrollbar_mode scrollbar_mode);
	tscrollbar_mode get_vertical_scrollbar_mode() const
	{
		return vertical_scrollbar_mode_;
	}

	/** @note shouldn't be called after being shown in a dialog. */
	void set_horizontal_scrollbar_mode(const tscrollbar_mode scrollbar_mode);
	tscrollbar_mode get_horizontal_scrollbar_mode() const
	{
		return horizontal_scrollbar_mode_;
	}

	tgrid* content_grid()
	{
		return content_grid_;
	}
	const tgrid* content_grid() const
	{
		return content_grid_;
	}

	const SDL_Rect& content_visible_area() const
	{
		return content_visible_area_;
	}

	/***** ***** ***** scrollbar helpers ***** ****** *****/

	/**
	 * Scrolls the vertical scrollbar.
	 *
	 * @param scroll              The position to scroll to.
	 */
	void scroll_vertical_scrollbar(const tscrollbar_::tscroll scroll);

	/**
	 * Scrolls the horizontal scrollbar.
	 *
	 * @param scroll              The position to scroll to.
	 */
	void scroll_horizontal_scrollbar(const tscrollbar_::tscroll scroll);

	/**
	 * Callback when the scrollbar moves (NOTE maybe only one callback needed).
	 * Maybe also make protected or private and add a friend.
	 */
	void vertical_scrollbar_moved()
	{
		scrollbar_moved();
	}

	void horizontal_scrollbar_moved()
	{
		scrollbar_moved();
	}

protected:
	/**
	 * Shows a certain part of the content.
	 *
	 * When the part to be shown is bigger as the visible viewport the top
	 * left of the wanted rect will be the top left of the viewport.
	 *
	 * @param rect                The rect which should be visible.
	 */
	void show_content_rect(const SDL_Rect& rect);

	/*
	 * The widget contains the following three grids.
	 *
	 * * _vertical_scrollbar_grid containing at least a widget named
	 *   _vertical_scrollbar
	 *
	 * * _horizontal_scrollbar_grid containing at least a widget named
	 *   _horizontal_scrollbar
	 *
	 * * _content_grid a grid which holds the contents of the area.
	 *
	 * NOTE maybe just hardcode these in the finalize phase...
	 *
	 */

	/**
	 * Sets the status of the scrollbar buttons.
	 *
	 * This is needed after the scrollbar moves so the status of the buttons
	 * will be active or inactive as needed.
	 */
	void set_scrollbar_button_status();

	/**
	 * Notification if the content of a child needs a resize.
	 *
	 * When a resize is required the container first can try to handle it
	 * itself. If it can't honor the request the function will call @ref
	 * twindow::invalidate_layout().
	 *
	 * @note Calling this function on a widget with size == (0, 0) results
	 * false but doesn't call invalidate_layout, the engine expects to be in
	 * build up phase with the layout already invalidated.
	 *
	 * @param force_sizing        If the contents fit do we want to force a
	 *                            resize? This is needed in the MP lobby since
	 *                            items might not be properly placed yet.
	 *                            (The listboxes with the player info need it.)
	 *
	 * @returns                   True if the resize is handled, false
	 *                            otherwise.
	 */
	bool content_resize_request(const bool force_sizing = false);

	/**
	 * Request from the content to modify the size of the container.
	 *
	 * When the wanted resize fails the function will call @ref
	 * twindow::invalidate_layout().
	 *
	 *
	 * @note Calling this function on a widget with size == (0, 0) results
	 * false but doesn't call invalidate_layout, the engine expects to be in
	 * build up phase with the layout already invalidated.
	 *
	 * @note If @ref twindow::get_need_layout() is true the function returns
	 * false and doesn't try to fit the contents since a layout phase will be
	 * triggered anyway.
	 *
	 * @note This function might replace the @ref content_resize_request above.
	 *
	 * @param width_modification  The wanted modification to the width:
	 *                            * negative values reduce width.
	 *                            * zero leave width as is.
	 *                            * positive values increase width.
	 * @param height_modification The wanted modification to the height:
	 *                            * negative values reduce height.
	 *                            * zero leave height as is.
	 *                            * positive values increase height.
	 *
	 * @returns                   True is wanted modification is accepted false
	 *                            otherwise.
	 */
	bool content_resize_request(const int width_modification,
								const int height_modification);

private:
	/**
	 * Helper for @ref content_resize_request.
	 *
	 * Handle the width modification.
	 */
	bool content_resize_width(const int width_modification);

	/**
	 * Helper for @ref content_resize_request.
	 *
	 * Handle the height modification.
	 */
	bool content_resize_height(const int height_modification);

protected:
	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/

	/**
	 * Home key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_home(SDLMod modifier, bool& handled);

	/**
	 * End key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_end(SDLMod modifier, bool& handled);

	/**
	 * Page up key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_page_up(SDLMod modifier, bool& handled);

	/**
	 * Page down key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_page_down(SDLMod modifier, bool& handled);


	/**
	 * Up arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_up_arrow(SDLMod modifier, bool& handled);

	/**
	 * Down arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_down_arrow(SDLMod modifier, bool& handled);

	/**
	 * Left arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_left_arrow(SDLMod modifier, bool& handled);

	/**
	 * Right arrow key pressed.
	 *
	 * @param modifier            The SDL keyboard modifier when the key was
	 *                            pressed.
	 * @param handled             If the function handles the key it should
	 *                            set handled to true else do not modify it.
	 *                            This is used in the keyboard event
	 *                            changing.
	 */
	virtual void handle_key_right_arrow(SDLMod modifier, bool& handled);

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate {
		ENABLED,
		DISABLED,
		COUNT
	};

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

	/**
	 * The mode of how to show the scrollbar.
	 *
	 * This value should only be modified before showing, doing it while
	 * showing results in UB.
	 */
	tscrollbar_mode vertical_scrollbar_mode_, horizontal_scrollbar_mode_;

	/** These are valid after finalize_setup(). */
	tgrid* vertical_scrollbar_grid_, *horizontal_scrollbar_grid_;

	/** These are valid after finalize_setup(). */
	tscrollbar_* vertical_scrollbar_, *horizontal_scrollbar_;

	/** The grid that holds the content. */
	tgrid* content_grid_;

	/** Dummy spacer to hold the contents location. */
	tspacer* content_;

	/**
	 * Cache for the visible area for the content.
	 *
	 * The visible area for the content needs to be updated when scrolling.
	 */
	SDL_Rect content_visible_area_;

	/** The builder needs to call us so we do our setup. */
	void finalize_setup(); // FIXME make protected

	/**
	 * Function for the subclasses to do their setup.
	 *
	 * This function is called at the end of finalize_setup().
	 */
	virtual void finalize_subclass()
	{
	}

	/** See @ref twidget::layout_children. */
	virtual void layout_children() OVERRIDE;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) OVERRIDE;

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE;

	/**
	 * Sets the size of the content grid.
	 *
	 * This function normally just updates the content grid but can be
	 * overridden by a subclass.
	 *
	 * @param origin              The origin for the content.
	 * @param size                The size of the content.
	 */
	virtual void set_content_size(const tpoint& origin, const tpoint& size);

	/** Helper function which needs to be called after the scollbar moved. */
	void scrollbar_moved();

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_sdl_key_down(const event::tevent event,
									 bool& handled,
									 const SDLKey key,
									 SDLMod modifier);

	void signal_handler_sdl_wheel_up(const event::tevent event, bool& handled);
	void signal_handler_sdl_wheel_down(const event::tevent event,
									   bool& handled);
	void signal_handler_sdl_wheel_left(const event::tevent event,
									   bool& handled);
	void signal_handler_sdl_wheel_right(const event::tevent event,
										bool& handled);
};

} // namespace gui2

#endif
