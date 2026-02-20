/*
	Copyright (C) 2024 - 2025
	by Subhraman Sarkar (babaissarkar) <sbmskmm@protonmail.com>
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

#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include "cursor.hpp"
#include "font/constants.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "log.hpp"
#include "serialization/markup.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#include <boost/format.hpp>
#include <boost/multi_array.hpp>
#include <functional>
#include <numeric>
#include <string>
#include <utility>

static lg::log_domain log_rich_label("gui/widget/rich_label");
#define DBG_GUI_RL LOG_STREAM(debug, log_rich_label)

// Enable this to draw borders around links.
// Useful for debugging misplaced links.
#define LINK_DEBUG_BORDER false

namespace gui2
{
namespace
{
using namespace std::string_literals;

/** Possible formatting tags, must be the same as those in gui2::text_shape::draw */
const std::array format_tags{ "bold"s, "b"s, "italic"s, "i"s, "underline"s, "u"s };
}

// ------------ WIDGET -----------{

REGISTER_WIDGET(rich_label)

rich_label::rich_label(const implementation::builder_rich_label& builder)
	: styled_widget(builder, type())
	, state_(ENABLED)
	, can_wrap_(true)
	, link_aware_(builder.link_aware)
	, link_color_(font::YELLOW_COLOR)
	, predef_colors_()
	, font_size_(font::SIZE_NORMAL)
	, can_shrink_(true)
	, text_alpha_(ALPHA_OPAQUE)
	, init_w_(0)
	, size_(0, 0)
	, padding_(builder.padding)
{
	const auto conf = cast_config_to<rich_label_definition>();
	assert(conf);
	text_color_enabled_ = conf->text_color_enabled;
	text_color_disabled_ = conf->text_color_disabled;
	font_family_ = conf->font_family;
	font_size_ = conf->font_size;
	font_style_ = conf->font_style;
	link_color_ = conf->link_color;
	predef_colors_.insert(conf->colors.begin(), conf->colors.end());
	set_text_alignment(builder.text_alignment);
	set_label(builder.label_string);
}

color_t rich_label::get_color(const std::string& color)
{
	const auto iter = predef_colors_.find(color);
	return (iter != predef_colors_.end()) ? iter->second : font::string_to_color(color);
}

