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
	, padding_(5)
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

void rich_label::add_image(config& curr_item, std::string name, std::string align, bool floating, point& img_size) {
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
	curr_item["y"] = floating ? "(img_y + pos_y)" : "(pos_y)";
	curr_item["h"] = "(image_height)";
	curr_item["w"] = "(image_width)";

	// Sizing
	if (floating) {
		img_size.x = get_image_size(curr_item).x;
		img_size.y += get_image_size(curr_item).y;
	} else {
		img_size.x += get_image_size(curr_item).x + padding_;
		img_size.y = get_image_size(curr_item).y;
	}

	std::stringstream actions;
	actions << "([";
	if (floating) {

		if (align == "left") {
			x_ = img_size.x + padding_;
			actions << "set_var('pos_x', image_width + padding)";
		} else if (align == "right") {
			x_ = 0;
			actions << "set_var('pos_x', 0)";
			actions << ",";
			actions << "set_var('ww', image_width)";
		}

		img_size.y += padding_;
		actions << "," <<  "set_var('img_y', img_y + image_height + padding)";

	} else {
		x_ = img_size.x;
		actions << "set_var('pos_x', pos_x + image_width + padding)";
	}
	actions << "])";

	curr_item["actions"] = actions.str();
	actions.str("");
}

void rich_label::add_link(config& curr_item, std::string name, std::string dest, int img_width) {
	// TODO algorithm needs to be text_alignment independent

	DBG_GUI_RL << "add_link, x=" << x_ << " width=" << img_width;

	setup_text_renderer(curr_item, w_ - x_ - img_width);
	point t_start = get_xy_from_offset(utf8::size(curr_item["text"].str()));

	DBG_GUI_RL << "link text start:" << t_start;

	std::string link_text = name.empty() ? dest : name;
	add_text_with_attribute(curr_item, link_text, "color", link_color_.to_hex_string().substr(1));

	setup_text_renderer(curr_item, w_ - x_ - img_width);
	point t_end = get_xy_from_offset(utf8::size(curr_item["text"].str()));
	DBG_GUI_RL << "link text end:" << t_end;

	point link_start(x_ + t_start.x, prev_blk_height_ + t_start.y);
	t_end.y += font::get_max_height(font::SIZE_NORMAL);

	// TODO link after right aligned images

	// Add link
	if (t_end.x > t_start.x) {
		point link_size = t_end - t_start;
		rect link_rect = {
				link_start.x,
				link_start.y,
				link_size.x,
				link_size.y,
		};
		links_.push_back(std::pair(link_rect, dest));

		DBG_GUI_RL << "added link at rect: " << link_rect;

	} else {
		//link straddles two lines, break into two rects
		point t_size(w_ - link_start.x - (x_ == 0 ? img_width : 0), t_end.y - t_start.y);
		point link_start2(x_, link_start.y + font::get_max_height(font::SIZE_NORMAL));
		point t_size2(t_end.x, t_end.y - t_start.y);

		rect link_rect = {
				link_start.x,
				link_start.y,
				t_size.x,
				t_size.y,
		};

		rect link_rect2 = {
				link_start2.x,
				link_start2.y,
				t_size2.x,
				t_size2.y,
		};

		links_.push_back(std::pair(link_rect, dest));
		links_.push_back(std::pair(link_rect2, dest));

		DBG_GUI_RL << "added link at rect 1: " << link_rect;
		DBG_GUI_RL << "added link at rect 2: " << link_rect2;
	}
}

size_t rich_label::get_split_location(std::string text, int img_height) {
	point wrap_position = get_column_line(point(w_, img_height));

	size_t len = 0;
	for (int i = 0; i < wrap_position.y; i++) {
		len += utf8::size(font::get_text_renderer().get_lines()[i]);
	}
	len += wrap_position.x;

	// break only at word boundary
	char c;
	while((c = text.at(len)) != ' ') {
		len--;
	}

	return len;
}

