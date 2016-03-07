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

#include "gui/widgets/tree_view_node.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER                                                       \
	get_control_type() + " [" + tree_view().id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

ttree_view_node::ttree_view_node(
		const std::string& id,
		const std::vector<tnode_definition>& node_definitions,
		ttree_view_node* parent_node,
		ttree_view& parent_tree_view,
		const std::map<std::string /* widget id */, string_map>& data)
	: twidget()
	, parent_node_(parent_node)
	, tree_view_(parent_tree_view)
	, grid_()
	, children_()
	, node_definitions_(node_definitions)
	, toggle_(NULL)
	, label_(NULL)
	, unfolded_(false)
	, callback_state_change_()
	, callback_state_to_folded_()
	, callback_state_to_unfolded_()
{
	grid_.set_parent(this);
	set_parent(&parent_tree_view);
	if(id != "root") {
		FOREACH(const AUTO & node_definition, node_definitions_)
		{
			if(node_definition.id == id) {
				node_definition.builder->build(&grid_);
				init_grid(&grid_, data);

				twidget* toggle_widget = grid_.find("tree_view_node_icon", false);
				toggle_ = dynamic_cast<tselectable_*>(toggle_widget);

				if(toggle_) {
					toggle_widget->set_visible(twidget::tvisible::hidden);
					toggle_widget->connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(
							&ttree_view_node::signal_handler_left_button_click,
							this,
							_2));
					toggle_widget->connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(
							&ttree_view_node::signal_handler_left_button_click,
							this,
							_2), event::tdispatcher::back_post_child);

					if(node_definition.unfolded) {
						toggle_->set_value(1);
						unfolded_ = true;
					}
				}

				if(parent_node_ && parent_node_->toggle_) {
					dynamic_cast<twidget&>(*parent_node_->toggle_).set_visible(
							twidget::tvisible::visible);
				}

				twidget* widget = find_widget<twidget>(
						&grid_, "tree_view_node_label", false, false);

				label_ = dynamic_cast<tselectable_*>(widget);
				if(label_) {
					widget->connect_signal<event::LEFT_BUTTON_CLICK>(
							boost::bind(
									&ttree_view_node::
											 signal_handler_label_left_button_click,
									this,
									_2,
									_3,
									_4),
							event::tdispatcher::front_child);
					widget->connect_signal<event::LEFT_BUTTON_CLICK>(
							boost::bind(
									&ttree_view_node::
											 signal_handler_label_left_button_click,
									this,
									_2,
									_3,
									_4),
							event::tdispatcher::front_pre_child);

					if(!tree_view().selected_item_) {
						tree_view().selected_item_ = this;
						label_->set_value(true);
					}
				}

				return;
			}
		}

		VALIDATE(false, _("Unknown builder id for tree view node."));
	}
	else {
		unfolded_ = true;
	}
}

ttree_view_node::~ttree_view_node()
{
	if(/*tree_view() &&*/ tree_view().selected_item_ == this) {
		tree_view().selected_item_ = NULL;
	}
}

ttree_view_node& ttree_view_node::add_child(
		const std::string& id,
		const std::map<std::string /* widget id */, string_map>& data,
		const int index)
{

	boost::ptr_vector<ttree_view_node>::iterator itor = children_.end();

	if(static_cast<size_t>(index) < children_.size()) {
		itor = children_.begin() + index;
	}

	itor = children_.insert(
			itor,
			new ttree_view_node(
					id, node_definitions_, this, tree_view(), data));

	if(is_folded() || is_root_node()) {
		return *itor;
	}

	if(tree_view().get_size() == tpoint(0, 0)) {
		return *itor;
	}

	assert(tree_view().content_grid());
	const int current_width = tree_view().content_grid()->get_width();

	// Calculate width modification.
	tpoint best_size = itor->get_best_size();
	best_size.x += get_indention_level() * tree_view().indention_step_size_;
	const unsigned width_modification
			= best_size.x > current_width ? best_size.x - current_width : 0;

	// Calculate height modification.
	const int height_modification = best_size.y;
	assert(height_modification > 0);

	// Request new size.
	tree_view().resize_content(width_modification, height_modification, -1, itor->calculate_ypos());

	return *itor;
}

