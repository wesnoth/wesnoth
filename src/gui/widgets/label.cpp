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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/label.hpp"

#include "gui/core/log.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/dialogs/message.hpp"

#include "cursor.hpp"
#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <string>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(label)

label::label(const implementation::builder_label& builder)
	: styled_widget(builder, type())
	, state_(ENABLED)
	, can_wrap_(builder.wrap)
	, characters_per_line_(builder.characters_per_line)
	, link_aware_(builder.link_aware)
	, link_color_(color_t::from_hex_string("ffff00"))
	, can_shrink_(builder.can_shrink)
	, text_alpha_(ALPHA_OPAQUE)
{
	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&label::signal_handler_left_button_click, this, std::placeholders::_3));
	connect_signal<event::RIGHT_BUTTON_CLICK>(
		std::bind(&label::signal_handler_right_button_click, this, std::placeholders::_3));
	connect_signal<event::MOUSE_MOTION>(
		std::bind(&label::signal_handler_mouse_motion, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&label::signal_handler_mouse_leave, this, std::placeholders::_3));
}

void label::update_canvas()
{
	// Inherit.
	styled_widget::update_canvas();

	for(auto& tmp : get_canvases()) {
		tmp.set_variable("text_alpha", wfl::variant(text_alpha_));
	}
}

void label::set_text_alpha(unsigned short alpha)
{
	if(alpha != text_alpha_) {
		text_alpha_ = alpha;
		update_canvas();
		queue_redraw();
	}
}

void label::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

void label::set_link_aware(bool link_aware)
{
	if(link_aware != link_aware_) {
		link_aware_ = link_aware;
		update_canvas();
		queue_redraw();
	}
}

void label::set_link_color(const color_t& color)
{
	if(color != link_color_) {
		link_color_ = color;
		update_canvas();
		queue_redraw();
	}
}

void label::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}

void label::signal_handler_left_button_click(bool& handled)
{
	DBG_GUI_E << "label click";

	if (!get_link_aware()) {
		return; // without marking event as "handled".
	}

	if (!desktop::open_object_is_supported()) {
		show_message("", _("Opening links is not supported, contact your packager"), dialogs::message::auto_close);
		handled = true;
		return;
	}

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Clicked Link:\"" << link << "\"";

	const int res = show_message(_("Open link?"), link, dialogs::message::yes_no_buttons);
	if(res == gui2::retval::OK) {
		desktop::open_object(link);
	}

	handled = true;
}

void label::signal_handler_right_button_click(bool& handled)
{
	DBG_GUI_E << "label right click";

	if (!get_link_aware()) {
		return ; // without marking event as "handled".
	}

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Right Clicked Link:\"" << link << "\"";

	desktop::clipboard::copy_to_clipboard(link);

	(void) show_message("", _("Copied link!"), dialogs::message::auto_close);

	handled = true;
}

void label::signal_handler_mouse_motion(bool& handled, const point& coordinate)
{
	DBG_GUI_E << "label mouse motion";

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	point mouse = coordinate;

	mouse.x -= get_x();
	mouse.y -= get_y();

	update_mouse_cursor(!get_label_link(mouse).empty());

	handled = true;
}

void label::signal_handler_mouse_leave(bool& handled)
{
	DBG_GUI_E << "label mouse leave";

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	// We left the widget, so just unconditionally reset the cursor
	update_mouse_cursor(false);

	handled = true;
}

void label::update_mouse_cursor(bool enable)
{
	// Someone else may set the mouse cursor for us to something unusual (e.g.
	// the WAIT cursor) so we ought to mess with that only if it's set to
	// NORMAL or HYPERLINK.

	if(enable && cursor::get() == cursor::NORMAL) {
		cursor::set(cursor::HYPERLINK);
	} else if(!enable && cursor::get() == cursor::HYPERLINK) {
		cursor::set(cursor::NORMAL);
	}
}

// }---------- DEFINITION ---------{

label_definition::label_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing label " << id;

	load_resolutions<resolution>(cfg);
}

label_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, link_color(cfg["link_color"].empty() ? color_t::from_hex_string("ffff00") : color_t::from_rgba_string(cfg["link_color"].str()))
{
	// Note the order should be the same as the enum state_t is label.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("label_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("label_definition][resolution", "state_disabled")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_label::builder_label(const config& cfg)
	: builder_styled_widget(cfg)
	, wrap(cfg["wrap"].to_bool())
	, characters_per_line(cfg["characters_per_line"].to_unsigned())
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
	, can_shrink(cfg["can_shrink"].to_bool(false))
	, link_aware(cfg["link_aware"].to_bool(false))
{
}

std::unique_ptr<widget> builder_label::build() const
{
	auto lbl = std::make_unique<label>(*this);

	const auto conf = lbl->cast_config_to<label_definition>();
	assert(conf);

	lbl->set_text_alignment(text_alignment);
	lbl->set_link_color(conf->link_color);

	DBG_GUI_G << "Window builder: placed label '" << id << "' with definition '"
			  << definition << "'.";

	return lbl;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
