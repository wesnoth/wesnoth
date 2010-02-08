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

#ifndef GUI_WIDGETS_TREE_VIEW_HPP_INCLUDED
#define GUI_WIDGETS_TREE_VIEW_HPP_INCLUDED

#include "gui/widgets/scrollbar_container.hpp"
#include "gui/auxiliary/window_builder/tree_view.hpp"

// Can be changed to forward declaration once ilor is finished with the lobby.
#include "gui/widgets/tree_view_node.hpp"

namespace gui2 {

class ttree_view
		: public tscrollbar_container
{
	friend struct implementation::tbuilder_tree_view;
	friend class ttree_view_node;
public:

	// Can be removed once ilor is done with the  lobby.
	typedef ttree_view_node tnode;
	typedef implementation::tbuilder_tree_view::tnode tnode_definition;

	/**
	 * Constructor.
	 *
	 * @param has_minimum         Does the listbox need to have one item
	 *                            selected.
	 * @param has_maximum         Can the listbox only have one item
	 *                            selected.
	 * @param placement           How are the items placed.
	 * @param select              Select an item when selected, if false it
	 *                            changes the visible state instead.
	 */
	ttree_view(const std::vector<tnode_definition>& node_definitions);

	using tscrollbar_container::finalize_setup;

	ttree_view_node& get_root_node() { return *root_node_; }

	ttree_view_node& add_node(const std::string& id
			, const std::map<std::string /* widget id */, string_map>& data)
	{
		return get_root_node().add_child(id, data);
	}

	void remove_node(ttree_view_node* tree_view_node);

	/** Inherited from tscrollbar_container. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/)  {}
//		{ state_ = active ? ENABLED : DISABLED; }

	bool empty() const { return root_node_->empty(); }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_indention_step_size(const unsigned indention_step_size)
	{
		indention_step_size_ = indention_step_size;
	}

	ttree_view_node* selected_item() { return selected_item_; }

	const ttree_view_node* selected_item() const { return selected_item_; }

	void set_selection_change_callback(boost::function<void()> callback)
	{
		selection_change_callback_ = callback;
	}

protected:

	/***** ***** ***** ***** keyboard functions ***** ***** ***** *****/
#if 0
	/** Inherited from tscrollbar_container. */
	void handle_key_up_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_down_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_left_arrow(SDLMod modifier, bool& handled);

	/** Inherited from tscrollbar_container. */
	void handle_key_right_arrow(SDLMod modifier, bool& handled);
#endif
private:

	/**
	 * @todo evaluate which way the dependancy should go.
	 *
	 * We no depend on the implementation, maybe the implementation should
	 * depend on us instead.
	 */
	const std::vector<tnode_definition> node_definitions_;

	unsigned indention_step_size_;

	ttree_view_node* root_node_;

	ttree_view_node* selected_item_;

	boost::function<void ()> selection_change_callback_;

	/** Inherited from tcontainer_. */
	virtual void finalize_setup();

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;

};

} // namespace gui2

#endif