void rich_label::set_label(const t_string& text)
{
	// Initialization
	w_ = (w_ == 0) ? styled_widget::calculate_best_size().x : w_;
	DBG_GUI_RL << "Width: " << w_;
	h_ = 0;
	unparsed_text_ = text;
	text_dom_.clear();
	links_.clear();
	help::topic_text marked_up_text(text);
	std::vector<std::string> parsed_text = marked_up_text.parsed_text();

	config* curr_item = nullptr;
	optional_config_impl<config> child;

	bool is_image = false;
	bool floating = false;
	bool new_text_block = false;
	bool needs_size_update = true;
	bool in_table = false;
	point img_size;
	unsigned col_width = 0;
	unsigned max_col_height = 0;
	prev_blk_height_ = 0;
	txt_height_ = 0;

	for (size_t i = 0; i < parsed_text.size(); i++) {
		bool last_entry = (i == parsed_text.size() - 1);
		std::string line = parsed_text.at(i);

		if (!line.empty() && line.at(0) == '[') {
			config cfg;
			::read(cfg, line);

			if ((child = cfg.optional_child("img"))) {

				std::string name = child["src"];
				floating = child["float"].to_bool();
				std::string align = child["align"];

				curr_item = &(text_dom_.add_child("image"));
				add_image(*curr_item, name, align, floating, img_size);

				is_image = true;
				new_text_block = true;

				DBG_GUI_RL << "image: src=" << name << ", size=" << get_image_size(*curr_item);

			} else {

				if (is_image && (!floating)) {
					x_ = 0;
					(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + image_height + padding)])";
				}

				if (curr_item == nullptr || new_text_block) {
					prev_blk_height_ += txt_height_ + padding_;
					txt_height_ = 0;

					curr_item = &(text_dom_.add_child("text"));
					default_text_config(curr_item);
					new_text_block = false;
				}

				// }---------- TEXT TAGS -----------{
				int tmp_h = get_text_size(*curr_item, w_ - x_).y;

				if ((child = cfg.optional_child("ref"))) {

					add_link(*curr_item, child["text"], child["dst"], img_size.x);
					is_image = false;

					DBG_GUI_RL << "ref: dst=" << child["dst"];

				} else if ((child = cfg.optional_child("bold")) || (child = cfg.optional_child("b"))) {

					add_text_with_attribute(*curr_item, child["text"], "bold");
					is_image = false;

					DBG_GUI_RL << "bold: text=" << child["text"];

				} else if ((child = cfg.optional_child("italic")) || (child = cfg.optional_child("i"))) {

					add_text_with_attribute(*curr_item, child["text"], "italic");
					is_image = false;

					DBG_GUI_RL << "italic: text=" << child["text"];

				} else if ((child = cfg.optional_child("underline")) || (child = cfg.optional_child("u"))) {

					add_text_with_attribute(*curr_item, child["text"], "underline");
					is_image = false;

					DBG_GUI_RL << "u: text=" << child["text"];

				} else if ((child = cfg.optional_child("header")) || (child = cfg.optional_child("h"))) {

					// Header starts in a new line

					append_if_not_empty(&((*curr_item)["text"]), "\n");
					append_if_not_empty(&((*curr_item)["attr_name"]), ",");
					append_if_not_empty(&((*curr_item)["attr_start"]), ",");
					append_if_not_empty(&((*curr_item)["attr_end"]), ",");
					append_if_not_empty(&((*curr_item)["attr_data"]), ",");

					std::stringstream header_text;
					header_text << child["text"].str() + "\n";
					std::vector<std::string> attrs = {"color", "size"};
					std::vector<std::string> attr_data;
					attr_data.push_back(font::TITLE_COLOR.to_hex_string().substr(1));
					attr_data.push_back(std::to_string(font::SIZE_TITLE));

					add_text_with_attributes((*curr_item), header_text.str(), attrs, attr_data);

					is_image = false;

					DBG_GUI_RL << "h: text=" << child["text"];

				} else if ((child = cfg.optional_child("span")) || (child = cfg.optional_child("format"))) {

					std::vector<std::string> attrs;
					std::vector<std::string> attr_data;

					DBG_GUI_RL << "span/format: text=" << child["text"];
					DBG_GUI_RL << "attributes:";

					for (const auto& attr : child.value().attribute_range()) {
						if (attr.first != "text") {
							attrs.push_back(attr.first);
							attr_data.push_back(attr.second);
							DBG_GUI_RL << attr.first << "=" << attr.second;
						}
					}

					add_text_with_attributes((*curr_item), child["text"], attrs, attr_data);
					is_image = false;

				// }---------- TABLE TAGS -----------{
				} else if ((child = cfg.optional_child("table"))) {

					in_table = true;

					// setup column width
					unsigned columns = child["col"].to_int();
					unsigned width = child["width"].to_int();
					width = width > 0 ? width : w_;
					col_width = width/columns;

					// start on a new line
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + if(ih > text_height, ih, text_height)), set_var('tw', width - pos_x - %d), set_var('ih', 0)])") % col_width);
					x_ = 0;
					prev_blk_height_ += std::max(img_size.y, get_text_size(*curr_item, w_ - img_size.x).y);
					txt_height_ = 0;

					new_text_block = true;

					DBG_GUI_RL << "start table : " << "col=" << columns;
					DBG_GUI_RL << "col_width : " << col_width;

				} else if (cfg.optional_child("jump")) {

					if (col_width > 0) {

						max_col_height = std::max(max_col_height, txt_height_);
						max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
						txt_height_ = 0;
						x_ += col_width;

						DBG_GUI_RL << "jump to next column";

						(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', pos_x + %d), set_var('tw', width - pos_x - %d)])") % col_width % col_width);

						if (!is_image) {
							new_text_block = true;
						}
					}

				} else if (cfg.optional_child("break") || cfg.optional_child("br")) {

					if (in_table) {

						max_col_height = std::max(max_col_height, txt_height_);
						max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));

						//linebreak
						x_ = 0;
						prev_blk_height_ += max_col_height;
						max_col_height = 0;
						txt_height_ = 0;

						(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %d + %d), set_var('tw', width - pos_x - %d)])") % max_col_height % padding_ % col_width);

					}

					DBG_GUI_RL << "linebreak: " << (in_table ? max_col_height : w_);

					if (!is_image) {
						new_text_block = true;
					}

				} else if (cfg.optional_child("endtable")) {

					DBG_GUI_RL << "end table: " << max_col_height;
					max_col_height = std::max(max_col_height, txt_height_);
					max_col_height = std::max(max_col_height, static_cast<unsigned>(img_size.y));
					(*curr_item)["actions"] = boost::str(boost::format("([set_var('pos_x', 0), set_var('pos_y', pos_y + %d), set_var('tw', 0)])") % max_col_height);

					//linebreak and reset col_width
					col_width = 0;
					x_ = 0;
					prev_blk_height_ += max_col_height;
					max_col_height = 0;
					txt_height_ = 0;

					if (!last_entry) {
						new_text_block = true;
					}

					in_table = false;
				}

				if (needs_size_update) {
					int ah = get_text_size(*curr_item, w_ - x_).y;
					// update text size and widget height
					if (tmp_h > ah) {
						tmp_h = 0;
					}

					txt_height_ += ah - tmp_h;
				}
			}

		} else if (!line.empty()) {
			DBG_GUI_RL << "text: text=" << line.substr(1, 20) << "...";

			// Start the text in a new paragraph if a newline follows after an image
			if (is_image && (!floating)) {
					if (line.at(0) == '\n') {
						x_ = 0;
						(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('pos_y', pos_y + image_height + padding)])";
						line = line.substr(1, line.size());
						needs_size_update = true;
					} else {
						needs_size_update = false;
					}
			}

			if (curr_item == nullptr || new_text_block) {
				prev_blk_height_ += txt_height_ + padding_;
				txt_height_ = 0;

				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);
				new_text_block = false;
			}

			(*curr_item)["font_size"] = font::SIZE_NORMAL;

			int tmp_h = get_text_size(*curr_item, w_ - x_).y;

			(*curr_item)["text"] = (*curr_item)["text"].str() + line;

			point text_size;
			text_size.x = get_text_size(*curr_item, w_ - (x_ == 0 ? img_size.x : x_)).x - x_;
			text_size.y = get_text_size(*curr_item, w_ - (x_ == 0 ? img_size.x : x_)).y;

			if ( floating && (img_size.y > 0) && (text_size.y > img_size.y) ) {
				DBG_GUI_RL << "wrap start";

				size_t len = get_split_location((*curr_item)["text"].str(), img_size.y);
				t_string* removed_part = new t_string((*curr_item)["text"].str().substr(len+1));
				(*curr_item)["text"] = (*curr_item)["text"].str().substr(0, len);
				(*curr_item)["actions"] = "([set_var('pos_x', 0), set_var('ww', 0), set_var('pos_y', pos_y + text_height)])";

				// New text block
				x_ = 0;
				prev_blk_height_ += img_size.y + padding_;
				// TODO excess line gets added, so that needs to be compensated
				txt_height_ = 0;
				img_size = point(0,0);
				floating = false;

				curr_item = &(text_dom_.add_child("text"));
				default_text_config(curr_item);

				add_text_with_attribute(*curr_item, *removed_part);

			} else if ((img_size.y > 0) && (text_size.y < img_size.y)) {
				DBG_GUI_RL << "no wrap";
				if (is_image) {
					(*curr_item)["actions"] = "([set_var('pos_y', pos_y + image_height)])";
				} else {
					(*curr_item)["actions"] = "([set_var('pos_y', pos_y + text_height)])";
				}
			}

			int ah = get_text_size(*curr_item, w_ - x_).y;
			// update text size and widget height
			if (tmp_h > ah) {
				tmp_h = 0;
			}

			txt_height_ += ah - tmp_h;

			is_image = false;
		}

		// Height Update
		if (!is_image && !floating && img_size.y > 0) {
			if (needs_size_update) {
				prev_blk_height_ += img_size.y;
			}
			img_size = point(0,0);
		}


		DBG_GUI_RL << "Item :" << curr_item->debug();
		DBG_GUI_RL << "X: " << x_;
		DBG_GUI_RL << "Prev block height: " << prev_blk_height_ << " Current text block height: " << txt_height_;
		DBG_GUI_RL << "Height: " << h_;
		h_ = txt_height_ + prev_blk_height_;

		// reset all variables to zero, otherwise they grow infinitely
		if (last_entry) {
			if (static_cast<unsigned>(img_size.y) > h_) {
				h_ = img_size.y;
			}
			h_ += font::get_line_spacing_factor() * font::get_max_height(font::SIZE_NORMAL);

			config& break_cfg = text_dom_.add_child("text");
			default_text_config(&break_cfg);
			break_cfg["text"] = " ";
			break_cfg["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0), set_var('img_x', 0), set_var('img_y', 0)])";
		}

		DBG_GUI_RL << "-----------";

	} // for loop ends

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
