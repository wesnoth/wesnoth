/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/iterator/walker_tree_node.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view.hpp"
#include "sdl/rect.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + get_tree_view().id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

tree_view_node::tree_view_node(
		const std::string& id,
		tree_view_node* parent_node,
		tree_view& parent_tree_view,
		const std::map<std::string /* widget id */, string_map>& data)
	: widget()
	, parent_node_(parent_node)
	, tree_view_(&parent_tree_view)
	, grid_()
	, children_()
	, toggle_(nullptr)
	, label_(nullptr)
	, unfolded_(false)
	, callback_state_change_()
	, callback_state_to_folded_()
	, callback_state_to_unfolded_()
{
	grid_.set_parent(this);
	set_parent(&parent_tree_view);

	if(id == "root") {
		unfolded_ = true;
		return;
	}

	for(const auto& node_definition : get_tree_view().get_node_definitions()) {
		if(node_definition.id != id) {
			continue;
		}

		node_definition.builder->build(&grid_);
		init_grid(&grid_, data);

		if(parent_node_ && parent_node_->toggle_) {
			dynamic_cast<widget&>(*parent_node_->toggle_).set_visible(widget::visibility::visible);
		}

		if(node_definition.unfolded) {
			unfolded_ = true;
		}

		widget* toggle_widget = grid_.find("tree_view_node_toggle", false);
		toggle_ = dynamic_cast<selectable_item*>(toggle_widget);

		if(toggle_) {
			toggle_widget->set_visible(widget::visibility::hidden);

			toggle_widget->connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
					&tree_view_node::signal_handler_left_button_click, this, _2));
			toggle_widget->connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
					&tree_view_node::signal_handler_left_button_click, this, _2),
					event::dispatcher::back_post_child);

			if(unfolded_) {
				toggle_->set_value(1);
			}
		}

		widget* label_widget = grid_.find("tree_view_node_label", false);
		label_ = dynamic_cast<selectable_item*>(label_widget);

		if(label_) {
			label_widget->connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
					&tree_view_node::signal_handler_label_left_button_click, this, _2, _3, _4),
					event::dispatcher::front_child);
			label_widget->connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
					&tree_view_node::signal_handler_label_left_button_click, this, _2, _3, _4),
					event::dispatcher::front_pre_child);

			if(!get_tree_view().selected_item_) {
				get_tree_view().selected_item_ = this;
				label_->set_value(true);
			}
		}

		return;
	}

	// Only reached if no matching node definiton was found in the above loop
	FAIL_WITH_DEV_MESSAGE(_("Unknown builder id for tree view node."), id);
}

tree_view_node::~tree_view_node()
{
	if(tree_view_ && get_tree_view().selected_item_ == this) {
		get_tree_view().selected_item_ = nullptr;
	}
}

void tree_view_node::clear_before_destruct()
{
	tree_view_ = nullptr;
	for(auto& child : children_) {
		child->clear_before_destruct();
	}
}

tree_view_node& tree_view_node::add_child(
		const std::string& id,
		const std::map<std::string /* widget id */, string_map>& data,
		const int index)
{
	auto itor = children_.end();

	if(static_cast<size_t>(index) < children_.size()) {
		itor = children_.begin() + index;
	}

	itor = children_.emplace(itor, new tree_view_node(id, this, get_tree_view(), data));

	if(is_folded() /*|| is_root_node()*/) {
		return **itor;
	}

	if(get_tree_view().get_size() == point()) {
		return **itor;
	}

	assert(get_tree_view().content_grid());
	const point current_size = get_tree_view().content_grid()->get_size();

	// Calculate width modification.
	// This increases tree width if the width of the new node is greater than the current width.
	point best_size = (*itor)->get_best_size();
	best_size.x += get_indentation_level() * get_tree_view().indentation_step_size_;

	const int width_modification = best_size.x > current_size.x
		? best_size.x - current_size.x
		: 0;

	// Calculate height modification.
	// For this, we only increase height if the best size of the tree (that is, the size with the new node)
	// is larger than its current size. This prevents the scrollbar being reserved even when there's obviously
	// enough visual space.

	// Throw away cached best size to force a recomputation.
	get_tree_view().layout_initialize(false);

	const point tree_best_size = get_tree_view().get_best_size();

	const int height_modification = tree_best_size.y > current_size.y && get_tree_view().layout_size() == point()
		? tree_best_size.y - current_size.y
		: 0;

	assert(height_modification >= 0);

	// Request new size.
	get_tree_view().resize_content(width_modification, height_modification, -1, (*itor)->calculate_ypos());

	return **itor;
}

