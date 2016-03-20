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

#include "gui/widgets/tree_view.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/tree_view.hpp"
#include "gui/auxiliary/window_builder/tree_view.hpp"
#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET(tree_view)

ttree_view::ttree_view(const std::vector<tnode_definition>& node_definitions)
	: tscrollbar_container(2)
	, node_definitions_(node_definitions)
	, indention_step_size_(0)
	, need_layout_(false)
	, root_node_(new ttree_view_node("root",
									 node_definitions_,
									 NULL,
									 *this,
									 std::map<std::string, string_map>()))
	, selected_item_(NULL)
	, selection_change_callback_()
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
			boost::bind(&ttree_view::signal_handler_left_button_down, this, _2),
			event::tdispatcher::back_pre_child);
}

ttree_view_node& ttree_view::add_node(
		const std::string& id,
		const std::map<std::string /* widget id */, string_map>& data)
{
	return get_root_node().add_child(id, data);
}

void ttree_view::remove_node(ttree_view_node* node)
{
	assert(node && node != root_node_ && node->parent_node_);
	const tpoint node_size = node->get_size();

	boost::ptr_vector<ttree_view_node>::iterator itor
			= node->parent_node_->children_.begin();

	for(; itor != node->parent_node_->children_.end(); ++itor) {
		if(&*itor == node) {
			break;
		}
	}

	assert(itor != node->parent_node_->children_.end());

	node->parent_node_->children_.erase(itor);

	if(get_size() == tpoint(0, 0)) {
		return;
	}

	// Don't shrink the width, need to think about a good algorithm to do so.
	resize_content(0, -node_size.y);
}

void
ttree_view::child_populate_dirty_list(twindow& caller,
									  const std::vector<twidget*>& call_stack)
{
	// Inherited.
	tscrollbar_container::child_populate_dirty_list(caller, call_stack);

	assert(root_node_);
	root_node_->impl_populate_dirty_list(caller, call_stack);
}

void ttree_view::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool ttree_view::empty() const
{
	return root_node_->empty();
}

void ttree_view::layout_children()
{
	layout_children(false);
}

void ttree_view::resize_content(const int width_modification,
								const int height_modification,
						const int width__modification_pos,
						const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " width_modification " << width_modification
			  << " height_modification " << height_modification << ".\n";

	if(content_resize_request(width_modification, height_modification, width__modification_pos, height_modification_pos)) {

		// Calculate new size.
		tpoint size = content_grid()->get_size();
		size.x += width_modification;
		size.y += height_modification;

		// Set new size.
		content_grid()->set_size(size);

		// Set status.
		need_layout_ = true;
		// If the content grows assume it "overwrites" the old content.
		if(width_modification < 0 || height_modification < 0) {
			set_is_dirty(true);
		}
		horizontal_scrollbar_moved();
		DBG_GUI_L << LOG_HEADER << " succeeded.\n";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.\n";
	}
}

void ttree_view::layout_children(const bool force)
{
	assert(root_node_ && content_grid());

	if(need_layout_ || force) {
		root_node_->place(indention_step_size_,
						  get_origin(),
						  content_grid()->get_size().x);
		root_node_->set_visible_rectangle(content_visible_area_);

		need_layout_ = false;
		horizontal_scrollbar_moved();
	}
}

void ttree_view::finalize_setup()
{
	// Inherited.
	tscrollbar_container::finalize_setup();

	assert(content_grid());
	content_grid()->set_rows_cols(1, 1);
	content_grid()->set_child(root_node_,
							  0,
							  0,
							  tgrid::VERTICAL_GROW_SEND_TO_CLIENT
							  | tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT,
							  0);
}

const std::string& ttree_view::get_control_type() const
{
	static const std::string type = "tree_view";
	return type;
}

void ttree_view::signal_handler_left_button_down(const event::tevent event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	get_window()->keyboard_capture(this);
}
template<ttree_view_node* (ttree_view_node::*func) ()>
ttree_view_node* ttree_view::get_next_node()
{	
	ttree_view_node* selected = selected_item();
	if(!selected) {
		return NULL;
	}
	ttree_view_node* visible = selected->get_last_visible_parent_node();
	if(visible != selected) {
		return visible;
	}
	return (selected->*func)();
}

template<ttree_view_node* (ttree_view_node::*func) ()>
bool ttree_view::handle_up_down_arrow()
{
	if(ttree_view_node* next = get_next_node<func>())
	{
		next->select_node();
		SDL_Rect visible = content_visible_area();
		SDL_Rect rect = next->get_grid().get_rectangle();
		visible.y = rect.y;// - content_grid()->get_y();
		visible.h = rect.h;
		show_content_rect(visible);
		return true;
	}
	return false;
}

void ttree_view::handle_key_up_arrow(SDLMod modifier, bool& handled)
{
	if(handle_up_down_arrow<&ttree_view_node::get_selectable_node_above>()) {
		handled = true;
	}
	else {
		tscrollbar_container::handle_key_up_arrow(modifier, handled);	
	}
}

void ttree_view::handle_key_down_arrow(SDLMod modifier, bool& handled)
{
	if(handle_up_down_arrow<&ttree_view_node::get_selectable_node_below>()) {
		handled = true;
	}
	else {
		tscrollbar_container::handle_key_down_arrow(modifier, handled);	
	}
}


void ttree_view::handle_key_left_arrow(SDLMod modifier, bool& handled)
{
	ttree_view_node* selected = selected_item();
	if(!selected || selected->is_folded()) {
		tscrollbar_container::handle_key_left_arrow(modifier, handled);
		return;
	}
	selected->fold();
	handled = true;
}

void ttree_view::handle_key_right_arrow(SDLMod modifier, bool& handled)
{
	ttree_view_node* selected = selected_item();
	if(!selected || !selected->is_folded()) {
		tscrollbar_container::handle_key_right_arrow(modifier, handled);
		return;
	}
	selected->unfold();
	handled = true;
}

} // namespace gui2
