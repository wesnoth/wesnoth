/*
   Copyright (C) 2010 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/tree_view.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/tree_view.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/tree_view.hpp"
#include "utils/foreach.tpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_tree_view::tbuilder_tree_view(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, indention_step_size(cfg["indention_step_size"])
	, nodes()
{

	FOREACH(const AUTO & node, cfg.child_range("node"))
	{
		nodes.push_back(tnode(node));
	}

	VALIDATE(!nodes.empty(), _("No nodes defined for a tree view."));
}

twidget* tbuilder_tree_view::build() const
{
	/*
	 *  TODO see how much we can move in the constructor instead of
	 *  building in several steps.
	 */
	ttree_view* widget = new ttree_view(nodes);

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	widget->set_indention_step_size(indention_step_size);

	DBG_GUI_G << "Window builder: placed tree_view '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const ttree_view_definition::tresolution>
	conf = boost::
			dynamic_pointer_cast<const ttree_view_definition::tresolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

tbuilder_tree_view::tnode::tnode(const config& cfg)
	: id(cfg["id"]), builder(NULL)
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("node", "id"));

	VALIDATE(id != "root",
			 _("[node]id 'root' is reserved for the implementation."));

	const config& node_definition = cfg.child("node_definition");

	VALIDATE(node_definition, _("No node defined."));

	builder = new tbuilder_grid(node_definition);
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{tree_view_description}
 *
 *        A tree view is a control that holds several items of the same or
 *        different types. The items shown are called tree view nodes and when
 *        a node has children, these can be shown or hidden. Nodes that contain
 *        children need to provide a clickable button in order to fold or
 *        unfold the children.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_tree_view
 *
 * == Tree view ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="tree_view"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @macro = tree_view_description
 *
 * List with the tree view specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *
 *     indention_step_size & unsigned & 0 &
 *                                     The number of pixels every level of
 *                                     nodes is indented from the previous
 *                                     level. $
 *
 *     node & section &  &             The tree view can contain multiple node
 *                                     sections. This part needs more
 *                                     documentation. $
 * @end{table}
 * @begin{tag}{name="node"}{min=0}{max=-1}
 * @begin{table}{config}
 *     id & string & "" &  $
 * @end{table}
 * @begin{tag}{name="node_definition"}{min=0}{max=-1}{super="gui/window/resolution/grid"}
 * @begin{table}{config}
 *     return_value_id & string & "" &  $
 * @end{table}
 * @end{tag}{name="node_definition"}
 * @end{tag}{name="node"}
 * @end{tag}{name="tree_view"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 * NOTE more documentation and examples are needed.
 */ // TODO annotate node
