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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/scroll_text.hpp"

#include "gui/widgets/multiline_text.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <iostream>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(scroll_text)

scroll_text::scroll_text(const implementation::builder_scroll_text& builder)
	: scrollbar_container(builder, type())
	, state_(ENABLED)
	, wrap_on_(false)
	, text_alignment_(builder.text_alignment)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&scroll_text::signal_handler_left_button_down, this, std::placeholders::_2),
		event::dispatcher::back_pre_child);
}

multiline_text* scroll_text::get_internal_text_box()
{
	if(content_grid()) {
		return dynamic_cast<multiline_text*>(content_grid()->find("_text", false));
	}

	return nullptr;
}

void scroll_text::set_label(const t_string& label)
{
	if(multiline_text* widget = get_internal_text_box()) {
		widget->set_value(label);
		widget->set_label(label);

		bool resize_needed = !content_resize_request();
		if(resize_needed && get_size() != point()) {
			place(get_origin(), get_size());
		}
	}
}

std::string scroll_text::get_value()
{
	if(multiline_text* widget = get_internal_text_box()) {
		return widget->get_value();
	} else {
		return "";
	}
}

void scroll_text::set_text_alignment(const PangoAlignment text_alignment)
{
	// Inherit.
	styled_widget::set_text_alignment(text_alignment);

	text_alignment_ = text_alignment;

	if(multiline_text* widget = get_internal_text_box()) {
		widget->set_text_alignment(text_alignment_);
	}
}

void scroll_text::set_use_markup(bool use_markup)
{
	// Inherit.
	styled_widget::set_use_markup(use_markup);

	if(multiline_text* widget = get_internal_text_box()) {
		widget->set_use_markup(use_markup);
	}
}

void scroll_text::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool scroll_text::get_active() const
{
	return state_ != DISABLED;
}

unsigned scroll_text::get_state() const
{
	return state_;
}

void scroll_text::finalize_subclass()
{
	multiline_text* text = get_internal_text_box();
	assert(text);

	text->set_editable(is_editable());
	text->set_label(get_label());
	text->set_text_alignment(text_alignment_);
	text->set_use_markup(get_use_markup());
	connect_signal_notify_modified(*text, std::bind(&scroll_text::refresh, this));
}

void scroll_text::refresh() {
	multiline_text* text = get_internal_text_box();
	assert(text);

	if (text->scroll_horiz_ == scrollbar_base::BEGIN) {
		scroll_horizontal_scrollbar(text->scroll_horiz_);
		text->scroll_horiz_ = scrollbar_base::HALF_JUMP_FORWARD;
	} else if (text->scroll_horiz_ == scrollbar_base::END) {
		// scroll to the end of line instead of end of scrolling range
		// cursor is already set at the end of the line by multiline_text
		const point& cursor_pos = text->get_cursor_pos();
		scroll_horizontal_scrollbar_by(cursor_pos.x - 10);

		text->scroll_horiz_ = scrollbar_base::HALF_JUMP_BACKWARDS;
	} else {
		const point& cursor_pos = text->get_cursor_pos();
		const SDL_Rect& visible_area = content_visible_area();
		int line_end_pos = text->get_line_end_pos();
		int scrollbar_width = vertical_scrollbar()->get_size().x;

		if (text->scroll_horiz_ == scrollbar_base::HALF_JUMP_FORWARD) {
			if (cursor_pos.x > visible_area.w - scrollbar_width) {
				scroll_horizontal_scrollbar_by(text->get_char_width());
			}
		} else {
			if (cursor_pos.x < line_end_pos - visible_area.w + scrollbar_width) {
				scroll_horizontal_scrollbar_by(-static_cast<int>(text->get_char_width()));
			}
		}
	}

	if (text->scroll_vert_ == scrollbar_base::BEGIN
			|| text->scroll_vert_ == scrollbar_base::END) {
		scroll_vertical_scrollbar(text->scroll_vert_);
		text->scroll_vert_ = scrollbar_base::HALF_JUMP_FORWARD;
	} else {
		if (text->get_line_no() > 1) {
			//scroll_vertical_scrollbar(text->scroll_vert_);
			if (text->scroll_vert_ == scrollbar_base::HALF_JUMP_FORWARD) {
				scroll_vertical_scrollbar_by(text->get_line_height());
			} else {
				scroll_vertical_scrollbar_by(-static_cast<int>(text->get_line_height()));
			}
		} else if (text->get_line_no() > text->get_lines_count() - 1) {
			scroll_vertical_scrollbar(scrollbar_base::END);
		} else {
			scroll_vertical_scrollbar(scrollbar_base::BEGIN);
		}
	}
	get_window()->queue_redraw();
}

void scroll_text::set_can_wrap(bool can_wrap)
{
	wrap_on_ = can_wrap;
}

bool scroll_text::can_wrap() const
{
	return true;
}

void scroll_text::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);
}

// }---------- DEFINITION ---------{

scroll_text_definition::scroll_text_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing scroll label " << id;

	load_resolutions<resolution>(cfg);
}

scroll_text_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is scroll_text.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for scroll label control")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for scroll label control")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", _("No grid defined for scroll label control"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_scroll_text::builder_scroll_text(const config& cfg)
	: implementation::builder_styled_widget(cfg)
	, vertical_scrollbar_mode(get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
	, editable(cfg["editable"].to_bool(true))
{
	/** Horizontal scrollbar default to auto. AUTO_VISIBLE_FIRST_RUN doesn't work. */
	if (horizontal_scrollbar_mode == scrollbar_container::AUTO_VISIBLE_FIRST_RUN) {
		horizontal_scrollbar_mode = scrollbar_container::AUTO_VISIBLE;
	}
}

std::unique_ptr<widget> builder_scroll_text::build() const
{
	auto widget = std::make_unique<scroll_text>(*this);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);
	widget->set_editable(editable);
	widget->set_text_alignment(text_alignment);

	const auto conf = widget->cast_config_to<scroll_text_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);
	widget->finalize_setup();

	DBG_GUI_G << "Window builder: placed scroll label '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
