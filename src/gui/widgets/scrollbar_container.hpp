/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_WIDGETS_SCROLLBAR_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_SCROLLBAR_CONTAINER_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

class tscrollbar_;
class tspacer;

/**
 * Base class for creating containers with one or two scrollbar(s).
 *
 * For now users can't instanciate this class directly and needs to use small
 * wrapper classes. Maybe in the future users can use the class directly.
 *
 * @todo events are not yet send to the content grid.
 */
class tscrollbar_container
	: public tcontainer_
{
	friend class tdebug_layout_graph;

	friend struct tbuilder_scroll_label;
	friend class tlistbox;

public:

	tscrollbar_container(const unsigned canvas_count);

	~tscrollbar_container() { delete content_grid_; }

	/** The way to handle the showing or hiding of the scrollbar. */
	enum tscrollbar_mode {
		SHOW,                     /**<
								   * The scrollbar is always shown, whether
								   * needed or not.
								   */
		HIDE,                     /**<
								   * The scrollbar is never shown even not when
								   * needed. There's also no space reserved for
								   * the scrollbar.
								   */
		SHOW_WHEN_NEEDED          /**<
								   * The scrollbar is shown when the number of
								   * items is larger as the visible items. The
								   * space for the scrollbar is always
								   * reserved, just in case it's needed after
								   * the initial sizing (due to adding items).
								   */
	};

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tcontainer_. */
	void layout_init();

	/** Inherited from tcontainer_. */
	bool can_wrap() const
	{
		// Note this function is called before the object is finalized.
		return content_grid_
			? content_grid_->can_wrap()
			: false;
	}

	/** Inherited from twidget (not tcontainer_). */
	void layout_wrap(const unsigned maximum_width);

	/** Inherited from twidget (not tcontainer_). */
	bool has_vertical_scrollbar() const
		{ return vertical_scrollbar_mode_ != HIDE; }

	/** Inherited from tcontainer_. */
	void layout_use_vertical_scrollbar(const unsigned maximum_height);

	/** Inherited from twidget (not tcontainer_). */
	bool has_horizontal_scrollbar() const
		{ return horizontal_scrollbar_mode_ != HIDE; }

	/** Inherited from tcontainer_. */
	void layout_use_horizontal_scrollbar(const unsigned maximum_width);

private:
	/** Inherited from tcontainer_. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from tcontainer_. */
	void set_size(const tpoint& origin, const tpoint& size);

	/** Inherited from tcontainer_. */
	void set_origin(const tpoint& origin);

	/** Inherited from tcontainer_. */
	void set_visible_area(const SDL_Rect& area);

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event,
		bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode);

	/** Inherited from tcontainer_. */
	bool get_active() const { return state_ != DISABLED; }

	/** Inherited from tcontainer_. */
	unsigned get_state() const { return state_; }

	/** Inherited from tcontainer_. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active);

	/** Inherited from tcontainer_. */
	const twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) const;

	/** Import overloaded versions. */
	using tcontainer_::find_widget;

	/** Inherited from tcontainer_. */
	bool does_block_easy_close() const;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_vertical_scrollbar_mode(const tscrollbar_mode scrollbar_mode);
	tscrollbar_mode get_vertical_scrollbar_mode() const
		{ return vertical_scrollbar_mode_; }

	void set_horizontal_scrollbar_mode(const tscrollbar_mode scrollbar_mode);
	tscrollbar_mode get_horizontal_scrollbar_mode() const
		{ return horizontal_scrollbar_mode_; }

	void vertical_scrollbar_click(twidget* caller);
	void horizontal_scrollbar_click(twidget* caller);

	tgrid *content_grid() { return content_grid_; }
	const tgrid *content_grid() const { return content_grid_; }

	/**
	 * Callback when the scrollbar moves (NOTE maybe only one callback needed).
	 * Maybe also make protected or private and add a friend.
	 */
	void vertical_scrollbar_moved(twidget* /*caller*/)
		{ scrollbar_moved(); }

	void horizontal_scrollbar_moved(twidget* /*caller*/)
		{ scrollbar_moved(); }

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
	enum tstate { ENABLED, DISABLED, COUNT };

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
	tscrollbar_mode
		vertical_scrollbar_mode_,
		horizontal_scrollbar_mode_;

	/** These are valid after finalize_setup(). */
	tgrid
		*vertical_scrollbar_grid_,
		*horizontal_scrollbar_grid_;

	/** These are valid after finalize_setup(). */
	tscrollbar_
		*vertical_scrollbar_,
		*horizontal_scrollbar_;

	/** The grid that holds the content. */
	tgrid *content_grid_;

	/** Dummy spacer to hold the contents location. */
	tspacer *content_;

	/**
	 * Cache for the visible area for the content.
	 *
	 * The visible area for the content needs to be updated when scrolling.
	 */
	SDL_Rect content_visible_area_;

	/** The builder needs to call us so we do our setup. */
	void finalize_setup();

	/**
	 * Function for the subclasses to do their setup.
	 *
	 * This function is called at the end of finalize_setup().
	 */
	virtual void finalize_subclass() {}

	/** Inherited from tcontainer_. */
	void impl_draw_children(surface& frame_buffer);

	/** Inherited from tcontainer_. */
	void child_populate_dirty_list(twindow& caller,
		const std::vector<twidget*>& call_stack);

	/** Sets the visible state of the vertical scrollbar. */
	void show_vertical_scrollbar();

	/** Sets the visible state of the horizontal scrollbar. */
	void show_horizontal_scrollbar();

	/** Helper function which needs to be called after the scollbar moved. */
	void scrollbar_moved();

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const
	{
		static const std::string type = "scrollbar_container";
		return type;
	}

};

} // namespace gui2

#endif

