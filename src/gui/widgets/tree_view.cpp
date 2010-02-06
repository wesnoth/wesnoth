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
#include "gui/widgets/control.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"

#include <boost/bind.hpp>

#define LOG_NODE_SCOPE_HEADER \
		get_control_type() + " [" + parent_widget_->id() + "] " + __func__
#define LOG_NODE_HEADER LOG_NODE_SCOPE_HEADER + ':'

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

#undef DBG_GUI_L
#define DBG_GUI_L std::cerr

namespace gui2 {

ttree_view::tnode::tnode(const std::string& id
		, const std::vector<tnode_definition>& node_definitions
		, tnode* parent
		, ttree_view* parent_widget
		, const std::map<std::string /* widget id */, string_map>& data
		)
	: parent_(parent)
	, parent_widget_(parent_widget) // need to store? also used in set_parent
	, grid_()
	, children_()
	, node_definitions_(node_definitions)
	, icon_(NULL)
	, label_(NULL)
{
	set_parent(parent_widget);
	grid_.set_parent(this);
	if(id != "root") {
		foreach(const tnode_definition& node_definition, node_definitions_) {
			if(node_definition.id == id) {
				node_definition.builder->build(&grid_);
				init_grid(&grid_, data);

				icon_ = find_widget<ttoggle_button>(
						  &grid_
						, "tree_view_node_icon"
						, false
						, false);

				if(icon_) {
					icon_->set_visible(twidget::HIDDEN);
					icon_->connect_signal<event::LEFT_BUTTON_CLICK>(
							boost::bind(&ttree_view::tnode::
								signal_handler_left_button_click
								, this, _2));

				}

				if(parent_ && parent_->icon_) {
					parent_->icon_->set_visible(twidget::VISIBLE);
				}

				twidget& widget = find_widget<twidget>(
						  &grid_
						, "tree_view_node_label"
						, false);

				label_ = dynamic_cast<tselectable_*>(&widget);
				if(label_) {
					widget.connect_signal<event::LEFT_BUTTON_CLICK>(
							  boost::bind(&ttree_view::tnode::
								signal_handler_label_left_button_click
								, this, _2, _3, _4)
							, event::tdispatcher::front_child);
				}

				return;
			}
		}

		/** @todo enable after 1.9. */
#if 0
		// FIXME add node id
//		VALIDATE(false, _("Unknown builder id for tree view node."));
#else
		assert(false);
#endif

	}
}

ttree_view::tnode& ttree_view::tnode::add_child(
		  const std::string& id
		, const std::map<std::string /* widget id */, string_map>& data
		, const int)
{
	children_.push_back(new tnode(
				  id
				, node_definitions_
				, this
				, parent_widget_
				, data));
	return children_.back();
}

ttree_view::tnode& ttree_view::tnode::parent()
{
	assert(!is_root_node());

	return *parent_;
}

bool ttree_view::tnode::is_folded() const
{
	return icon_ && icon_->get_value();
}

void ttree_view::tnode::fold(const bool /*recursive*/)
{
	// FIXME set state

	parent_widget_->set_size(
			  parent_widget_->get_origin()
			, parent_widget_->get_size());
}

void ttree_view::tnode::unfold(const texpand_mode /*mode*/)
{
	// FIXME set state

	parent_widget_->set_size(
			  parent_widget_->get_origin()
			, parent_widget_->get_size());
}

struct ttree_view_node_implementation
{

