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

#ifndef GUI_WIDGETS_CONTAINER_HPP_INCLUDED
#define GUI_WIDGETS_CONTAINER_HPP_INCLUDED

#include "gui/widgets/grid.hpp"
#include "gui/widgets/control.hpp"

namespace gui2 {

/**
 * A generic container base class.
 *
 * A container is a class build with multiple items either acting as one
 * widget.
 *
 */
class tcontainer_ : public tcontrol
{
	friend class tdebug_layout_graph;
public:
	tcontainer_(const unsigned canvas_count) :
		tcontrol(canvas_count),
		grid_()
	{
		grid_.set_parent(this);
	}

	/**
	 * Returns the size of the client area.
	 *
	 * The client area is the area available for widgets.
	 */
	virtual SDL_Rect get_client_rect() const { return get_rect(); }

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	void layout_init();
	void layout_init2(const bool full_initialization);

private:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from twidget. */
	bool can_wrap() const { return grid_.can_wrap() || twidget::can_wrap(); }

	/** Inherited from twidget. */
	void layout_wrap(const unsigned maximum_width);

	/**
	 * Inherited from twidget.
	 *
	 * Since we can't define a good default behaviour we force the inheriting
	 * classes to define this function. So inheriting classes act as one widget
	 * others as a collection of multiple objects.
	 */
	bool has_vertical_scrollbar() const
		{ return grid_.has_vertical_scrollbar(); }

	/** Inherited from twidget. */
	void layout_use_vertical_scrollbar(const unsigned maximum_height);

	/**
	 * Inherited from twidget.
	 *
	 * Since we can't define a good default behaviour we force the inheriting
	 * classes to define this function. So inheriting classes act as one widget
	 * others as a collection of multiple objects.
	 */
	bool has_horizontal_scrollbar() const
		{ return grid_.has_horizontal_scrollbar(); }

	/** Inherited from twidget. */
	void layout_use_horizontal_scrollbar(const unsigned maximum_width);

	/** Inherited from twidget. */
	void set_size(const tpoint& origin, const tpoint& size);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from twidget.*/
	bool has_widget(const twidget* widget) const
		{ return grid_.has_widget(widget); }

	/** Inherited from twidget. */
	void set_origin(const tpoint& origin);

	/** Inherited from twidget. */
	void set_visible_area(const SDL_Rect& area);

	/** Inherited from twidget. */
	void impl_draw_children(surface& frame_buffer)
		{ grid_.draw_children(frame_buffer); }

	/** Inherited from twidget. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack)
	{
		grid_.child_populate_dirty_list(caller, call_stack);
	}

	/** Inherited from tcontrol. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active)
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol. */
	const twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) const
		{ return grid_.find_widget(coordinate, must_be_active); }

	/** Inherited from tcontrol.*/
	twidget* find_widget(const std::string& id, const bool must_be_active)
	{
		twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active);
	}

	/** Inherited from tcontrol.*/
	const twidget* find_widget(const std::string& id, const bool must_be_active) const
	{
		const twidget* result = tcontrol::find_widget(id, must_be_active);
		return result ? result : grid_.find_widget(id, must_be_active);
	}

	/** Import overloaded versions. */
	using tcontrol::find_widget;

	/** Inherited from tcontrol. */
	void set_active(const bool active);

	/**
	 * Inherited from tcontrol.
	 *
	 * NOTE normally containers don't block, but their children may. But
	 * normally the state for the children is set as well so we don't need to
	 * delegate the request to our children.
	 */
	bool does_block_easy_close() const { return false; }

	/***** **** ***** ***** wrappers to the grid **** ********* *****/

	tgrid::iterator begin() { return grid_.begin(); }
	tgrid::iterator end() { return grid_.end(); }

	unsigned add_row(const unsigned count = 1)
		{ return grid_.add_row(count); }

	void set_rows(const unsigned rows) { grid_.set_rows(rows); }
	unsigned int get_rows() const { return grid_.get_rows(); }

	void set_cols(const unsigned cols) { grid_.set_cols(cols); }
	unsigned int get_cols() const { return grid_.get_cols(); }

	void set_rows_cols(const unsigned rows, const unsigned cols)
		{ grid_.set_rows_cols(rows, cols); }

	void set_child(twidget* widget, const unsigned row,
		const unsigned col, const unsigned flags, const unsigned border_size)
		{ grid_.set_child(widget, row, col, flags, border_size); }

	void set_row_grow_factor(const unsigned row, const unsigned factor)
		{ grid_.set_row_grow_factor(row, factor); }

	void set_col_grow_factor(const unsigned col, const unsigned factor)
		{ grid_.set_col_grow_factor(col, factor); }

protected:
	/***** ***** ***** setters / getters for members ***** ****** *****/

	const tgrid& grid() const { return grid_; }
	tgrid& grid() { return grid_; }

private:

	/** The grid which holds the child objects. */
	tgrid grid_;

	/** Returns the space used by the border. */
	virtual tpoint border_space() const { return tpoint(0, 0); }

	/**
	 * Helper for set_active.
	 *
	 * This function should set the control itself active. It's called by
	 * set_active if the state needs to change. The widget is set to dirty() by
	 * set_active so we only need to change the state.
	 */
	virtual void set_self_active(const bool active) = 0;
};

} // namespace gui2

#endif

