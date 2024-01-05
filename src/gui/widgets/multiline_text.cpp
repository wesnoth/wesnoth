/*
	Copyright (C) 2008 - 2023
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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/multiline_text.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/game.hpp"
#include "serialization/unicode.hpp"
#include "font/text.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"

#include <functional>
#include <iostream>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multiline_text)

multiline_text::multiline_text(const implementation::builder_styled_widget& builder)
	: text_box_base(builder, type())
	, history_()
	, max_input_length_(0)
	, text_x_offset_(0)
	, text_y_offset_(0)
	, text_height_(0)
	, dragging_(false)
	, line_no_(0)
{
	set_wants_mouse_left_double_click();

	connect_signal<event::MOUSE_MOTION>(std::bind(
			&multiline_text::signal_handler_mouse_motion, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&multiline_text::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&multiline_text::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(std::bind(
			&multiline_text::signal_handler_left_button_double_click, this, std::placeholders::_2, std::placeholders::_3));

	const auto conf = cast_config_to<multiline_text_definition>();
	assert(conf);

	set_font_size(get_text_font_size());
	set_font_style(conf->text_font_style);

	update_offsets();
}

void multiline_text::place(const point& origin, const point& size)
{
	// Inherited.
	styled_widget::place(origin, size);

	set_maximum_width(get_text_maximum_width());
	set_maximum_height(get_text_maximum_height(), false);

	set_maximum_length(max_input_length_);

	update_offsets();
}

void multiline_text::update_canvas()
{
	/***** Gather the info *****/

	// Set the cursor info.
	const unsigned start = get_selection_start();
	const int length = get_selection_length();

	// Set the cursor info.
	const unsigned edit_start = get_composition_start();
	const int edit_length = get_composition_length();

	set_maximum_length(max_input_length_);

	PangoEllipsizeMode ellipse_mode = PANGO_ELLIPSIZE_NONE;
	if(!can_wrap()) {
		if((start + length) > (get_length() / 2)) {
			ellipse_mode = PANGO_ELLIPSIZE_START;
		} else {
			ellipse_mode = PANGO_ELLIPSIZE_END;
		}
	}
	set_ellipse_mode(ellipse_mode);

	// Set the selection info
	unsigned start_offset = 0;
	unsigned end_offset = 0;
	if(length == 0) {
		// No nothing.
	} else if(length > 0) {
		start_offset = get_cursor_position(start).x;
		end_offset = get_cursor_position(start + length).x;
	} else {
		start_offset = get_cursor_position(start + length).x;
		end_offset = get_cursor_position(start).x;
	}

	// Set the composition info
	unsigned comp_start_offset = 0;
	unsigned comp_end_offset = 0;
	if(edit_length == 0) {
		// No nothing.
	} else if(edit_length > 0) {
		comp_start_offset = get_cursor_position(edit_start).x;
		comp_end_offset = get_cursor_position(edit_start + edit_length).x;
	} else {
		comp_start_offset = get_cursor_position(edit_start + edit_length).x;
		comp_end_offset = get_cursor_position(edit_start).x;
	}

	set_line_no_from_offset();

	/***** Set in all canvases *****/

	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	for(auto & tmp : get_canvases())
	{

		tmp.set_variable("text", wfl::variant(get_value()));
		tmp.set_variable("text_x_offset", wfl::variant(text_x_offset_));
		tmp.set_variable("text_y_offset", wfl::variant(text_y_offset_));
		tmp.set_variable("text_maximum_width", wfl::variant(max_width));
		tmp.set_variable("text_maximum_height", wfl::variant(max_height));

		tmp.set_variable("cursor_offset",
						 wfl::variant(get_cursor_position(start + length).x));
		tmp.set_variable("cursor_offset_y",
						 wfl::variant(get_cursor_position(start + length).y));

		tmp.set_variable("selection_offset", wfl::variant(start_offset));
		tmp.set_variable("selection_width", wfl::variant(end_offset - start_offset));
		tmp.set_variable("selection_height", wfl::variant(get_sel_height()));
		tmp.set_variable("text_wrap_mode", wfl::variant(ellipse_mode));

		tmp.set_variable("composition_offset", wfl::variant(comp_start_offset));
		tmp.set_variable("composition_width", wfl::variant(comp_end_offset - comp_start_offset));

		tmp.set_variable("hint_text", wfl::variant(hint_text_));
		tmp.set_variable("hint_image", wfl::variant(hint_image_));
	}
}

void multiline_text::delete_char(const bool before_cursor)
{
	if(before_cursor) {
		set_cursor(get_selection_start() - 1, false);
	}

	set_selection_length(1);

	delete_selection();
}

void multiline_text::delete_selection()
{
	if(get_selection_length() == 0) {
		return;
	}

	// If we have a negative range change it to a positive range.
	// This makes the rest of the algorithms easier.
	int len = get_selection_length();
	unsigned start = get_selection_start();
	if(len < 0) {
		len = -len;
		start -= len;
	}

	std::string tmp = get_value();
	set_value(utf8::erase(tmp, start, len));
	set_cursor(start, false);
}

void multiline_text::handle_mouse_selection(point mouse, const bool start_selection)
{
	mouse.x -= get_x();
	mouse.y -= get_y();
	// FIXME we don't test for overflow in width
	if(mouse.x < static_cast<int>(text_x_offset_)
	   || mouse.y < static_cast<int>(text_y_offset_)
//	   || mouse.y >= static_cast<int>(text_y_offset_ + text_height_)) {
	   || mouse.y >= static_cast<int>(text_y_offset_ + get_lines_count() * font::get_line_spacing_factor() * text_height_)) {
		return;
	}

	point cursor_pos = get_column_line(point(mouse.x - text_x_offset_, mouse.y - text_y_offset_));
	int offset = cursor_pos.x;
	int line = cursor_pos.y;

	if(offset < 0) {
		return;
	}

	line_no_ = line;

	const auto& lines = get_lines();
	for (unsigned int i = 0; i < line_no_; ++i) {
		offset += lines.at(i).size() + 1;
	}

	set_cursor(offset, !start_selection);

	update_canvas();
	queue_redraw();
	dragging_ |= start_selection;
}

