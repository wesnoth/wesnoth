/*
	Copyright (C) 2023 - 2024
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
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

#include "gui/widgets/scrollbar_container.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

#include "gui/widgets/multiline_text.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct builder_scroll_text;
}

class scroll_text : public scrollbar_container
{
	friend struct implementation::builder_scroll_text;

public:
	explicit scroll_text(const implementation::builder_scroll_text& builder);

//	/** See @ref styled_widget::set_label. */
	virtual void set_label(const t_string& label) override;

//	void set_value(const std::string text);

	void set_value(const t_string& label) {
		set_label(label);
	}

	std::string get_value();

	/** See @ref styled_widget::set_text_alignment. */
	virtual void set_text_alignment(const PangoAlignment text_alignment) override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	bool can_wrap() const override;
	void set_can_wrap(bool can_wrap);

	void set_link_aware(bool l);

	/** See @ref styled_widget::get_link_aware. */
	virtual bool get_link_aware() const override
	{
		return link_aware_;
	}

	void set_editable(bool editable)
	{
		editable_ = editable;
	}

	bool is_editable() const
	{
		return editable_;
	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
	};

	// It's not needed for now so keep it disabled, no definition exists yet.
	// void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	bool wrap_on_;

	PangoAlignment text_alignment_;

	bool editable_;

	point max_size_;

	bool link_aware_;

	void finalize_subclass() override;

	/** Used for moving scrollbars.
	    Has to be called from signal notify_modified, otherwise
	    doesn't work after invalidate_layout. */
	void refresh();

	unsigned get_horizontal_position() {
		assert(horizontal_scrollbar());
		return horizontal_scrollbar()->get_positioner_offset();
	}

	unsigned get_vertical_position() {
		assert(vertical_scrollbar());
		return vertical_scrollbar()->get_positioner_offset();
	}

	void place(const point& origin, const point& size) override;

	/** See @ref widget::calculate_best_size. */
	point calculate_best_size() const override;

	/** Sets the size of the text beyond which scrollbars should be visible. */
	void set_max_size(point max_size);

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

	multiline_text* get_internal_text_box();

private:
	/***** ***** ***** inherited ****** *****/

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_left_button_down(const event::ui_event event);
};

// }---------- DEFINITION ---------{

struct scroll_text_definition : public styled_widget_definition
{
	explicit scroll_text_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_scroll_text : public builder_scrollbar_container
{
	explicit builder_scroll_text(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	const PangoAlignment text_alignment;
	bool editable;
	bool link_aware;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