unsigned tree_view_node::get_indentation_level() const
{
	unsigned level = 0;

	const tree_view_node* node = this;
	while(!node->is_root_node()) {
		node = &node->parent_node();
		++level;
	}

	return level;
}

tree_view_node& tree_view_node::parent_node()
{
	assert(!is_root_node());
	return *parent_node_;
}

const tree_view_node& tree_view_node::parent_node() const
{
	assert(!is_root_node());
	return *parent_node_;
}

void tree_view_node::request_reduce_width(const unsigned /*maximum_width*/)
{
	/* DO NOTHING */
}

void tree_view_node::fold(const bool recursive)
{
	if(!is_folded()) {
		fold_internal();
		if(toggle_) {
			toggle_->set_value(false);
		}
	}

	if(recursive) {
		for(auto& child_node : children_) {
			child_node->fold(true);
		}
	}
}

void tree_view_node::unfold(const bool recursive)
{
	if(is_folded()) {
		unfold_internal();
		if(toggle_) {
			toggle_->set_value(true);
		}
	}

	if(recursive) {
		for(auto& child_node : children_) {
			child_node->unfold(true);
		}
	}
}

void tree_view_node::fold_internal()
{
	const point current_size(get_current_size().x, get_unfolded_size().y);
	const point new_size = get_folded_size();

	const int width_modification = std::max(0, new_size.x - current_size.x);
	const int height_modification = new_size.y - current_size.y;
	assert(height_modification <= 0);

	get_tree_view().resize_content(width_modification, height_modification, -1, calculate_ypos());
	unfolded_ = false;

	if(callback_state_to_folded_) {
		callback_state_to_folded_(*this);
	}
}

void tree_view_node::unfold_internal()
{
	const point current_size(get_current_size().x, get_folded_size().y);
	const point new_size = get_unfolded_size();

	const int width_modification = std::max(0, new_size.x - current_size.x);
	const int height_modification = new_size.y - current_size.y;
	assert(height_modification >= 0);

	get_tree_view().resize_content(width_modification, height_modification, -1, calculate_ypos());
	unfolded_ = true;

	if(callback_state_to_unfolded_) {
		callback_state_to_unfolded_(*this);
	}
}

void tree_view_node::clear()
{
	/** @todo Also try to find the optimal width. */
	int height_reduction = 0;

	if(!is_folded()) {
		for(const auto & node : children_)
		{
			height_reduction += node->get_current_size().y;
		}
	}

	children_.clear();

	if(height_reduction == 0) {
		return;
	}

	get_tree_view().resize_content(0, -height_reduction,  -1, calculate_ypos());
}

struct tree_view_node_implementation
{
private:
	template <class W, class It>
	static W* find_at_aux(It begin,
						  It end,
						  const point& coordinate,
						  const bool must_be_active)
	{
		for(It it = begin; it != end; ++it) {
			if(W* widget = (*it)->find_at(coordinate, must_be_active)) {
				return widget;
			}
		}
		return nullptr;
	}

public:
	template <class W>
	static W* find_at(utils::const_clone_ref<tree_view_node, W> tree_view_node,
					  const point& coordinate,
					  const bool must_be_active)
	{
		if(W* widget = tree_view_node.grid_.find_at(coordinate, must_be_active)) {
			return widget;
		}

		if(tree_view_node.is_folded()) {
			return nullptr;
		}

		return find_at_aux<W>(tree_view_node.children_.begin(),
							  tree_view_node.children_.end(),
							  coordinate,
							  must_be_active);
	}
};

