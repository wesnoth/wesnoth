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
	, txt_height_(0)
	, prev_txt_height_(0)
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

void rich_label::add_text_with_attribute(config& text_cfg, std::string text, std::string attr_name, std::string extra_data) {
	size_t start = text_cfg["text"].str().size();

	text_cfg["text"] = text_cfg["text"].str() + text;

	if (!attr_name.empty()) {
		text_cfg["attr_name"] = (text_cfg["attr_name"].str().empty() ? "" : (text_cfg["attr_name"].str() + ",")) + attr_name;
		text_cfg["attr_start"] = (text_cfg["attr_start"].str().empty() ? "" : (text_cfg["attr_start"].str() + ",")) +  std::to_string(start);
		text_cfg["attr_end"] = (text_cfg["attr_end"].str().empty() ? "" : (text_cfg["attr_end"].str() + ",")) + std::to_string(text_cfg["text"].str().size());
		if (!extra_data.empty()) {
			text_cfg["attr_color"] = (text_cfg["attr_color"].str().empty() ? "" : (text_cfg["attr_color"].str() + ",")) + extra_data;
		}
	}
	// Clear variables to stop them from growing too large
//	if (last_entry) {
//		text_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
//	}
}

size_t rich_label::get_split_location(int img_height) {
	point wrap_position = get_column_line(point(w_, img_height));

	size_t len = 0;
	for (int i = 0; i < wrap_position.y; i++) {
		len += utf8::size(font::get_text_renderer().get_lines()[i]);
	}
	len += wrap_position.x;

	PLAIN_LOG << "(gsl) : " << "ih : " << img_height << "len : " << len;
	return len;
}

void rich_label::set_label(const t_string& text)
{
	// Initialization
	w_ = 800;
	h_ = 0;
	unparsed_text_ = text;
	text_dom_.clear();
	links_.clear();
	help::topic_text marked_up_text(text);
	std::vector<std::string> parsed_text =  marked_up_text.parsed_text();

	config* text_ptr = nullptr;

	bool is_image = false;
	point wrap_position;
	point img_size;
	unsigned col_width = 0;
	unsigned max_col_height = 0;

	for (size_t i = 0; i < parsed_text.size(); i++) {
		bool last_entry = (i == parsed_text.size() - 1);
		std::string line = parsed_text.at(i);

		if (!line.empty() && line.at(0) == '[') {
			config cfg;
			std::istringstream stream(line);
			::read(cfg, line);

			if (cfg.optional_child("img")) {
				if (text_ptr != nullptr) {
					(*text_ptr)["actions"] = "([set_var('pos_y', pos_y + text_height)])";
				}

				config& img = text_dom_.add_child("image");
				img["name"] = cfg.mandatory_child("img")["src"];
				std::string align = cfg.mandatory_child("img")["align"];
				if (align.empty()) {
					align = "left";
				}

				bool floating = cfg.mandatory_child("img")["float"].to_bool();

				PLAIN_LOG << "float :" << cfg.mandatory_child("img")["float"];

				if (floating) {
					if (align == "right") {
						img["x"] = "(width - image_width - img_x)";
					} else {
						// left aligned images are default for now
						img["x"] = "(img_x)";
					}
				} else {
					if (align == "right") {
						img["x"] = "(width - image_width - pos_x)";
					} else {
						// left aligned images are default for now
						img["x"] = "(pos_x)";
					}
				}

				if (floating) {
					img["y"] = "(img_y)";
				} else {
					img["y"] = "(pos_y)";
				}
				img["h"] = "(image_height)";
				img["w"] = "(image_width)";

				if (last_entry) {
					img["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
				} else {
					if (floating) {
						if (align == "left") {
							img["actions"] = "([set_var('pos_x', image_width), set_var('img_y', img_y + image_height), set_var('iw', image_width), set_var('ih', image_height)])";
						} else if (align == "right") {
							img["actions"] = "([set_var('pos_x', 0), set_var('img_y', img_y + image_height), set_var('iw', image_width), set_var('ih', image_height)])";
						}
					} else {
						if (align == "left") {
							img["actions"] = "([set_var('pos_x', pos_x + image_width), set_var('iw', 0), set_var('ih', image_height)])";
						} else if (align == "right") {
							img["actions"] = "([set_var('pos_x', 0), set_var('iw', 0), set_var('ih', image_height)])";
						}
					}
				}

				if (floating) {
					img_size.x = get_image_size(img).x;
					img_size.y += get_image_size(img).y;
				} else {
					img_size.x += get_image_size(img).x;
					img_size.y = get_image_size(img).y;
				}

				y_ = h_;
				h_ += get_image_size(img).y;

				if (align == "left") {
					x_ = img_size.x;
				}

				PLAIN_LOG << "(img) x :" << x_ << ", y :" << y_ << ", h: " << h_;

				if (!floating) {
					is_image = true;
				}

			} else {
				// reset image positioning
				if (is_image) {
					prev_txt_height_ +=img_size.y;
					img_size = point(0,0);

//					PLAIN_LOG << "inserting break";
					config& break_cfg = text_dom_.add_child("text");
					default_text_config(&break_cfg);
					break_cfg["text"] = " ";
					break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + ih)])";
					is_image = false;

					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);
				}

				if (!text_ptr) {
					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);
				}

				// Text type tags

				int tmp_h = get_text_size(*text_ptr, w_ - img_size.x).y;

				if (cfg.optional_child("ref")) {
					add_text_with_attribute((*text_ptr), cfg.mandatory_child("ref")["text"], "fgcolor", font::YELLOW_COLOR.to_hex_string().substr(1, (*text_ptr)["text"].str().size()));

					// Add links
					point t_size = get_text_size(*text_ptr);
					unsigned x2 = t_size.x;
					rect link_rect = {static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(x2-x_), static_cast<int>(h_-y_)};
					links_.push_back(std::pair(link_rect, cfg.mandatory_child("ref")["dst"]));

				} else if (cfg.optional_child("bold")) {

					add_text_with_attribute((*text_ptr), cfg.mandatory_child("bold")["text"], "bold");

					PLAIN_LOG << "(bold) x :" << x_ << ", y :" << y_ << ", h: " << h_;

				} else if (cfg.optional_child("italic")) {

					add_text_with_attribute((*text_ptr), cfg.mandatory_child("italic")["text"], "italic");

					PLAIN_LOG << "(italic) x :" << x_ << ", y :"  << y_ << ", h: " << h_;

				} else if (cfg.optional_child("header")) {
					// Header starts in a new line/paragraph
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + if(ih > text_height, ih, text_height)), set_var('iw', 0), set_var('ih', 0)])";
					x_ = 0;
					prev_txt_height_ += std::max(img_size.y, get_text_size(*text_ptr, w_ - img_size.x).y);