unsigned ttree_view_node::get_indention_level() const
{
	unsigned level = 0;

	const ttree_view_node* node = this;
	while(!node->is_root_node()) {
		node = &node->parent_node();
		++level;
	}

	return level;
}

ttree_view_node& ttree_view_node::parent_node()
{
	assert(!is_root_node());
	return *parent_node_;
}

const ttree_view_node& ttree_view_node::parent_node() const
{
	assert(!is_root_node());
	return *parent_node_;
}

ttree_view& ttree_view_node::tree_view()
{
	return tree_view_;
}

void ttree_view_node::request_reduce_width(const unsigned /*maximum_width*/)
{
	/* DO NOTHING */
}

const ttree_view& ttree_view_node::tree_view() const
{
	return tree_view_;
}

bool ttree_view_node::is_folded() const
{
	return !unfolded_;
}

void ttree_view_node::fold(/*const bool recursive*/)
{
	if(is_folded()) {
		fold_internal();
		if(toggle_) {
			toggle_->set_value(false);
		}
	}
}

void ttree_view_node::unfold(/*const texpand_mode mode*/)
{
	if(!is_folded()) {
		unfold_internal();
		if(toggle_) {
			toggle_->set_value(true);
		}
	}
}

void ttree_view_node::fold_internal()
{
	const tpoint current_size(get_current_size().x, get_unfolded_size().y);
	const tpoint new_size = get_folded_size();

	const int width_modification = std::max(0, new_size.x - current_size.x);
	const int height_modification = new_size.y - current_size.y;
	assert(height_modification <= 0);

	tree_view().resize_content(width_modification, height_modification, -1, calculate_ypos());
	unfolded_ = false;

	if(callback_state_to_folded_) {
		callback_state_to_folded_(*this);
	}
}

void ttree_view_node::unfold_internal()
{
	const tpoint current_size(get_current_size().x, get_folded_size().y);
	const tpoint new_size = get_unfolded_size();

	const int width_modification = std::max(0, new_size.x - current_size.x);
	const int height_modification = new_size.y - current_size.y;
	assert(height_modification >= 0);

	tree_view().resize_content(width_modification, height_modification, -1, calculate_ypos());
	unfolded_ = true;

	if(callback_state_to_unfolded_) {
		callback_state_to_unfolded_(*this);
	}
}

void ttree_view_node::clear()
{
	/** @todo Also try to find the optimal width. */
	int height_reduction = 0;

	if(!is_folded()) {
		FOREACH(const AUTO & node, children_)
		{
			height_reduction += node.get_current_size().y;
		}
	}

	children_.clear();

	if(height_reduction == 0) {
		return;
	}

	tree_view().resize_content(0, -height_reduction,  -1, calculate_ypos());
}

struct ttree_view_node_implementation
{
private:
	template <class W, class It>
	static W* find_at_aux(It begin,
						  It end,
						  const tpoint& coordinate,
						  const bool must_be_active)
	{
		for(It it = begin; it != end; ++it) {
			if(W* widget = it->find_at(coordinate, must_be_active)) {
				return widget;
			}
		}
		return NULL;
	}

public:
	template <class W>
	static W* find_at(typename utils::tconst_clone<ttree_view_node,
												   W>::reference tree_view_node,
					  const tpoint& coordinate,
					  const bool must_be_active)
	{
		if(W* widget
		   = tree_view_node.grid_.find_at(coordinate, must_be_active)) {

			return widget;
		}

		if(tree_view_node.is_folded()) {
			return NULL;
		}

		return find_at_aux<W>(tree_view_node.children_.begin(),
							  tree_view_node.children_.end(),
							  coordinate,
							  must_be_active);
	}
};