widget* tree_view_node::find_at(const point& coordinate, const bool must_be_active)
{
	return tree_view_node_implementation::find_at<widget>(*this, coordinate, must_be_active);
}

const widget* tree_view_node::find_at(const point& coordinate, const bool must_be_active) const
{
	return tree_view_node_implementation::find_at<const widget>(*this, coordinate, must_be_active);
}

widget* tree_view_node::find(const std::string& id, const bool must_be_active)
{
	widget* result = widget::find(id, must_be_active);
	if(result) {
		return result;
	}

	result = grid_.find(id, must_be_active);
	if(result) {
		return result;
	}

	for(auto& child : children_) {
		result = child->find(id, must_be_active);
		if(result) {
			return result;
		}
	}

	return nullptr;
}

const widget* tree_view_node::find(const std::string& id, const bool must_be_active) const
{
	const widget* result = widget::find(id, must_be_active);
	if(result) {
		return result;
	}

	result = grid_.find(id, must_be_active);
	if(result) {
		return result;
	}

	for(const auto& child : children_) {
		result = child->find(id, must_be_active);
		if(result) {
			return result;
		}
	}

	return nullptr;
}

void tree_view_node::impl_populate_dirty_list(window& caller, const std::vector<widget*>& call_stack)
{
	std::vector<widget*> my_call_stack = call_stack;
	grid_.populate_dirty_list(caller, my_call_stack);

	if(is_folded()) {
		return;
	}

	for(auto& node : children_) {
		std::vector<widget*> child_call_stack = call_stack;
		node->impl_populate_dirty_list(caller, child_call_stack);
	}
}

point tree_view_node::calculate_best_size() const
{
	return calculate_best_size(-1, get_tree_view().indentation_step_size_);
}

bool tree_view_node::disable_click_dismiss() const
{
	return true;
}

point tree_view_node::get_current_size(bool assume_visible) const
{
	if(!assume_visible && parent_node_ && parent_node_->is_folded()) {
		return point();
	}

	point size = get_folded_size();
	if(is_folded()) {
		return size;
	}

	for(const auto& node : children_) {
		if(node->grid_.get_visible() == widget::visibility::invisible) {
			continue;
		}

		point node_size = node->get_current_size();

		size.y += node_size.y;
		size.x = std::max(size.x, node_size.x);
	}

	return size;
}

point tree_view_node::get_folded_size() const
{
	point size = grid_.get_best_size();
	if(get_indentation_level() > 1) {
		size.x += (get_indentation_level() - 1) * get_tree_view().indentation_step_size_;
	}
	return size;
}

point tree_view_node::get_unfolded_size() const
{
	point size = grid_.get_best_size();
	if(get_indentation_level() > 1) {
		size.x += (get_indentation_level() - 1) * get_tree_view().indentation_step_size_;
	}

	for(const auto& node : children_) {
		if(node->grid_.get_visible() == widget::visibility::invisible) {
			continue;
		}

		point node_size = node->get_current_size(true);

		size.y += node_size.y;
		size.x = std::max(size.x, node_size.x);
	}

	return size;
}

point tree_view_node::calculate_best_size(const int indentation_level,
											const unsigned indentation_step_size) const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	point best_size = grid_.get_best_size();
	if(indentation_level > 0) {
		best_size.x += indentation_level * indentation_step_size;
	}

	DBG_GUI_L << LOG_HEADER << " own grid best size " << best_size << ".\n";

	for(const auto& node : children_) {
		if(node->grid_.get_visible() == widget::visibility::invisible) {
			continue;
		}

		const point node_size = node->calculate_best_size(indentation_level + 1, indentation_step_size);

		if(!is_folded()) {
			best_size.y += node_size.y;
		}

		best_size.x = std::max(best_size.x, node_size.x);
	}

	DBG_GUI_L << LOG_HEADER << " result " << best_size << ".\n";
	return best_size;
}

void tree_view_node::set_origin(const point& origin)
{
	// Inherited.
	widget::set_origin(origin);

	// Using layout_children seems to fail.
	place(get_tree_view().indentation_step_size_, origin, get_size().x);
}

