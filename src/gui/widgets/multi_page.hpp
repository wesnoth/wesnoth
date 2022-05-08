/*
	Copyright (C) 2008 - 2022
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

#pragma once

#include "gui/widgets/container_base.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct builder_multi_page;
}

class generator_base;

/**
 * @ingroup GUIWidgetWML
 *
 * A multi page is a control that contains several 'pages' of which only one is visible.
 * The pages can contain the same widgets containing the same 'kind' of data or look completely different.
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|Defines the grid with the widgets to place on the page.
 * A multipage has no states. List with the multi page specific variables:
 * Key            |Type                              |Default  |Description
 * ---------------|----------------------------------|---------|-----------
 * page_definition| @ref guivartype_section "section"|mandatory|This defines how a multi page item looks. It must contain the grid definition for at least one page.
 * page_data      | @ref guivartype_section "section"|[]       |A grid alike section which stores the initial data for the multi page. Every row must have the same number of columns as the 'page_definition'.
 */
class multi_page : public container_base
{
	friend struct implementation::builder_multi_page;
	friend class debug_layout_graph;

public:
	explicit multi_page(const implementation::builder_multi_page& builder);

	/***** ***** ***** ***** Page handling. ***** ***** ****** *****/

	/**
	 * Adds single page to the grid.
	 *
	 * This function expect a page to one multiple widget.
	 *
	 * @param item                The data to send to the set_members of the
	 *                            widget.
	 *
	 * @returns                   The grid of the newly added page.
	 */
	grid& add_page(const string_map& item);

	/**
	 * Adds single page to the grid.
	 *
	 * This function expect a page to one multiple widget.
	 *
	 * @param item                The data to send to the set_members of the
	 *                            widget.
	 *
	 * @param type                the id of the [page_definition] that shoduol be used
	 *
	 * @param insert_pos          the position where th new page is inserted, usually
	 *                            -1 for 'at end'
	 *
	 * @returns                   The grid of the newly added page.
	 */
	grid& add_page(const std::string& type, int insert_pos, const string_map& item);

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
	 *
	 * @returns                   The grid of the newly added page.
	 */
	grid& add_page(const std::map<std::string /* widget id */, string_map>& data);

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
	 *
	 * @param type                the id of the [page_definition] that should be used
	 *
	 * @param insert_pos          the position where th new page is inserted, usually
	 *                            -1 for 'at end'
	 *
	 * @returns                   The grid of the newly added page.
	 */
	grid& add_page(const std::string& type, int insert_pos, const std::map<std::string /* widget id */, string_map>& data);

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
	 * Selects a page.
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
	const grid& page_grid(const unsigned page) const;

	/**
	 * Returns the grid for the page.
	 *
	 * @param page                The page to get the grid from, the caller
	 *                            has to make sure the page is a valid page.
	 *
	 * @returns                   The grid of the wanted page.
	 */
	grid& page_grid(const unsigned page);

	/***** ***** ***** inherited ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

private:
	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_page_builders(const builder_grid_map& page_builders)
	{
		assert(!page_builders.empty());
		page_builders_ = page_builders;
	}

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param generator           Generator for the list
	 * @param page_data           The initial data to fill the widget with.
	 */
	void finalize(std::unique_ptr<generator_base> generator, const std::vector<string_map>& page_data);

	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the scrollbar_container super class and freed when it's grid is freed.
	 */
	generator_base* generator_;

	/** Contains the builder for the new items. */
	builder_grid_map page_builders_;

	/** See @ref widget::impl_draw_background. */
	virtual void impl_draw_background(int x_offset, int y_offset) override;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;
};

// }---------- DEFINITION ---------{

struct multi_page_definition : public styled_widget_definition
{
	explicit multi_page_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_multi_page : public builder_styled_widget
{
	explicit builder_multi_page(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	builder_grid_map builders;

	/**
	 * Multi page data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<std::map<std::string, t_string>> data;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