twidget* ttree_view_node::find_at(const tpoint& coordinate,
								  const bool must_be_active)
{
	return ttree_view_node_implementation::find_at<twidget>(
			*this, coordinate, must_be_active);
}

const twidget* ttree_view_node::find_at(const tpoint& coordinate,
										const bool must_be_active) const
{
	return ttree_view_node_implementation::find_at<const twidget>(
			*this, coordinate, must_be_active);
}

twidget* ttree_view_node::find(const std::string& id, const bool must_be_active)
{
	twidget* result = twidget::find(id, must_be_active);
	return result ? result : grid_.find(id, must_be_active);
}

const twidget* ttree_view_node::find(const std::string& id,
									 const bool must_be_active) const
{
	const twidget* result = twidget::find(id, must_be_active);
	return result ? result : grid_.find(id, must_be_active);
}

void ttree_view_node::impl_populate_dirty_list(
		twindow& caller, const std::vector<twidget*>& call_stack)
{
	std::vector<twidget*> child_call_stack = call_stack;
	grid_.populate_dirty_list(caller, child_call_stack);

	if(is_folded()) {
		return;
	}

	FOREACH(AUTO & node, children_)
	{
		std::vector<twidget*> child_call_stack = call_stack;
		node.impl_populate_dirty_list(caller, child_call_stack);
	}
}

tpoint ttree_view_node::calculate_best_size() const
{
	return calculate_best_size(-1, tree_view().indention_step_size_);
}

bool ttree_view_node::disable_click_dismiss() const
{
	return true;
}

tpoint ttree_view_node::get_current_size(bool assume_visible) const
{
	if(!assume_visible && parent_node_ && parent_node_->is_folded()) {
		return tpoint(0, 0);
	}

	tpoint size = get_folded_size();
	if(is_folded()) {
		return size;
	}

	for(boost::ptr_vector<ttree_view_node>::const_iterator itor
		= children_.begin();
		itor != children_.end();
		++itor) {

		const ttree_view_node& node = *itor;

		if(node.grid_.get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		tpoint node_size = node.get_current_size();

		size.y += node_size.y;
		size.x = std::max(size.x, node_size.x);
	}

	return size;
}

tpoint ttree_view_node::get_folded_size() const
{
	tpoint size = grid_.get_best_size();
	if(get_indention_level() > 1) {
		size.x += (get_indention_level() - 1)
				  * tree_view().indention_step_size_;
	}
	return size;
}

tpoint ttree_view_node::get_unfolded_size() const
{
	tpoint size = grid_.get_best_size();
	if(get_indention_level() > 1) {
		size.x += (get_indention_level() - 1)
				  * tree_view().indention_step_size_;
	}

	for(boost::ptr_vector<ttree_view_node>::const_iterator itor
		= children_.begin();
		itor != children_.end();
		++itor) {

		const ttree_view_node& node = *itor;

		if(node.grid_.get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		tpoint node_size = node.get_current_size(true);

		size.y += node_size.y;
		size.x = std::max(size.x, node_size.x);
	}

	return size;
}

tpoint ttree_view_node::calculate_best_size(const int indention_level,
											const unsigned indention_step_size)
		const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	tpoint best_size = grid_.get_best_size();
	if(indention_level > 0) {
		best_size.x += indention_level * indention_step_size;
	}

	DBG_GUI_L << LOG_HEADER << " own grid best size " << best_size << ".\n";

	for(boost::ptr_vector<ttree_view_node>::const_iterator itor
		= children_.begin();
		itor != children_.end();
		++itor) {

		const ttree_view_node& node = *itor;

		if(node.grid_.get_visible() == twidget::tvisible::invisible) {
			continue;
		}

		const tpoint node_size = node.calculate_best_size(indention_level + 1,
														  indention_step_size);

		if(!is_folded()) {
			best_size.y += node_size.y;
		}
		best_size.x = std::max(best_size.x, node_size.x);
	}

	DBG_GUI_L << LOG_HEADER << " result " << best_size << ".\n";
	return best_size;
}

void ttree_view_node::set_origin(const tpoint& origin)
{
	// Inherited.
	twidget::set_origin(origin);

	// Using layout_children seems to fail.
	place(tree_view().indention_step_size_, origin, get_size().x);
}

void ttree_view_node::place(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	twidget::place(origin, size);

	tree_view().layout_children(true);
}

unsigned ttree_view_node::place(const unsigned indention_step_size,
								tpoint origin,
								unsigned width)
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " origin " << origin << ".\n";

	const unsigned offset = origin.y;
	tpoint best_size = grid_.get_best_size();
	best_size.x = width;
	grid_.place(origin, best_size);

	if(!is_root_node()) {
		origin.x += indention_step_size;
		assert(width >= indention_step_size);
		width -= indention_step_size;
	}
	origin.y += best_size.y;

	if(is_folded()) {
		DBG_GUI_L << LOG_HEADER << " folded node done.\n";
		return origin.y - offset;
	}

	DBG_GUI_L << LOG_HEADER << " set children.\n";
	FOREACH(AUTO & node, children_)
	{
		origin.y += node.place(indention_step_size, origin, width);
	}

	// Inherited.
	twidget::set_size(tpoint(width, origin.y - offset));

	DBG_GUI_L << LOG_HEADER << " result " << (origin.y - offset) << ".\n";
	return origin.y - offset;
}