void tree_view_node::place(const point& origin, const point& size)
{
	// Inherited.
	widget::place(origin, size);

	get_tree_view().layout_children(true);
}

unsigned tree_view_node::place(const unsigned indentation_step_size,
								point origin,
								unsigned width)
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " origin " << origin << ".\n";

	const unsigned offset = origin.y;
	point best_size = grid_.get_best_size();
	best_size.x = width;

	grid_.place(origin, best_size);

	if(!is_root_node()) {
		origin.x += indentation_step_size;
		assert(width >= indentation_step_size);
		width -= indentation_step_size;
	}

	origin.y += best_size.y;

	if(is_folded()) {
		DBG_GUI_L << LOG_HEADER << " folded node done.\n";
		return origin.y - offset;
	}

	DBG_GUI_L << LOG_HEADER << " set children.\n";
	for(auto & node : children_) {
		origin.y += node->place(indentation_step_size, origin, width);
	}

	// Inherited.
	widget::set_size(point(width, origin.y - offset));

	DBG_GUI_L << LOG_HEADER << " result " << (origin.y - offset) << ".\n";
	return origin.y - offset;
}

void tree_view_node::set_visible_rectangle(const SDL_Rect& rectangle)
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " rectangle " << rectangle << ".\n";
	grid_.set_visible_rectangle(rectangle);

	if(is_folded()) {
		DBG_GUI_L << LOG_HEADER << " folded node done.\n";
		return;
	}

	for(auto & node : children_) {
		node->set_visible_rectangle(rectangle);
	}
}

void tree_view_node::impl_draw_children(surface& frame_buffer,
										 int x_offset,
										 int y_offset)
{
	grid_.draw_children(frame_buffer, x_offset, y_offset);

	if(is_folded()) {
		return;
	}

	for(auto & node : children_) {
		node->impl_draw_children(frame_buffer, x_offset, y_offset);
	}
}

void tree_view_node::signal_handler_left_button_click(const event::ui_event event)
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

	fire(event::NOTIFY_MODIFIED, *this, nullptr);

	if(callback_state_change_) {
		callback_state_change_(*this);
	}
}

void tree_view_node::signal_handler_label_left_button_click(
		const event::ui_event event, bool& handled, bool& halt)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	assert(label_);

	// Normally, this is only an event hook and not full handling; however, if
	// the currently selected item was selected, we halt the event to prevent
	// deselection (which would leave no items selected).
	if(label_->get_value()) {
		halt = handled = true;
		return;
	}

	// Select the new item if a different one was selected
	if(get_tree_view().selected_item_ && get_tree_view().selected_item_->label_) {
		get_tree_view().selected_item_->label_->set_value(false);
	}

	get_tree_view().selected_item_ = this;

	if(get_tree_view().selection_change_callback_) {
		get_tree_view().selection_change_callback_(get_tree_view());
	}
}

void tree_view_node::init_grid(
		grid* g,
		const std::map<std::string /* widget id */, string_map>& data)
{
	assert(g);

	for(unsigned row = 0; row < g->get_rows(); ++row) {
		for(unsigned col = 0; col < g->get_cols(); ++col) {
			widget* wgt = g->get_widget(row, col);
			assert(wgt);

			// toggle_button* btn = dynamic_cast<toggle_button*>(widget);

			if(toggle_panel* panel = dynamic_cast<toggle_panel*>(wgt)) {
				panel->set_child_members(data);
			} else if(grid* child_grid = dynamic_cast<grid*>(wgt)) {
				init_grid(child_grid, data);
			} else if(styled_widget* control = dynamic_cast<styled_widget*>(wgt)) {
				auto itor = data.find(control->id());

				if(itor == data.end()) {
					itor = data.find("");
				}

				if(itor != data.end()) {
					control->set_members(itor->second);
				}
				// control->set_members(data);
			} else {
				// ERROR_LOG("Widget type '" << typeid(*widget).name() << "'.");
			}
		}
	}
}

const std::string& tree_view_node::get_control_type() const
{
	static const std::string type = "tree_view_node";
	return type;
}

