/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SLIDER_HPP_INCLUDED
#define GUI_WIDGETS_SLIDER_HPP_INCLUDED

#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/scrollbar.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

/** A slider. */
class slider : public scrollbar_base, public integer_selector
{
public:
	slider();

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** See @ref widget::calculate_best_size. */
	virtual point calculate_best_size() const override;

public:
	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from integer_selector. */
	void set_value(const int value) override;

	/** Inherited from integer_selector. */
	int get_value() const override
	{
		return minimum_value_ + get_item_position() * get_step_size();
	}

	/** Inherited from integer_selector. */
	void set_minimum_value(const int minimum_value) override;

	/** Inherited from integer_selector. */
	int get_minimum_value() const override
	{
		return minimum_value_;
	}

	/** Inherited from integer_selector. */
	void set_maximum_value(const int maximum_value) override;

	/** Inherited from integer_selector. */
	int get_maximum_value() const override
	// The number of items needs to include the begin and end so count - 1.
	{
		return minimum_value_ + get_item_count() - 1;
	}
	typedef std::function<t_string(int /*current position*/, int /*num positions*/)> tlabel_creator;
	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_best_slider_length(const unsigned length)
	{
		best_slider_length_ = length;
		set_is_dirty(true);
	}

	void set_minimum_value_label(const t_string& minimum_value_label)
	{
		minimum_value_label_ = minimum_value_label;
	}

	void set_maximum_value_label(const t_string& maximum_value_label)
	{
		maximum_value_label_ = maximum_value_label;
	}

	void set_value_labels(const std::vector<t_string>& value_labels);

	void set_value_labels(const tlabel_creator& value_labels)
	{
		value_labels_ = value_labels;
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
	/** Inherited from tscrollbar. */
	void child_callback_positioner_moved() override;

private:
	/** The best size for the slider part itself, if 0 ignored. */
	unsigned best_slider_length_;

	/**
	 * The minimum value the slider holds.
	 *
	 * The maximum value is minimum + item_count_.
	 * The current value is minimum + item_position_.
	 */
	int minimum_value_;

	/** Inherited from tscrollbar. */
	unsigned get_length() const override
	{
		return get_width();
	}

	/** Inherited from tscrollbar. */
	unsigned minimum_positioner_length() const override;

	/** Inherited from tscrollbar. */
	unsigned maximum_positioner_length() const override;

	/** Inherited from tscrollbar. */
	unsigned offset_before() const override;

	/** Inherited from tscrollbar. */
	unsigned offset_after() const override;

	/** Inherited from tscrollbar. */
	bool on_positioner(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	int on_bar(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	bool in_orthogonal_range(const point& coordinate) const override;

	/** Inherited from tscrollbar. */
	int get_length_difference(const point& original, const point& current) const override
	{
		return current.x - original.x;
	}

	/** Inherited from tscrollbar. */
	//void move_positioner(const int distance) override;

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
	 * This allows the slider to show custom texts instead of the values.
	 * This vector should have the same amount of items as options for the
	 * sliders. When set these texts are shown instead of the values. It also
	 * overrides minimum_value_label_ and maximum_value_label_.
	 */
	tlabel_creator value_labels_;

	/**
	 * When initially pessing the positioner and every time a new value is chosen through dragging,
	 * this value is upda with the mouse position at the time. This allows the widget to track
	 * how far the mouse has moved since setting the last value.
	 */
	point current_item_mouse_position_;

	//void update_current_item_mouse_position();

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/**
	 * Handlers for keyboard input
	 */
	void handle_key_decrease(bool& handled);
	void handle_key_increase(bool& handled);

	/**
	 * Signal handlers:
	 */
	void signal_handler_sdl_key_down(const event::ui_event event,
									 bool& handled,
									 const SDL_Keycode key);

	//void signal_handler_left_button_down(const event::ui_event event, bool& handled);

	// In this subclass, only used to grab keyboard focus -
	// see tscrollbar class for more handling of this event.
	void signal_handler_left_button_up(const event::ui_event event,
									   bool& handled);
};

// }---------- DEFINITION ---------{

struct slider_definition : public styled_widget_definition
{
	explicit slider_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		unsigned minimum_positioner_length;
		unsigned maximum_positioner_length;

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

	widget* build() const;

private:
	unsigned best_slider_length_;
	int minimum_value_;
	int maximum_value_;
	unsigned step_size_;
	int value_;

	t_string minimum_value_label_;
	t_string maximum_value_label_;

	std::vector<t_string> value_labels_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