//					PLAIN_LOG << "header, pth :" << prev_txt_height_;
					txt_height_ = 0;

					// Header config
					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);

					size_t start = 0;
					(*text_ptr)["text"] = cfg.mandatory_child("header")["text"];
					(*text_ptr)["attr_name"] = "fgcolor,fontsize";
					(*text_ptr)["attr_start"] = std::to_string(start) + "," + std::to_string(start);
					(*text_ptr)["attr_end"] =  std::to_string((*text_ptr)["text"].str().size()) + "," + std::to_string((*text_ptr)["text"].str().size());
					// TODO add font::GOLD_COLOR and remove hardcoded color value
					(*text_ptr)["attr_color"] = "baac7d," + std::to_string(font::SIZE_TITLE);

					PLAIN_LOG << "(header) x :" << x_ << ", y :" << y_ << ", h: " << h_;


//				} else if(line == "[link]") {
//				} else if(line == "[format]") {

				} else if(cfg.optional_child("table")) {
					// setup column width
					unsigned columns = cfg.mandatory_child("table")["col"].to_int();
					col_width = w_/columns;

					// start on a new line
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + if(ih > text_height, ih, text_height)), set_var('iw', width - pos_x - " + std::to_string(col_width) + "), set_var('ih', 0)])";
					x_ = 0;
					prev_txt_height_ += std::max(img_size.y, get_text_size(*text_ptr, w_ - img_size.x).y);
					txt_height_ = 0;

					PLAIN_LOG << "start table : " << "col=" << columns;
					PLAIN_LOG << "col_width : " << col_width;

					// new text block
					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);

				} else if(cfg.optional_child("jump")) {
					if (col_width > 0) {
						PLAIN_LOG << "new block/col";
//						PLAIN_LOG << "text height : " << txt_height_;

						max_col_height = std::max(max_col_height, txt_height_);
						txt_height_ = 0;
						x_ += col_width;
						(*text_ptr)["actions"] = "([set_var('pos_x', pos_x + " + std::to_string(col_width) + "),"
								+ " set_var('iw', width - pos_x - " + std::to_string(col_width) + ")])";

						// new text block
						text_ptr = &(text_dom_.add_child("text"));
						default_text_config(text_ptr);
					}
				} else if( cfg.optional_child("break") || cfg.optional_child("br") ) {
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + "+ std::to_string(max_col_height) +"), set_var('iw', width - pos_x - " + std::to_string(col_width) + ")])";

					x_ = 0;
					prev_txt_height_ += max_col_height;
					max_col_height = 0;
					txt_height_ = 0;

					// new text block
					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);

				} else if(cfg.optional_child("endtable")) {
					(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + "+ std::to_string(max_col_height) +"), set_var('iw', width - pos_x - " + std::to_string(col_width) + ")])";

					col_width = 0;
					x_ = 0;
					prev_txt_height_ += max_col_height;
					max_col_height = 0;
					txt_height_ = 0;

					// new text block
					text_ptr = &(text_dom_.add_child("text"));
					default_text_config(text_ptr);

					PLAIN_LOG << "end table";
				}


				if (tmp_h > get_text_size(*text_ptr, w_ - img_size.x).y) {
					tmp_h = 0;
				}
				txt_height_ += get_text_size(*text_ptr, w_ - img_size.x).y - tmp_h;
				if (img_size.y < static_cast<int>(txt_height_+prev_txt_height_)) {
					img_size = point(0,0);
					h_ = txt_height_ + prev_txt_height_;
				}

			}

		} else if (!line.empty()) {
			// reset image positioning
			if (is_image) {
				prev_txt_height_ += img_size.y;
				img_size = point(0,0);

//				PLAIN_LOG << "inserting break";
				config& break_cfg = text_dom_.add_child("text");
				default_text_config(&break_cfg);
				break_cfg["text"] = " ";
				break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + ih)])";
				is_image = false;

				text_ptr = &(text_dom_.add_child("text"));
				default_text_config(text_ptr);
			}

			if (!text_ptr) {
				text_ptr = &(text_dom_.add_child("text"));
				default_text_config(text_ptr);
			}

			int tmp_h = get_text_size(*text_ptr, w_ - img_size.x).y;
			(*text_ptr)["text"] = (*text_ptr)["text"].str() + line;

			if ( (img_size.y > 0) && (get_text_size(*text_ptr, w_ - img_size.x).y > img_size.y) ) {
				size_t len = get_split_location(img_size.y);

				t_string* removed_part = new t_string((*text_ptr)["text"].str().substr(len));

				(*text_ptr)["text"] = (*text_ptr)["text"].str().substr(0, len);
				(*text_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + text_height), set_var('iw', 0), set_var('ih', 0)])";

				x_ = 0;
				prev_txt_height_ += std::max(img_size.y, get_text_size(*text_ptr, w_ - img_size.x).y);
				txt_height_ = 0;

				text_ptr = &(text_dom_.add_child("text"));
				default_text_config(text_ptr);

				add_text_with_attribute(*text_ptr, *removed_part);

			} else if ((img_size.y > 0) && (get_text_size(*text_ptr, w_ - img_size.x).y < img_size.y)) {
				(*text_ptr)["actions"] = "([set_var('pos_y', pos_y + ih)])";
			}

			if (tmp_h > get_text_size(*text_ptr, w_ - img_size.x).y) {
				tmp_h = 0;
			}

			txt_height_ += get_text_size(*text_ptr, w_ - img_size.x).y - tmp_h;

			if (img_size.y < static_cast<int>(txt_height_+prev_txt_height_)) {
				img_size = point(0,0);
				h_ = txt_height_ + prev_txt_height_;
			}

		}

		if (last_entry) {
			config& break_cfg = text_dom_.add_child("text");
			default_text_config(&break_cfg);
			break_cfg["text"] = " ";
			break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0), set_var('img_x', 0), set_var('img_y', 0)])";
		}
		//padding
		h_ += 20;
	} // for loop ends
