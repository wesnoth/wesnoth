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

#pragma once

#include "gui/widgets/text_box_base.hpp"

namespace gui2
{
namespace implementation
{
struct builder_text_box;
}

// ------------ WIDGET -----------{

/**
 * Class for text input history.
 *
 * The history of text items can be stored in the preferences. This class
 * handles that. Every item needs an id by which the history is loaded and
 * saved.
 */
class text_history
{
public:
	/**
	 * Gets history that matches id.
	 *
	 * @param id                  The id of the history to look for.
	 * @param enabled             The enabled state of the history.
	 *
	 * @returns                   The history object.
	 */
	static text_history get_history(const std::string& id, const bool enabled);

	text_history() : history_(0), pos_(0), enabled_(false)
	{
	}

	/**
	 * Push string into the history.
	 *
	 * If the string is empty or the same as the last item in the history this
	 * function is a nop.
	 *
	 * @param text                   The text to push in the history.
	 */
	void push(const std::string& text);

	/**
	 * One step up in the history.
	 *
	 * Pushes text to the history if at the end.
	 *
	 * @param text                The text to push in the history.
	 *
	 * @returns                   The current value of the history.
	 */
	std::string up(const std::string& text = "");

	/**
	 * One step down in the history.
	 *
	 * Pushes text to the history if at the end.
	 *
	 * @param text                The text to push in the history.
	 *
	 * @returns                   The current value of the history.
	 */
	std::string down(const std::string& text = "");

	/**
	 * Gets the current history value.
	 *
	 * @returns                   If enabled return the current history
	 *                            position, otherwise an empty string is
	 *                            returned.
	 */
	std::string get_value() const;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_enabled(bool enabled = true)
	{
		enabled_ = enabled;
	}
	bool get_enabled() const
	{
		return enabled_;
	}

private:
	text_history(std::vector<std::string>* history, const bool enabled)
		: history_(history), pos_(history->size()), enabled_(enabled)
	{
	}

	/** The items in the history. */
	std::vector<std::string>* history_;

	/** The current position in the history. */
	unsigned pos_;

	/** Is the history enabled. */
	bool enabled_;
};

/** Class for a single line text area. */
class text_box : public text_box_base
{
	friend struct implementation::builder_text_box;

public:
	explicit text_box(const implementation::builder_styled_widget& builder);

	/** Saves the text in the widget to the history. */
	void save_to_history()
	{
		history_.push(get_value());
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_history(const std::string& id)
	{
		history_ = text_history::get_history(id, true);
	}

	void set_max_input_length(const size_t length)
	{
		max_input_length_ = length;
	}

	void clear()
	{
		set_value("");
	}

protected:
	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::update_canvas. */
	virtual void update_canvas() override;

	/** Inherited from text_box_base. */
	void goto_end_of_line(const bool select = false) override
	{
		goto_end_of_data(select);
	}

	/** Inherited from text_box_base. */
	void goto_start_of_line(const bool select = false) override
	{
		goto_start_of_data(select);
	}

	/** Inherited from text_box_base. */
	void delete_char(const bool before_cursor) override;

	/** Inherited from text_box_base. */
	void delete_selection() override;

	void handle_mouse_selection(point mouse, const bool start_selection);

private:
	/** The history text for this widget. */
	text_history history_;

	/** The maximum length of the text input. */
	size_t max_input_length_;

	/**
	 * The x offset in the widget where the text starts.
	 *
	 * This value is needed to translate a location in the widget to a location
	 * in the text.
	 */
	unsigned text_x_offset_;

	/**
	 * The y offset in the widget where the text starts.
	 *
	 * Needed to determine whether a click is on the text.
	 */
	unsigned text_y_offset_;

	/**
	 * The height of the text itself.
	 *
	 * Needed to determine whether a click is on the text.
	 */
	unsigned text_height_;

	/** Updates text_x_offset_ and text_y_offset_. */
	void update_offsets();

	/** Is the mouse in dragging mode, this affects selection in mouse move */
	bool dragging_;

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Unhandled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_up_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
	{
	}

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Unhandled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_down_arrow(SDL_Keymod /*modifier*/, bool& /*handled*/) override
	{
	}

	/**
	 * Goes one item up in the history.
	 *
	 * @returns                   True if there's a history, false otherwise.
	 */
	bool history_up();

	/**
	 * Goes one item down in the history.
	 *
	 * @returns                   True if there's a history, false otherwise.
	 */
	bool history_down();

	/** Inherited from text_box_base. */
	void handle_key_tab(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from text_box_base. */
	void handle_key_clear_line(SDL_Keymod modifier, bool& handled) override;

	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_mouse_motion(const event::ui_event event,
									 bool& handled,
									 const point& coordinate);

	void signal_handler_left_button_down(const event::ui_event event,
										 bool& handled);

	void signal_handler_left_button_up(const event::ui_event event,
									   bool& handled);

	void signal_handler_left_button_double_click(const event::ui_event event,
												 bool& handled);
};

// }---------- DEFINITION ---------{

struct text_box_definition : public styled_widget_definition
{
	explicit text_box_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		typed_formula<unsigned> text_x_offset;
		typed_formula<unsigned> text_y_offset;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_text_box : public builder_styled_widget
{
public:
	explicit builder_text_box(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;

	std::string history;

	size_t max_input_length;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
