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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/multiline_text.hpp"

#include "cursor.hpp"
#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/unicode.hpp"
#include "font/text.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multiline_text)

multiline_text::multiline_text(const implementation::builder_multiline_text& builder)
	: text_box_base(builder, type())
	, history_()
	, max_input_length_(builder.max_input_length)
	, text_x_offset_(0)
	, text_y_offset_(0)
	, text_height_(0)
	, dragging_(false)
	, link_aware_(builder.link_aware)
	, hint_text_(builder.hint_text)
	, hint_image_(builder.hint_image)
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

void multiline_text::set_link_aware(bool link_aware)
{
	if(link_aware != link_aware_) {
		link_aware_ = link_aware;
		update_canvas();
		queue_redraw();
	}
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
	const int length = static_cast<int>(get_selection_length());

	// Set the composition info.
	const unsigned edit_start = get_composition_start();
	const int edit_length = get_composition_length();

	set_maximum_length(max_input_length_);

	unsigned comp_start_offset = 0;
	unsigned comp_end_offset = 0;

	if(edit_length == 0) {
		// Do nothing.
	} else if(edit_length > 0) {
		comp_start_offset = get_cursor_position(edit_start).x;
		comp_end_offset = get_cursor_position(edit_start + edit_length).x;
	} else {
		comp_start_offset = get_cursor_position(edit_start + edit_length).x;
		comp_end_offset = get_cursor_position(edit_start).x;
	}

	// Set the selection info
	unsigned start_offset = 0;
	unsigned end_offset = 0;
	if(length == 0) {
		start_offset = start;
		end_offset = start_offset;
	} else if(length > 0) {
		start_offset = start;
		end_offset = start + length;
	} else {
		start_offset = start + length;
		end_offset = start;
	}

	/***** Set in all canvases *****/

	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();
	unsigned byte_pos = start + length;
	if (get_use_markup() && (start + length > utf8::size(plain_text()) + 1)) {
		byte_pos = utf8::size(plain_text());
	}
	const point cpos = get_cursor_pos_from_index(byte_pos);

	for(auto & tmp : get_canvases())
	{

		tmp.set_variable("text", wfl::variant(get_value()));
		tmp.set_variable("text_markup", wfl::variant(get_use_markup()));
		tmp.set_variable("text_x_offset", wfl::variant(text_x_offset_));
		tmp.set_variable("text_y_offset", wfl::variant(text_y_offset_));
		tmp.set_variable("text_maximum_width", wfl::variant(max_width));
		tmp.set_variable("text_maximum_height", wfl::variant(max_height));
		tmp.set_variable("text_link_aware", wfl::variant(get_link_aware()));
		tmp.set_variable("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));

		tmp.set_variable("editable", wfl::variant(is_editable()));

		tmp.set_variable("highlight_start", wfl::variant(start_offset));
		tmp.set_variable("highlight_end", wfl::variant(end_offset));

		tmp.set_variable("cursor_offset_x", wfl::variant(cpos.x));
		tmp.set_variable("cursor_offset_y", wfl::variant(cpos.y));

		tmp.set_variable("composition_offset", wfl::variant(comp_start_offset));
		tmp.set_variable("composition_width", wfl::variant(comp_end_offset - comp_start_offset));

		tmp.set_variable("hint_text", wfl::variant(hint_text_));
		tmp.set_variable("hint_image", wfl::variant(hint_image_));
	}
}

void multiline_text::delete_char(const bool before_cursor)
{
	if(!is_editable()) {
		return;
	}

	if(before_cursor) {
		set_cursor(get_selection_start() - 1, false);
	}

	set_selection_length(1);

	delete_selection();
}

void multiline_text::delete_selection()
{
	if(get_selection_length() == 0 || (!is_editable()) ) {
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

	update_layout();
}

void multiline_text::handle_mouse_selection(point mouse, const bool start_selection)
{
	mouse -= get_origin();
	point text_offset(text_x_offset_, text_y_offset_);
	// FIXME we don't test for overflow in width
	if(mouse < text_offset
		|| mouse.y >= static_cast<int>(text_y_offset_ + get_lines_count() * font::get_line_spacing_factor() * text_height_))
	{
		return;
	}

	const auto& [offset, line] = get_column_line(mouse - text_offset);

	if(offset < 0) {
		return;
	}

	set_cursor(offset + get_line_start_offset(line), !start_selection);

	update_canvas();
	queue_redraw();
	dragging_ |= start_selection;
}

unsigned multiline_text::get_line_end_offset(unsigned line_no) {
	const auto line = get_line(line_no);
	return (line->start_index + line->length);
}

unsigned multiline_text::get_line_start_offset(unsigned line_no) {
	return get_line(line_no)->start_index;
}

void multiline_text::update_offsets()
{
	const auto conf = cast_config_to<multiline_text_definition>();
	assert(conf);

	text_height_ = font::get_max_height(get_text_font_size(), get_font_family());

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
	if(!is_editable())
	{
		return;
	}

	if(modifier & KMOD_CTRL) {
		if(!(modifier & KMOD_SHIFT)) {
			handled = history_up();
		} else {
			handled = history_down();
		}
	} else {
		handled = true;
		insert_char("\t");
	}
}

void multiline_text::handle_key_enter(SDL_Keymod modifier, bool& handled)
{
	if (is_editable() && !(modifier & (KMOD_CTRL | KMOD_ALT | KMOD_GUI))) {
		insert_char("\n");
		handled = true;
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

	unsigned offset = get_selection_start();
	const unsigned line_num = get_line_number(offset);

	if (line_num == get_lines_count()-1) {
		return;
	}

	const unsigned line_start = get_line_start_offset(line_num);
	const unsigned next_line_start = get_line_start_offset(line_num+1);
	const unsigned next_line_end = get_line_end_offset(line_num+1);

	offset = std::min(offset - line_start + next_line_start, next_line_end) + get_selection_length();

	if (offset <= get_length()) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}

	update_canvas();
	queue_redraw();
}

void multiline_text::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;

	unsigned offset = get_selection_start();
	const unsigned line_num = get_line_number(offset);

	if (line_num == 0) {
		return;
	}

	const unsigned line_start = get_line_start_offset(line_num);
	const unsigned prev_line_start = get_line_start_offset(line_num-1);
	const unsigned prev_line_end = get_line_end_offset(line_num-1);

	offset = std::min(offset - line_start + prev_line_start, prev_line_end) + get_selection_length();

	/* offset is unsigned int */
	if (offset <= get_length()) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}

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
	} else {
		if(!get_link_aware()) {
			return; // without marking event as "handled"
		}

		point mouse = coordinate - get_origin();
		if (!get_label_link(mouse).empty()) {
			cursor::set(cursor::HYPERLINK);
		} else {
			cursor::set(cursor::IBEAM);
		}
	}

	handled = true;
}

