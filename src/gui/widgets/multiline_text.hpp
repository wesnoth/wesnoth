/*
	Copyright (C) 2008 - 2024
	by babaissarkar(Subhraman Sarkar) <suvrax@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/text_box_base.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/scrollbar_container.hpp"

namespace gui2
{
namespace implementation
{
struct builder_multiline_text;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * Base class for a multiline text area.
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
 * editable     | @ref guivartype_bool "bool"        |"true"   |If the contents of the text box can be edited.
 */
class multiline_text : public text_box_base
{
	friend struct implementation::builder_multiline_text;

public:
	explicit multiline_text(const implementation::builder_styled_widget& builder);

	/** See @ref widget::can_wrap. */
	bool can_wrap() const override
	{
		return false;
	}

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

	unsigned get_line_no()
	{
		set_line_num_from_offset();
		return line_num_;
	}

	point get_cursor_pos()
	{
		return get_cursor_position(get_selection_start());
	}

	int get_line_end_pos()
	{
		return get_cursor_position(get_line_end_offset(line_num_)).x;
	}

	point get_text_end_pos()
	{
		return get_cursor_position(get_length());
	}

protected:
	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** See @ref widget::place. */
	virtual void place(const point& origin, const point& size) override;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::update_canvas. */
	virtual void update_canvas() override;

	/** Inherited from text_box_base. */
	void insert_char(const std::string& unicode) override
	{
		text_box_base::insert_char(unicode);
		update_layout();
	}

	/** Inherited from text_box_base. */
	void set_cursor(const std::size_t offset, const bool select, const bool autoscroll = true)
	{
		text_box_base::set_cursor(offset, select);
		set_line_num_from_offset();

		if (autoscroll) {
			// Whenever cursor moves, this tells scroll_text to update the scrollbars
			update_layout();
		}
	}

	/** Inherited from text_box_base. */
	void goto_end_of_line(const bool select = false) override
	{
		set_line_num_from_offset();
		set_cursor(get_line_end_offset(line_num_), select);
	}

	/** Inherited from text_box_base. */
	void goto_start_of_line(const bool select = false) override
	{
		set_line_num_from_offset();
		set_cursor(get_line_start_offset(line_num_), select);
	}

	/** Inherited from text_box_base. */
	void goto_end_of_data(const bool select = false) override
	{
		text_box_base::goto_end_of_data(select);
		update_layout();
	}

	/** Inherited from text_box_base. */
	void goto_start_of_data(const bool select = false) override
	{
		text_box_base::goto_start_of_data(select);
		update_layout();
	}

	/** Inherited from text_box_base. */
	void paste_selection(const bool mouse) override
	{
		text_box_base::paste_selection(mouse);
		update_layout();
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
	std::size_t max_input_length_;

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

	/** Line number of text */
	unsigned line_num_;

	/** utility function to calculate and set line_num_ from offset */
	void set_line_num_from_offset();
	unsigned get_line_num_from_offset(unsigned offset);

	/** Utility function to calculate the offset of the end of the line
	 */
	unsigned get_line_end_offset(unsigned line_no);
	/** Utility function to calculate the offset of the end of the line
	 */
	unsigned get_line_start_offset(unsigned line_no);

	/** Update layout. To be called when text size changes */
	void update_layout() {
		set_label(get_value());
		get_window()->invalidate_layout();
	}


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

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Handled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_left_arrow(SDL_Keymod modifier, bool& handled) override
	{
		text_box_base::handle_key_left_arrow(modifier, handled);
		update_layout();
	}

	/**
	 * Inherited from text_box_base.
	 *
	 * Unmodified                 Handled.
	 * Control                    Ignored.
	 * Shift                      Ignored.
	 * Alt                        Ignored.
	 */
	void handle_key_right_arrow(SDL_Keymod modifier, bool& handled) override
	{
		text_box_base::handle_key_right_arrow(modifier, handled);
		update_layout();
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
	void handle_key_enter(SDL_Keymod modifier, bool& handled) override;

	/** Inherited from text_box_base. */
	void handle_key_clear_line(SDL_Keymod modifier, bool& handled) override;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
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

struct multiline_text_definition : public styled_widget_definition
{
	explicit multiline_text_definition(const config& cfg);

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

struct builder_multiline_text : public builder_styled_widget
{
public:
	explicit builder_multiline_text(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	std::string history;

	std::size_t max_input_length;

	t_string hint_text;
	std::string hint_image;

	bool editable;
	bool wrap;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
