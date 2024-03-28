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

#pragma once

#include "gui/widgets/styled_widget.hpp"

#include "font/standard_colors.hpp"
#include "gui/core/widget_definition.hpp"
#include "help/help_impl.hpp"
#include "serialization/parser.hpp"

#include <iostream>
#include <string>

namespace gui2
{
namespace implementation
{
	struct builder_rich_label;
}

// ------------ WIDGET -----------{

/**
 * @ingroup GUIWidgetWML
 *
 * A rich_label displays text that can be wrapped but no scrollbars are provided.
 *
 * A rich_label has two states because rich_labels are often used as visual indication of the state of the widget it rich_labels.
 *
 * The following states exist:
 * * state_enabled - the rich_label is enabled.
 * * state_disabled - the rich_label is disabled.
 *
 * Key                |Type                                |Default |Description
 * -------------------|------------------------------------|--------|-------------
 * link_color         | @ref guivartype_string "string"    |\#ffff00|The color to render links with. This string will be used verbatim in pango markup for each link.
 *
 * The rich_label specific variables:
 * Key                |Type                                |Default|Description
 * -------------------|------------------------------------|-------|-------------
 * wrap               | @ref guivartype_bool "bool"        |false  |Is wrapping enabled for the rich_label.
 * characters_per_line| @ref guivartype_unsigned "unsigned"|0      |Sets the maximum number of characters per line. The amount is an approximate since the width of a character differs. E.g. iii is smaller than MMM. When the value is non-zero it also implies can_wrap is true. When having long strings wrapping them can increase readability, often 66 characters per line is considered the optimum for a one column text.
 * text_alignment     | @ref guivartype_h_align "h_align"  |left   |The way the text is aligned inside the canvas.
 * can_shrink         | @ref guivartype_bool "bool"        |false  |Whether the rich_label can shrink past its optimal size.
 * link_aware         | @ref guivartype_bool "bool"        |false  |Whether the rich_label is link aware. This means it is rendered with links highlighted, and responds to click events on those links.
 */
class rich_label : public styled_widget
{
	friend struct implementation::builder_rich_label;

public:
	explicit rich_label(const implementation::builder_rich_label& builder);

	/** See @ref widget::can_wrap. */
	virtual bool can_wrap() const override
	{
		return can_wrap_ || characters_per_line_ != 0;
	}

	/** See @ref styled_widget::get_characters_per_line. */
	virtual unsigned get_characters_per_line() const override
	{
		return characters_per_line_;
	}

	/** See @ref styled_widget::get_link_aware. */
	virtual bool get_link_aware() const override
	{
		return link_aware_;
	}

	/** See @ref styled_widget::get_link_aware. */
	virtual color_t get_link_color() const override
	{
		return link_color_;
	}

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override
	{
		return state_ != DISABLED;
	}

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override
	{
		return state_;
	}

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override
	{
		return false;
	}

	/** See @ref widget::can_mouse_focus. */
	virtual bool can_mouse_focus() const override
	{
		return !tooltip().empty() || get_link_aware();
	}

	/** See @ref styled_widget::update_canvas. */
	virtual void update_canvas() override;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_can_wrap(const bool wrap)
	{
		can_wrap_ = wrap;
	}

	void set_characters_per_line(const unsigned characters_per_line)
	{
		characters_per_line_ = characters_per_line;
	}

	void set_link_aware(bool l);

	void set_link_color(const color_t& color);

	void set_can_shrink(bool can_shrink)
	{
		can_shrink_ = can_shrink;
	}

	void set_text_alpha(unsigned short alpha);

	const t_string& get_label() const
	{
		return unparsed_text_.empty() ? styled_widget::get_label() : unparsed_text_;
	}

