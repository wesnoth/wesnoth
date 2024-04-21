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
#include <boost/format.hpp>

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
	wfl::map_formula_callable variables = setup_text_renderer(text_cfg, width);
	return point(variables.query_value("text_width").as_int(), variables.query_value("text_height").as_int());
}

wfl::map_formula_callable rich_label::setup_text_renderer(config text_cfg, unsigned width) {
	// Set up fake render to calculate text position
	wfl::action_function_symbol_table functions;
	wfl::map_formula_callable variables;
	variables.add("text", wfl::variant(text_cfg["text"].str()));
	variables.add("width", wfl::variant(width > 0 ? width : w_));
	variables.add("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
	variables.add("fake_draw", wfl::variant(true));
	tshape_ = std::make_unique<gui2::text_shape>(text_cfg, functions);
	tshape_->draw(variables);
	return variables;
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
			text_cfg["attr_data"] = (text_cfg["attr_data"].str().empty() ? "" : (text_cfg["attr_data"].str() + ",")) + extra_data;
		}
	}
}

size_t rich_label::get_split_location(int img_height) {
	point wrap_position = get_column_line(point(w_, img_height));

	size_t len = 0;
	for (int i = 0; i < wrap_position.y; i++) {
		len += utf8::size(font::get_text_renderer().get_lines()[i]);
	}
	len += wrap_position.x;

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

	config* curr_item = nullptr;

	bool is_image = false;
	bool floating = false;
	bool new_text_block = false;
	point wrap_position;
	point img_size;
	unsigned col_width = 0;
	unsigned max_col_height = 0;

	for (size_t i = 0; i < parsed_text.size(); i++) {
		bool last_entry = (i == parsed_text.size() - 1);
		std::string line = parsed_text.at(i);

		if (!line.empty() && line.at(0) == '[') {
			config cfg;
			::read(cfg, line);

			if (cfg.optional_child("img")) {

				floating = cfg.mandatory_child("img")["float"].to_bool();

				curr_item = &(text_dom_.add_child("image"));
				(*curr_item)["name"] = cfg.mandatory_child("img")["src"];
				std::string align = cfg.mandatory_child("img")["align"];
				if (align.empty()) {
					align = "left";
				}

				if (floating) {
					if (align == "right") {
						(*curr_item)["x"] = "(width - image_width - img_x)";
					} else {
						// left aligned images are default for now
						(*curr_item)["x"] = "(img_x)";
					}
				} else {
					if (align == "right") {
						(*curr_item)["x"] = "(width - image_width - pos_x)";
					} else {
						// left aligned images are default for now
						(*curr_item)["x"] = "(pos_x)";
					}
				}

//				if (floating) {
//					(*curr_item)["y"] = "(img_y)";
//				} else {
					(*curr_item)["y"] = "(pos_y)";
//				}
				(*curr_item)["h"] = "(image_height)";
				(*curr_item)["w"] = "(image_width)";

				bool break_line = cfg.mandatory_child("img")["break"].to_bool();

				// Sizing
				if (floating) {
					img_size.x = get_image_size(*curr_item).x;
					img_size.y += get_image_size(*curr_item).y;
				} else {
					img_size.x += get_image_size(*curr_item).x;
					img_size.y = get_image_size(*curr_item).y;
				}

				std::stringstream actions;
				actions << "([";
				if (floating) {
					if (align == "left") {
						x_ = img_size.x + 5;
						actions << "set_var('pos_x', image_width + 5)";
					} else if (align == "right") {
						x_ = 0;
						actions << "set_var('pos_x', 0)";
						actions << ",";
						actions << "set_var('ww', image_width)";
					}
					actions << "," <<  "set_var('img_y', img_y + image_height + 5)";
				} else {
					if (align == "left" && !break_line) {
						x_ += img_size.x + 5;
						actions << "set_var('pos_x', pos_x + image_width + 5)";						
					} else {
						if (align == "right" && !break_line) {
							actions << "set_var('ww', image_width)";
							actions << ",";
						}
						x_ = 0;
						actions << "set_var('pos_x', 0)";
					}
				}
				if (break_line) {
					actions << "," << "set_var('pos_y', pos_y + image_height + 5)";
				}
				actions << "])";

				(*curr_item)["actions"] = actions.str();
				actions.str("");

				//FIXME image could be in the middle of a text block

				is_image = true;
				if (break_line && !floating) {
					new_text_block = true;
				}

				PLAIN_LOG << "(img) x :" << x_ << ", y :" << y_ << ", h: " << h_;

			} else {
				if (curr_item == nullptr || new_text_block) {
					curr_item = &(text_dom_.add_child("text"));
					default_text_config(curr_item);
					new_text_block = false;
				}
				
				if (is_image && !floating) {
					prev_txt_height_ += img_size.y + 5;
					img_size = point(0,0);
					
					curr_item = &(text_dom_.add_child("text"));
					default_text_config(curr_item);
				}

				// Text type tags

				int tmp_h = get_text_size(*curr_item, w_ - img_size.x).y;

				if (cfg.optional_child("ref")) {
					setup_text_renderer(*curr_item);
					point t_start = get_xy_from_offset((*curr_item)["text"].str().size());

					std::string link_text = cfg.mandatory_child("ref")["text"].str();
					link_text = link_text.empty() ? cfg.mandatory_child("ref")["dst"] : link_text;

					add_text_with_attribute((*curr_item), link_text, "color", font::YELLOW_COLOR.to_hex_string().substr(1));

					setup_text_renderer(*curr_item);
					point t_end = get_xy_from_offset((*curr_item)["text"].str().size());
					// TODO Needs to be adjusted if ref's font size is changed
					t_end.y += font::get_max_height(font::SIZE_NORMAL);

					// Add link
					// TODO only works if text is on the right side of the image
					rect link_rect = {
							static_cast<int>(x_ + t_start.x),
							static_cast<int>(prev_txt_height_ + t_start.y),
							static_cast<int>(t_end.x - t_start.x),
							static_cast<int>(t_end.y - t_start.y)
					};

					links_.push_back(std::pair(link_rect, cfg.mandatory_child("ref")["dst"]));

//					is_image = false;

					PLAIN_LOG << "(ref) x :" << x_ << " y :" << y_ << " h :" << h_ << " pth : " << prev_txt_height_;

				} else if (cfg.optional_child("bold")||cfg.optional_child("b")) {

					if (cfg.optional_child("bold")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("bold")["text"], "bold");
					} else if (cfg.optional_child("b")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("b")["text"], "bold");
					}

					is_image = false;

				} else if (cfg.optional_child("italic")||cfg.optional_child("i")) {

					if (cfg.optional_child("italic")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("italic")["text"], "italic");
					} else if (cfg.optional_child("i")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("i")["text"], "italic");
					}

					is_image = false;
				} else if (cfg.optional_child("underline")||cfg.optional_child("u")) {

					if (cfg.optional_child("underline")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("underline")["text"], "underline");
					} else if (cfg.optional_child("u")) {
						add_text_with_attribute((*curr_item), cfg.mandatory_child("u")["text"], "underline");
					}

					is_image = false;

				} else if (cfg.optional_child("header")||cfg.optional_child("h")) {
					// Header starts in a new line/paragraph
					if (is_image) {
						(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + image_height)])";
					} else {
						(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + text_height)])";
					}
					prev_txt_height_ += std::max(img_size.y, get_text_size(*curr_item, w_ - img_size.x).y);
					txt_height_ = 0;

					// Header config
					curr_item = &(text_dom_.add_child("text"));
					default_text_config(curr_item);

					size_t start = 0;
					if (cfg.optional_child("header")) {
						(*curr_item)["text"] = cfg.mandatory_child("header")["text"];
					} else if (cfg.optional_child("h")) {
						(*curr_item)["text"] = cfg.mandatory_child("h")["text"];
					}
					(*curr_item)["attr_name"] = "color,size";
					(*curr_item)["attr_start"] = std::to_string(start) + "," + std::to_string(start);
					(*curr_item)["attr_end"] =
							std::to_string((*curr_item)["text"].str().size())
							+ ","
							+ std::to_string((*curr_item)["text"].str().size());
					(*curr_item)["attr_data"] =
							font::TITLE_COLOR.to_hex_string().substr(1)
							+ ","
							+ std::to_string(font::SIZE_TITLE);

					prev_txt_height_ += get_text_size(*curr_item, w_ - img_size.x).y;
					is_image = false;
					new_text_block = true;

				} else if(cfg.optional_child("span")||cfg.optional_child("format")) {
					//TODO add bold, italic and underline
				
					config& span_cfg = cfg.mandatory_child("span");
					size_t start = (*curr_item)["text"].str().size();
					PLAIN_LOG << (*curr_item)["text"];
					(*curr_item)["text"] = span_cfg["text"].str();
					size_t end = (*curr_item)["text"].str().size();

					PLAIN_LOG << span_cfg["text"];
					PLAIN_LOG << (*curr_item)["text"];

					std::stringstream attrs;
					std::stringstream attr_start;
					std::stringstream attr_end;
					std::stringstream attr_data;

					size_t i = 1;
					for (const auto& attr : span_cfg.attribute_range()) {
						if (attr.first != "text") {
							PLAIN_LOG << attr.first << "," << attr.second;
							attrs << attr.first;
							attr_start << start;
							attr_end << end;
							attr_data << attr.second;
							if (i != span_cfg.attribute_count() - 1) {
								PLAIN_LOG << i << "," << span_cfg.attribute_count();
								attrs << ",";
								attr_start << ",";
								attr_end << ",";
								attr_data << ",";
							}
						}
						i++;
					}

					(*curr_item)["attr_name"] = attrs.str();
					(*curr_item)["attr_start"] = attr_start.str();
					(*curr_item)["attr_end"] = attr_end.str();
					(*curr_item)["attr_data"] = attr_data.str();

					PLAIN_LOG << curr_item->debug();

				} else if(cfg.optional_child("table")) {
					// setup column width
					unsigned columns = cfg.mandatory_child("table")["col"].to_int();
					unsigned width = cfg.mandatory_child("table")["width"].to_int();
					width = width > 0 ? width : w_;
					col_width = width/columns;

					// start on a new line
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + if(ih > text_height, ih, text_height)), set_var('tw', width - pos_x - %d), set_var('ih', 0)])") % col_width);
					x_ = 0;
					prev_txt_height_ += std::max(img_size.y, get_text_size(*curr_item, w_ - img_size.x).y);
					txt_height_ = 0;

					new_text_block = true;

					PLAIN_LOG << "start table : " << "col=" << columns;
					PLAIN_LOG << "col_width : " << col_width;

				} else if(cfg.optional_child("jump")) {
					if (col_width > 0) {
						PLAIN_LOG << "(jump) new block/col";

						max_col_height = std::max(max_col_height, txt_height_);
						max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
						txt_height_ = 0;
						x_ += col_width;

						(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', pos_x + %d), set_var('tw', width - pos_x - %d)])") % col_width % col_width);

						PLAIN_LOG << curr_item->debug();

						if (!is_image) {
							new_text_block = true;
						}
					}
				} else if( cfg.optional_child("break") || cfg.optional_child("br") ) {
					max_col_height = std::max(max_col_height, txt_height_);
					max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
					PLAIN_LOG << "(br) " << max_col_height;
					PLAIN_LOG << curr_item->debug();
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %d + 3), set_var('tw', width - pos_x - %d)])") % max_col_height % col_width);

					//linebreak
					x_ = 0;
					prev_txt_height_ += max_col_height;
					max_col_height = 0;
					txt_height_ = 0;

					if (!is_image) {
						new_text_block = true;
					}

				} else if(cfg.optional_child("endtable")) {
					PLAIN_LOG << "(endtable) " << max_col_height;
					PLAIN_LOG << curr_item->debug();
					max_col_height = std::max(max_col_height, txt_height_);
					max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %d), set_var('tw', 0)])") % max_col_height);

					//linebreak and reset col_width
					col_width = 0;
					x_ = 0;
					prev_txt_height_ += max_col_height;
					max_col_height = 0;
					txt_height_ = 0;

					if (!last_entry) {
						new_text_block = true;
					}
				}

				if (x_ != 0) {
					// update text positin if there is no line break
					setup_text_renderer(*curr_item);
					x_ = get_xy_from_offset((*curr_item)["text"].str().size()).x;
				}

				// update text size and widget height
				if (tmp_h > get_text_size(*curr_item, w_ - img_size.x).y) {
					tmp_h = 0;
				}
				txt_height_ += get_text_size(*curr_item, w_ - img_size.x).y - tmp_h;
				if (img_size.y < static_cast<int>(txt_height_+prev_txt_height_)) {
					img_size = point(0,0);
					y_ = h_;
					h_ = txt_height_ + prev_txt_height_;
				}

			}

		} else if (!line.empty()) {
			if (curr_item == nullptr || new_text_block) {
				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);
				new_text_block = false;
			}
			
			if (is_image && !floating) {
				prev_txt_height_ += img_size.y + 5;
				img_size = point(0,0);
				
				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);
			}
			
			PLAIN_LOG << "(text) x :" << x_ << ", y :" << y_ << ", h: " << h_;

			(*curr_item)["font_size"] = font::SIZE_NORMAL;
			
			int tmp_h = get_text_size(*curr_item, w_ - img_size.x).y;

			(*curr_item)["text"] = (*curr_item)["text"].str() + line;

			if ( floating && (img_size.y > 0) && (get_text_size(*curr_item, w_ - img_size.x).y > img_size.y) ) {
				PLAIN_LOG << "wrap start";

				size_t len = get_split_location(img_size.y);

				t_string* removed_part = new t_string((*curr_item)["text"].str().substr(len));

				(*curr_item)["text"] = (*curr_item)["text"].str().substr(0, len);
				(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + text_height)])";

				x_ = 0;
				prev_txt_height_ += std::max(img_size.y, get_text_size(*curr_item, w_ - img_size.x).y);
				txt_height_ = 0;

				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);

				add_text_with_attribute(*curr_item, *removed_part);

			} else if ((img_size.y > 0) && (get_text_size(*curr_item, w_ - img_size.x).y < img_size.y)) {
				PLAIN_LOG << "no wrap";
				if (is_image) {
					(*curr_item)["actions"] = "([set_var('pos_y', pos_y + image_height)])";
				} else {
					(*curr_item)["actions"] = "([set_var('pos_y', pos_y + text_height)])";
				}
			}

			// update text size and widget height
			if (tmp_h > get_text_size(*curr_item, w_ - img_size.x).y) {
				tmp_h = 0;
			}

			txt_height_ += get_text_size(*curr_item, w_ - img_size.x).y - tmp_h;

			if (img_size.y < static_cast<int>(txt_height_+prev_txt_height_)) {
				img_size = point(0,0);
				y_ = h_;
				h_ = txt_height_ + prev_txt_height_;
			}

			is_image = false;
		}

		// reset all variables to zero, otherwise they grow infinitely
		if (last_entry) {
			if (is_image) {
				y_ = h_;
				h_ += get_image_size(*curr_item).y + 5;
			}
		
			config& break_cfg = text_dom_.add_child("text");
			default_text_config(&break_cfg);
			break_cfg["text"] = " ";
			break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0), set_var('img_x', 0), set_var('img_y', 0)])";
		}

	} // for loop ends

	// padding, avoids text from getting cut off at the bottom
