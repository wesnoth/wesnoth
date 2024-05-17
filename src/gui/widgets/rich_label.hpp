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

#pragma once

#include "gui/widgets/styled_widget.hpp"

#include "font/standard_colors.hpp"
#include "gui/core/canvas_private.hpp"
#include "gui/core/widget_definition.hpp"
#include "help/help_impl.hpp"
#include "serialization/parser.hpp"

namespace gui2
{
namespace implementation
{
	struct builder_rich_label;
}

// ------------ WIDGET -----------{

/**
 *
 * A rich_label takes marked up text and shows it correctly formatted and wrapped but no scrollbars are provided.
 */
class rich_label : public styled_widget
{
	friend struct implementation::builder_rich_label;

public:
	explicit rich_label(const implementation::builder_rich_label& builder);

	virtual bool can_wrap() const override
	{
		return can_wrap_ || characters_per_line_ != 0;
	}

	virtual unsigned get_characters_per_line() const override
	{
		return characters_per_line_;
	}

	virtual bool get_link_aware() const override
	{
		return link_aware_;
	}

	virtual color_t get_link_color() const override
	{
		return link_color_;
	}

	virtual void set_active(const bool active) override;

	virtual bool get_active() const override
	{
		return state_ != DISABLED;
	}

	virtual unsigned get_state() const override
	{
		return state_;
	}

	bool disable_click_dismiss() const override
	{
		return false;
	}

	virtual bool can_mouse_focus() const override
	{
		return !tooltip().empty() || get_link_aware();
	}

	virtual void update_canvas() override;

	/* **** ***** ***** setters / getters for members ***** ****** **** */

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

	void set_label(const t_string& text) override;

	void register_link_callback(std::function<void(std::string)> link_handler)
	{
		link_handler_ = link_handler;
	}

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

	/** shapes used for size calculation */
	std::unique_ptr<text_shape> tshape_;
	std::unique_ptr<image_shape> ishape_;

	/** Width and height of the canvas */
	unsigned w_, h_, x_, y_;

	/** Padding */
	unsigned padding_;

	/** Height of current text block */
	unsigned txt_height_;

	/** Height of all previous text blocks, combined */
	unsigned prev_txt_height_;

	/** template for canvas text config */
	void default_text_config(config* txt_ptr, t_string text = "");

	void add_text_with_attribute(config& text_cfg, std::string text, std::string attr_name = "", std::string extra_data = "");
	void add_text_with_attributes(config& text_cfg, std::string text, std::vector<std::string> attr_names, std::vector<std::string> extra_data);

	void start_new_text_block(config* text_cfg, unsigned txt_height_);

	void append_if_not_empty(config_attribute_value* key, std::string suffix) {
		if (!key->str().empty()) {
			*key = key->str() + suffix;
		}
	}

	/** size calculation functions */
	point get_text_size(config text_cfg, unsigned width = 0);
	point get_image_size(config img_cfg);

	wfl::map_formula_callable setup_text_renderer(config text_cfg, unsigned width = 0);

	size_t get_split_location(std::string text, int img_height);

	/** link variables and functions */
	std::vector<std::pair<rect, std::string>> links_;

	std::function<void(std::string)> link_handler_;

	point get_column_line(const point& position) const
	{
		return font::get_text_renderer().get_column_line(position);
	}

	point get_xy_from_offset(const unsigned offset) const
	{
		return font::get_text_renderer().get_cursor_position(offset);
	}

	point calculate_best_size() const
	{
		return point(w_, h_);
	}

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

	/* **** ***** ***** signal handlers ***** ****** **** */

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
