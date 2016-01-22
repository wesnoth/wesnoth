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

#ifndef GUI_WIDGETS_TREE_VIEW_NODE_HPP_INCLUDED
#define GUI_WIDGETS_TREE_VIEW_NODE_HPP_INCLUDED

#include "gui/auxiliary/window_builder/tree_view.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

namespace gui2
{

class tselectable_;
class ttoggle_button;
class ttree_view;

class ttree_view_node : public twidget
{
	friend struct ttree_view_node_implementation;
	friend class ttree_view;

public:
	typedef implementation::tbuilder_tree_view::tnode tnode_definition;
	ttree_view_node(
			const std::string& id,
			const std::vector<tnode_definition>& node_definitions,
			ttree_view_node* parent_node,
			ttree_view& parent_tree_view,
			const std::map<std::string /* widget id */, string_map>& data);

	~ttree_view_node();

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
	ttree_view_node&
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
	ttree_view_node&
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
		return parent_node_ == NULL;
	}

	/**
	 * The indention level of the node.
	 *
	 * The root node starts at level 0.
	 */
	unsigned get_indention_level() const;

	/** Does the node have children? */
	bool empty() const
	{
		return children_.empty();
	}

	/** Is the node folded? */
	bool is_folded() const;

#if 0
	enum texpand_mode
	{
		  recursive_restore // recursively restores collapse mode
		, recursive_expand // recursively expands the children
		, not_recursive
	};



	// If recursive all children will be closed recursively causing
	// restore expaning not to expand anything
//		void fold(const bool recursive); // FIXME implement

//		void unfold(const texpand_mode mode); // FIXME implement
#endif

	/**
	 * See @ref twidget::create_walker.
	 *
	 * @todo Implement properly.
	 */
	virtual iterator::twalker_* create_walker() OVERRIDE
	{
		return NULL;
	}

	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const OVERRIDE;

	/**
	 * The "size" of the widget.
	 *
	 * @todo Rename this function, names to close to the size of the widget.
	 */
	size_t size() const
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
	ttree_view_node& parent_node();

	/** The const version of @ref parent_node. */
	const ttree_view_node& parent_node() const;

	ttree_view& tree_view();

	const ttree_view& tree_view() const;

	ttree_view_node& get_child_at(int index);
	/**
		calculates the node indicies that we need to get from the root node to this node.
	*/
	std::vector<int> describe_path();
private:

	int calculate_ypos();
	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE;

	/**
	 * Our parent node.
	 *
	 * All nodes except the root node have a parent node.
	 */
	ttree_view_node* parent_node_;

	/** The tree view that owns us. */
	ttree_view& tree_view_;

	/** Grid holding our contents. */
	tgrid grid_;

	/**
	 * Our children.
	 *
	 * We want the returned child nodes to remain stable so store pointers.
	 */
	boost::ptr_vector<ttree_view_node> children_;

	/**
	 * The node definitions known to use.
	 *
	 * This list is needed to create new nodes.
	 *
	 * @todo Maybe store this list in the tree_view to avoid copying the
	 * reference.
	 */
	const std::vector<tnode_definition>& node_definitions_;

	/** The icon to show the folded state. */
	ttoggle_button* icon_;

	/** The label to show our selected state. */
	tselectable_* label_;

	/**
	 * "Inherited" from twidget.
	 *
	 * This version needs to call its children, which are it's child nodes.
	 */
	void impl_populate_dirty_list(twindow& caller,
								  const std::vector<twidget*>& call_stack);

	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;

	tpoint calculate_best_size(const int indention_level,
							   const unsigned indention_step_size) const;
	/** @param assume_visible: if false (default) it will return 0 if the parent node is folded*/
	tpoint get_current_size(bool assume_visible = false) const;
	tpoint get_folded_size() const;
	tpoint get_unfolded_size() const;

	/** See @ref twidget::set_origin. */
	virtual void set_origin(const tpoint& origin) OVERRIDE;

	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	unsigned
	place(const unsigned indention_step_size, tpoint origin, unsigned width);

	/** See @ref twidget::set_visible_rectangle. */
	virtual void set_visible_rectangle(const SDL_Rect& rectangle) OVERRIDE;

	/** See @ref twidget::impl_draw_children. */
	virtual void impl_draw_children(surface& frame_buffer,
									int x_offset,
									int y_offset) OVERRIDE;

	// FIXME rename to icon
	void signal_handler_left_button_click(const event::tevent event);

	void signal_handler_label_left_button_click(const event::tevent event,
												bool& handled,
												bool& halt);

	void
	init_grid(tgrid* grid,
			  const std::map<std::string /* widget id */, string_map>& data);

	/**
	 * Returns the control_type of the @ref ttree_view_node.
	 *
	 * This class does not derive from @ref tcontrol but the function behaves
	 * similar as @ref tcontrol::get_control_type.
	 */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif
