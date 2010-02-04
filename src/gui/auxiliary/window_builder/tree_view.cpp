/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/tree_view.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/tree_view.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/tree_view.hpp"
#include "wml_exception.hpp"

namespace gui2 {

namespace implementation {

tbuilder_tree_view::tbuilder_tree_view(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, indention_step_size(
			lexical_cast_default<unsigned>(cfg["indention_step_size"]))
	, nodes()
{

	foreach(const config &node, cfg.child_range("node")) {
		nodes.push_back(tnode(node));
	}

	/** @todo activate after the string freeze. */
#if 0
//	VALIDATE(!nodes.empty(), _("No nodes defined for a tree view."));
#else
	assert(!nodes.empty());
#endif
}

twidget* tbuilder_tree_view::build() const
{
	/*
	 *  TODO see how much we can move in the constructor instead of
	 *  builing in several steps.
	 */
	ttree_view *widget = new ttree_view(nodes);

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	widget->set_indention_step_size(indention_step_size);

	DBG_GUI_G << "Window builder: placed tree_view '"
		<< id << "' with defintion '"
		<< definition << "'.\n";

	boost::intrusive_ptr<const ttree_view_definition::tresolution> conf =
		boost::dynamic_pointer_cast
		<const ttree_view_definition::tresolution>(widget->config());
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

tbuilder_tree_view::tnode::tnode(const config& cfg)
	: id(cfg["id"])
	, builder(NULL)
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("node", "id"));
	/** @todo activate after the string freeze. */
#if 0
//	VALIDATE(id != "root",
			_("[node]id 'root' is reserved for the implentation."));
#else
	assert(id != "root");
#endif

	const config& node_definition = cfg.child("node_definition");

	/** @todo activate after the string freeze. */
#if 0
//	VALIDATE(node_definition, _("No node defined."));
#else
	assert(node_definition);
#endif

	builder = new tbuilder_grid(node_definition);
}

} // namespace implementation

} // namespace gui2

