/*
	Copyright (C) 2008 - 2025
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

#include "gui/widgets/text_box.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/preferences.hpp"
#include "serialization/unicode.hpp"
#include <functional>
#include "wml_exception.hpp"
#include "gettext.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(text_box)

text_history text_history::get_history(const std::string& id,
										 const bool enabled)
{
	std::vector<std::string>* vec = prefs::get().get_history(id);
	return text_history(vec, enabled);
}

void text_history::push(const std::string& text)
{
	if(!enabled_) {
		return;
	} else {
		if(!text.empty() && (history_->empty() || text != history_->back())) {
			history_->push_back(text);
		}

		pos_ = history_->size();
	}
}

std::string text_history::up(const std::string& text)
{

	if(!enabled_) {
		return "";
	} else if(pos_ == history_->size()) {
		unsigned curr = pos_;
		push(text);
		pos_ = curr;
	}

	if(pos_ != 0) {
		--pos_;
	}

	return get_value();
}

std::string text_history::down(const std::string& text)
{
	if(!enabled_) {
		return "";
	} else if(pos_ == history_->size()) {
		push(text);
	} else {
		pos_++;
	}

	return get_value();
}

std::string text_history::get_value() const
{
	if(!enabled_ || pos_ == history_->size()) {
		return "";
	} else {
		return history_->at(pos_);
	}
}

text_box::text_box(const implementation::builder_styled_widget& builder)
	: text_box_base(builder, type())
	, history_()
	, max_input_length_(0)
	, text_x_offset_(0)
	, text_y_offset_(0)
	, text_height_(0)
	, dragging_(false)
{
	set_wants_mouse_left_double_click();

	connect_signal<event::MOUSE_MOTION>(std::bind(
			&text_box::signal_handler_mouse_motion, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&text_box::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&text_box::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(std::bind(
			&text_box::signal_handler_left_button_double_click, this, std::placeholders::_2, std::placeholders::_3));

	const auto conf = cast_config_to<text_box_definition>();
	assert(conf);

	set_font_size(get_text_font_size());
	set_font_style(conf->text_font_style);

	update_offsets();
}

void text_box::place(const point& origin, const point& size)
{
	// Inherited.
	styled_widget::place(origin, size);

	set_maximum_width(get_text_maximum_width());
	set_maximum_height(get_text_maximum_height(), false);

	set_maximum_length(max_input_length_);

	update_offsets();
}

void text_box::update_canvas()
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

		tmp.set_variable("editable", wfl::variant(is_editable()));

		tmp.set_variable("cursor_offset",
						 wfl::variant(get_cursor_position(start + length).x));

		tmp.set_variable("selection_offset", wfl::variant(start_offset));
		tmp.set_variable("selection_width", wfl::variant(end_offset - start_offset));
		tmp.set_variable("text_wrap_mode", wfl::variant(ellipse_mode));

		tmp.set_variable("composition_offset", wfl::variant(comp_start_offset));
		tmp.set_variable("composition_width", wfl::variant(comp_end_offset - comp_start_offset));

		tmp.set_variable("hint_text", wfl::variant(hint_text_));
		tmp.set_variable("hint_image", wfl::variant(hint_image_));
	}
}

void text_box::delete_char(const bool before_cursor)
{
	if(before_cursor) {
		set_cursor(get_selection_start() - 1, false);
	}

	set_selection_length(1);

	delete_selection();
}

void text_box::delete_selection()
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

void text_box::handle_mouse_selection(point mouse, const bool start_selection)
{
	mouse.x -= get_x();
	mouse.y -= get_y();
	// FIXME we don't test for overflow in width
	if(mouse.x < static_cast<int>(text_x_offset_)
	   || mouse.y < static_cast<int>(text_y_offset_)
	   || mouse.y >= static_cast<int>(text_y_offset_ + text_height_)) {
		return;
	}

	int offset = get_column_line(point(mouse.x - text_x_offset_, mouse.y - text_y_offset_)).x;

	if(offset < 0) {
		return;
	}


	set_cursor(offset, !start_selection);
	update_canvas();
	queue_redraw();
	dragging_ |= start_selection;
}

void text_box::update_offsets()
{
	const auto conf = cast_config_to<text_box_definition>();
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

bool text_box::history_up()
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

bool text_box::history_down()
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

void text_box::handle_key_tab(SDL_Keymod modifier, bool& handled)
{
	if(modifier & KMOD_CTRL) {
		if(!(modifier & KMOD_SHIFT)) {
			handled = history_up();
		} else {
			handled = history_down();
		}
	}
}

void text_box::handle_key_clear_line(SDL_Keymod /*modifier*/, bool& handled)
{
	handled = true;

	set_value("");
}

void text_box::signal_handler_mouse_motion(const event::ui_event event,
											bool& handled,
											const point& coordinate)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".";

	if(dragging_) {
		handle_mouse_selection(coordinate, false);
	}

	handled = true;
}

void text_box::signal_handler_left_button_down(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->capture_and_show_keyboard(this);
	get_window()->mouse_capture();

	handle_mouse_selection(get_mouse_position(), true);

	handled = true;
}

void text_box::signal_handler_left_button_up(const event::ui_event event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	dragging_ = false;
	handled = true;
}

void
text_box::signal_handler_left_button_double_click(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	select_all();
	handled = true;
}

// }---------- DEFINITION ---------{

text_box_definition::text_box_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing text_box " << id;

	load_resolutions<resolution>(cfg);
}

text_box_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, text_x_offset(cfg["text_x_offset"])
	, text_y_offset(cfg["text_y_offset"])
{
	// Note the order should be the same as the enum state_t in text_box.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("text_box_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("text_box_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("text_box_definition][resolution", "state_focused")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_hovered", missing_mandatory_wml_tag("text_box_definition][resolution", "state_hovered")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_text_box::builder_text_box(const config& cfg)
	: builder_styled_widget(cfg)
	, history(cfg["history"])
	, max_input_length(cfg["max_input_length"].to_size_t())
	, hint_text(cfg["hint_text"].t_str())
	, hint_image(cfg["hint_image"])
	, editable(cfg["editable"].to_bool(true))
{
}

std::unique_ptr<widget> builder_text_box::build() const
{
	auto widget = std::make_unique<text_box>(*this);

	// A textbox doesn't have a label but a text
	widget->set_value(label_string);

	if(!history.empty()) {
		widget->set_history(history);
	}

	widget->set_max_input_length(max_input_length);
	widget->set_hint_data(hint_text, hint_image);
	widget->set_editable(editable);

	DBG_GUI_G << "Window builder: placed text box '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