//	PLAIN_LOG << "h :" << h_;

//	PLAIN_LOG << text_dom_.debug();
} // function ends

void rich_label::default_text_config(config* txt_ptr, t_string text, bool last_entry) {
	(*txt_ptr)["text"] = text;
	(*txt_ptr)["font_size"] = 16;
	(*txt_ptr)["x"] = "(pos_x)";
	(*txt_ptr)["y"] = "(debug_print('dtc, y', pos_y))";
	(*txt_ptr)["w"] = "(text_width)";
	(*txt_ptr)["h"] = "(text_height)";
	(*txt_ptr)["maximum_width"] = "(width - iw)";

	if (last_entry) {
		(*txt_ptr)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
	} else {
		(*txt_ptr)["actions"] = "([set_var('pos_y', pos_y+text_height)])";
	}
}

void rich_label::update_canvas()
{
	for(canvas& tmp : get_canvases()) {
		tmp.set_variable("pos_x", wfl::variant(0));
		tmp.set_variable("pos_y", wfl::variant(0));
		tmp.set_variable("img_x", wfl::variant(0));
		tmp.set_variable("img_y", wfl::variant(0));
		tmp.set_variable("ih", wfl::variant(0));
		tmp.set_variable("iw", wfl::variant(0));
		// Disable ellipsization so that text wrapping can work
		tmp.set_variable("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
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