	template<class W>
	static W* find_at(
			  typename tconst_duplicator<W, ttree_view::tnode>::type&
				tree_view_node
			, const tpoint& coordinate, const bool must_be_active)
	{
		std::cerr
				<< " Checking at " << coordinate
				<< " must be active " << must_be_active
				<< ".\n";

		if(W* widget =
				tree_view_node.grid_.find_at(coordinate, must_be_active)) {

			std::cerr
					<< " found the widget in our grid, id '" << widget->id()
					<< "'.\n";

			return widget;
		}

		if(tree_view_node.is_folded()) {
			std::cerr << " folded found nothing.\n";
			return NULL;
		}

		typedef typename tconst_duplicator<W, ttree_view::tnode>::type thack;
		foreach(thack& node, tree_view_node.children_) {
			if(W* widget = node./*grid_.*/find_at(coordinate, must_be_active)) {
				std::cerr
						<< " found the widget in a child grid, id '"
						<< widget->id() << "'.\n";
				return widget;
			}
		}

		std::cerr << " found nothing.\n";
		return NULL;
	}
};

twidget* ttree_view::tnode::find_at(
		  const tpoint& coordinate
		, const bool must_be_active)
{
	return ttree_view_node_implementation::find_at<twidget>(
			*this, coordinate, must_be_active);
}

const twidget* ttree_view::tnode::find_at(
		  const tpoint& coordinate
		, const bool must_be_active) const
{
	return ttree_view_node_implementation::find_at<const twidget>(
			*this, coordinate, must_be_active);
}

void ttree_view::tnode::impl_populate_dirty_list(twindow& caller
		, const std::vector<twidget*>& call_stack)
{
	std::vector<twidget*> child_call_stack = call_stack;
	grid_.populate_dirty_list(caller, child_call_stack);

	if(is_folded()) {
		return;
	}

	foreach(tnode& node, children_) {
		std::vector<twidget*> child_call_stack = call_stack;
		node.impl_populate_dirty_list(caller, child_call_stack);
	}
}

tpoint ttree_view::tnode::calculate_best_size(const int indention_level
		, const unsigned indention_step_size) const
{
	log_scope2(log_gui_layout, LOG_NODE_SCOPE_HEADER);

	tpoint best_size = grid_.get_best_size();
	if(indention_level > 0) {
		best_size.x += indention_level * indention_step_size;
	}

	if(is_folded()) {

		DBG_GUI_L << LOG_NODE_HEADER
				<< " Folded grid return own best size " << best_size << ".\n";
		return best_size;
	}

	DBG_GUI_L << LOG_NODE_HEADER << " own grid best size " << best_size << ".\n";

	foreach(const tnode& node, children_) {

		if(node.grid_.get_visible() == twidget::INVISIBLE) {
			continue;
		}

		const tpoint node_size = node.calculate_best_size(indention_level + 1,
				indention_step_size);

		best_size.y += node_size.y;
		best_size.x = std::max(best_size.x, node_size.x);
	}

	DBG_GUI_L << LOG_NODE_HEADER << " result " << best_size << ".\n";
	return best_size;
}

void ttree_view::tnode::set_origin(const tpoint& origin)
{
	// Inherited.
	twidget::set_origin(origin);

	set_size(40, origin);
}

void ttree_view::tnode::set_size(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	twidget::set_size(origin, size);

	set_size(40, origin);
}

unsigned ttree_view::tnode::set_size(
	  const unsigned indention_step_size
	, tpoint origin)
{
	log_scope2(log_gui_layout, LOG_NODE_SCOPE_HEADER);
	DBG_GUI_L << LOG_NODE_HEADER << " origin " << origin << ".\n";

	const unsigned offset = origin.y;
	const tpoint best_size = grid_.get_best_size();
	grid_.set_size(origin, best_size);

	if(!is_root_node()) {
		origin.x += indention_step_size;
	}
	origin.y += best_size.y;

	if(is_folded()) {
		DBG_GUI_L << LOG_NODE_HEADER << " folded node done.\n";
		return origin.y - offset;
	}

	DBG_GUI_L << LOG_NODE_HEADER << " set children.\n";
	foreach(tnode& node, children_) {
		origin.y += node.set_size(indention_step_size, origin);
	}

	DBG_GUI_L << LOG_NODE_HEADER << " result " << ( origin.y - offset) << ".\n";
	return origin.y - offset;
}

void ttree_view::tnode::set_visible_area(const SDL_Rect& area)
{
	log_scope2(log_gui_layout, LOG_NODE_SCOPE_HEADER);
	DBG_GUI_L << LOG_NODE_HEADER << " area " << area << ".\n";
	grid_.set_visible_area(area);

	if(is_folded()) {
		DBG_GUI_L << LOG_NODE_HEADER << " folded node done.\n";
		return;
	}

	foreach(tnode& node, children_) {
		node.set_visible_area(area);
	}
}

void ttree_view::tnode::impl_draw_children(surface& frame_buffer)
{
	grid_.draw_children(frame_buffer);

	if(is_folded()) {
		return;
	}

	foreach(tnode& node, children_) {
		node.impl_draw_children(frame_buffer);
	}
}

void ttree_view::tnode::signal_handler_left_button_click(
		const event::tevent event)
{
	DBG_GUI_E << LOG_NODE_HEADER << ' ' << event << ".\n";

	assert(icon_);
/*
	if(!icon_->get_value()) {
		fold(false);
	} else {
		unfold(recursive_restore);
	}
*/
	parent_widget_->set_size(
			  parent_widget_->get_origin()
			, parent_widget_->get_size());
}

void ttree_view::tnode::signal_handler_label_left_button_click(
		  const event::tevent event
		, bool& handled
		, bool& halt)
{
	DBG_GUI_E << LOG_NODE_HEADER << ' ' << event << ".\n";

	assert(label_);

	if(label_->get_value()) {
		// Forbid deselecting
		halt = handled = true;
	} else {
		// Deselect current item
		//
		// TODO implement
	}

}

void ttree_view::tnode::init_grid(tgrid* grid
		, const std::map<std::string /* widget id */, string_map>& data)
{
	assert(grid);

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);

			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
//			ttoggle_button* btn = dynamic_cast<ttoggle_button*>(widget);
			ttoggle_panel* panel = dynamic_cast<ttoggle_panel*>(widget);
			tcontrol* ctrl = dynamic_cast<tcontrol*>(widget);

