/*
	Copyright (C) 2012 - 2024
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

#include "gui/core/window_builder.hpp"
#include "gui/widgets/widget.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct builder_viewport;
} // namespace implementation

class viewport : public widget
{
	friend struct viewport_implementation;

public:
	viewport(const implementation::builder_viewport& builder,
			  const builder_widget::replacements_map& replacements);

	~viewport();

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/** See @ref widget::layout_initialize. */
	virtual void layout_initialize(const bool full_initialization) override;

	/** See @ref widget::impl_draw_children. */
	virtual void impl_draw_children() override;

	/** See @ref widget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) override;

	/** See @ref widget::find_at. */
	virtual widget* find_at(const point& coordinate,
							 const bool must_be_active) override;

	/** See @ref widget::find_at. */
	virtual const widget* find_at(const point& coordinate,
								   const bool must_be_active) const override;

	/** See @ref widget::find. */
	widget* find(const std::string_view id, const bool must_be_active) override;

	/** See @ref widget::find. */
	const widget* find(const std::string_view id, const bool must_be_active) const override;

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

	/** See @ref widget::create_walker. */
	virtual iteration::walker_ptr create_walker() override;

private:
	std::unique_ptr<widget> widget_;
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_viewport : public builder_widget
{
	explicit builder_viewport(const config& cfg);

	virtual std::unique_ptr<widget> build() const override;

	virtual std::unique_ptr<widget> build(const replacements_map& replacements) const override;

	builder_widget_ptr widget_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
