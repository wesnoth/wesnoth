/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/viewport.hpp"

#include "gui/auxiliary/iterator/walker.hpp"
#include "gui/core/log.hpp"
#include "config.hpp"
#include "utils/const_clone.hpp"

#define LOG_SCOPE_HEADER "viewport [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * Helper to implement private functions without modifying the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions. It also facilitates to create duplicates of functions for a const
 * and a non-const member function.
 */
struct viewport_implementation
{
	/**
	 * Implementation for the wrappers for
	 * [const] widget* pane::find_at(const point&, const bool) [const].
	 *
	 * @tparam W                  A pointer to the pane.
	 */
	template <class W>
	static utils::const_clone_ptr<widget, W>
	find_at(W viewport, point coordinate, const bool must_be_active)
	{

		/*
		 * First test whether the mouse is at the pane.
		 */
		if(viewport->widget::find_at(coordinate, must_be_active) != viewport) {
			return nullptr;
		}

		/*
		 * The widgets are placed at coordinate 0,0 so adjust the offset to
		 * that coordinate system.
		 */
		coordinate.x -= viewport->get_x();
		coordinate.y -= viewport->get_y();

		return viewport->widget_->find_at(coordinate, must_be_active);
	}

	template <class W>
	static utils::const_clone_ptr<widget, W>
	find(W viewport, const std::string& id, const bool must_be_active)
	{
		if(viewport->widget::find(id, must_be_active)) {
			return viewport;
		} else {
			return viewport->widget_->find(id, must_be_active);
		}
	}
};

viewport::viewport(const implementation::builder_viewport& builder,
					 const builder_widget::replacements_map& replacements)
	: widget(builder)
	, widget_(builder.widget_->build(replacements))
{
	widget_->set_parent(this);
}

viewport::~viewport()
{
}

void viewport::place(const point& origin, const point& size)
{
	widget::place(origin, size);

	widget_->place(point(), widget_->get_best_size());
}

void viewport::layout_initialize(const bool full_initialization)
{
	widget::layout_initialize(full_initialization);

	if(widget_->get_visible() != widget::visibility::invisible) {
		widget_->layout_initialize(full_initialization);
	}
}

void viewport::impl_draw_children()
{
	if(widget_->get_visible() != widget::visibility::invisible) {
		widget_->draw_background();
		widget_->draw_children();
		widget_->draw_foreground();
	}
}

void viewport::request_reduce_width(const unsigned /*maximum_width*/)
{
}

widget* viewport::find_at(const point& coordinate, const bool must_be_active)
{
	return viewport_implementation::find_at(this, coordinate, must_be_active);
}

const widget* viewport::find_at(const point& coordinate,
								  const bool must_be_active) const
{
	return viewport_implementation::find_at(this, coordinate, must_be_active);
}

widget* viewport::find(const std::string& id, const bool must_be_active)
{
	return viewport_implementation::find(this, id, must_be_active);
}

const widget* viewport::find(const std::string& id, const bool must_be_active)
		const
{
	return viewport_implementation::find(this, id, must_be_active);
}

point viewport::calculate_best_size() const
{
	return widget_->get_best_size();
}

bool viewport::disable_click_dismiss() const
{
	return false;
}

iteration::walker_ptr viewport::create_walker()
{
	return nullptr;
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{viewport_description}
 *
 *        A viewport is an special widget used to view only a part of the
 *        widget it `holds'.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_viewport
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="viewport"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @begin{tag}{name="widget"}{min="1"}{max="1"}{super="gui/window/resolution/grid/row/column"}
 * == Label ==
 *
 * @macro = viewport_description
 *
 * List with the label specific variables:
 * @begin{table}{config}
 *     widget & section & &       Holds a single widget like a grid cell.$
 * @end{table}
 * @end{tag}{name="widget"}
 * @end{tag}{name="viewport"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_viewport::builder_viewport(const config& cfg)
	: builder_widget(cfg)
	, widget_(create_widget_builder(cfg.child("widget", "[viewport]")))
{
}

widget_ptr builder_viewport::build() const
{
	return build(replacements_map());
}

widget_ptr builder_viewport::build(const replacements_map& replacements) const
{
	return std::make_shared<viewport>(*this, replacements);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
