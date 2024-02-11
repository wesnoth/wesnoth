/*
	Copyright (C) 2008 - 2024
	by babaissarkar(Subhraman Sarkar) <suvrax@gmail.com>
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

class label;
class spacer;

namespace implementation
{
struct builder_scroll_text;
}

/**
 * @ingroup GUIWidgetWML
 *
 * Scrollable text area
 *
 * This version shows a scrollbar if the text gets too long and has some scrolling features.
 * In general this widget is slower as the normal label so the normal label should be preferred.
 *
 * Key          |Type                        |Default  |Description
 * -------------|----------------------------|---------|-----------
 * grid         | @ref guivartype_grid "grid"|mandatory|A grid containing the widgets for main widget.
 *
 * TODO: we need one definition for a vertical scrollbar since this is the second time we use it.
 *
 * ID (return value)|Type                        |Default  |Description
 * -----------------|----------------------------|---------|-----------
 * _content_grid    | @ref guivartype_grid "grid"|mandatory|A grid which should only contain one label widget.
 * _scrollbar_grid  | @ref guivartype_grid "grid"|mandatory|A grid for the scrollbar (Merge with listbox info.)
 * The following states exist:
 * * state_enabled - the scroll label is enabled.
 * * state_disabled - the scroll label is disabled.
 * List with the scroll label specific variables:
 * Key                      |Type                                            |Default     |Description
 * -------------------------|------------------------------------------------|------------|-----------
 * vertical_scrollbar_mode  | @ref guivartype_scrollbar_mode "scrollbar_mode"|initial_auto|Determines whether or not to show the scrollbar.
 * horizontal_scrollbar_mode| @ref guivartype_scrollbar_mode "scrollbar_mode"|initial_auto|Determines whether or not to show the scrollbar.
 */
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

	/** See @ref styled_widget::set_use_markup. */
	virtual void set_use_markup(bool use_markup) override;

	/** See @ref container_base::set_self_active. */
	virtual void set_self_active(const bool active) override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	bool can_wrap() const override;
	void set_can_wrap(bool can_wrap);

	void set_editable(bool editable)
	{
		editable_ = editable;
	}

	bool is_editable()
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

	void place(const point& origin, const point& size);

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

struct builder_scroll_text : public builder_styled_widget
{
	explicit builder_scroll_text(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	scrollbar_container::scrollbar_mode vertical_scrollbar_mode;
	scrollbar_container::scrollbar_mode horizontal_scrollbar_mode;
	const PangoAlignment text_alignment;
	bool editable;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