void ttree_view_node::set_visible_rectangle(const SDL_Rect& rectangle)
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " rectangle " << rectangle << ".\n";
	grid_.set_visible_rectangle(rectangle);

	if(is_folded()) {
		DBG_GUI_L << LOG_HEADER << " folded node done.\n";
		return;
	}

	FOREACH(AUTO & node, children_)
	{
		node.set_visible_rectangle(rectangle);
	}
}

void ttree_view_node::impl_draw_children(surface& frame_buffer,
										 int x_offset,
										 int y_offset)
{
	grid_.draw_children(frame_buffer, x_offset, y_offset);

	if(is_folded()) {
		return;
	}

	FOREACH(AUTO & node, children_)
	{
		node.impl_draw_children(frame_buffer, x_offset, y_offset);
	}
}

void
ttree_view_node::signal_handler_left_button_click(const event::tevent event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	/**
	 * @todo Rewrite this sizing code for the folding/unfolding.
	 *
	 * The code works but feels rather hacky, so better move back to the
	 * drawingboard for 1.9.
	 */
	const bool unfolded_new = toggle_->get_value_bool();
	if(unfolded_ == unfolded_new) {
		return;
	}
	unfolded_ = unfolded_new;
	is_folded() ? fold_internal() : unfold_internal();

	if(callback_state_change_) {
		callback_state_change_(*this);
	}
}

void ttree_view_node::signal_handler_label_left_button_click(
		const event::tevent event, bool& handled, bool& halt)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	assert(label_);

	// We only snoop on the event so normally don't touch the handled, else if
	// we snoop in preexcept when halting.

	if(label_->get_value()) {
		// Forbid deselecting
		halt = handled = true;
	} else {
		// Deselect current item
		if(tree_view().selected_item_ && tree_view().selected_item_->label_) {
			tree_view().selected_item_->label_->set_value(false);
		}

		tree_view().selected_item_ = this;

		if(tree_view().selection_change_callback_) {
			tree_view().selection_change_callback_(tree_view());
		}
	}
}

void ttree_view_node::init_grid(
		tgrid* grid,
		const std::map<std::string /* widget id */, string_map>& data)
{
	assert(grid);

	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);

			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			// ttoggle_button* btn =
			// dynamic_cast<ttoggle_button*>(widget);
			ttoggle_panel* panel = dynamic_cast<ttoggle_panel*>(widget);
			tcontrol* ctrl = dynamic_cast<tcontrol*>(widget);

			if(panel) {
				panel->set_child_members(data);
			} else if(child_grid) {
				init_grid(child_grid, data);
			} else if(ctrl) {
				std::map<std::string, string_map>::const_iterator itor
						= data.find(ctrl->id());

				if(itor == data.end()) {
					itor = data.find("");
				}
				if(itor != data.end()) {
					ctrl->set_members(itor->second);
				}
				// ctrl->set_members(data);
			} else {

				// ERROR_LOG("Widget type '" << typeid(*widget).name() << "'.");
			}
		}
	}
}