//	h_ += 20;

} // function ends

void rich_label::default_text_config(config* txt_ptr, t_string text) {
	(*txt_ptr)["text"] = text;
	(*txt_ptr)["font_size"] = font::SIZE_NORMAL;
	(*txt_ptr)["x"] = "(pos_x)";
	(*txt_ptr)["y"] = "(pos_y)";
	(*txt_ptr)["w"] = "(text_width)";
	(*txt_ptr)["h"] = "(text_height)";
	// tw -> table width, used for wrapping text inside table cols
	// ww -> wrap width, used for wrapping around floating image
	(*txt_ptr)["maximum_width"] = "(width - tw - pos_x - ww)";
	(*txt_ptr)["actions"] = "([set_var('pos_y', pos_y+text_height)])";
}

void rich_label::update_canvas()
{
	for(canvas& tmp : get_canvases()) {
		tmp.set_variable("pos_x", wfl::variant(0));
		tmp.set_variable("pos_y", wfl::variant(0));
		tmp.set_variable("img_x", wfl::variant(0));
		tmp.set_variable("img_y", wfl::variant(0));
		tmp.set_variable("tw", wfl::variant(0));
		tmp.set_variable("ww", wfl::variant(0));
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

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	PLAIN_LOG << "(mouse)" << mouse.x << "," << mouse.y;
	PLAIN_LOG << "link count :" << links_.size();
	for (const auto& entry : links_) {
		PLAIN_LOG << "link [" << entry.first.x << "," << entry.first.y << ","
		<< entry.first.x + entry.first.w << "," << entry.first.y + entry.first.h  << "]";

		if (entry.first.contains(mouse)) {
			PLAIN_LOG << "Clicked link! dst = " << entry.second;
			if (link_handler_) {
				link_handler_(entry.second);
			} else {
				PLAIN_LOG << "No registered link handler found";
			}

		}
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