unsigned multiline_text::get_line_no_from_offset(unsigned offset) {
	unsigned i = 0, line_start = 0, line_end = 0, line_no = 0;
	if ( offset <= get_length() ) {
		for(const auto& line : get_lines()) {
			line_start = line_end;
			line_end += line.size() + 1;
			if ((offset > line_start) && (offset <= line_end)) {
				line_no = i;
				break;
			} else {
				i++;
			}
		}
	}
	return line_no;
}

void multiline_text::set_line_no_from_offset()
{
	line_no_ = get_line_no_from_offset(get_selection_start());
}

unsigned multiline_text::get_sel_height()
{
	return get_line_no_from_offset(get_selection_start() + get_selection_length()) * text_height_ * font::get_line_spacing_factor();
}

void multiline_text::update_offsets()
{
	const auto conf = cast_config_to<multiline_text_definition>();
	assert(conf);

	text_height_ = font::get_max_height(get_text_font_size());

	wfl::map_formula_callable variables;
	variables.add("height", wfl::variant(get_height()));
	variables.add("width", wfl::variant(get_width()));
	variables.add("text_font_height", wfl::variant(text_height_));

	text_x_offset_ = conf->text_x_offset(variables);
	text_y_offset_ = conf->text_y_offset(variables);

	// Since this variable doesn't change set it here instead of in
	// update_canvas().
	for(auto & tmp : get_canvases())
	{
		tmp.set_variable("text_font_height", wfl::variant(text_height_));
	}

	// Force an update of the canvas since now text_font_height is known.
	update_canvas();
}

bool multiline_text::history_up()
{
	if(!history_.get_enabled()) {
		return false;
	}

	const std::string str = history_.up(get_value());
	if(!str.empty()) {
		set_value(str);
	}
	return true;
}

bool multiline_text::history_down()
{
	if(!history_.get_enabled()) {
		return false;
	}

	const std::string str = history_.down(get_value());
	if(!str.empty()) {
		set_value(str);
	}
	return true;
}

void multiline_text::handle_key_tab(SDL_Keymod modifier, bool& handled)
{
	if(modifier & KMOD_CTRL) {
		if(!(modifier & KMOD_SHIFT)) {
			handled = history_up();
		} else {
			handled = history_down();
		}
	}
}

void multiline_text::handle_key_clear_line(SDL_Keymod /*modifier*/, bool& handled)
{
	handled = true;

	set_value("");
}

void multiline_text::handle_key_down_arrow(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;

	set_line_no_from_offset();
	size_t offset = get_selection_start();
	size_t offset_change = get_lines().at(line_no_).size() + 1;

	if (line_no_ < get_lines_count()-1) {
		line_no_ ++;
		offset += offset_change;
		offset += get_selection_length();
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}

//	std::cout << "row :" << line_no_ << " ";
//	std::cout << "offset :" << count << std::endl;

	update_canvas();
	queue_redraw();
}

void multiline_text::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;

	set_line_no_from_offset();
	size_t offset = get_selection_start();
	size_t offset_change = get_lines().at(line_no_).size() + 1;

	if (line_no_ > 0) {
		line_no_ --;
		offset -= offset_change;
		offset += get_selection_length();
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}

//	std::cout << "row :" << line_no_ <<  " ";
//	std::cout << "offset :" << count << std::endl;

	update_canvas();
	queue_redraw();
}

void multiline_text::signal_handler_mouse_motion(const event::ui_event event,
											bool& handled,
											const point& coordinate)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".";

	if(dragging_) {
		handle_mouse_selection(coordinate, false);
	}

	handled = true;
}

void multiline_text::signal_handler_left_button_down(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	/*
	 * Copied from the base class see how we can do inheritance with the new
	 * system...
	 */
	get_window()->keyboard_capture(this);
	get_window()->mouse_capture();

	handle_mouse_selection(get_mouse_position(), true);

	handled = true;
}

void multiline_text::signal_handler_left_button_up(const event::ui_event event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	dragging_ = false;
	handled = true;
}

void
multiline_text::signal_handler_left_button_double_click(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	select_all();
	handled = true;
}

// }---------- DEFINITION ---------{

multiline_text_definition::multiline_text_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multiline_text " << id;

	load_resolutions<resolution>(cfg);
}

multiline_text_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, text_x_offset(cfg["text_x_offset"])
	, text_y_offset(cfg["text_y_offset"])
{
	// Note the order should be the same as the enum state_t in multiline_text.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for editable text box")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for editable text box")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", _("Missing required state for editable text box")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_hovered", _("Missing required state for editable text box")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_multiline_text::builder_multiline_text(const config& cfg)
	: builder_styled_widget(cfg)
	, history(cfg["history"])
	, max_input_length(cfg["max_input_length"])
	, hint_text(cfg["hint_text"].t_str())
	, hint_image(cfg["hint_image"])
{
}

std::unique_ptr<widget> builder_multiline_text::build() const
{
	auto widget = std::make_unique<multiline_text>(*this);

	// A textbox doesn't have a label but a text
	widget->set_value(label_string);

	if(!history.empty()) {
		widget->set_history(history);
	}

	widget->set_max_input_length(max_input_length);
	widget->set_hint_data(hint_text, hint_image);

	DBG_GUI_G << "Window builder: placed text box '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
