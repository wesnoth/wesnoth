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

#ifndef GUI_WIDGETS_TREE_VIEW_NODE_HPP_INCLUDED
#define GUI_WIDGETS_TREE_VIEW_NODE_HPP_INCLUDED

#include "gui/auxiliary/window_builder/tree_view.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

namespace gui2 {

class tselectable_;
class ttoggle_button;
class ttree_view;

class ttree_view_node
	: public twidget
{
	friend struct ttree_view_node_implementation;
	friend class ttree_view;

public:
	typedef implementation::tbuilder_tree_view::tnode tnode_definition;
	ttree_view_node(const std::string& id
			, const std::vector<tnode_definition>& node_definitions
			, ttree_view_node* parent_node
			, ttree_view* parent_widget
			, const std::map<
				std::string /* widget id */, string_map>& data);

	~ttree_view_node();

	/**
	 * Adds a child item to the list.
	 */
	ttree_view_node& add_child(const std::string& id
			, const std::map<std::string /* widget id */, string_map>& data
			, const int index = -1);

	/**
	 * Adds a sibbling to the end of the list.
	 */
	ttree_view_node& add_sibling(const std::string& id
			, const std::map<std::string /* widget id */, string_map>& data)
	{
		assert(!is_root_node());
		return parent_node().add_child(id, data);
	}

	bool is_root_node() const { return parent_node_ == NULL; }

	unsigned get_indention_level() const;

	/**
	 * Returns the parent node, can't be used on the root node.
	 */
	ttree_view_node& parent_node();
	const ttree_view_node& parent_node() const;

	bool empty() const { return children_.empty(); }

	enum texpand_mode
	{
		  recursive_restore // recursively restores collapse mode
		, recursive_expand // recursively expands the children
		, not_recursive
	};


	bool is_folded() const;

	// If recursive all children will be closed recursively causing
	// restore expaning not to expand anything
//		void fold(const bool recursive); // FIXME implement

//		void unfold(const texpand_mode mode); // FIXME implement

	twidget* find_at(const tpoint& coordinate, const bool must_be_active);

	const twidget* find_at(
			  const tpoint& coordinate
			, const bool must_be_active) const;

	/** Inherited from twidget.*/
	twidget* find(const std::string& id, const bool must_be_active)
	{
		twidget* result = twidget::find(id, must_be_active);
		return result ? result : grid_.find(id, must_be_active);
	}

	/** Inherited from twidget.*/
	const twidget* find(const std::string& id
			, const bool must_be_active) const
	{
		const twidget* result = twidget::find(id, must_be_active);
		return result ? result : grid_.find(id, must_be_active);
	}

	size_t size() const { return children_.size(); }

	void clear();

private:

	void request_reduce_width(unsigned int) {}

	ttree_view_node* parent_node_;

	ttree_view* parent_widget_;

	tgrid grid_;

	// We want the returned child nodes to remain stable.
	boost::ptr_vector<ttree_view_node> children_;

	const std::vector<tnode_definition>& node_definitions_;

	ttoggle_button* icon_;

	tselectable_* label_;

	/**
	 * "Inherited" from twidget.
	 *
	 * This version needs to call its children, which are it's child nodes.
	 */
	void impl_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);

	tpoint calculate_best_size() const;

	bool disable_click_dismiss() const { return true; }

	tpoint calculate_best_size(const int indention_level
			, const unsigned indention_step_size) const;

	tpoint get_current_size() const;
	tpoint get_folded_size() const { return grid_.get_size(); }
	tpoint get_unfolded_size() const;

	void set_origin(const tpoint& origin);

	void place(const tpoint& origin, const tpoint& size);

	unsigned place(
			  const unsigned indention_step_size
			, tpoint origin
			, unsigned width);

	void set_visible_area(const SDL_Rect& area);

	void impl_draw_children(surface& frame_buffer);

	// FIXME rename to icon
	void signal_handler_left_button_click(const event::tevent event);

	void signal_handler_label_left_button_click(
			  const event::tevent event
			, bool& handled
			, bool& halt);

	void init_grid(tgrid* grid
			, const std::map<
				std::string /* widget id */, string_map>& data);

	const std::string& get_control_type() const;

};

} // namespace gui2

#endif


