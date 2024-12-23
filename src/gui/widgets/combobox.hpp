/*
	Copyright (C) 2024
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

#include "gui/widgets/text_box_base.hpp"
#include "gui/dialogs/drop_down_menu.hpp"

namespace gui2
{
namespace implementation
{
struct builder_combobox;
}

// ------------ WIDGET -----------{

/**
 * Class for a combobox.
 * A widget that allows the user to input text or to select predefined options from a list
 * accessed by clicking the dropdown arrow.
 */

class combobox : public text_box_base
{
	friend struct implementation::builder_combobox;

public:
	explicit combobox(const implementation::builder_combobox& builder);

	void set_max_input_length(const std::size_t length)
	{
		max_input_length_ = length;
	}

    std::size_t get_max_input_length() const
    {
        return max_input_length_;
    }

    void set_hint_text(const std::string& text)
    {
        hint_text_ = text;
        update_canvas();
    }

    std::string get_hint_text() const
    {
        return hint_text_;
    }

    void set_hint_image(const std::string& image)
    {
        hint_image_ = image;
        update_canvas();
    }

    std::string get_hint_image() const
    {
        return hint_image_;
    }


	void set_hint_data(const std::string& text, const std::string& image)
	{
		hint_text_ = text;
		hint_image_ = image;

		update_canvas();
	}

	void clear()
	{
		set_value("");
	}

	int get_item_count() const
	{
		return values_.size();
	}

	void set_values(const std::vector<::config>& values, unsigned selected = 0);
	void set_selected(unsigned selected, bool fire_event = true);
	unsigned get_selected() const { return selected_; }

protected:
	/* **** ***** ***** ***** layout functions ***** ***** ***** **** */

	virtual void place(const point& origin, const point& size) override;

	/* **** ***** ***** ***** Inherited ***** ***** ***** **** */

	virtual void update_canvas() override;

	void goto_end_of_line(const bool select = false) override
	{
		goto_end_of_data(select);
	}

	void goto_start_of_line(const bool select = false) override
	{
		goto_start_of_data(select);
	}

	void delete_char(const bool before_cursor) override;

	void delete_selection() override;

	void handle_mouse_selection(point mouse, const bool start_selection);

private:

	/** The maximum length of the text input. */
	std::size_t max_input_length_;

	/** Size of the dropdown icon
	 * TODO : Should be dynamically loaded from image
	 */
	unsigned const ICON_SIZE = 25;

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

	/** Helper text to display (such as "Search") if the combo box is empty. */
	std::string hint_text_;

	/** Image (such as a magnifying glass) that accompanies the help text. */
	std::string hint_image_;

	std::vector<::config> values_;

	unsigned selected_;

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Handled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_up_arrow(SDL_Keymod /*modifier*/, bool& handled) override;

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Handled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_down_arrow(SDL_Keymod /*modifier*/, bool& handled) override;


	/** Inherited from text_box_base. */
	void handle_key_clear_line(SDL_Keymod modifier, bool& handled) override;

	/** Update the mouse cursor based on whether it is over button area or text area */
	void update_mouse_cursor();

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/* **** ***** ***** signal handlers ***** ****** **** */

	void signal_handler_mouse_enter(const event::ui_event /*event*/, bool& /*handled*/);
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

struct combobox_definition : public styled_widget_definition
{
	explicit combobox_definition(const config& cfg);

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

struct builder_combobox : public builder_styled_widget
{
public:
	explicit builder_combobox(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	std::size_t max_input_length;

	t_string hint_text;
	std::string hint_image;

private:
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