const std::string& ttree_view_node::get_control_type() const
{
	static const std::string type = "tree_view_node";
	return type;
}

ttree_view_node& ttree_view_node::get_child_at(int index)
{
	assert(static_cast<size_t>(index) < children_.size());
	return children_[index];
}

std::vector<int> ttree_view_node::describe_path()
{
	if(is_root_node()) {
		return std::vector<int>();
	}
	else {
		std::vector<int> res = parent_node_->describe_path();
		const boost::ptr_vector<ttree_view_node>& parents_childs = parent_node_->children_;
		for(int i = 0, size = parents_childs.size(); i < size; ++i) {
			if(&parents_childs[i] == this) {
				res.push_back(i);
				return res;
			}
		}
		assert(!"tree_view_node was not found in parent nodes children");
		throw "assertion ignored"; //To silence 'no return value in this codepath' warning.
	}
}
int ttree_view_node::calculate_ypos()
{
	if(!parent_node_) {
		return 0;
	}
	int res = parent_node_->calculate_ypos();
	FOREACH(const AUTO& node, parent_node_->children_) {
		if(&node == this) {
			break;
		}
		res += node.get_current_size(true).y;
	}
	return res;
}
ttree_view_node* ttree_view_node::get_last_visible_parent_node()
{
	if(!parent_node_) {
		return this;
	}
	ttree_view_node* res = parent_node_->get_last_visible_parent_node();
	return res == parent_node_ && !res->is_folded() ? this : res;
}

ttree_view_node* ttree_view_node::get_node_above()
{
	assert(!is_root_node());
	ttree_view_node* cur = NULL;
	for(size_t i = 0; i < parent_node_->size(); ++i) {
		if(&parent_node_->children_[i] == this) {
			if(i == 0) {
				return parent_node_->is_root_node() ? NULL : parent_node_;
			}
			else {
				cur = &parent_node_->children_[i - 1];
				break;
			}
		}
	}
	while(!cur->is_folded() && cur->size() > 0) {
		cur = &cur->get_child_at(cur->size() - 1);
	}
	return cur;
}

ttree_view_node* ttree_view_node::get_node_below()
{
	assert(!is_root_node());
	if(!is_folded() && size() > 0) {
		return &get_child_at(0);
	}
	ttree_view_node* cur = this;
	while(cur->parent_node_ != NULL) {
		ttree_view_node& parent = *cur->parent_node_;

		for(size_t i = 0; i < parent.size(); ++i) {
			if(&parent.children_[i] == cur) {
				if(i < parent.size() - 1) {
					return &parent.children_[i + 1];
				}
				else {
					cur = &parent;
				}
				break;
			}
		}	
	}
	return NULL;
}
ttree_view_node* ttree_view_node::get_selectable_node_above()
{
	ttree_view_node* above = this;
	do {
		above = above->get_node_above();
	} while(above != NULL && above->label_ == NULL);
	return above;
}
ttree_view_node* ttree_view_node::get_selectable_node_below()
{
	ttree_view_node* below = this;
	do {
		below = below->get_node_below();
	} while(below != NULL && below->label_ == NULL);
	return below;

}
void ttree_view_node::select_node()
{
	if(!label_ || label_->get_value_bool()) {
		return;
	}

	if(tree_view().selected_item_ && tree_view().selected_item_->label_) {
		tree_view().selected_item_->label_->set_value(false);
	}
	tree_view().selected_item_ = this;

	if(tree_view().selection_change_callback_) {
		tree_view().selection_change_callback_(tree_view());
	}
	label_->set_value_bool(true);
}

} // namespace gui2
