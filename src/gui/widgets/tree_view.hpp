/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_TREE_VIEW_HPP_INCLUDED
#define GUI_WIDGETS_TREE_VIEW_HPP_INCLUDED

#include "gui/widgets/scrollbar_container.hpp"
#include "gui/auxiliary/window_builder/tree_view.hpp"

namespace gui2
{

class ttree_view_node;

class ttree_view : public tscrollbar_container
{
	friend struct implementation::tbuilder_tree_view;
	friend class ttree_view_node;

public:
	typedef implementation::tbuilder_tree_view::tnode tnode_definition;

	explicit ttree_view(const std::vector<tnode_definition>& node_definitions);

	using tscrollbar_container::finalize_setup;

	ttree_view_node& get_root_node()
	{
		return *root_node_;
	}

	ttree_view_node&
	add_node(const std::string& id,
			 const std::map<std::string /* widget id */, string_map>& data);

	void remove_node(ttree_view_node* tree_view_node);

	/** See @ref twidget::child_populate_dirty_list. */
	virtual void
	child_populate_dirty_list(twindow& caller,
							  const std::vector<twidget*>& call_stack) OVERRIDE;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) OVERRIDE;

	bool empty() const;

	/** See @ref twidget::layout_children. */
	virtual void layout_children() OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_indention_step_size(const unsigned indention_step_size)
	{
		indention_step_size_ = indention_step_size;
	}

	ttree_view_node* selected_item()
	{
		return selected_item_;
	}

	const ttree_view_node* selected_item() const
	{
		return selected_item_;
	}

	void set_selection_change_callback(boost::function<void(twidget&)> callback)
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

	bool need_layout_;

	ttree_view_node* root_node_;

	ttree_view_node* selected_item_;

	boost::function<void(twidget&)> selection_change_callback_;

	/**
	 * Resizes the content.
	 *
	 * The resize either happens due to resizing the content or invalidate the
	 * layout of the window.
	 *
	 * @param width_modification  The wanted modification to the width:
	 *                            * negative values reduce width.
	 *                            * zero leave width as is.
	 *                            * positive values increase width.
	 * @param height_modification The wanted modification to the height:
	 *                            * negative values reduce height.
	 *                            * zero leave height as is.
	 *                            * positive values increase height.
	 */
	void resize_content(const int width_modification,
						const int height_modification);

	/** Layouts the children if needed. */
	void layout_children(const bool force);

	/** Inherited from tcontainer_. */
	virtual void finalize_setup();

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::tevent event);
};

} // namespace gui2

#endif
