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

#include "gui/widgets/text_box_base.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/dialogs/drop_down_menu.hpp"

namespace gui2
{
namespace implementation
{
struct builder_combobox;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * Class for a editable combobox.
 *
 * The resolution for a text box also contains the following keys:
 * Key          |Type                                    |Default  |Description
 * -------------|----------------------------------------|---------|-----------
 * text_x_offset| @ref guivartype_f_unsigned "f_unsigned"|""       |The x offset of the text in the text box. This is needed for the code to determine where in the text the mouse clicks, so it can set the cursor properly.
 * text_y_offset| @ref guivartype_f_unsigned "f_unsigned"|""       |The y offset of the text in the text box.
 * The following states exist:
 * * state_enabled - the text box is enabled.
 * * state_disabled - the text box is disabled.
 * * state_focussed - the text box has the focus of the keyboard.
 * The following variables exist:
 * Key          |Type                                |Default  |Description
 * -------------|------------------------------------|---------|-----------
 * label        | @ref guivartype_t_string "t_string"|""       |The initial text of the text box.
 * history      | @ref guivartype_string "string"    |""       |The name of the history for the text box. A history saves the data entered in a text box between the games. With the up and down arrow it can be accessed. To create a new history item just add a new unique name for this field and the engine will handle the rest.
 */
class combobox : public text_box_base
{
	friend struct implementation::builder_combobox;

public:
	explicit combobox(const implementation::builder_styled_widget& builder);

	std::vector<::config> values_;

	unsigned selected_;

	void set_max_input_length(const std::size_t length)
	{
		max_input_length_ = length;
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

	void set_values(const std::vector<::config>& values, unsigned selected = 0);

	void set_selected(unsigned selected, bool fire_event = true);

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

	/** Helper text to display (such as "Search") if the text box is empty. */
	std::string hint_text_;

	/** Image (such as a magnifying glass) that accompanies the help text. */
	std::string hint_image_;

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

	/***** ***** ***** signal handlers ***** ****** *****/

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

	std::string history;

	std::size_t max_input_length;

	t_string hint_text;
	std::string hint_image;

private:
	std::vector<::config> options_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
