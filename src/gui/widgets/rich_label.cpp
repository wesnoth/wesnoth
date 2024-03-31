/*
	Copyright (C) 2024
	by Subhraman Sarkar <suvrax@gmail.com>
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

#include "gui/widgets/rich_label.hpp"

#include "gui/core/log.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/dialogs/message.hpp"

#include "cursor.hpp"
#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "help/help_impl.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <string>
#include <iostream>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(rich_label)

rich_label::rich_label(const implementation::builder_rich_label& builder)
	: styled_widget(builder, type())
	, state_(ENABLED)
	, can_wrap_(builder.wrap)
	, characters_per_line_(builder.characters_per_line)
	, link_aware_(builder.link_aware)
	, link_color_(color_t::from_hex_string("ffff00"))
	, can_shrink_(builder.can_shrink)
	, text_alpha_(ALPHA_OPAQUE)
	, unparsed_text_()
	, w_(0)
	, h_(0)
{
	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&rich_label::signal_handler_left_button_click, this, std::placeholders::_3));
	connect_signal<event::RIGHT_BUTTON_CLICK>(
		std::bind(&rich_label::signal_handler_right_button_click, this, std::placeholders::_3));
	connect_signal<event::MOUSE_MOTION>(
		std::bind(&rich_label::signal_handler_mouse_motion, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&rich_label::signal_handler_mouse_leave, this, std::placeholders::_3));
}

void rich_label::set_label(const t_string& text)
{
	unparsed_text_ = text;
	text_dom_.clear();
	help::topic_text marked_up_text(text);
	std::vector<std::string> parsed_text =  marked_up_text.parsed_text();
	for (size_t i = 0; i < parsed_text.size(); i++) {
		bool last_entry = (i == parsed_text.size() - 1);
		std::string line = parsed_text.at(i);
		if (!line.empty() && line.at(0) == '[') {
			config cfg;
			std::istringstream stream(line);
			::read(cfg, line);

			if (cfg.optional_child("ref")) {
				config& txt_r = default_text_config(cfg.mandatory_child("ref")["text"], last_entry);
				txt_r["attr_name"] = "fgcolor";
				txt_r["attr_start"] = "0";
				txt_r["attr_end"] = txt_r["text"].str().size();
				txt_r["attr_color"] = font::YELLOW_COLOR.to_hex_string().substr(1, txt_r["text"].str().size());
			} else if (cfg.optional_child("bold")) {
				config& txt_b = default_text_config(cfg.mandatory_child("bold")["text"], last_entry);
				txt_b["attr_name"] = "bold";
				txt_b["attr_start"] = "0";
				txt_b["attr_end"] = txt_b["text"].str().size();
			} else if (cfg.optional_child("italic")) {
				config& txt_i = default_text_config(cfg.mandatory_child("italic")["text"], last_entry);
				txt_i["attr_name"] = "italic";
				txt_i["attr_start"] = "0";
				txt_i["attr_end"] = txt_i["text"].str().size();
			} else if (cfg.optional_child("header")) {
				config& txt_h = default_text_config(cfg.mandatory_child("header")["text"], last_entry);
				txt_h["font_style"] = "bold";
				txt_h["font_size"] = font::SIZE_TITLE;
			} else if (cfg.optional_child("img")) {
				config& img = text_dom_.add_child("image");
				img["name"] = cfg.mandatory_child("img")["src"];
				img["x"] = "(pos_x)";
				img["y"] = "(pos_y)";
				img["h"] = "(image_height)";
				img["w"] = "(image_width)";

				if (last_entry) {
					img["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
				} else {
					img["actions"] = "([set_var('pos_x', pos_x+image_width), set_var('ih', image_height), set_var('iw', image_width)])";
				}

//					config& rect = text_dom_.add_child("rectangle");
//					rect["x"] = "(pos_x)";
//					rect["y"] = "(pos_y)";
//					rect["h"] = "(ih)";
//					rect["w"] = "(iw)";
//					rect["border_thickness"] = "1";
//					rect["border_color"] = "255,255,255,255";
//					std::cout << img.debug() << std::endl;
			}
			//			} else if(line == "[link]") {
			//			} else if(line == "[jump]") {
			//			} else if(line == "[format]") {

		} else {
			default_text_config(line);
		}
	}
}

config& rich_label::default_text_config(t_string text, bool last_entry) {
	config& txt = text_dom_.add_child("text");
	txt["text"] = text;
	txt["font_size"] = 20;
	txt["color"] = "([186, 172, 125, 255])";
	txt["x"] = "(pos_x)";
	txt["y"] = "(pos_y)";
	txt["w"] = "(text_width)";
	txt["h"] = "(text_height)";
	if (last_entry) {
		txt["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
	} else {
		txt["actions"] = "([set_var('pos_x', pos_x+text_width)])";
	}
	return txt;
}

void rich_label::update_canvas()
{
	for(canvas& tmp : get_canvases()) {
		tmp.set_variable("pos_x", wfl::variant(0));
		tmp.set_variable("pos_y", wfl::variant(0));
		tmp.set_variable("ih", wfl::variant(0));
		tmp.set_variable("iw", wfl::variant(0));
		set_label(get_label());
		tmp.set_cfg(text_dom_, true);
		tmp.set_variable("text_alpha", wfl::variant(text_alpha_));
	}
}

void rich_label::set_text_alpha(unsigned short alpha)
{
	if(alpha != text_alpha_) {
		text_alpha_ = alpha;
		update_canvas();
		queue_redraw();
	}
}

void rich_label::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

void rich_label::set_link_aware(bool link_aware)
{
	if(link_aware != link_aware_) {
		link_aware_ = link_aware;
		update_canvas();
		queue_redraw();
	}
}

void rich_label::set_link_color(const color_t& color)
{
	if(color != link_color_) {
		link_color_ = color;
		update_canvas();
		queue_redraw();
	}
}

void rich_label::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}

void rich_label::signal_handler_left_button_click(bool& handled)
{
	DBG_GUI_E << "rich_label click";

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

void rich_label::signal_handler_right_button_click(bool& handled)
{
	DBG_GUI_E << "rich_label right click";

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

	desktop::clipboard::copy_to_clipboard(link, false);

	(void) show_message("", _("Copied link!"), dialogs::message::auto_close);

	handled = true;
}

void rich_label::signal_handler_mouse_motion(bool& handled, const point& coordinate)
{
	DBG_GUI_E << "rich_label mouse motion";

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	point mouse = coordinate;

	mouse.x -= get_x();
	mouse.y -= get_y();

	update_mouse_cursor(!get_label_link(mouse).empty());

	handled = true;
}

void rich_label::signal_handler_mouse_leave(bool& handled)
{
	DBG_GUI_E << "rich_label mouse leave";

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	// We left the widget, so just unconditionally reset the cursor
	update_mouse_cursor(false);

	handled = true;
}

void rich_label::update_mouse_cursor(bool enable)
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

rich_label_definition::rich_label_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing rich_label " << id;

	load_resolutions<resolution>(cfg);
}

rich_label_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, link_color(cfg["link_color"].empty() ? color_t::from_hex_string("ffff00") : color_t::from_rgba_string(cfg["link_color"].str()))
{
	// Note the order should be the same as the enum state_t is rich_label.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("rich_label_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("rich_label_definition][resolution", "state_disabled")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_rich_label::builder_rich_label(const config& cfg)
	: builder_styled_widget(cfg)
	, wrap(cfg["wrap"].to_bool())
	, characters_per_line(cfg["characters_per_line"])
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
	, can_shrink(cfg["can_shrink"].to_bool(false))
	, link_aware(cfg["link_aware"].to_bool(false))
{
}

std::unique_ptr<widget> builder_rich_label::build() const
{
	auto lbl = std::make_unique<rich_label>(*this);

	const auto conf = lbl->cast_config_to<rich_label_definition>();
	assert(conf);

	lbl->set_text_alignment(text_alignment);
	lbl->set_link_color(conf->link_color);
	lbl->set_label(lbl->get_label());

	DBG_GUI_G << "Window builder: placed rich_label '" << id << "' with definition '"
			  << definition << "'.";

	return lbl;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
