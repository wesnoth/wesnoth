/*
	Copyright (C) 2016 - 2024
	by Jyrki Vesterinen <sandgtx@gmail.com>
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

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

namespace implementation
{
struct builder_size_lock;
}

/**
 * @ingroup GUIWidgetWML
 *
 * A fixed-size widget that wraps an arbitrary widget and forces it to the given size.
 *
 * A size lock contains one child widget and forces it to have the specified size.
 * This can be used, for example, when there are two list boxes in different rows of the same grid
 * and it's desired that only one list box changes size when its contents change.
 *
 * A size lock has no states.
 * Key          |Type                                    |Default  |Description
 * -------------|----------------------------------------|---------|-----------
 * widget       | @ref guivartype_section "section"      |mandatory|The widget.
 * width        | @ref guivartype_f_unsigned "f_unsigned"|mandatory|The width of the widget.
 * height       | @ref guivartype_f_unsigned "f_unsigned"|mandatory|The height of the widget.
 */
class size_lock : public container_base
{
	friend struct implementation::builder_size_lock;

public:
	explicit size_lock(const implementation::builder_size_lock& builder);

	bool get_active() const override
	{
		return true;
	}

	unsigned get_state() const override
	{
		return 0;
	}

	/** See @ref widget::place. */
	void place(const point& origin, const point& size) override;

	/** See @ref widget::layout_children. */
	void layout_children() override;

protected:
	point calculate_best_size() const override;

private:
	typed_formula<unsigned> width_;
	typed_formula<unsigned> height_;

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
	void finalize(const builder_widget& widget_builder);

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/** See @ref container_base::set_self_active */
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

	virtual std::unique_ptr<widget> build() const override;

	typed_formula<unsigned> width_;
	typed_formula<unsigned> height_;

private:
	builder_widget_const_ptr content_;
};
}
}
