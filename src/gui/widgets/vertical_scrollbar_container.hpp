/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define NEW_DRAW

#ifndef GUI_WIDGETS_VERTICAL_SCROLLBAR_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_VERTICAL_SCROLLBAR_CONTAINER_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

class tscrollbar_;

/** Base class for creating containers with a vertical scrollbar. */
class tvertical_scrollbar_container_ : public tcontainer_
{
	friend class tdebug_layout_graph;

	// Builders need to be able to finalize the object.
	friend struct tbuilder_listbox;
	friend struct tbuilder_scroll_label;

	// Callbacks can call update routines. Note these are not further declared
	// here only need external linkage to be friends.
	friend void callback_scrollbar_button(twidget*);
	friend void callback_scrollbar(twidget*);
public:

	tvertical_scrollbar_container_(const unsigned canvas_count);

	~tvertical_scrollbar_container_();

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

	/**
	 * Selects an entire row.
	 *
	 * @param row                 The row to (de)select.
	 * @param select              true select, false deselect.
	 *
	 * @returns                   false if deselecting wasn't allowed.
	 *                            true otherwise.
	 */
	virtual bool select_row(const unsigned /*row*/, const bool /*select*/ = true)
		{ return false; }

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tcontainer_. */
	void layout_init();

	/** Inherited from tcontainer_. */
	bool can_wrap() const { return content_can_wrap(); }

	/** Inherited from twidget (not tcontainer_). */
	void layout_wrap(const unsigned maximum_width);

	/** Inherited from twidget (not tcontainer_). */
	bool has_vertical_scrollbar() const { return scrollbar_mode_ != HIDE; }

	/** Inherited from tcontainer_. */
	void layout_use_vertical_scrollbar(const unsigned maximum_height);

private:
	/** Inherited from tcontainer_. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from tcontainer. */
	void set_size(const tpoint& origin, const tpoint& size);

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event, bool& handled,
		SDLKey key, SDLMod modifier, Uint16 unicode);
#ifndef NEW_DRAW
	/** Inherited from tcontainer. */
	void draw(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);
#else
	// FIXME look whether this widget will remain until then don't implement
	// the new functions.
#endif
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

	void set_callback_value_change(void (*callback) (twidget* caller))
		{ callback_value_change_ = callback; }

	void set_scrollbar_mode(const tscrollbar_mode scrollbar_mode);
	tscrollbar_mode get_scrollbar_mode() const { return scrollbar_mode_; }

protected:

	/**
	 * When the value of the selected item has been changed this function
	 * should be called.
	 */
	void value_changed();

	/**
	 * The widget contains named widgets:
	 * - _scrollbar_grid a grid containing all items regarding the scrollbar and
	 *   associated buttons etc.
	 * - _scrollbar a scrollbar itself.
	 * - _content_grid a grid containing the widgets in the container.
	 *   Subclasses may define extra named widgets in this container for their
	 *   own purposes.
	 */

	/**
	 * Returns the scrollbar grid.
	 *
	 * This always returns the grid, regardless of the mode (active or not).
	 *
	 * @param must_exist          If true the grid must exist and the
	 *                            function will fail if that's not the case. If
	 *                            true the pointer returned is always valid.
	 *
	 * @returns                   A pointer to the grid or NULL.
	 */
	tgrid* find_scrollbar_grid(const bool must_exist = true);

	/** The const version. */
	const tgrid* find_scrollbar_grid(const bool must_exist = true) const;


	/** See find_scrollbar_grid, but returns the scrollbar instead. */
	tscrollbar_* find_scrollbar(const bool must_exist = true);

	/** The const version. */
	const tscrollbar_* find_scrollbar(const bool must_exist = true) const;

	/** See find_scrollbar_grid, but returns the content grid instead. */
	tgrid* content_find_grid(const bool must_exist = true);
	const tgrid* content_find_grid(const bool must_exist = true) const;

	/**
	 * Sets the status of the scrollbar buttons.
	 *
	 * This is needed after the scrollbar moves so the status of the buttons
	 * will be active or inactive as needed.
	 */
	void set_scrollbar_button_status();

