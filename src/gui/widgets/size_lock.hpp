/*
   Copyright (C) 2016 - 2017 Jyrki Vesterinen <sandgtx@gmail.com>
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

#include <gui/widgets/container_base.hpp>

#include <gui/auxiliary/typed_formula.hpp>
#include <gui/core/widget_definition.hpp>
#include <gui/core/window_builder.hpp>
#include <gui/widgets/generator.hpp>

namespace gui2
{

namespace implementation
{
struct builder_size_lock;
}

/* A fixed-size widget that wraps an arbitrary widget and forces it to the given size. */

class size_lock : public container_base
{
	friend struct implementation::builder_size_lock;

public:
	explicit size_lock(const implementation::builder_size_lock& builder);

	/** See @ref control::get_active. */
	bool get_active() const override
	{
		return true;
	}

	/** See @ref control::get_state. */
	unsigned get_state() const override
	{
		return 0;
	}

	/** See @ref widget::place. */
	void place(const point& origin, const point& size) override;

	/** See @ref widget::layout_children. */
	void layout_children() override;

	void set_target_size(const point& size)
	{
		size_ = size;
	}

protected:
	point calculate_best_size() const override
	{
		return size_;
	}

private:
	point size_;

	/**
	 * Points to the actual widget.
	 *
	 * The widget is owned by container_base (the base class).
	 */
	widget* widget_;

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param widget_builder      The builder to build the contents of the
	 *                            widget.
	 */
	void finalize(builder_widget_const_ptr widget_builder);

	/** See @ref control::get_control_type. */
	const std::string& get_control_type() const override
	{
		static const std::string control_type = "size_lock";
		return control_type;
	}

	/** See @ref container_::set_self_active. */
	void set_self_active(const bool) override
	{
		// DO NOTHING
	}
};

struct size_lock_definition : public styled_widget_definition
{
	explicit size_lock_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

namespace implementation
{

struct builder_size_lock : public builder_styled_widget
{
	explicit builder_size_lock(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

private:
	builder_widget_const_ptr content_;
	typed_formula<unsigned> width_;
	typed_formula<unsigned> height_;
};
}
}
