/*
	Copyright (C) 2024
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
#include "serialization/unicode.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <string>

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
	, x_(0)
	, y_(0)
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

point rich_label::get_text_size(config text_cfg, unsigned width) {
	wfl::action_function_symbol_table functions;
	wfl::map_formula_callable variables;
	variables.add("text", wfl::variant(text_cfg["text"].str()));
	variables.add("width", wfl::variant(width > 0 ? width : w_));
	variables.add("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
	variables.add("fake_draw", wfl::variant(true));
	tshape_ = std::make_unique<gui2::text_shape>(text_cfg, functions);
	tshape_->draw(variables);
	return point(variables.query_value("text_width").as_int(), variables.query_value("text_height").as_int());
}

point rich_label::get_image_size(config img_cfg) {
	wfl::action_function_symbol_table functions;
	wfl::map_formula_callable variables;
	variables.add("fake_draw", wfl::variant(true));
	ishape_ = std::make_unique<gui2::image_shape>(img_cfg, functions);
	ishape_->draw(variables);
	return point(variables.query_value("image_width").as_int(), variables.query_value("image_height").as_int());
}

void rich_label::add_text_with_attribute(config& text_cfg, std::string text, bool last_entry, std::string attr_name, std::string extra_data) {
	size_t start = text_cfg["text"].str().size();
	text_cfg["text"] = text_cfg["text"].str() + text;
	text_cfg["attr_name"] = (text_cfg["attr_name"].str().empty() ? "" : (text_cfg["attr_name"].str() + ",")) + attr_name;
	text_cfg["attr_start"] = (text_cfg["attr_start"].str().empty() ? "" : (text_cfg["attr_start"].str() + ",")) +  std::to_string(start);
	text_cfg["attr_end"] = (text_cfg["attr_end"].str().empty() ? "" : (text_cfg["attr_end"].str() + ",")) + std::to_string(text_cfg["text"].str().size());
	if (!extra_data.empty()) {
		text_cfg["attr_color"] = (text_cfg["attr_color"].str().empty() ? "" : (text_cfg["attr_color"].str() + ",")) + extra_data;
	}
	// Clear variables to stop them from growing too large
	if (last_entry) {
		text_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
	}
}

void rich_label::set_label(const t_string& text)
{
	// Initial size
	w_ = 800;
	h_ = 0;
	unparsed_text_ = text;
	text_dom_.clear();
	links_.clear();
	help::topic_text marked_up_text(text);
	std::vector<std::string> parsed_text =  marked_up_text.parsed_text();

	config* text_ptr = &(text_dom_.add_child("text"));
	default_text_config(text_ptr);
//	bool is_image = false;
	point wrap_position;
	point img_size;

	for (size_t i = 0; i < parsed_text.size(); i++) {
		bool last_entry = (i == parsed_text.size() - 1);
		std::string line = parsed_text.at(i);

		if (!line.empty() && line.at(0) == '[') {
			config cfg;
			std::istringstream stream(line);
			::read(cfg, line);

			std::string text_buffer, attributes, starts, stops, colors;

			if (cfg.optional_child("ref")) {
				add_text_with_attribute((*text_ptr), cfg.mandatory_child("ref")["text"], last_entry, "fgcolor", font::YELLOW_COLOR.to_hex_string().substr(1, (*text_ptr)["text"].str().size()));

				// find out the rectangle inside which the link is located
				point t_size = get_text_size(*text_ptr);
				unsigned x2 = t_size.x;
				rect link_rect = {static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(x2-x_), static_cast<int>(h_-y_)};

				links_.push_back(std::pair(link_rect, cfg.mandatory_child("ref")["dst"]));

				update_positions(*text_ptr);

			} else if (cfg.optional_child("bold")) {

				add_text_with_attribute((*text_ptr), cfg.mandatory_child("bold")["text"], last_entry, "bold");

				update_positions(*text_ptr);

				PLAIN_LOG << "(bold) x :" << x_ << ", y :" << y_ << ", h: " << h_;

			} else if (cfg.optional_child("italic")) {

				add_text_with_attribute((*text_ptr), cfg.mandatory_child("italic")["text"], last_entry, "italic");

				update_positions(*text_ptr);

				PLAIN_LOG << "(italic) x :" << x_ << ", y :"  << y_ << ", h: " << h_;

			} else if (cfg.optional_child("header")) {

				size_t start = (*text_ptr)["text"].str().size();
				(*text_ptr)["text"] = (*text_ptr)["text"].str() + cfg.mandatory_child("header")["text"];
				(*text_ptr)["attr_name"] = ((*text_ptr)["attr_name"].str().empty()? "" : ((*text_ptr)["attr_name"].str() + ",")) + "fgcolor,fontsize";
				(*text_ptr)["attr_start"] = ((*text_ptr)["attr_start"].str().empty()? "" : ((*text_ptr)["attr_start"].str() + ",")) + std::to_string(start) + "," + std::to_string(start);
				(*text_ptr)["attr_end"] = ((*text_ptr)["attr_end"].str().empty()? "" : ((*text_ptr)["attr_end"].str() + ",")) + std::to_string((*text_ptr)["text"].str().size()) + "," + std::to_string((*text_ptr)["text"].str().size());
				// TODO add font::GOLD_COLOR and remove hardcoded color value
				(*text_ptr)["attr_color"] = ((*text_ptr)["attr_color"].str().empty()? "" : ((*text_ptr)["attr_color"].str() + ",")) + "baac7d," + std::to_string(font::SIZE_TITLE);

				if (last_entry) {
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
				}

				update_positions(*text_ptr);

			} else if (cfg.optional_child("img")) {
				(*text_ptr)["actions"] = "([set_var('pos_y', pos_y + text_height)])";

				config& img = text_dom_.add_child("image");
				img["name"] = cfg.mandatory_child("img")["src"];
				std::string align = cfg.mandatory_child("img")["align"];
				if (align == "left") {
					img["x"] = "(pos_x)";
				} else if (align == "right") {
					img["x"] = "(width-image_width - pos_x)";
				} else {
					img["x"] = "((width-image_width)/2.0)";
				}
				img["y"] = "(pos_y)";
				img["h"] = "(image_height)";
				img["w"] = "(image_width)";

				if (last_entry) {
					img["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
				} else {
					if (align == "left") {
						img["actions"] = "([set_var('pos_x', pos_x + image_width + 5), set_var('iw', image_width)])"; //Padding 5
					} else if (align == "right") {
						img["actions"] = "([set_var('pos_x', 0), set_var('iw', image_width)])";
					}
				}

				img_size = get_image_size(img);
				y_ = h_;
				if (align == "left") {
					x_ += img_size.x;
				}
				PLAIN_LOG << "(img) x :" << x_ << ", y :" << y_ << ", h: " << h_;

				text_ptr = &(text_dom_.add_child("text"));
				default_text_config(text_ptr);

			}
			//			} else if(line == "[link]") {
			//			} else if(line == "[jump]") {
			//			} else if(line == "[format]") {

		} else if (!line.empty()) {
			(*text_ptr)["text"] = (*text_ptr)["text"].str() + line;
			if (last_entry) {
				(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
			}

			point t_size = get_text_size(*text_ptr, w_ - img_size.y);
			y_ = h_;
			h_ = t_size.y > h_ ? t_size.y : h_;

			PLAIN_LOG << "(text) x :" << x_ << ", y :" << y_ << ", h: " << h_;
			PLAIN_LOG << img_size.y << ", " << t_size.y;

			if ( (img_size.y > 0) && (t_size.y > img_size.y) ) {

				PLAIN_LOG << "Wrap needed";
				wrap_position = get_column_line(point(w_, img_size.y));
				PLAIN_LOG << "wrap x :" << wrap_position.x << ", wrap y :" << wrap_position.y;

				size_t len = 0;
				for (int i = 0; i < wrap_position.y; i++) {
					len += utf8::size(font::get_text_renderer().get_lines()[i]);
				}

				t_string* removed_part = new t_string((*text_ptr)["text"].str().substr(len));
				(*text_ptr)["text"] = (*text_ptr)["text"].str().substr(0, len);

				(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + text_height + 5), set_var('iw', 0)])";

				text_ptr = &(text_dom_.add_child("text"));
				default_text_config(text_ptr);

				(*text_ptr)["text"] = *removed_part;

				if (last_entry) {
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
				}

				img_size = point(0,0);

				point t_size = get_text_size(*text_ptr);
				y_ = h_;
				h_ += t_size.y;
//				h_ = t_size.y > h_ ? t_size.y : h_;

			}
		}
	}
}

void rich_label::default_text_config(config* txt_ptr, t_string text, bool last_entry) {
	(*txt_ptr)["text"] = text;
	(*txt_ptr)["font_size"] = 16;
	(*txt_ptr)["x"] = "(pos_x)";
	(*txt_ptr)["y"] = "(pos_y)";
	(*txt_ptr)["w"] = "(text_width)";
	(*txt_ptr)["h"] = "(text_height)";

	(*txt_ptr)["maximum_width"] = "(width - iw - 5)";
	if (last_entry) {
		(*txt_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
	} else {
		(*txt_ptr)["actions"] = "([set_var('pos_y', pos_y+text_height)])";
	}
}

void rich_label::update_positions(config text_cfg) {
	point t_size = get_text_size(text_cfg);
	y_ = h_;
	h_ = t_size.y > h_ ? t_size.y : h_;
}

void rich_label::update_canvas()
{
	for(canvas& tmp : get_canvases()) {
		tmp.set_variable("pos_x", wfl::variant(0));
		tmp.set_variable("pos_y", wfl::variant(0));
		tmp.set_variable("ih", wfl::variant(0));
		tmp.set_variable("iw", wfl::variant(0));
		// Disable ellipsization so that text wrapping can work
		tmp.set_variable("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
//		set_label(get_label());
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

//	if (!get_link_aware()) {
//		return; // without marking event as "handled".
//	}
//
//	if (!desktop::open_object_is_supported()) {
//		show_message("", _("Opening links is not supported, contact your packager"), dialogs::message::auto_close);
//		handled = true;
//		return;
//	}

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	PLAIN_LOG << mouse.x << "," << mouse.y;
	for (const auto& entry : links_) {
		PLAIN_LOG << entry.first.x << "," << entry.first.y;
		PLAIN_LOG << entry.first.w << "," << entry.first.h;

		if (entry.first.contains(mouse)) {
			DBG_GUI_E << "Clicked link! dst = " << entry.second;
			if (link_handler_) {
				link_handler_(entry.second);
			} else {
				DBG_GUI_E << "No registered link handler found";
			}

		}
	}

//	std::string link = get_label_link(mouse);

//	if (link.length() == 0) {
//		return ; // without marking event as "handled"
//	}

//	DBG_GUI_E << "Clicked Link:\"" << link << "\"";

//	const int res = show_message(_("Open link?"), link, dialogs::message::yes_no_buttons);
//	if(res == gui2::retval::OK) {
//		desktop::open_object(link);
//	}

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