			if(panel) {
				panel->set_child_members(data);
			} else if(child_grid) {
				init_grid(child_grid, data);
			} else if(ctrl) {
				std::map<std::string, string_map>::const_iterator itor =
						data.find(ctrl->id());

				if(itor == data.end()) {
					itor = data.find("");
				}
				if(itor != data.end()) {
					ctrl->set_members(itor->second);
				}
//				ctrl->set_members(data);
			} else {

//				ERROR_LOG("Widget type '"
//						<< typeid(*widget).name() << "'.");
			}
		}
	}

}
ttree_view::ttree_view(const std::vector<tnode_definition>& node_definitions)
	: tscrollbar_container(2)
	, node_definitions_(node_definitions)
	, indention_step_size_(0)
	, root_node_(new tnode(
		  "root"
		, node_definitions_
		, NULL
		, this
		, std::map<std::string, string_map>()))
{
}

void ttree_view::child_populate_dirty_list(twindow& caller
		, const std::vector<twidget*>& call_stack)
{
	// Inherited.
	tscrollbar_container::child_populate_dirty_list(caller, call_stack);

	assert(root_node_);
	root_node_->impl_populate_dirty_list(caller, call_stack);
}

const std::string& ttree_view::tnode::get_control_type() const
{
	static const std::string type = "tree_view_node";
	return type;
}

namespace {

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
		tgrid* content_grid, twidget* widget, const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = find_widget<tgrid>(grid, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<tgrid>(content_grid, id, true, false);
	}
	assert(parent_grid);
	parent_grid = dynamic_cast<tgrid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void ttree_view::finalize_setup()
{
	tgrid* g = new tgrid();

	g->set_rows_cols(1, 1);
	g->set_child(
			  root_node_
			, 0
			, 0
			, tgrid::VERTICAL_GROW_SEND_TO_CLIENT
				| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
			, 0);

	swap_grid(NULL, &grid(), g/*root_node_*/, "_content_grid");

	// Inherited.
	tscrollbar_container::finalize_setup();
}

const std::string& ttree_view::get_control_type() const
{
	static const std::string type = "tree_view";
	return type;
}

} // namespace gui2

