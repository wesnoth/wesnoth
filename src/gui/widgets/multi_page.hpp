/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef GUI_WIDGETS_MULTI_PAGE_HPP_INCLUDED
#define GUI_WIDGETS_MULTI_PAGE_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2
{

namespace implementation
{
struct tbuilder_multi_page;
}

class tgenerator_;

/** The multi page class. */
class tmulti_page : public tcontainer_
{
	friend struct implementation::tbuilder_multi_page;
	friend class tdebug_layout_graph;

public:
	tmulti_page();

	/***** ***** ***** ***** Page handling. ***** ***** ****** *****/

	/**
	 * Adds single page to the grid.
	 *
	 * This function expect a page to one multiple widget.
	 *
	 * @param item                The data to send to the set_members of the
	 *                            widget.
	 */
	void add_page(const string_map& item);

	/**
	 * Adds single page to the grid.
	 *
	 * This function expect a page to have multiple widgets (either multiple
	 * columns or one column with multiple widgets).
	 *
	 *
	 * @param data                The data to send to the set_members of the
	 *                            widgets. If the member id is not an empty
	 *                            string it is only send to the widget that
	 *                            has the wanted id (if any). If the member
	 *                            id is an empty string, it is send to all
	 *                            members. Having both empty and non-empty
	 *                            id's gives undefined behavior.
	 */
	void
	add_page(const std::map<std::string /* widget id */, string_map>& data);

	/**
	 * Removes a page in the multi page.
	 *
	 * @param page                The page to remove, when not in
	 *                            range the function is ignored.
	 * @param count               The number of pages to remove, 0 means all
	 *                            pages (starting from page).
	 */
	void remove_page(const unsigned page, unsigned count = 1);

	/** Removes all pages in the multi page, clearing it. */
	void clear();

	/** Returns the number of pages. */
	unsigned get_page_count() const;

	/**
	 * Selectes a page.
	 *
	 * @param page                The page to select.
	 * @param select              Select or deselect the page.
	 */
	void select_page(const unsigned page, const bool select = true);

	/**
	 * Returns the selected page.
	 *
	 * @returns                   The selected page.
	 * @retval -1                 No page selected.
	 */
	int get_selected_page() const;

	/**
	 * Returns the grid for the page.
	 *
	 * @param page                The page to get the grid from, the caller
	 *                            has to make sure the page is a valid page.
	 *
	 * @returns                   The grid of the wanted page.
	 */
	const tgrid& page_grid(const unsigned page) const;

	/**
	 * Returns the grid for the page.
	 *
	 * @param page                The page to get the grid from, the caller
	 *                            has to make sure the page is a valid page.
	 *
	 * @returns                   The grid of the wanted page.
	 */
	tgrid& page_grid(const unsigned page);

	/***** ***** ***** inherited ***** ****** *****/

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_page_builder(tbuilder_grid_ptr page_builder)
	{
		page_builder_ = page_builder;
	}

private:
	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param page_data           The initial data to fill the widget with.
	 */
	void finalize(const std::vector<string_map>& page_data);

	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the tscrollbar_container super class and freed when it's grid is
	 * freed.
	 */
	tgenerator_* generator_;

	/** Contains the builder for the new items. */
	tbuilder_grid_const_ptr page_builder_;

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) OVERRIDE;
};

} // namespace gui2

#endif
