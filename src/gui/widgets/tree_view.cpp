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

#include "gui/widgets/tree_view.hpp"

#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(tree_view)

tree_view::tree_view(const std::vector<node_definition>& node_definitions)
	: scrollbar_container()
	, node_definitions_(node_definitions)
	, indentation_step_size_(0)
	, need_layout_(false)
	, root_node_(new tree_view_node("root",
									 nullptr,
									 *this,
									 std::map<std::string, string_map>()))
	, selected_item_(nullptr)
	, selection_change_callback_()
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
			std::bind(&tree_view::signal_handler_left_button_down, this, _2),
			event::dispatcher::back_pre_child);
}
tree_view::~tree_view()
{
	if (root_node_) {
		root_node_->clear_before_destruct();
	}
}
tree_view_node& tree_view::add_node(
		const std::string& id,
		const std::map<std::string /* widget id */, string_map>& data,
		const int index)
{
	return get_root_node().add_child(id, data, index);
}

int tree_view::remove_node(tree_view_node* node)
{
	assert(node && node != root_node_ && node->parent_node_);
	const point node_size = node->get_size();

	tree_view_node::node_children_vector& siblings = node->parent_node_->children_;

	auto node_itor = std::find(siblings.begin(), siblings.end(), *node);
	assert(node_itor != siblings.end());

	const int position = node_itor - siblings.begin();

	siblings.erase(node_itor);

	if(get_size() != point()) {
		// Don't shrink the width, need to think about a good algorithm to do so.
		resize_content(0, -node_size.y);
	}

	return position;
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
						const int width__modification_pos,
						const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " width_modification " << width_modification
			  << " height_modification " << height_modification << ".\n";

	if(content_resize_request(width_modification, height_modification, width__modification_pos, height_modification_pos)) {

		// Calculate new size.
		point size = content_grid()->get_size();
		size.x += width_modification;
		size.y += height_modification;

		// Set new size.
		content_grid()->set_size(size);

		// Set status.
		need_layout_ = true;

		horizontal_scrollbar_moved();
		DBG_GUI_L << LOG_HEADER << " succeeded.\n";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.\n";
	}
}

void tree_view::layout_children(const bool force)
{
	assert(root_node_ && content_grid());

	if(need_layout_ || force) {
		root_node_->place(indentation_step_size_,
						  get_origin(),
						  content_grid()->get_size().x);
		root_node_->set_visible_rectangle(content_visible_area_);

		need_layout_ = false;
		horizontal_scrollbar_moved();
	}
}

void tree_view::finalize_setup()
{
	// Inherited.
	scrollbar_container::finalize_setup();

	assert(content_grid());
	content_grid()->set_rows_cols(1, 1);
	content_grid()->set_child(root_node_,
							  0,
							  0,
							  grid::VERTICAL_GROW_SEND_TO_CLIENT
							  | grid::HORIZONTAL_GROW_SEND_TO_CLIENT,
							  0);
}

const std::string& tree_view::get_control_type() const
{
	static const std::string type = "tree_view";
	return type;
}

void tree_view::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	get_window()->keyboard_capture(this);
}
template<tree_view_node* (tree_view_node::*func) ()>
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

template<tree_view_node* (tree_view_node::*func) ()>
bool tree_view::handle_up_down_arrow()
{
	if(tree_view_node* next = get_next_node<func>())
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

void tree_view::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
{
	if(handle_up_down_arrow<&tree_view_node::get_selectable_node_above>()) {
		handled = true;
	}
	else {
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void tree_view::handle_key_down_arrow(SDL_Keymod modifier, bool& handled)
{
	if(handle_up_down_arrow<&tree_view_node::get_selectable_node_below>()) {
		handled = true;
	}
	else {
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
	DBG_GUI_P << "Parsing tree view " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_tree_view
 *
 * == Tree view ==
 *
 * @macro = tree_view_description
 *
 * The documentation is not written yet.
 *
 * The following states exist:
 * * state_enabled, the listbox is enabled.
 * * state_disabled, the listbox is disabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="ree_view_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="ree_view_definition"}
 * @end{parent}{name="gui/"}
 */
tree_view_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is listbox.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{tree_view_description}
 *
 *        A tree view is a styled_widget that holds several items of the same or
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
 *     indentation_step_size & unsigned & 0 &
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

namespace implementation
{

builder_tree_view::builder_tree_view(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, indentation_step_size(cfg["indentation_step_size"])
	, nodes()
{

	for(const auto & node : cfg.child_range("node"))
	{
		nodes.emplace_back(node);
	}

	VALIDATE(!nodes.empty(), _("No nodes defined for a tree view."));
}

widget* builder_tree_view::build() const
{
	/*
	 *  TODO see how much we can move in the constructor instead of
	 *  building in several steps.
	 */
	tree_view* widget = new tree_view(nodes);

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	widget->set_indentation_step_size(indentation_step_size);

	DBG_GUI_G << "Window builder: placed tree_view '" << id
			  << "' with definition '" << definition << "'.\n";

	std::shared_ptr<const tree_view_definition::resolution>
	conf = std::static_pointer_cast<const tree_view_definition::resolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

tree_node::tree_node(const config& cfg)
	: id(cfg["id"])
	, unfolded(cfg["unfolded"].to_bool(false))
	, builder(nullptr)
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("node", "id"));

	VALIDATE(id != "root",
			 _("[node]id 'root' is reserved for the implementation."));

	const config& node_definition = cfg.child("node_definition");

	VALIDATE(node_definition, _("No node defined."));

	builder = std::make_shared<builder_grid>(node_definition);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
