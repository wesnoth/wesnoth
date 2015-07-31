/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_TREE_VIEW_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_TREE_VIEW_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include "gui/widgets/scrollbar_container.hpp"

namespace gui2
{

namespace implementation
{

struct tbuilder_tree_view : public tbuilder_control
{
	explicit tbuilder_tree_view(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

	tscrollbar_container::tscrollbar_mode vertical_scrollbar_mode;
	tscrollbar_container::tscrollbar_mode horizontal_scrollbar_mode;

	unsigned indention_step_size;

	struct tnode
	{
		explicit tnode(const config& cfg);

		std::string id;
		tbuilder_grid_ptr builder;
	};

	/**
	 * The types of nodes in the tree view.
	 *
	 * Since we expect the amount of nodes to remain low it's stored in a
	 * vector and not in a map.
	 */
	std::vector<tnode> nodes;

	/*
	 * NOTE this class doesn't have a data section, so it can only be filled
	 * with data by the engine. I think this poses no limit on the usage since
	 * I don't foresee that somebody wants to pre-fill a tree view. If the need
	 * arises the data part can be added.
	 */
};

} // namespace implementation

} // namespace gui2

#endif
