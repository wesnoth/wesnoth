/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct builder_matrix;
}

class state_default
{
public:
	state_default();

	void set_active(const bool active);

	bool get_active() const;

	unsigned get_state() const;

protected:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
	};

private:
	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;
};

template <class STATE>
class control_NEW : public styled_widget, public STATE
{
public:
	control_NEW(const implementation::builder_styled_widget& builder,
				 const std::string& control_type)
		: styled_widget(builder, control_type)

	{
	}

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override
	{
		STATE::set_active(active);
	}

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override
	{
		return STATE::get_active();
	}

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override
	{
		return STATE::get_state();
	}
};

typedef control_NEW<state_default> tbase;

/** The matrix class. */
class matrix : public tbase
{
	friend class debug_layout_graph;

private:
	explicit matrix(const implementation::builder_matrix& builder);

public:
	static matrix* build(const implementation::builder_matrix& builder);

	/***** ***** ***** ***** Item handling. ***** ***** ****** *****/

	unsigned create_item(const std::map<std::string, string_map>& item_data,
						 const std::map<std::string, std::string>& tags);


	/***** ***** ***** ***** Inherited operations. ***** ***** ****** *****/

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::layout_initialize. */
	virtual void layout_initialize(const bool full_initialization) override;

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override;

	/** See @ref widget::layout_children. */
	virtual void layout_children() override;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** See @ref widget::find. */
	widget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string& id,
						const bool must_be_active) const override;

	/***** ***** ***** ***** Forwarded to pane_. ***** ***** ****** *****/
	/**
	 * Sorts the contents of the pane.
	 *
	 * @param compare_functor     The functor to use to sort the items.
	 */
	void sort(const pane::tcompare_functor& compare_functor)
	{
		/********************** OUTLINE *******************/
		pane_->sort(compare_functor);
	}

	/**
	 * Filters the contents of the pane.
	 *
	 * if the @p filter_functor returns @c true the item shown, else it's
	 * hidden.
	 *
	 * @param filter_functor      The functor to determine whether an item
	 *                            should be shown or hidden.
	 */
	void filter(const pane::tfilter_functor& filter_functor)
	{
		/********************** OUTLINE *******************/
		pane_->filter(filter_functor);
	}

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/** See @ref widget::create_walker. */
	virtual iteration::walker_base* create_walker() override;

	/**
	 * Returns a grid in the pane.
	 *
	 * @param id                  The id of the item whose grid to return. The
	 *                            id is the value returned by
	 *                            @ref create_item().
	 *
	 * @returns                   The wanted grid.
	 * @retval nullptr               The id isn't associated with an item.
	 */
	grid* get_grid(const unsigned id);

	/**
	 * Returns a grid in the pane.
	 *
	 * @param id                  The id of the item whose grid to return. The
	 *                            id is the value returned by
	 *                            @ref create_item().
	 *
	 * @returns                   The wanted grid.
	 * @retval nullptr               The id isn't associated with an item.
	 */
	const grid* get_grid(const unsigned id) const;

private:
	/** The grid containing our children. */
	grid content_;

	/**
	 * Contains the pane used for adding new items to the matrix.
	 *
	 * The pane is owned by a grid in the content layer.
	 */
	pane* pane_;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct matrix_definition : public styled_widget_definition
{

	explicit matrix_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr content;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_matrix : public builder_styled_widget
{
	explicit builder_matrix(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;

	builder_grid_ptr builder_top;
	builder_grid_ptr builder_bottom;

	builder_grid_ptr builder_left;
	builder_grid_ptr builder_right;

	builder_widget_ptr builder_main;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
