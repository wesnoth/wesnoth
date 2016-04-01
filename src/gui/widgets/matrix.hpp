/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_MATRIX_HPP_INCLUDED
#define GUI_WIDGETS_MATRIX_HPP_INCLUDED

#include "gui/widgets/control.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct tbuilder_matrix;
}

class tstate_default
{
public:
	tstate_default();

	void set_active(const bool active);

	bool get_active() const;

	unsigned get_state() const;

protected:
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

private:
	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;
};

template <class STATE>
class tcontrol_NEW : public tcontrol, public STATE
{
public:
	tcontrol_NEW(const implementation::tbuilder_control& builder,
				 const std::string& control_type)
		: tcontrol(builder, STATE::COUNT, control_type)

	{
	}

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) override
	{
		STATE::set_active(active);
	}

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const override
	{
		return STATE::get_active();
	}

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const override
	{
		return STATE::get_state();
	}
};

typedef tcontrol_NEW<tstate_default> tbase;

/** The matrix class. */
class tmatrix : public tbase
{
	friend class tdebug_layout_graph;

private:
	explicit tmatrix(const implementation::tbuilder_matrix& builder);

public:
	static tmatrix* build(const implementation::tbuilder_matrix& builder);

	/***** ***** ***** ***** Item handling. ***** ***** ****** *****/

	unsigned create_item(const std::map<std::string, string_map>& item_data,
						 const std::map<std::string, std::string>& tags);


	/***** ***** ***** ***** Inherited operations. ***** ***** ****** *****/

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) override;

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) override;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override;

	/** See @ref twidget::layout_children. */
	virtual void layout_children() override;

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) override;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) override;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const override;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const override;

	/***** ***** ***** ***** Forwarded to pane_. ***** ***** ****** *****/
	/**
	 * Sorts the contents of the pane.
	 *
	 * @param compare_functor     The functor to use to sort the items.
	 */
	void sort(const tpane::tcompare_functor& compare_functor)
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
	void filter(const tpane::tfilter_functor& filter_functor)
	{
		/********************** OUTLINE *******************/
		pane_->filter(filter_functor);
	}

private:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const override;

public:
	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/** See @ref twidget::create_walker. */
	virtual iterator::twalker_* create_walker() override;

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
	tgrid* grid(const unsigned id);

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
	const tgrid* grid(const unsigned id) const;

private:
	/** The grid containing our children. */
	tgrid content_;

	/**
	 * Contains the pane used for adding new items to the matrix.
	 *
	 * The pane is owned by a grid in the content layer.
	 */
	tpane* pane_;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct tmatrix_definition : public tcontrol_definition
{

	explicit tmatrix_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tbuilder_grid_ptr content;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_matrix : public tbuilder_control
{
	explicit tbuilder_matrix(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

	tscrollbar_container::tscrollbar_mode vertical_scrollbar_mode;
	tscrollbar_container::tscrollbar_mode horizontal_scrollbar_mode;

	tbuilder_grid_ptr builder_top;
	tbuilder_grid_ptr builder_bottom;

	tbuilder_grid_ptr builder_left;
	tbuilder_grid_ptr builder_right;

	tbuilder_widget_ptr builder_main;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