	void set_label(const t_string& text) override
	{
		unparsed_text_ = text;
		text_dom_.clear();
//		styled_widget::set_label(text);
		help::topic_text marked_up_text(text);
		std::vector<std::string> parsed_text =  marked_up_text.parsed_text();
		for (size_t i = 0; i < parsed_text.size(); i++) {
			bool last_entry = (i == parsed_text.size() - 1);
			std::string line = parsed_text.at(i);
			std::cout << "line :" << i << ", " << last_entry << std::endl;
			if (!line.empty() && line.at(0) == '[') {
//				std::cout << "wml" << std::endl;
				config cfg;
				std::istringstream stream(line);
				::read(cfg, line);

//				std::cout << cfg.debug() << std::endl;

				if (cfg.optional_child("ref")) {
					config& txt_r = default_text_config(cfg.mandatory_child("ref")["text"], last_entry);
					txt_r["attr_name"] = "fgcolor";
					txt_r["attr_start"] = "0";
					txt_r["attr_end"] = txt_r["text"].str().size();
					std::cout << "size : " << txt_r["text"].str().size() << std::endl;
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
					img["x"] = "(debug_print('[img] pos_x', pos_x))";
					img["y"] = "(pos_y)";
					img["h"] = "(image_height)";
					img["w"] = "(image_width)";

					if (last_entry) {
						img["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
					} else {
						img["actions"] = "([set_var('pos_x', pos_x+image_width)])";
					}
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

//	point calculate_best_size() const
//	{
//		return point(w_, h_);
//	}

//	void place(const point& origin, const point& size) {
//		styled_widget::place(origin, size);
//	}

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
	};

	void set_state(const state_t state);

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	/** Holds the rich_label can wrap or not. */
	bool can_wrap_;

	/**
	 * The maximum number of characters per line.
	 *
	 * The maximum is not an exact maximum, it uses the average character width.
	 */
	unsigned characters_per_line_;

	/**
	 * Whether the rich_label is link aware, rendering links with special formatting
	 * and handling click events.
	 */
	bool link_aware_;

	/**
	 * What color links will be rendered in.
	 */
	color_t link_color_;

	bool can_shrink_;

	unsigned short text_alpha_;

	/** Inherited from styled_widget. */
	virtual bool text_can_shrink() override
	{
		return can_shrink_;
	}

	/** structure tree of the marked up text after parsing */
	config text_dom_;

	/** The unparsed/raw text */
	t_string unparsed_text_;

	/** Width and height of the canvas */
	unsigned w_, h_;

	/** template for canvas text config */
	config& default_text_config(t_string text, bool last_entry = false) {
		config& txt = text_dom_.add_child("text");
		txt["text"] = text;
		txt["font_size"] = 20;
		txt["color"] = "([186, 172, 125, 255])";
		txt["x"] = "(debug_print('[txt] pos_x', pos_x))";
		txt["y"] = "(pos_y)";
		txt["w"] = "(text_width)";
		txt["h"] = "(text_height)";
//		txt["actions"] = "([set_var('pos_x', pos_x+text_width), set_var('pos_y', pos_y+text_height)])";
		if (last_entry) {
			txt["actions"] = "([set_var('pos_x', 0), set_var('pos_y', 0)])";
		} else {
			txt["actions"] = "([set_var('pos_x', pos_x+text_width)])";
		}
		return txt;
	}

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/***** ***** ***** signal handlers ***** ****** *****/

	/**
	 * Left click signal handler: checks if we clicked on a hyperlink
	 */
	void signal_handler_left_button_click(bool& handled);

	/**
	 * Right click signal handler: checks if we clicked on a hyperlink, copied to clipboard
	 */
	void signal_handler_right_button_click(bool& handled);

	/**
	 * Mouse motion signal handler: checks if the cursor is on a hyperlink
	 */
	void signal_handler_mouse_motion(bool& handled, const point& coordinate);

	/**
	 * Mouse leave signal handler: checks if the cursor left a hyperlink
	 */
	void signal_handler_mouse_leave(bool& handled);

	/**
	 * Implementation detail for (re)setting the hyperlink cursor.
	 */
	void update_mouse_cursor(bool enable);
};

// }---------- DEFINITION ---------{

struct rich_label_definition : public styled_widget_definition
{

	explicit rich_label_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		color_t link_color;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_rich_label : public builder_styled_widget
{
	builder_rich_label(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	bool wrap;

	unsigned characters_per_line;

	PangoAlignment text_alignment;

	bool can_shrink;
	bool link_aware;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