tree_view_node& tree_view_node::get_child_at(int index)
{
	assert(static_cast<size_t>(index) < children_.size());
	return *children_[index];
}

std::vector<int> tree_view_node::describe_path()
{
	if(is_root_node()) {
		return std::vector<int>();
	}

	std::vector<int> res = parent_node_->describe_path();
	for(size_t i = 0; i < parent_node_->count_children(); ++i) {
		if(parent_node_->children_[i].get() == this) {
			res.push_back(i);
			return res;
		}
	}

	assert(!"tree_view_node was not found in parent nodes children");
	throw "assertion ignored"; //To silence 'no return value in this codepath' warning.
}

int tree_view_node::calculate_ypos()
{
	if(!parent_node_) {
		return 0;
	}

	int res = parent_node_->calculate_ypos();
	for(const auto& node : parent_node_->children_) {
		if(node.get() == this) {
			break;
		}

		res += node->get_current_size(true).y;
	}

	return res;
}

tree_view_node* tree_view_node::get_last_visible_parent_node()
{
	if(!parent_node_) {
		return this;
	}

	tree_view_node* res = parent_node_->get_last_visible_parent_node();
	return res == parent_node_ && !res->is_folded() ? this : res;
}

tree_view_node* tree_view_node::get_node_above()
{
	assert(!is_root_node());

	tree_view_node* cur = nullptr;
	for(size_t i = 0; i < parent_node_->count_children(); ++i) {
		if(parent_node_->children_[i].get() == this) {
			if(i == 0) {
				return parent_node_->is_root_node() ? nullptr : parent_node_;
			} else {
				cur = parent_node_->children_[i - 1].get();
				break;
			}
		}
	}

	while(!cur->is_folded() && cur->count_children() > 0) {
		cur = &cur->get_child_at(cur->count_children() - 1);
	}

	return cur;
}

tree_view_node* tree_view_node::get_node_below()
{
	assert(!is_root_node());
	if(!is_folded() && count_children() > 0) {
		return &get_child_at(0);
	}

	tree_view_node* cur = this;
	while(cur->parent_node_ != nullptr) {
		tree_view_node& parent = *cur->parent_node_;

		for(size_t i = 0; i < parent.count_children(); ++i) {
			if(parent.children_[i].get() == cur) {
				if(i < parent.count_children() - 1) {
					return parent.children_[i + 1].get();
				} else {
					cur = &parent;
				}
				break;
			}
		}
	}

	return nullptr;
}

tree_view_node* tree_view_node::get_selectable_node_above()
{
	tree_view_node* above = this;
	do {
		above = above->get_node_above();
	} while(above != nullptr && above->label_ == nullptr);
	return above;
}

tree_view_node* tree_view_node::get_selectable_node_below()
{
	tree_view_node* below = this;
	do {
		below = below->get_node_below();
	} while(below != nullptr && below->label_ == nullptr);
	return below;
}

void tree_view_node::select_node(bool expand_parents)
{
	if(!label_ || label_->get_value_bool()) {
		return;
	}

	if(expand_parents) {
		tree_view_node* root = &get_tree_view().get_root_node();
		for(tree_view_node* cur = parent_node_; cur != root; cur = cur->parent_node_) {
			cur->unfold();
		}
	}

	if(get_tree_view().selected_item_ && get_tree_view().selected_item_->label_) {
		get_tree_view().selected_item_->label_->set_value(false);
	}
	get_tree_view().selected_item_ = this;

	if(get_tree_view().selection_change_callback_) {
		get_tree_view().selection_change_callback_(get_tree_view());
	}
	label_->set_value_bool(true);
}

void tree_view_node::layout_initialize(const bool full_initialization)
{
	// Inherited.
	widget::layout_initialize(full_initialization);
	grid_.layout_initialize(full_initialization);

	// Clear child caches.
	for(auto & child : children_) {
		child->layout_initialize(full_initialization);
	}
}

iteration::walker_base* tree_view_node::create_walker()
{
	return new gui2::iteration::tree_node(*this, children_);
}

} // namespace gui2