private:

	/**
	 * The mode of how to show the scrollbar.
	 *
	 * This value should only be modified before showing, doing it while
	 * showing results in UB.
	 */
	tscrollbar_mode scrollbar_mode_;

	tgrid* scrollbar_grid_;

	void show_scrollbar(const bool show);

	/**
	 * This callback is used when the selection is changed due to a user event.
	 * The name is not fully appropriate for the event but it's chosen to be
	 * generic.
	 */
	void (*callback_value_change_) (twidget* caller);

	/** The builder needs to call us so we can write in the proper callbacks. */
	void finalize_setup();

	/** After doing it's own finalization finalize_setup() calls us. */
	virtual void finalize() {}

	/** Callback when the scrollbar moves. */
	void scrollbar_moved(twidget* /*caller*/)
		{ set_scrollbar_button_status(); set_dirty(); }
	/**
	 * When an item scrollbar control button is clicked we need to move the
	 * scrollbar and update the list.
	 */
	void scrollbar_click(twidget* caller);

	/**
	 * Is the wanted item active?
	 *
	 * Some subclasses might have items that can be inactive, those shouldn't
	 * be selected. So add a test here.
	 *
	 * @param item                The item to check.
	 *
	 * @returns                   True active, false inactive.
	 */
	virtual bool get_item_active(const unsigned /*item*/) const { return true; }

	/** Returns the selected row. */
	virtual unsigned get_selected_row() const;

	/***** ***** (pure) virtuals for the subclasses ****** *****/

	/**
	 * Draws the content part of the widget.
	 *
	 * See draw_content for more info.
	 */
#ifndef NEW_DRAW
	virtual void draw_content(surface& surface, const bool force,
	        const bool invalidate_background) = 0;
#endif
	/**
	 * Finds a widget in the content area.
	 *
	 * See content_find_widget for more info.
	 */
	virtual twidget* content_find_widget(
		const tpoint& coordinate, const bool must_be_active) = 0;

	/** The const version. */
	virtual const twidget* content_find_widget(
		const tpoint& coordinate, const bool must_be_active) const = 0;

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/**
	 * For the content we "copy paste" the method used for the normal widgets
	 * for determining the best size and layout methods.
	 */

protected:
	/**
	 * Returns the best size for the content part.
	 *
	 * See get_best_size() for more info.
	 */
	tpoint content_get_best_size() const;

	/**
	 * Calculates the best size for the content part.
	 *
	 * See calculate_best_size() for more info.
	 */
	virtual tpoint content_calculate_best_size() const = 0;
private:

	/**
	 * Returns whether or not the content can wrap.
	 *
	 * See can_wrap() for more info.
	 */
	virtual bool content_can_wrap() const { return false; }

	/** Does the layout_wrap() for the content. */
	virtual void content_layout_wrap(const unsigned /*maximum_width*/) {}

	/** Does the use_vertical_scrollbar() for the content. */
	virtual void content_use_vertical_scrollbar(const unsigned maximum_height) = 0;

	/**
	 * Sets the size for the content.
	 *
	 * This is a notification after the size of the content grid has been set
	 * so the function only needs to update its state if applicable.
	 *
	 * @param rect                The new size of the content grid.
	 */
	virtual void content_set_size(const SDL_Rect& rect) = 0;

	/***** ***** ***** setters / getters for members ***** ****** *****/

protected:

	const tpoint& content_layout_size() const { return content_layout_size_; }
	void set_content_layout_size(const tpoint& size)
		{ content_layout_size_ = size; }

private:

	tpoint content_layout_size_;

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	mutable tpoint content_last_best_size_;
#endif
};



} // namespace gui2

#endif