wfl::map_formula_callable rich_label::setup_text_renderer(config text_cfg, unsigned width) const
{
	// Set up fake render to calculate text position
	static wfl::action_function_symbol_table functions;
	wfl::map_formula_callable variables;
	variables.add("text", wfl::variant(text_cfg["text"]));
	variables.add("width", wfl::variant(width));
	variables.add("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
	variables.add("fake_draw", wfl::variant(true));
	gui2::text_shape{text_cfg, functions}.draw(variables);
	return variables;
}

point rich_label::get_text_size(config& text_cfg, unsigned width) const
{
	wfl::map_formula_callable variables = setup_text_renderer(text_cfg, width);
	return {
		variables.query_value("text_width").as_int(),
		variables.query_value("text_height").as_int()
	};
}

point rich_label::get_image_size(config& img_cfg) const
{
	static wfl::action_function_symbol_table functions;
	wfl::map_formula_callable variables;
	variables.add("fake_draw", wfl::variant(true));
	gui2::image_shape{img_cfg, functions}.draw(variables);
	return {
		variables.query_value("image_width").as_int(),
		variables.query_value("image_height").as_int()
	};
}

std::pair<std::size_t, std::size_t> rich_label::add_text(config& curr_item, const std::string& text)
{
	auto& attr = curr_item["text"];
	std::size_t start = attr.str().size();
	attr = attr.str() + text;
	std::size_t end = attr.str().size();
	return { start, end };
}

void rich_label::add_attribute(
	config& curr_item,
	const std::string& attr_name,
	const std::string& extra_data,
	std::size_t start,
	std::size_t end)
{
	if (start == end && start != 0) {
		return;
	}

	config& cfg = curr_item.add_child("attribute");
	cfg["name"] = attr_name;
	// No need to set any keys that's aren't given
	if (start != 0) {
		cfg["start"] = start;
	}
	if (end != 0) {
		cfg["end"] = end;
	}
	if (!extra_data.empty()) {
		cfg["value"] = extra_data;
	}
}

std::pair<std::size_t, std::size_t> rich_label::add_text_with_attribute(
	config& curr_item,
	const std::string& text,
	const std::string& attr_name,
	const std::string& extra_data)
{
	const auto [start, end] = add_text(curr_item, text);
	add_attribute(curr_item, attr_name, extra_data, start, end);
	return { start, end };
}

void rich_label::add_link(
	config& curr_item,
	const std::string& name,
	const std::string& dest,
	const point& origin,
	int img_width)
{
	// TODO algorithm needs to be text_alignment independent

	DBG_GUI_RL << "add_link: " << name << "->" << dest;
	DBG_GUI_RL << "origin: " << origin;
	DBG_GUI_RL << "width: " << img_width;

	point t_start, t_end;

	setup_text_renderer(curr_item, init_w_ - origin.x - img_width);
	t_start = origin + get_xy_from_offset(utf8::size(curr_item["text"].str()));
	DBG_GUI_RL << "link text start:" << t_start;

	std::string link_text = name.empty() ? dest : name;
	add_text_with_attribute(curr_item, link_text, "color", link_color_.to_hex_string());

	setup_text_renderer(curr_item, init_w_ - origin.x - img_width);
	t_end.x = origin.x + get_xy_from_offset(utf8::size(curr_item["text"].str())).x;
	DBG_GUI_RL << "link text end:" << t_end;

	// TODO link after right aligned images

	// Add link
	if(t_end.x > t_start.x) {
		rect link_rect{ t_start, point{t_end.x - t_start.x, font::get_max_height(font_size_) }};
		links_.emplace_back(link_rect, dest);

		DBG_GUI_RL << "added link at rect: " << link_rect;

	} else {
		//link straddles two lines, break into two rects
		int text_height = font::get_max_height(font_size_);

		point t_size(size_.x - t_start.x - (origin.x == 0 ? img_width : 0), t_end.y - t_start.y);
		point link_start2(origin.x, t_start.y + font::get_line_spacing_factor() * text_height);
		point t_size2(t_end.x, t_end.y - t_start.y);

		rect link_rect{ t_start, point{ t_size.x, text_height } };
		rect link_rect2{ link_start2, point{ t_size2.x, text_height } };

		links_.emplace_back(link_rect, dest);
		links_.emplace_back(link_rect2, dest);

		DBG_GUI_RL << "added link at rect 1: " << link_rect;
		DBG_GUI_RL << "added link at rect 2: " << link_rect2;
	}
}

std::size_t rich_label::get_split_location(std::string_view text, const point& pos)
{
	std::size_t len = get_offset_from_xy(pos);
	if (len >= text.size() - 1) {
		return text.size() - 1;
	}

	// break only at word boundary
	char c;
	while(!std::isspace(c = text[len])) {
		len--;
		if(len == 0) {
			break;
		}
	}

	return len;
}

void rich_label::set_dom(const config& dom)
{
	dom_ = dom;
	std::tie(shapes_, size_) = get_parsed_text(dom, point(0,0), init_w_, true);
	update_canvas();
	queue_redraw();
}

void rich_label::set_label(const t_string& text)
{
	set_dom(markup::parse_text(text));
}

void rich_label::request_reduce_width(const unsigned maximum_width)
{
	if(maximum_width != static_cast<unsigned>(size_.x)) {
		init_w_ = maximum_width;
		set_dom(dom_);
	}
	styled_widget::request_reduce_width(maximum_width);
}

point rich_label::calculate_best_size() const
{
	return size_;
}

void rich_label::place(const point& origin, const point& size)
{
	if(size.x != size_.x) {
		init_w_ = size.x;
		set_dom(dom_);
	}
	styled_widget::place(origin, size);
}

std::pair<config, point> rich_label::get_parsed_text(
	const config& parsed_text,
	const point& origin,
	const unsigned init_width,
	const bool finalize)
{
	// Initial width
	DBG_GUI_RL << "Initial width: " << init_width;

	// Initialization
	unsigned x = 0;
	unsigned prev_blk_height = origin.y;
	unsigned text_height = 0;
	unsigned h = 0;
	unsigned w = 0;

	if(finalize) {
		links_.clear();
	}

	config text_dom;
	config* curr_item = nullptr;

	bool is_text = false;
	bool is_image = false;
	bool wrap_mode = false;
	bool new_text_block = false;

	point pos(origin);
	point float_pos, float_size;
	point img_size;

	DBG_GUI_RL << parsed_text.debug();

	for(const auto [orig_key, child] : parsed_text.all_children_view()) {

		const std::string key = (orig_key == "img" && !child["float"].to_bool(false)) ? "inline_image" : orig_key;

		DBG_GUI_RL << "\n Trying to layout tag: " << key;

		if(key == "img") {
			prev_blk_height += text_height;
			text_height = 0;

			const std::string& align = child["align"].str("left");

			curr_item = &(text_dom.add_child("image"));
			(*curr_item)["name"] = child["src"];
			(*curr_item)["x"] = 0;
			(*curr_item)["y"] = 0;
			(*curr_item)["w"] = "(image_width)";
			(*curr_item)["h"] = "(image_height)";

			const point& curr_img_size = get_image_size(*curr_item);

			if (align == "right") {
				float_pos.x = init_width - curr_img_size.x;
			} else if (align == "middle" || align == "center") {
				// works for single image only
				float_pos.x = float_size.x + (init_width - curr_img_size.x)/2;
			}

			if (is_image) {
				float_pos.y += float_size.y;
			}

			(*curr_item)["x"] = float_pos.x;
			(*curr_item)["y"] = pos.y + float_pos.y;

			float_size.x = curr_img_size.x + padding_;
			float_size.y += curr_img_size.y + padding_;

			x = ((align == "left") ? float_size.x : 0);
			pos.x += ((align == "left") ? float_size.x : 0);

			wrap_mode = true;

			w = std::max(w, x);

			is_image = true;
			is_text = false;
			new_text_block = true;

			DBG_GUI_RL << key << ": src=" << child["src"] << ", size=" << img_size;
			DBG_GUI_RL << "wrap turned on.";
		} else if(key == "clear") {
			// Moves the text below the preceding floating image and turns off wrapping
			wrap_mode = false;
			prev_blk_height += float_size.y;
			pos.y += float_size.y;
			float_size = point(0, 0);
			pos.x = origin.x;

			DBG_GUI_RL << key;
			DBG_GUI_RL << "wrap turned off";
		} else if(key == "table") {
			if(curr_item == nullptr) {
				curr_item = &(text_dom.add_child("text"));
				default_text_config(curr_item, pos, init_width);
				new_text_block = false;
			}

			// table doesn't support floating images alongside
			img_size = point(0,0);
			float_size = point(0,0);
			x = origin.x;
			prev_blk_height += text_height + padding_;
			text_height = 0;
			pos = point(origin.x, prev_blk_height + padding_);

			// init table vars
			unsigned col_idx = 0, row_idx = 0;
			unsigned rows = child.child_count("row");
			unsigned columns = 1;
			if(rows > 0) {
				columns = child.mandatory_child("row").child_count("col");
			}
			columns = (columns == 0) ? 1 : columns;
			int init_cell_width;
			if(child["width"] == "fill") {
				init_cell_width = init_width/columns;
			} else {
				init_cell_width = child["width"].to_int(init_width)/columns;
			}
			std::vector<int> col_widths(columns, 0);
			std::vector<int> row_heights(rows, 0);

			is_text = false;
			new_text_block = true;
			is_image = false;

			DBG_GUI_RL << "start table: " << "row=" << rows << " col=" << columns
			           << " width=" << init_cell_width*columns;

			const auto get_padding = [this](const config::attribute_value& val) {
				if(val.blank()) {
					return std::array{ padding_, padding_ };
				} else {
					auto paddings = utils::split(val.str(), ' ');
					if(paddings.size() == 1) {
						return std::array{ std::stoi(paddings[0]), std::stoi(paddings[0]) };
					} else {
						return std::array{ std::stoi(paddings[0]), std::stoi(paddings[1]) };
					}
				}
			};

			std::array<int, 2> row_paddings;
			boost::multi_array<point, 2> cell_sizes(boost::extents[rows][columns]);

			// optimal col width calculation
			for(const config& row : child.child_range("row")) {
				pos.x = origin.x;
				col_idx = 0;

				// order: top padding|bottom padding
				row_paddings = get_padding(row["padding"]);

				pos.y += row_paddings[0];
				for(const config& col : row.child_range("col")) {
					DBG_GUI_RL << "table cell origin (pre-layout): " << pos.x << ", " << pos.y;
					config col_cfg;
					col_cfg.append_children(col);
					config& col_txt_cfg = col_cfg.add_child("text");
					col_txt_cfg.append_attributes(col);

					// order: left padding|right padding
					std::array<int, 2> col_paddings = get_padding(col["padding"]);
					int cell_width = init_cell_width - col_paddings[0] - col_paddings[1];

					pos.x += col_paddings[0];
					// attach data
					auto links = links_;
					cell_sizes[row_idx][col_idx] = get_parsed_text(col_cfg, pos, init_cell_width).second;
					links_ = links;

					// column post-processing
					row_heights[row_idx] = std::max(row_heights[row_idx], cell_sizes[row_idx][col_idx].y);
					if(!child["width"].empty()) {
						col_widths[col_idx] = cell_width;
					}
					col_widths[col_idx] = std::max(col_widths[col_idx], cell_sizes[row_idx][col_idx].x);
					if(child["width"].empty()) {
						col_widths[col_idx] = std::min(col_widths[col_idx], cell_width);
					}

					DBG_GUI_RL << "table row " << row_idx << " height: " << row_heights[row_idx]
					           << "col " << col_idx << " width: " << col_widths[col_idx];

					pos.x += cell_width;
					pos.x += col_paddings[1];
					col_idx++;
				}

				pos.y += row_heights[row_idx] + row_paddings[1];
				row_idx++;
			}

			// table layouting
			row_idx = 0;
			pos = point(origin.x, prev_blk_height);
			for(const config& row : child.child_range("row")) {
				pos.x = origin.x;
				col_idx = 0;

				if(!row["bgcolor"].blank()) {
					config bg_base;
					config& bgbox = bg_base.add_child("rectangle");
					bgbox["x"] = origin.x;
					bgbox["y"] = pos.y;
					bgbox["w"] = std::accumulate(col_widths.begin(), col_widths.end(), 0) + 2*(row_paddings[0] + row_paddings[1])*columns;
					bgbox["h"] = row_paddings[0] + row_heights[row_idx] + row_paddings[1];
					bgbox["fill_color"] = get_color(row["bgcolor"].str()).to_rgba_string();
					text_dom.append(std::move(bg_base));
				}

				row_paddings = get_padding(row["padding"]);
				pos.y += row_paddings[0];

				for(const config& col : row.child_range("col")) {
					DBG_GUI_RL << "table row " << row_idx << " height: " << row_heights[row_idx]
					           << "col " << col_idx << " width: " << col_widths[col_idx];
					DBG_GUI_RL << "cell origin: " << pos;

					config col_cfg;
					col_cfg.append_children(col);
					config& col_txt_cfg = col_cfg.add_child("text");
					col_txt_cfg.append_attributes(col);

					// order: left padding|right padding
					std::array<int, 2> col_paddings = get_padding(col["padding"]);

					pos.x += col_paddings[0];

					const std::string& valign = row["valign"].str("center");
					const std::string& halign = col["halign"].str("left");

					// set position according to alignment keys
					point text_pos(pos);
					if (valign == "center" || valign == "middle") {
						text_pos.y += (row_heights[row_idx] - cell_sizes[row_idx][col_idx].y)/2;
					} else if (valign == "bottom") {
						text_pos.y += row_heights[row_idx] - cell_sizes[row_idx][col_idx].y;
					}
					if (halign == "center" || halign == "middle") {
						text_pos.x += (col_widths[col_idx] - cell_sizes[row_idx][col_idx].x)/2;
					} else if (halign == "right") {
						text_pos.x += col_widths[col_idx] - cell_sizes[row_idx][col_idx].x;
					}

					// attach data
					auto [table_elem, size] = get_parsed_text(col_cfg, text_pos, col_widths[col_idx]);
					text_dom.append(std::move(table_elem));
					pos.x += col_widths[col_idx];
					pos.x += col_paddings[1];

					auto [_, end_cfg] = text_dom.all_children_view().back();
					end_cfg["maximum_width"] = col_widths[col_idx];

					DBG_GUI_RL << "jump to next column";

					if(!is_image) {
						new_text_block = true;
					}
					is_image = false;
					col_idx++;
				}

				pos.y += row_heights[row_idx];
				pos.y += row_paddings[1];
				DBG_GUI_RL << "row height: " << row_heights[row_idx];
				row_idx++;
			}

			w = std::max(w, static_cast<unsigned>(pos.x));
			prev_blk_height = pos.y;
			text_height = 0;
			pos.x = origin.x;

			is_image = false;
			is_text = false;

			x = origin.x;

		} else {
			std::string line = child["text"];

			if (!finalize && (line.empty() && key == "text")) {
				continue;
			}

			if (curr_item == nullptr || new_text_block) {
				curr_item = &(text_dom.add_child("text"));
				default_text_config(curr_item, pos, init_width - pos.x - float_size.x);
				new_text_block = false;
			}

			// }---------- TEXT TAGS -----------{
			int tmp_h = get_text_size(*curr_item, init_width - (x == 0 ? float_size.x : x)).y;

			if(is_text && key == "text") {
				add_text_with_attribute(*curr_item, "\n\n");
			}

			is_text = false;
			is_image = false;

			if (key == "inline_image") {

				// Inline image is rendered as a custom text glyph (pango shape attribute)
				// FIXME: If linebreak (\n) is followed by an inline image
				// the text size is calculated wrongly as being decreased.
				// Workaround: append a zero width space always in front of the image.
				add_text(*curr_item, "\u200b");
				add_text_with_attribute(*curr_item, "\ufffc", "image", child["src"]);

				DBG_GUI_RL << key << ": src=" << child["src"];

			} else if(key == "ref") {

				add_link(*curr_item, line, child["dst"], point(x + origin.x, prev_blk_height), float_size.x);

				DBG_GUI_RL << key << ": dst=" << child["dst"];

			} else if(utils::contains(format_tags, key)) {
				// TODO only the formatting tags here support nesting

				add_text_with_attribute(*curr_item, line, key);

				// Calculate the location of the nested children
				setup_text_renderer(*curr_item, init_w_ - origin.x - float_size.x);
				point child_origin = origin + get_xy_from_offset(utf8::size((*curr_item)["text"].str()));
				child_origin.y += prev_blk_height;

				config parsed_children = get_parsed_text(child, child_origin, init_width).first;

				for(const auto [parsed_key, parsed_cfg] : parsed_children.all_children_view()) {
					if(parsed_key == "text") {
						const auto [start, end] = add_text(*curr_item, parsed_cfg["text"]);
						for (const config& attr : parsed_cfg.child_range("attribute")) {
							add_attribute(*curr_item, attr["name"], attr["value"], start + attr["start"].to_int(), start + attr["end"].to_int());
						}
						add_attribute(*curr_item, key, "", start, end);
					} else {
						text_dom.add_child(parsed_key, parsed_cfg);
					}
				}

				DBG_GUI_RL << key << ": text=" << gui2::debug_truncate(line);

			} else if(key == "header" || key == "h") {

				const auto [start, end] = add_text(*curr_item, line);
				add_attribute(*curr_item, "weight", "heavy", start, end);
				add_attribute(*curr_item, "color", "white", start, end);
				add_attribute(*curr_item, "size", std::to_string(font::SIZE_TITLE - 2), start, end);

				DBG_GUI_RL << key << ": text=" << line;

			} else if(key == "character_entity") {

				line = "&" + child["name"].str() + ";";

				const auto [start, end] = add_text(*curr_item, line);
				add_attribute(*curr_item, "face",  "monospace", start, end);
				add_attribute(*curr_item, "color", "red", start, end);

				DBG_GUI_RL << key << ": text=" << line;

			} else if(key == "span" || key == "format") {

				const auto [start, end] = add_text(*curr_item, line);
				DBG_GUI_RL << "span/format: text=" << line;
				DBG_GUI_RL << "attributes:";

				for (const auto& [key, value] : child.attribute_range()) {
					if (key != "text") {
						add_attribute(*curr_item, key, value, start, end);
						DBG_GUI_RL << key << "=" << value;
					}
				}

			} else if (key == "text") {

				DBG_GUI_RL << "text: text=" << gui2::debug_truncate(line) << "...";

				add_text(*curr_item, line);

				point text_size = get_text_size(*curr_item, init_width - (x == 0 ? float_size.x : x));

				is_text = true;

				// Text wrapping around floating images
				if(wrap_mode && (float_size.y > 0) && (text_size.y > float_size.y)) {
					DBG_GUI_RL << "wrap start";

					const std::string full_text = (*curr_item)["text"].str();

					std::size_t len = get_split_location(full_text, point(init_width - float_size.x, float_size.y * video::get_pixel_scale()));

					DBG_GUI_RL << "wrap around area: " << float_size;

					if(len > 0) {
						// first part of the text
						(*curr_item)["text"] = full_text.substr(0, len);
					}

					(*curr_item)["maximum_width"] = init_width - float_size.x;
					float_size = point(0,0);

					// Height update
					int ah = get_text_size(*curr_item, init_width - float_size.x).y;
					if(tmp_h > ah) {
						tmp_h = 0;
					}
					text_height += ah - tmp_h;
					prev_blk_height += text_height;
					pos = point(origin.x, prev_blk_height);

					DBG_GUI_RL << "wrap: " << prev_blk_height << "," << text_height;
					text_height = 0;

					// New text block
					x = origin.x;
					wrap_mode = false;

					if (len > 0) {
						// layout rest of the text
						curr_item = &(text_dom.add_child("text"));
						default_text_config(curr_item, pos, init_width - pos.x);
						tmp_h = get_text_size(*curr_item, init_width).y;
						// get_split_location always splits at word bounds,
						// so substr(len) will include a space. we skip that.
						add_text_with_attribute(*curr_item, full_text.substr(len+1));
					}
				} else if((float_size.y > 0) && (text_size.y < float_size.y)) {
					//TODO padding?
					// text height less than floating image's height, don't split
					DBG_GUI_RL << "no wrap";
					pos.y += text_size.y;
				}

				if(!wrap_mode) {
					float_size = point(0,0);
				}
			}

			point size = get_text_size(*curr_item, init_width - (x == 0 ? float_size.x : x));
			// update text size and widget height
			if(tmp_h > size.y) {
				tmp_h = 0;
			}
			w = std::max(w, x + static_cast<unsigned>(size.x));

			text_height += size.y - tmp_h;
			pos.y += size.y - tmp_h;
		}

		if(!is_image && !wrap_mode && img_size.y > 0) {
			img_size = point(0,0);
		}

		if(curr_item) {
			DBG_GUI_RL << "Item:\n" << curr_item->debug();
		}
		DBG_GUI_RL << "X: " << x;
		DBG_GUI_RL << "Prev block height: " << prev_blk_height << " Current text block height: " << text_height;
		DBG_GUI_RL << "Height: " << h;
		h = text_height + prev_blk_height;
		DBG_GUI_RL << "-----------";
	} // for loop ends

	if(w == 0) {
		w = init_width;
	}

	// DEBUG: draw boxes around links
	#if LINK_DEBUG_BORDER
	if(finalize) {
		for(const auto& entry : links_) {
			config& link_rect_cfg = text_dom.add_child("rectangle");
			link_rect_cfg["x"] = entry.first.x;
			link_rect_cfg["y"] = entry.first.y;
			link_rect_cfg["w"] = entry.first.w;
			link_rect_cfg["h"] = entry.first.h;
			link_rect_cfg["border_thickness"] = 1;
			link_rect_cfg["border_color"] = "255, 180, 0, 255";
		}
	}
	#endif

	// TODO float and a mix of floats and images and tables
	h = std::max(static_cast<unsigned>(img_size.y), h);

	DBG_GUI_RL << "[\n" << text_dom.debug() << "]\n";

	DBG_GUI_RL << "Width: " << w << " Height: " << h << " Origin: " << origin;
	return { text_dom, point(w, h - origin.y) };
} // function ends

void rich_label::default_text_config(
	config* txt_ptr,
	const point& pos,
	const int max_width,
	const t_string& text)
{
	if(txt_ptr != nullptr) {
		(*txt_ptr)["text"] = text;
		(*txt_ptr)["color"] = text_color_enabled_.to_rgba_string();
		(*txt_ptr)["font_family"] = font_family_;
		(*txt_ptr)["font_size"] = font_size_;
		(*txt_ptr)["font_style"] = font_style_;
		(*txt_ptr)["text_alignment"] = encode_text_alignment(get_text_alignment());
		(*txt_ptr)["line_spacing"] = 0;
		(*txt_ptr)["x"] = pos.x;
		(*txt_ptr)["y"] = pos.y;
		(*txt_ptr)["w"] = "(text_width)";
		(*txt_ptr)["h"] = "(text_height)";
		(*txt_ptr)["maximum_width"] = max_width;
		(*txt_ptr)["parse_text_as_formula"] = false;
		add_attribute(*txt_ptr,
			"line_height",
			std::to_string(font::get_line_spacing_factor()));
	}
}

void rich_label::update_canvas()
{
	for(canvas& tmp : get_canvases()) {
		tmp.set_shapes(shapes_, true);
		tmp.set_variable("width", wfl::variant(init_w_));
		tmp.set_variable("padding", wfl::variant(padding_));
		// Disable ellipsization so that text wrapping can work
		tmp.set_variable("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));
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

void rich_label::register_link_callback(std::function<void(std::string)> link_handler)
{
	if(!link_aware_) {
		return;
	}

	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&rich_label::signal_handler_left_button_click, this, std::placeholders::_3));
	connect_signal<event::MOUSE_MOTION>(
		std::bind(&rich_label::signal_handler_mouse_motion, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&rich_label::signal_handler_mouse_leave, this, std::placeholders::_3));
	link_handler_ = std::move(link_handler);
}


void rich_label::signal_handler_left_button_click(bool& handled)
{
	DBG_GUI_E << "rich_label click";

	if(!get_link_aware()) {
		return; // without marking event as "handled"
	}

	point mouse = get_mouse_position() - get_origin();

	DBG_GUI_RL << "(mouse) " << mouse;
	DBG_GUI_RL << "link count :" << links_.size();

	std::optional<std::string> click_target;
	for(const auto& entry : links_) {
		DBG_GUI_RL << "link " << entry.second;

		if(entry.first.contains(mouse)) {
			click_target = entry.second;
			break;
		}
	}

	if (click_target) {
		DBG_GUI_RL << "Clicked link! dst = " << *click_target;
		sound::play_UI_sound(settings::sound_button_click);
		if(link_handler_) {
			link_handler_(*click_target);
		} else {
			DBG_GUI_RL << "No registered link handler found";
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

	point mouse = coordinate - get_origin();

	for(const auto& entry : links_) {
		if(entry.first.contains(mouse)) {
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
	, text_color_enabled(color_t::from_rgba_string(cfg["text_font_color_enabled"].str()))
	, text_color_disabled(color_t::from_rgba_string(cfg["text_font_color_disabled"].str()))
	, link_color(cfg["link_color"].empty() ? font::YELLOW_COLOR : color_t::from_rgba_string(cfg["link_color"].str()))
	, font_family(cfg["text_font_family"].str())
	, font_size(cfg["text_font_size"].to_int(font::SIZE_NORMAL))
	, font_style(cfg["text_font_style"].str("normal"))
	, colors()
{
	if(auto colors_cfg = cfg.optional_child("colors")) {
		for(const auto& [name, value] : colors_cfg->attribute_range()) {
			colors.try_emplace(name, color_t::from_rgba_string(value.str()));
		}
	}

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
	, padding(cfg["padding"].to_int(5))
{
}

std::unique_ptr<widget> builder_rich_label::build() const
{
	DBG_GUI_G << "Window builder: placed rich_label '" << id << "' with definition '"
			  << definition << "'.";

	return std::make_unique<rich_label>(*this);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