void multiline_text::signal_handler_left_button_down(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);
	get_window()->mouse_capture();

	point mouse_pos = get_mouse_position();

	if (get_link_aware()) {
		std::string link = get_label_link(mouse_pos - get_origin());
		DBG_GUI_E << "Clicked Link:\"" << link << "\"";

		if (!link.empty()) {
			if (desktop::open_object_is_supported()) {
				if(show_message(_("Open link?"), link, dialogs::message::yes_no_buttons) == gui2::retval::OK) {
					desktop::open_object(link);
				}
			} else {
				desktop::clipboard::copy_to_clipboard(link);
				show_message("", _("Opening links is not supported, contact your packager. Link URL has been copied to the clipboard."), dialogs::message::auto_close);
			}
		} else {
			handle_mouse_selection(mouse_pos, true);
		}
	} else {
		handle_mouse_selection(mouse_pos, true);
	}

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
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("multiline_text_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("multiline_text_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("multiline_text_definition][resolution", "state_focused")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_hovered", missing_mandatory_wml_tag("multiline_text_definition][resolution", "state_hovered")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_multiline_text::builder_multiline_text(const config& cfg)
	: builder_styled_widget(cfg)
	, history(cfg["history"])
	, max_input_length(cfg["max_input_length"].to_size_t())
	, hint_text(cfg["hint_text"].t_str())
	, hint_image(cfg["hint_image"])
	, editable(cfg["editable"].to_bool(true))
	, wrap(cfg["wrap"].to_bool(true))
	, link_aware(cfg["link_aware"].to_bool(false))
{
}

std::unique_ptr<widget> builder_multiline_text::build() const
{
	auto widget = std::make_unique<multiline_text>(*this);

	widget->set_editable(editable);
	// A textbox doesn't have a label but a text
	widget->set_value(label_string);

	if(!history.empty()) {
		widget->set_history(history);
	}

	DBG_GUI_G << "Window builder: placed text box '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
