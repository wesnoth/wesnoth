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

#ifndef GUI_WIDGETS_PANE_HPP_INCLUDED
#define GUI_WIDGETS_PANE_HPP_INCLUDED

#include "gui/widgets/widget.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/placer.hpp"

#include "utils/boost_function_guarded.hpp"

#include <list>

typedef std::map<std::string, t_string> string_map;

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct tbuilder_pane;
} // namespace implementation

class tgrid;

class tpane : public twidget
{
	friend struct tpane_implementation;

public:
	struct titem
	{

		unsigned id;
		std::map<std::string, std::string> tags;

		tgrid* grid;
	};

	typedef boost::function<bool(const titem&, const titem&)> tcompare_functor;

	typedef boost::function<bool(const titem&)> tfilter_functor;

	/** @deprecated Use the second overload. */
	explicit tpane(const tbuilder_grid_ptr item_builder);

private:
	explicit tpane(const implementation::tbuilder_pane& builder);

public:
	static tpane* build(const implementation::tbuilder_pane& builder);

	/**
	 * Creates a new item.
	 */
	unsigned create_item(const std::map<std::string, string_map>& item_data,
						 const std::map<std::string, std::string>& tags);

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) override;

	/** See @ref twidget::layout_initialise. */
	virtual void layout_initialise(const bool full_initialisation) override;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override;

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

	/**
	 * Sorts the contents of the pane.
	 *
	 * @param compare_functor     The functor to use to sort the items.
	 */
	void sort(const tcompare_functor& compare_functor);

	/**
	 * Filters the contents of the pane.
	 *
	 * if the @p filter_functor returns @c true the item shown, else it's
	 * hidden.
	 *
	 * @param filter_functor      The functor to determine whether an item
	 *                            should be shown or hidden.
	 */
	void filter(const tfilter_functor& filter_functor);

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
	/** The items in the pane. */
	std::list<titem> items_;

	/** The builer for the items in the list. */
	tbuilder_grid_ptr item_builder_;

	/** The id generator for the items. */
	unsigned item_id_generator_;

	/** Helper to do the placement. */
	std::unique_ptr<tplacer_> placer_;

	/** Places the children on the pane. */
	void place_children();

	/**
	 * Moves the children on the pane.
	 *
	 * After certain operations, e.g. sorting the child widgets need to be
	 * placed again. This function does so, but avoids dirtying the widget so
	 * redrawing doesn't involve re-rendering the entire widget.
	 */
	void set_origin_children();

	/**
	 * Places or moves the children on the pane.
	 *
	 * If the child has its best size it's move else placed.
	 *
	 * @note It would probably be possible to merge all three placement
	 * routines into one and using a flag for what to do: place, set_origin or
	 * place_or_set_origin.
	 */
	void place_or_set_origin_children();

	/** Updates the placement for the child items. */
	void prepare_placement() const;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_request_placement(tdispatcher& dispatcher,
										  const event::tevent event,
										  bool& handled);
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_pane : public tbuilder_widget
{
	explicit tbuilder_pane(const config& cfg);

	twidget* build() const;

	twidget* build(const treplacements& replacements) const;

	tplacer_::tgrow_direction grow_direction;

	unsigned parallel_items;

	tbuilder_grid_ptr item_definition;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
