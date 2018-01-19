/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/widget.hpp"
#include "gui/widgets/grid.hpp"
#include <memory>

namespace gui2
{

namespace implementation {
	struct tree_node;
}

class selectable_item;
class tree_view;

class tree_view_node : public widget
{
	friend struct tree_view_node_implementation;
	friend class tree_view;

public:
	using node_children_vector = std::vector<std::unique_ptr<tree_view_node>>;

	bool operator==(const tree_view_node& node)
	{
		return &node == this;
	}

	tree_view_node(
			const std::string& id,
			tree_view_node* parent_node,
			tree_view& parent_tree_view,
			const std::map<std::string /* widget id */, string_map>& data);

	~tree_view_node();

	/**
	 * Adds a child item to the list of child nodes.
	 *
	 * @param id                  The id of the node definition to use for the
	 *                            new node.
	 * @param data                The data to send to the set_members of the
	 *                            widgets. If the member id is not an empty
	 *                            string it is only send to the widget that has
	 *                            the wanted id (if any). If the member id is an
	 *                            empty string, it is send to all members.
	 *                            Having both empty and non-empty id's gives
	 *                            undefined behavior.
	 * @param index               The item before which to add the new item,
	 *                            0 == begin, -1 == end.
	 */
	tree_view_node&
	add_child(const std::string& id,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const int index = -1);

	/**
	 * Adds a sibbling for a node at the end of the list.
	 *
	 * @param id                  The id of the node definition to use for the
	 *                            new node.
	 * @param data                The data to send to the set_members of the
	 *                            widgets. If the member id is not an empty
	 *                            string it is only send to the widget that has
	 *                            the wanted id (if any). If the member id is an
	 *                            empty string, it is send to all members.
	 *                            Having both empty and non-empty id's gives
	 *                            undefined behavior.
	 */
	tree_view_node&
	add_sibling(const std::string& id,
				const std::map<std::string /* widget id */, string_map>& data)
	{
		assert(!is_root_node());
		return parent_node().add_child(id, data);
	}

	/**
	 * Is this node the root node?
	 *
	 * When the parent tree view is created it adds one special node, the root
	 * node. This node has no parent node and some other special features so
	 * several code paths need to check whether they are the parent node.
	 */
	bool is_root_node() const
	{
		return parent_node_ == nullptr;
	}

	/**
	 * The indentation level of the node.
	 *
	 * The root node starts at level 0.
	 */
	unsigned get_indentation_level() const;

	/** Does the node have children? */
	bool empty() const
	{
		return children_.empty();
	}

	/** Is the node folded? */
	bool is_folded() const
	{
		return !unfolded_;
	}

#if 0
	// TODO: implement if different expand modes become necessary
	enum expand_mode {
		recursive_restore, // recursively restores collapse mode
		recursive_expand, // recursively expands the children
		not_recursive
	};
#endif

	void fold(const bool recursive = false);
	void unfold(const bool recursive = false);

	/**
	 * See @ref widget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iteration::walker_base* create_walker() override;

	node_children_vector& children()
	{
		return children_;
	}

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** See @ref widget::find. */
	widget* find(const std::string& id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string& id,
						const bool must_be_active) const override;

	/**
	 * The number of children in this widget.
	 */
	size_t count_children() const
	{
		return children_.size();
	}

	/**
	 * Removes all child items from the widget.
	 */
	void clear();

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/**
	 * Returns the parent node.
	 *
	 * @pre                       is_root_node() == false.
	 */
	tree_view_node& parent_node();

	/** The const version of @ref parent_node. */
	const tree_view_node& parent_node() const;

	tree_view& get_tree_view()
	{
		return *tree_view_;
	}

	const tree_view& get_tree_view() const
	{
		return *tree_view_;
	}

	tree_view_node& get_child_at(int index);

	/**
	 * Calculates the node indicies needed to get from the root node to this node.
	 */
	std::vector<int> describe_path();

	tree_view_node* get_last_visible_parent_node();
	tree_view_node* get_node_above();
	tree_view_node* get_node_below();
	tree_view_node* get_selectable_node_above();
	tree_view_node* get_selectable_node_below();
	void select_node(bool expand_parents = false);
	grid& get_grid() { return grid_; }
	void layout_initialize(const bool full_initialization) override;

	void clear_before_destruct();

private:
	int calculate_ypos();

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/**
	 * Our parent node.
	 *
	 * All nodes except the root node have a parent node.
	 */
	tree_view_node* parent_node_;

	/** The tree view that owns us. */
	tree_view* tree_view_;

	/** Grid holding our contents. */
	grid grid_;

	/**
	 * Our children.
	 *
	 * We want the returned child nodes to remain stable so store pointers.
	 */
	node_children_vector children_;

	/** The toggle for the folded state. */
	selectable_item* toggle_;

	/** The label to show our selected state. */
	selectable_item* label_;

	bool unfolded_;
	void fold_internal();
	void unfold_internal();

	/**
	 * "Inherited" from widget.
	 *
	 * This version needs to call its children, which are it's child nodes.
	 */
	void impl_populate_dirty_list(window& caller,
								  const std::vector<widget*>& call_stack);

	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	point calculate_best_size(const int indentation_level,
							   const unsigned indentation_step_size) const;
	/** @param assume_visible if false (default) it will return 0 if the parent node is folded*/
	point get_current_size(bool assume_visible = false) const;
	point get_folded_size() const;
	point get_unfolded_size() const;

	/** See @ref widget::set_origin. */
	virtual void set_origin(const point& origin) override;

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	unsigned
	place(const unsigned indentation_step_size, point origin, unsigned width);

	/** See @ref widget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) override;

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) override;

	// FIXME rename to icon
	void signal_handler_left_button_click(const event::ui_event event);

	void signal_handler_label_left_button_click(const event::ui_event event,
												bool& handled,
												bool& halt);

	void
	init_grid(grid* grid,
			  const std::map<std::string /* widget id */, string_map>& data);

	/**
	 * Returns the control_type of the @ref tree_view_node.
	 *
	 * This class does not derive from @ref styled_widget but the function behaves
	 * similar as @ref styled_widget::get_control_type.
	 */
	const std::string& get_control_type() const;
};

} // namespace gui2
