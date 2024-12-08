/*
	Copyright (C) 2010 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/widgets/window.hpp"
#include <functional>
#include "wml_exception.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{
// ------------ WIDGET -----------{

REGISTER_WIDGET(tree_view)

tree_view::tree_view(const implementation::builder_tree_view& builder)
	: scrollbar_container(builder, type())
	, node_definitions_(builder.nodes)
	, indentation_step_size_(builder.indentation_step_size)
	, need_layout_(false)
	, root_node_(nullptr)
	, selected_item_(nullptr)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&tree_view::signal_handler_left_button_down, this, std::placeholders::_2), event::dispatcher::back_pre_child);
}

tree_view::~tree_view()
{
	if(root_node_) {
		root_node_->clear_before_destruct();
	}
}

tree_view_node& tree_view::add_node(
	const std::string& id, const widget_data& data, const int index)
{
	return get_root_node().add_child(id, data, index);
}

std::pair<std::shared_ptr<tree_view_node>, int> tree_view::remove_node(tree_view_node* node)
{
	assert(node && node != root_node_ && node->parent_node_);
	const point node_size = node->get_size();

	tree_view_node::node_children_vector& siblings = node->parent_node_->children_;

	auto node_itor = std::find_if(siblings.begin(), siblings.end(), [node](const auto& c) { return c.get() == node; });

	assert(node_itor != siblings.end());

	auto old_node = std::move(*node_itor);
	old_node->parent_node_ = nullptr;

	const int position = std::distance(siblings.begin(), node_itor);

	siblings.erase(node_itor);

	if(get_size() != point()) {
		// Don't shrink the width, need to think about a good algorithm to do so.
		resize_content(0, -node_size.y);
	}

	return std::pair(std::move(old_node), position);
}

void tree_view::clear()
{
	get_root_node().clear();
	resize_content(0, -content_grid()->get_size().y);
}

void tree_view::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tree_view::empty() const
{
	return root_node_->empty();
}

void tree_view::layout_children()
{
	layout_children(false);
}

void tree_view::resize_content(const int width_modification,
		const int height_modification,
		const int width_modification_pos,
		const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size() << " width_modification "
			  << width_modification << " height_modification " << height_modification << ".";

	if(content_resize_request(
		width_modification,
		height_modification,
		width_modification_pos,
		height_modification_pos
	)) {
		// Calculate new size.
		point size = content_grid()->get_size();
		size.x += width_modification;
		size.y += height_modification;

		// Set new size.
		content_grid()->set_size(size);

		// Set status.
		need_layout_ = true;
		// If the content grows assume it "overwrites" the old content.
		if(width_modification < 0 || height_modification < 0) {
			queue_redraw();
		}
		horizontal_scrollbar_moved();
		DBG_GUI_L << LOG_HEADER << " succeeded.";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.";
	}
}

void tree_view::layout_children(const bool force)
{
	assert(root_node_ && content_grid());

	if(need_layout_ || force) {
		root_node_->place(indentation_step_size_, get_origin(), content_grid()->get_size().x);
		root_node_->set_visible_rectangle(content_visible_area_);

		need_layout_ = false;
		horizontal_scrollbar_moved();
	}
}

void tree_view::finalize_setup()
{
	// Inherited.
	scrollbar_container::finalize_setup();

	auto root = std::make_unique<tree_view_node>(root_node_id, nullptr, *this, widget_data{});
	root_node_ = root.get();

	assert(content_grid());
	content_grid()->set_rows_cols(1, 1);
	content_grid()->set_child(
		std::move(root), 0, 0, grid::VERTICAL_GROW_SEND_TO_CLIENT | grid::HORIZONTAL_GROW_SEND_TO_CLIENT, 0);
}

void tree_view::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);
}

template<tree_view_node* (tree_view_node::*func)()>
tree_view_node* tree_view::get_next_node()
{
	tree_view_node* selected = selected_item();
	if(!selected) {
		return nullptr;
	}

	tree_view_node* visible = selected->get_last_visible_parent_node();
	if(visible != selected) {
		return visible;
	}

	return (selected->*func)();
}

template<tree_view_node* (tree_view_node::*func)()>
bool tree_view::handle_up_down_arrow()
{
	if(tree_view_node* next = get_next_node<func>()) {
		next->select_node();
		SDL_Rect visible = content_visible_area();
		SDL_Rect rect = next->get_grid().get_rectangle();
		visible.y = rect.y; // - content_grid()->get_y();
		visible.h = rect.h;
		show_content_rect(visible);
		return true;
	}

	return false;
}

void tree_view::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
{
	if(handle_up_down_arrow<&tree_view_node::get_selectable_node_above>()) {
		handled = true;
	} else {
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void tree_view::handle_key_down_arrow(SDL_Keymod modifier, bool& handled)
{
	if(handle_up_down_arrow<&tree_view_node::get_selectable_node_below>()) {
		handled = true;
	} else {
		scrollbar_container::handle_key_down_arrow(modifier, handled);
	}
}

void tree_view::handle_key_left_arrow(SDL_Keymod modifier, bool& handled)
{
	tree_view_node* selected = selected_item();
	if(!selected || selected->is_folded()) {
		scrollbar_container::handle_key_left_arrow(modifier, handled);
		return;
	}

	selected->fold();
	handled = true;
}

void tree_view::handle_key_right_arrow(SDL_Keymod modifier, bool& handled)
{
	tree_view_node* selected = selected_item();
	if(!selected || !selected->is_folded()) {
		scrollbar_container::handle_key_right_arrow(modifier, handled);
		return;
	}

	selected->unfold();
	handled = true;
}

// }---------- DEFINITION ---------{

tree_view_definition::tree_view_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing tree view " << id;

	load_resolutions<resolution>(cfg);
}

tree_view_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, grid(nullptr)
{
	// Note the order should be the same as the enum state_t is listbox.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("tree_view_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("tree_view_definition][resolution", "state_disabled")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("tree_view_definition][resolution", "grid"));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{
builder_tree_view::builder_tree_view(const config& cfg)
	: builder_scrollbar_container(cfg)
	, indentation_step_size(cfg["indentation_step_size"].to_unsigned())
	, nodes()
{
	for(const auto& node : cfg.child_range("node")) {
		nodes.emplace_back(node);
	}

	VALIDATE(!nodes.empty(), _("No nodes defined for a tree view."));
}

std::unique_ptr<widget> builder_tree_view::build() const
{
	/*
	 *  TODO see how much we can move in the constructor instead of
	 *  building in several steps.
	 */
	auto widget = std::make_unique<tree_view>(*this);

	DBG_GUI_G << "Window builder: placed tree_view '" << id << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<tree_view_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);
	widget->finalize_setup();

	return widget;
}

tree_node::tree_node(const config& cfg)
	: id(cfg["id"])
	, unfolded(cfg["unfolded"].to_bool(false))
	, builder(nullptr)
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("node", "id"));

	// TODO: interpolate this value into the error message
	VALIDATE(id != tree_view::root_node_id, _("[node]id ‘root’ is reserved for the implementation."));

	auto node_definition = VALIDATE_WML_CHILD(cfg, "node_definition", missing_mandatory_wml_tag("node", "node_definition"));
	builder = std::make_shared<builder_grid>(node_definition);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
