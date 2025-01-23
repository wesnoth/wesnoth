/*
	Copyright (C) 2008 - 2024
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

#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/slider_base.hpp"

#include "gui/core/widget_definition.hpp"

namespace gui2
{
namespace implementation
{
struct builder_slider;
}

// ------------ WIDGET -----------{

class slider : public slider_base, public integer_selector
{
	friend struct implementation::builder_slider;

public:
	explicit slider(const implementation::builder_slider& builder);

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from integer_selector. */
	virtual void set_value(int value) override;

	/** Inherited from integer_selector. */
	virtual int get_value() const override
	{
		return minimum_value_ + get_slider_position() * get_step_size();
	}

	/** Inherited from integer_selector. */
	virtual int get_minimum_value() const override
	{
		return minimum_value_;
	}

	/** Inherited from integer_selector. */
	virtual int get_maximum_value() const override
	{
		// The number of items needs to include the begin and end so count - 1.
		return minimum_value_ + slider_get_item_last() * step_size_;
	}

	int get_item_count() const
	{
		assert(step_size_ != 0);
		return slider_get_item_last() * step_size_ + 1;
	}

	unsigned get_step_size() const
	{
		return step_size_;
	}

	void set_step_size(int step_size);

	/***** ***** ***** setters / getters for members ***** ****** *****/
	void set_best_slider_length(const unsigned length)
	{
		best_slider_length_ = length;
		queue_redraw(); // TODO: draw_manager - does the above change the size?
	}

	unsigned get_best_slider_length() const
	{
		return best_slider_length_;
	}

	void set_value_range(int min_value, int max_value);

	void set_minimum_value_label(const t_string& minimum_value_label)
	{
		minimum_value_label_ = minimum_value_label;
	}

	t_string get_minimum_value_label() const
	{
		return minimum_value_label_;
	}

	void set_maximum_value_label(const t_string& maximum_value_label)
	{
		maximum_value_label_ = maximum_value_label;
	}

	t_string get_maximum_value_label() const
	{
		return maximum_value_label_;
	}

	void set_value_labels(const std::vector<t_string>& value_labels);

	using label_generator = std::function<t_string(int /*current position*/, int /*num positions*/)>;

	void set_value_labels(const label_generator& generator)
	{
		value_label_generator_ = generator;
	}

	/**
	 * Returns the label shown for the current value.
	 *
	 * @returns                   The label for the current value, if no label
	 *                            for the current label is defined, it returns
	 *                            the result of get_value().
	 */
	t_string get_value_label() const;

protected:
	/** Inherited from scrollbar_base. */
	virtual void child_callback_positioner_moved() override;

private:
	/** The best size for the slider part itself, if 0 ignored. */
	unsigned best_slider_length_;

	/**
	 * The minimum value the slider holds.
	 *
	 * The maximum value is minimum + item_last_.
	 * The current value is minimum + item_position_.
	 */
	int minimum_value_;
	int step_size_;

	/** Inherited from scrollbar_base. */
	virtual unsigned get_length() const override
	{
		return get_width();
	}

	/** Inherited from scrollbar_base. */
	int positioner_length() const override;

	/** Inherited from scrollbar_base. */
	unsigned offset_before() const override;

	/** Inherited from scrollbar_base. */
	unsigned offset_after() const override;

	/** Inherited from scrollbar_base. */
	bool on_positioner(const point& coordinate) const override;

	/** Inherited from scrollbar_base. */
	int on_bar(const point& coordinate) const override;

	/** Inherited from scrollbar_base. */
	int get_length_difference(const point& original, const point& current) const override
	{
		return current.x - original.x;
	}

	/** Inherited from scrollbar_base. */
	// void move_positioner(const int distance) override;

	/** See @ref styled_widget::update_canvas. */
	virtual void update_canvas() override;

	/**
	 * When the slider shows the minimum value can show a special text.
	 * If this text is not empty this text is shown else the minimum value.
	 */
	t_string minimum_value_label_;

	/**
	 * When the slider shows the maximum value can show a special text.
	 * If this text is not empty this text is shown else the maximum value.
	 */
	t_string maximum_value_label_;

	/**
	 * Function to output custom value labels for the slider. When set
	 * its output is shown instead of the numeric values. It also overrides
	 * minimum_value_label_ and maximum_value_label_.
	 */
	label_generator value_label_generator_;

	/**
	 * When initially pressing the positioner and every time a new value is chosen through dragging,
	 * this value is updated with the mouse position at the time. This allows the widget to track
	 * how far the mouse has moved since setting the last value.
	 */
	point current_item_mouse_position_;

	// void update_current_item_mouse_position();

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/**
	 * Handlers for keyboard input
	 */
	void handle_key_decrease(bool& handled);
	void handle_key_increase(bool& handled);

	/**
	 * Signal handlers:
	 */
	void signal_handler_sdl_key_down(const event::ui_event event, bool& handled, const SDL_Keycode key);

	// void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	// In this subclass, only used to grab keyboard focus -
	// see scrollbar_base class for more handling of this event.
	void signal_handler_left_button_up(const event::ui_event event, bool& handled);
};

// }---------- DEFINITION ---------{

struct slider_definition : public styled_widget_definition
{
	explicit slider_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		unsigned positioner_length;

		unsigned left_offset;
		unsigned right_offset;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{
struct builder_slider : public builder_styled_widget
{
	explicit builder_slider(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	unsigned best_slider_length;

private:
	int minimum_value_;
	int maximum_value_;
	unsigned step_size_;
	int value_;

	t_string minimum_value_label_;
	t_string maximum_value_label_;

	/* This vector should have the same number of items as the slider's values. */
	std::vector<t_string> value_labels_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
