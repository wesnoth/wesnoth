/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/auxiliary/window_builder.hpp"

#include <boost/function.hpp>

#include <list>

namespace gui2 {

class tgrid;

class tpane
	: public twidget
{
public:

	struct titem
	{

		unsigned id;
		std::map<std::string, std::string> tags;

		tgrid* grid;
	};

	typedef
		boost::function<bool(const titem&, const titem&)>
		tcompare_functor;

	explicit tpane(const tbuilder_grid_ptr item_builder);

	/**
	 * Creates a new item.
	 *
	 * @note At the moment it's a simple label nothing fancy or controlable by
	 * the client code.
	 *
	 * @param label               The text on the label.
	 */
	unsigned create_item(
			  const std::map<std::string, string_map>& item_data
			, const std::map<std::string, std::string>& tags);

	/** Inherited from twidget. */
	void place(const tpoint& origin, const tpoint& size);

	/** Inherited from twidget. */
	void impl_draw_children(surface& frame_buffer, int x_offset, int y_offset);

	/** Inherited from twidget. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);


	/** Inherited from twidget. */
	void request_reduce_width(const unsigned maximum_width);

	/**
	 * Sorts the contents of the pane.
	 *
	 * @param compare_functor     The functor to use to sort the items.
	 */
	void sort(const tcompare_functor& compare_functor);

private:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;

public:
	/** Inherited from twidget. */
	bool disable_click_dismiss() const;

	/** Inherited from twidget. */
	virtual iterator::twalker_* create_walker();

private:

	/** The items in the pane. */
	std::list<titem> items_;

	/** The builer for the items in the list. */
	tbuilder_grid_ptr item_builder_;

	/** The id generator for the items. */
	unsigned item_id_generator_;

	/** Places the children on the pane. */
	void place_children();
};

} // namespace gui2

#endif
