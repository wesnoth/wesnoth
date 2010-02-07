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

#include "config.hpp"
#include "gui/widgets/scrollbar_container.hpp"
#include "gui/auxiliary/window_builder/tree_view.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

namespace gui2 {

class tselectable_;
class ttoggle_button;

class ttree_view
		: public tscrollbar_container
{
	friend struct implementation::tbuilder_tree_view;
public:

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


	/***** ***** ***** ***** Node handling. ***** ***** ****** *****/

	class tnode
		: public twidget
	{
		friend struct ttree_view_node_implementation;
		friend class ttree_view;
		tnode(const std::string& id
				, const std::vector<tnode_definition>& node_definitions
				, tnode* parent
				, ttree_view* parent_widget
				, const std::map<
					std::string /* widget id */, string_map>& data);
	public:

		/**
		 * Adds a child item to the list.
		 */
		tnode& add_child(const std::string& id
				, const std::map<std::string /* widget id */, string_map>& data
				, const int index = -1);

		/**
		 * Adds a sibbling to the end of the list.
		 */
		tnode& add_sibling(const std::string& id
				, const std::map<std::string /* widget id */, string_map>& data)
		{
			assert(!is_root_node());
			return parent().add_child(id, data);
		}

		bool is_root_node() const { return parent_ == NULL; }

		/**
		 * Returns the parent node, can't be used on the root node.
		 */
		tnode& parent();

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
	private:

		void request_reduce_width(unsigned int) {}

		tnode* parent_;

		ttree_view* parent_widget_;

		tgrid grid_;

		// We want the returned child nodes to remain stable.
		boost::ptr_vector<tnode> children_;

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

		tpoint calculate_best_size() const 
		{
			return calculate_best_size(-1, 40); // FIXME step only negative
		}

		bool disable_click_dismiss() const { return true; }

		tpoint calculate_best_size(const int indention_level
				, const unsigned indention_step_size) const;

		void set_origin(const tpoint& origin);

		void set_size(const tpoint& origin, const tpoint& size);

		unsigned set_size(
				  const unsigned indention_step_size
				, tpoint origin);

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

	tnode& get_root_node() { return *root_node_; }

	tnode& add_node(const std::string& id
			, const std::map<std::string /* widget id */, string_map>& data)
	{
		return get_root_node().add_child(id, data);
	}

	void remove_node(tnode* node);

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

	tnode* selected_item() { return selected_item_; }

	const tnode* selected_item() const { return selected_item_; }

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

	tnode* root_node_;

	tnode* selected_item_;

	boost::function<void ()> selection_change_callback_;

	/** Inherited from tcontainer_. */
	virtual void finalize_setup();

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;

};

} // namespace gui2

#endif

