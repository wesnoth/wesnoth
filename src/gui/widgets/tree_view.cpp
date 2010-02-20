/* $Id$ */
/*
   Copyright (C) 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/tree_view.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

ttree_view::ttree_view(const std::vector<tnode_definition>& node_definitions)
	: tscrollbar_container(2)
	, node_definitions_(node_definitions)
	, indention_step_size_(0)
	, need_layout_(false)
	, root_node_(new ttree_view_node(
		  "root"
		, node_definitions_
		, NULL
		, this
		, std::map<std::string, string_map>()))
	, selected_item_(NULL)
	, selection_change_callback_()
{
}

ttree_view_node& ttree_view::add_node(const std::string& id
		, const std::map<std::string /* widget id */, string_map>& data)
{
	return get_root_node().add_child(id, data);
}

void ttree_view::remove_node(ttree_view_node* node)
{
	assert(node && node != root_node_ && node->parent_);

	boost::ptr_vector<ttree_view_node>::iterator itor =
				  node->parent_->children_.begin();

	for( ; itor != node->parent_->children_.end(); ++itor) {
		if(&*itor == node) {
			break;
		}
	}

	assert(itor != node->parent_->children_.end());

	node->parent_->children_.erase(itor);

	if(get_size() == tpoint(0, 0)) {
		return;
	}

	/** @todo Test whether this resizing works properly. */
	if(content_resize_request()) {
		set_size(get_origin(), get_size());
	} else {
		twindow *window = get_window();
		assert(window);
		window->invalidate_layout();
	}
}

void ttree_view::child_populate_dirty_list(twindow& caller
		, const std::vector<twidget*>& call_stack)
{
	// Inherited.
	tscrollbar_container::child_populate_dirty_list(caller, call_stack);

	layout();

	assert(root_node_);
	root_node_->impl_populate_dirty_list(caller, call_stack);
}

bool ttree_view::empty() const
{
	return root_node_->empty();
}

void ttree_view::layout()
{
	if(need_layout_) {
		root_node_->set_size(indention_step_size_
			, get_origin()
			, root_node_->get_size().x);
		root_node_->set_visible_area(content_visible_area_);

		need_layout_ = false;
	}
}

void ttree_view::finalize_setup()
{
	// Inherited.
	tscrollbar_container::finalize_setup();

	assert(content_grid());
	content_grid()->set_rows_cols(1, 1);
	content_grid()->set_child(
			  root_node_
			, 0
			, 0
			, tgrid::VERTICAL_GROW_SEND_TO_CLIENT
				| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
			, 0);
}

const std::string& ttree_view::get_control_type() const
{
	static const std::string type = "tree_view";
	return type;
}

} // namespace gui2

