/*
	Copyright (C) 2024
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
#include "log.hpp"
#include "serialization/unicode.hpp"
#include "serialization/string_utils.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <string>
#include <boost/format.hpp>

static lg::log_domain log_rich_label("gui/widget/rich_label");
#define DBG_GUI_RL LOG_STREAM(debug, log_rich_label)

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(rich_label)

rich_label::rich_label(const implementation::builder_rich_label& builder)
	: styled_widget(builder, type())
	, state_(ENABLED)
	, can_wrap_(true)
	, link_aware_(true)
	, link_color_(font::YELLOW_COLOR)
	, can_shrink_(true)
	, text_alpha_(ALPHA_OPAQUE)
	, unparsed_text_()
	, w_(0)
	, h_(0)
	, x_(0)
	, padding_(0)
	, txt_height_(0)
	, prev_blk_height_(0)
{
	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&rich_label::signal_handler_left_button_click, this, std::placeholders::_3));
	connect_signal<event::MOUSE_MOTION>(
		std::bind(&rich_label::signal_handler_mouse_motion, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&rich_label::signal_handler_mouse_leave, this, std::placeholders::_3));
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

point rich_label::get_text_size(config text_cfg, unsigned width) {
	wfl::map_formula_callable variables = setup_text_renderer(text_cfg, width);
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

void rich_label::add_text_with_attribute(config& curr_item, std::string text, std::string attr_name, std::string extra_data) {
	size_t start = curr_item["text"].str().size();

	curr_item["text"] = curr_item["text"].str() + text;

	if (!attr_name.empty()) {
		append_if_not_empty(&curr_item["attr_name"], ",");
		curr_item["attr_name"] = curr_item["attr_name"].str() + attr_name;

		append_if_not_empty(&curr_item["attr_start"], ",");
		curr_item["attr_start"] = curr_item["attr_start"].str() + std::to_string(start);

		append_if_not_empty(&curr_item["attr_end"], ",");
		curr_item["attr_end"] = curr_item["attr_end"].str() + std::to_string(curr_item["text"].str().size());

		if (!extra_data.empty()) {
			append_if_not_empty(&curr_item["attr_data"], ",");
			curr_item["attr_data"] = curr_item["attr_data"].str() + extra_data;
		}
	}
}

void rich_label::add_text_with_attributes(config& curr_item, std::string text, std::vector<std::string> attr_names, std::vector<std::string> extra_data) {

	size_t start = curr_item["text"].str().size();
	curr_item["text"] = curr_item["text"].str() + text;

	if (!attr_names.empty()) {
		append_if_not_empty(&curr_item["attr_name"], ",");
		curr_item["attr_name"] = curr_item["attr_name"].str() + utils::join(attr_names);

		for (size_t i = 0; i < attr_names.size(); i++) {
			append_if_not_empty(&curr_item["attr_start"], ",");
			curr_item["attr_start"] = curr_item["attr_start"].str() + std::to_string(start);
			append_if_not_empty(&curr_item["attr_end"], ",");
			curr_item["attr_end"] = curr_item["attr_end"].str() + std::to_string(curr_item["text"].str().size());
		}

		if (!extra_data.empty()) {
			append_if_not_empty(&curr_item["attr_data"], ",");
			curr_item["attr_data"] = curr_item["attr_data"].str() + utils::join(extra_data);
		}
	}
}

void rich_label::add_image(config& curr_item, std::string name, std::string align, bool has_prev_image, bool is_prev_float, bool floating, point& img_size, point& float_size) {
	curr_item["name"] = name;

	if (align.empty()) {
		align = "left";
	}

	if (align == "right") {
		curr_item["x"] = floating ? "(width - image_width - img_x)" : "(width - image_width - pos_x)";
	} else if (align == "middle" || align == "center") {
		// works for single image only
		curr_item["x"] = floating ? "(img_x + (width - image_width)/2.0)" : "(pos_x + (width - image_width)/2.0)";
	} else {
		// left aligned images are default for now
		curr_item["x"] = floating ? "(img_x)" : "(pos_x)";
	}
	curr_item["y"] = (has_prev_image && floating) ? "(img_y + pos_y)" : "(pos_y)";
	curr_item["h"] = "(image_height)";
	curr_item["w"] = "(image_width)";

	// Sizing
	point curr_img_size = get_image_size(curr_item);
	if (floating) {
		float_size.x = curr_img_size.x + padding_;
		float_size.y += curr_img_size.y + padding_;
	} else {
		img_size.x += curr_img_size.x + padding_;
		img_size.y = std::max(img_size.y, curr_img_size.y);
		// FIXME: still doesn't cover the case where consecutive inline images have different heights
		if (!has_prev_image || (has_prev_image && is_prev_float)) {
			prev_blk_height_ += curr_img_size.y + padding_;
			float_size.y -= curr_img_size.y + padding_;
		}
	}

	std::stringstream actions;
	actions << "([";
	if (floating) {
		if (align == "left") {
			x_ = float_size.x;
			actions << "set_var('pos_x', image_width + padding)";
		} else if (align == "right") {
			x_ = 0;
			actions << "set_var('pos_x', 0)";
			actions << ",";
			actions << "set_var('ww', image_width + padding)";
		}

		actions << "," <<  "set_var('img_y', img_y + image_height + padding)";
	} else {
		x_ += img_size.x;
		actions << "set_var('pos_x', pos_x + image_width + padding)";
		// y coordinate is updated later, based on whether a linebreak follows
	}
	actions << "])";

	curr_item["actions"] = actions.str();
	actions.str("");
}

void rich_label::add_link(config& curr_item, std::string name, std::string dest, int img_width) {
	// TODO algorithm needs to be text_alignment independent

	DBG_GUI_RL << "add_link: " << name << "->" << dest;
	DBG_GUI_RL << "x=" << x_ << " width=" << img_width;

	point t_start, t_end;

	setup_text_renderer(curr_item, w_ - x_ - img_width);
	t_start.x = x_ + get_xy_from_offset(utf8::size(curr_item["text"].str())).x;
	t_start.y = prev_blk_height_ + get_xy_from_offset(utf8::size(curr_item["text"].str())).y;
	DBG_GUI_RL << "link text start:" << t_start;

	std::string link_text = name.empty() ? dest : name;
	add_text_with_attribute(curr_item, link_text, "color", link_color_.to_hex_string());

	setup_text_renderer(curr_item, w_ - x_ - img_width);
	t_end.x = x_ + get_xy_from_offset(utf8::size(curr_item["text"].str())).x;
	DBG_GUI_RL << "link text end:" << t_end;

	DBG_GUI_RL << "prev_blk_height_: " << prev_blk_height_ << ", text_height_: " << txt_height_;

	// TODO link after right aligned images

	// Add link
	if (t_end.x > t_start.x) {
		rect link_rect = {
			t_start.x,
			t_start.y,
			t_end.x -  t_start.x,
			font::get_max_height(font::SIZE_NORMAL),
		};
		links_.push_back(std::pair(link_rect, dest));

		DBG_GUI_RL << "added link at rect: " << link_rect;

	} else {
		//link straddles two lines, break into two rects
		point t_size(w_ - t_start.x - (x_ == 0 ? img_width : 0), t_end.y - t_start.y);
		point link_start2(x_, t_start.y + 1.3*font::get_max_height(font::SIZE_NORMAL));
		point t_size2(t_end.x, t_end.y - t_start.y);

		rect link_rect = {
				t_start.x,
				t_start.y,
				t_size.x,
				font::get_max_height(font::SIZE_NORMAL),
		};

		rect link_rect2 = {
				link_start2.x,
				link_start2.y,
				t_size2.x,
				font::get_max_height(font::SIZE_NORMAL),
		};

		links_.push_back(std::pair(link_rect, dest));
		links_.push_back(std::pair(link_rect2, dest));

		DBG_GUI_RL << "added link at rect 1: " << link_rect;
		DBG_GUI_RL << "added link at rect 2: " << link_rect2;
	}
}

size_t rich_label::get_split_location(std::string text, const point& pos) {
	font::get_text_renderer().set_maximum_width(pos.x);
	font::get_text_renderer().set_text(text, true);
	point wrap_position = get_column_line(pos);

	size_t len = 0;
	for (int i = 0; i < wrap_position.y; i++) {
		len += utf8::size(font::get_text_renderer().get_lines()[i]);
	}
	len += wrap_position.x;
	// size() and utf::size() can return different values
	len = len > (text.size()-1) ? text.size()-1 : len;

	// break only at word boundary
	char c;
	while((c = text[len]) != ' ') {
		len--;
		if (len == 0) {
			break;
		}
	}

	return len;
}

void rich_label::set_topic(const help::topic* topic)
{
	set_parsed_text(topic->text.parsed_text());
}

void rich_label::set_label(const t_string& text)
{
	unparsed_text_ = text;
	help::topic_text marked_up_text(text);
	const config& parsed_text = marked_up_text.parsed_text();
	set_parsed_text(parsed_text);
}

void rich_label::set_parsed_text(const config& parsed_text)
{
	// Initialization
	w_ = (w_ == 0) ? styled_widget::calculate_best_size().x : w_;
	DBG_GUI_RL << "Width: " << w_;
	x_ = 0;
	h_ = 0;
	text_dom_.clear();
	links_.clear();

	config* curr_item = nullptr;

	bool is_image = false;
	bool is_float = false;
	bool wrap_mode = false;
	bool new_text_block = false;

	prev_blk_height_ = 0;
	txt_height_ = 0;

	point img_size;
	point float_size;

	bool in_table = false;
	unsigned col_idx = 0;
	unsigned col_width = 0;
	unsigned max_col_height = 0;
	unsigned row_y = 0;

	PLAIN_LOG << parsed_text.debug();
	for(config::any_child tag : parsed_text.all_children_range()) {
			config& child = tag.cfg;

			if(tag.key == "img") {

				std::string name = child["src"];
				std::string align = child["align"];
				bool is_curr_float = child["float"].to_bool();

				if (wrap_mode) {
					// do nothing, keep float on
					wrap_mode = true;
				} else {
					// is current img floating
					if(is_curr_float) {
						wrap_mode = true;
					}
				}
				DBG_GUI_RL << "floating: " << wrap_mode << ", " << is_float;

				curr_item = &(text_dom_.add_child("image"));
				add_image(*curr_item, name, align, is_image, is_float, is_curr_float, img_size, float_size);

				DBG_GUI_RL << "image: src=" << name << ", size=" << get_image_size(*curr_item);

				is_image = true;
				is_float = is_curr_float;
				new_text_block = true;

			} else {

				if (is_image && (!is_float)) {
					x_ = 0;
					(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + image_height + padding)])";
				}

				if (curr_item == nullptr || new_text_block) {
					if (!in_table) {
						// table will calculate this by itself, no need to calculate here
						prev_blk_height_ += txt_height_ + padding_;
						txt_height_ = 0;
					}

					curr_item = &(text_dom_.add_child("text"));
					default_text_config(curr_item);
					new_text_block = false;
				}

				// }---------- TEXT TAGS -----------{
				int tmp_h = get_text_size(*curr_item, w_ - (x_ == 0 ? float_size.x : x_)).y;

				if(tag.key == "ref") {

					add_link(*curr_item, child["text"], child["dst"], float_size.x);
					is_image = false;

					DBG_GUI_RL << "ref: dst=" << child["dst"];

				} else if(tag.key == "bold" || tag.key == "b") {

					add_text_with_attribute(*curr_item, child["text"], "bold");
					is_image = false;

					DBG_GUI_RL << "bold: text=" << child["text"];

				} else if(tag.key == "italic" || tag.key == "i") {

					add_text_with_attribute(*curr_item, child["text"], "italic");
					is_image = false;

					DBG_GUI_RL << "italic: text=" << child["text"];

				} else if(tag.key == "underline" || tag.key == "u") {

					add_text_with_attribute(*curr_item, child["text"], "underline");
					is_image = false;

					DBG_GUI_RL << "u: text=" << child["text"];

				} else if(tag.key == "header" || tag.key == "h") {

					// Header starts in a new line

					append_if_not_empty(&((*curr_item)["attr_name"]), ",");
					append_if_not_empty(&((*curr_item)["attr_start"]), ",");
					append_if_not_empty(&((*curr_item)["attr_end"]), ",");
					append_if_not_empty(&((*curr_item)["attr_data"]), ",");

					std::stringstream header_text;
					header_text << child["text"].str();
					std::vector<std::string> attrs = {"face", "color", "size"};
					std::vector<std::string> attr_data;
					attr_data.push_back("serif");
					attr_data.push_back(font::string_to_color("white").to_hex_string());
					attr_data.push_back(std::to_string(font::SIZE_TITLE - 2));

					add_text_with_attributes((*curr_item), header_text.str(), attrs, attr_data);

					is_image = false;

					DBG_GUI_RL << "h: text=" << child["text"];

				} else if(tag.key == "span" || tag.key == "format") {

					std::vector<std::string> attrs;
					std::vector<std::string> attr_data;

					DBG_GUI_RL << "span/format: text=" << child["text"];
					DBG_GUI_RL << "attributes:";

					for (const auto& attr : child.attribute_range()) {
						if (attr.first != "text") {
							attrs.push_back(attr.first);
							attr_data.push_back(attr.second);
							DBG_GUI_RL << attr.first << "=" << attr.second;
						}
					}

					add_text_with_attributes((*curr_item), child["text"], attrs, attr_data);
					is_image = false;

				// }---------- TABLE TAGS -----------{

				} else if(tag.key == "table") {

					in_table = true;
					col_idx = 0;

					// table doesn't support floating images alongside
					img_size = point(0,0);
					float_size = point(0,0);

					// setup column width
					unsigned columns = child["col"].to_int();
					unsigned width = child["width"].to_int();
					width = width > 0 ? width : w_;
					col_width = width/columns;

					// start on a new line
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %s), set_var('tw', width - pos_x - %d)])") % (is_image ? "image_height" : "text_height") % col_width);
					x_ = 0;
					prev_blk_height_ += txt_height_;
					txt_height_ = 0;

					row_y = prev_blk_height_;

					config& link_rect_cfg = text_dom_.add_child("line");
					link_rect_cfg["x1"] = 0;
					link_rect_cfg["y1"] = prev_blk_height_;
					link_rect_cfg["x2"] = w_;
					link_rect_cfg["y2"] = prev_blk_height_;
					link_rect_cfg["color"] = "255, 0, 0, 255";

					new_text_block = true;

					DBG_GUI_RL << "start table : " << "col=" << columns;
					DBG_GUI_RL << "col_width : " << col_width;

				} else if(tag.key == "jump") {

					if (col_width > 0) {

						max_col_height = std::max(max_col_height, txt_height_);
						max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
						txt_height_ = 0;

						col_idx++;
						x_ = col_idx * col_width;

						DBG_GUI_RL << "jump to next column";

						(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', %d), set_var('tw', width - %d - %d)])") % x_ % x_ % col_width);

						prev_blk_height_ = row_y;

						if (!is_image) {
							new_text_block = true;
						}
					}

				} else if(tag.key == "break" || tag.key == "br") {

					if (in_table) {
						DBG_GUI_RL << "break";
						DBG_GUI_RL << txt_height_;
						DBG_GUI_RL << img_size;

						max_col_height = std::max(max_col_height, txt_height_);
						max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));

						DBG_GUI_RL << max_col_height;

						//linebreak
						col_idx = 0;
						x_ = 0;
						prev_blk_height_ += max_col_height + padding_;
						row_y = prev_blk_height_;

						(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %d + %d), set_var('tw', width - pos_x - %d)])") % max_col_height % padding_ % col_width);

						max_col_height = 0;
						txt_height_ = 0;
						img_size = point(0,0);
					}

					DBG_GUI_RL << "linebreak";

					if (!is_image) {
						new_text_block = true;
					}

				} else if(tag.key == "endtable") {

					DBG_GUI_RL << "end table: " << max_col_height;
					col_width = 0;
					in_table = false;
					row_y = 0;
				}

				int ah = get_text_size(*curr_item, w_ - (x_ == 0 ? float_size.x : x_)).y;
				// update text size and widget height
				if (tmp_h > ah) {
					tmp_h = 0;
				}

				txt_height_ += ah - tmp_h;
			}

		if (tag.key == "text") {
			std::string line = child["text"];
			DBG_GUI_RL << "text: text=" << line << "...";

			// Start the text in a new paragraph if a newline follows after an inline image
			if (is_image && (!is_float)) {
				if ((line[0] == '\n')) {
					x_ = 0;
					(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + image_height + padding)])";
				} else {
					prev_blk_height_ -= img_size.y + padding_;
				}
			}

			if (curr_item == nullptr || new_text_block) {
				if (!in_table) {
					// table will calculate this by itself, no need to calculate here
					prev_blk_height_ += txt_height_ + padding_;
					txt_height_ = 0;
				}

				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);
				new_text_block = false;
			}

			(*curr_item)["font_size"] = font::SIZE_NORMAL;

			int tmp_h = get_text_size(*curr_item, w_ - (x_ == 0 ? float_size.x : x_)).y;

			(*curr_item)["text"] = (*curr_item)["text"].str() + line;

			point text_size = get_text_size(*curr_item, w_ - (x_ == 0 ? float_size.x : x_));
			text_size.x -= x_;

			if (wrap_mode && (float_size.y > 0) && (text_size.y > float_size.y)) {
				DBG_GUI_RL << "wrap start";

				size_t len = get_split_location((*curr_item)["text"].str(), point(w_ - float_size.x, float_size.y));
				DBG_GUI_RL << "wrap around area: " << float_size;

				// first part of the text
				t_string* removed_part = new t_string((*curr_item)["text"].str().substr(len+1));
				(*curr_item)["text"] = (*curr_item)["text"].str().substr(0, len);
				(*curr_item)["maximum_width"] = w_ - float_size.x;
				(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('ww', 0), set_var('pos_y', pos_y + text_height + " + std::to_string(0.3*font::get_max_height(font::SIZE_NORMAL)) + ")])";

				// Height update
				int ah = get_text_size(*curr_item, w_ - float_size.x).y; // no x_?
				if (tmp_h > ah) {
					tmp_h = 0;
				}
				txt_height_ += ah - tmp_h;
				prev_blk_height_ += txt_height_ + 0.3*font::get_max_height(font::SIZE_NORMAL);
				DBG_GUI_RL << "wrap: " << prev_blk_height_ << "," << txt_height_;
				txt_height_ = 0;

				// New text block
				x_ = 0;
				wrap_mode = false;

				// rest of the text
				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);
				tmp_h = get_text_size(*curr_item, w_).y;
				add_text_with_attribute(*curr_item, *removed_part);

			} else if ((float_size.y > 0) && (text_size.y < float_size.y)) {
				// text height less than floating image's height, don't split
				DBG_GUI_RL << "no wrap";
				(*curr_item)["actions"] = "([set_var('pos_y', pos_y + text_height)])";
			}

			if (!wrap_mode) {
				float_size = point(0,0);
			}

			// Incremental height update for text
			int ah = get_text_size(*curr_item, w_ - (x_ == 0 ? float_size.x : x_)).y;
			if (tmp_h > ah) {
				tmp_h = 0;
			}

			txt_height_ += ah - tmp_h;

			is_image = false;
		}

		if (!is_image && !wrap_mode && img_size.y > 0) {
			if (!in_table) {
				img_size = point(0,0);
			}
		}

		if (curr_item) {
			DBG_GUI_RL << "Item:\n" << curr_item->debug();
		}
		DBG_GUI_RL << "X: " << x_;
		DBG_GUI_RL << "Prev block height: " << prev_blk_height_ << " Current text block height: " << txt_height_;
		DBG_GUI_RL << "Height: " << h_;
		h_ = txt_height_ + prev_blk_height_;
		DBG_GUI_RL << "-----------";
	} // for loop ends

	if (static_cast<unsigned>(img_size.y) > h_) {
		h_ = float_size.y;
	}

	// reset all canvas variables to zero, otherwise they grow infinitely
	config& break_cfg = text_dom_.add_child("text");
	default_text_config(&break_cfg);
	break_cfg["text"] = " ";
	break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0), set_var('img_x', 0), set_var('img_y', 0), set_var('ww', 0), set_var('tw', 0)])";

	DBG_GUI_RL << text_dom_.debug();
	DBG_GUI_RL << "Height: " << h_;

	// DEBUG: draw boxes around links
	#if false
	for (const auto& entry : links_) {
		config& link_rect_cfg = text_dom_.add_child("rectangle");
		link_rect_cfg["x"] = entry.first.x;
		link_rect_cfg["y"] = entry.first.y;
		link_rect_cfg["w"] = entry.first.w;
		link_rect_cfg["h"] = entry.first.h;
		link_rect_cfg["border_thickness"] = 1;
		link_rect_cfg["border_color"] = "255, 180, 0, 255";
	}
	#endif
} // function ends

void rich_label::default_text_config(config* txt_ptr, t_string text) {
	if (txt_ptr != nullptr) {
		(*txt_ptr)["text"] = text;
		(*txt_ptr)["font_size"] = font::SIZE_NORMAL;
		(*txt_ptr)["text_alignment"] = encode_text_alignment(get_text_alignment());
		(*txt_ptr)["x"] = "(pos_x)";
		(*txt_ptr)["y"] = "(pos_y)";
		(*txt_ptr)["w"] = "(text_width)";
		(*txt_ptr)["h"] = "(text_height)";
		// tw -> table width, used for wrapping text inside table cols
		// ww -> wrap width, used for wrapping around floating image
		(*txt_ptr)["maximum_width"] = "(width - pos_x - ww - tw)";
		(*txt_ptr)["actions"] = "([set_var('pos_y', pos_y + text_height)])";
	}
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
		tmp.set_variable("padding", wfl::variant(padding_));
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

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	DBG_GUI_RL << "(mouse)" << mouse.x << "," << mouse.y;
	DBG_GUI_RL << "link count :" << links_.size();

	for (const auto& entry : links_) {
		DBG_GUI_RL << "link [" << entry.first.x << "," << entry.first.y << ","
		<< entry.first.x + entry.first.w << "," << entry.first.y + entry.first.h  << "]";

		if (entry.first.contains(mouse)) {
			DBG_GUI_RL << "Clicked link! dst = " << entry.second;
			if (link_handler_) {
				link_handler_(entry.second);
			} else {
				DBG_GUI_RL << "No registered link handler found";
			}

		}
	}

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

	for (const auto& entry : links_) {
		if (entry.first.contains(mouse)) {
			update_mouse_cursor(true);
			handled = true;
			return;
		}
	}

	update_mouse_cursor(false);
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
	, link_color(cfg["link_color"].empty() ? font::YELLOW_COLOR : color_t::from_rgba_string(cfg["link_color"].str()))
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
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
	, link_aware(cfg["link_aware"].to_bool(true))
	, width(cfg["width"].to_int(500))
{
}

std::unique_ptr<widget> builder_rich_label::build() const
{
	auto lbl = std::make_unique<rich_label>(*this);

	const auto conf = lbl->cast_config_to<rich_label_definition>();
	assert(conf);

	lbl->set_text_alignment(text_alignment);
	lbl->set_link_aware(link_aware);
	lbl->set_link_color(conf->link_color);
	lbl->set_width(width);
	lbl->set_label(lbl->get_label());

	DBG_GUI_G << "Window builder: placed rich_label '" << id << "' with definition '"
			  << definition << "'.";

	return lbl;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
