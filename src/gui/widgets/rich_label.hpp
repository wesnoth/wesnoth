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

#pragma once

#include "color.hpp"
#include "gui/widgets/styled_widget.hpp"

#include "gui/core/canvas_private.hpp"
#include "gui/core/widget_definition.hpp"

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

	virtual void request_reduce_height(const unsigned /*maximum_height*/) override {};

	virtual void update_canvas() override;

	/* **** ***** ***** setters / getters for members ***** ****** **** */

	void set_can_wrap(const bool wrap)
	{
		can_wrap_ = wrap;
	}

	void set_link_aware(bool l);

	void set_link_color(const color_t& color);

	void set_can_shrink(bool can_shrink)
	{
		can_shrink_ = can_shrink;
	}

	void set_font_family(const std::string& font_family)
	{
		font_family_ = font_family;
	}

	void set_font_size(int font_size)
	{
		font_size_ = font_size;
	}

	void set_font_style(const std::string& font_style)
	{
		font_style_ = font_style;
	}

	void set_text_alpha(unsigned short alpha);

	// Show text marked up with help markup
	void set_label(const t_string& text) override;

	// Show a given DOM (given as a config)
	void set_dom(const config& dom);

	int get_width()
	{
		return init_w_;
	}

	// NOTE: set_label() must be done after this so the text can be
	// laid out with the new width.
	void set_width(const int width)
	{
		init_w_ = width;
	}

	// Given a parsed config from help markup,
	// layout it into a config that can be understood by canvas
	std::pair<config, point> get_parsed_text(
		const config& parsed_text,
		const point& origin,
		const unsigned init_width,
		const bool finalize = false);

	// Attaches a callback function that will be called when a link is clicked
	void register_link_callback(std::function<void(std::string)> link_handler);

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
	 * Base text color, enabled state
	 */
	color_t text_color_enabled_;

	/**
	 * Base text color, disabled state
	 */
	color_t text_color_disabled_;

	/**
	 * What color links will be rendered in.
	 */
	color_t link_color_;

	/**
	 * Color variables that can be used in place of colors strings, like `<row bgcolor=color1>`
	 */
	std::map<std::string, color_t> predef_colors_;

	/**
	 * Base font family
	 */
	std::string font_family_;

	/**
	 * Base font size
	 */
	int font_size_;

	/**
	 * Base font style
	 */
	std::string font_style_;

	bool can_shrink_;

	unsigned short text_alpha_;

	/** Inherited from styled_widget. */
	virtual bool text_can_shrink() override
	{
		return can_shrink_;
	}

	/** Final list of shapes to be drawn on the canvas. */
	config shapes_;

	/** Width and height of the canvas */
	unsigned init_w_;
	point size_;

	config dom_;

	/** Padding */
	int padding_;

	/** If color is a predefined color set in resolution, return it, otherwise decode using `font::string_to_color`. */
	color_t get_color(const std::string& color);

	/** Create template for text config that can be shown in canvas */
	void default_text_config(config* txt_ptr, const point& pos, const int max_width, const t_string& text = "");

	std::pair<std::size_t, std::size_t> add_text(config& curr_item, const std::string& text);
	void add_attribute(config& curr_item, const std::string& attr_name, const std::string& extra_data = "", std::size_t start = 0, std::size_t end = 0);
	std::pair<std::size_t, std::size_t> add_text_with_attribute(config& curr_item, const std::string& text, const std::string& attr_name = "", const std::string& extra_data = "");

	void add_link(config& curr_item, const std::string& name, const std::string& dest, const point& origin, int img_width);

	/** size calculation functions */
	point get_text_size(config& text_cfg, unsigned width = 0) const;
	point get_image_size(config& img_cfg) const;

	wfl::map_formula_callable setup_text_renderer(config text_cfg, unsigned width = 0) const;

	std::size_t get_split_location(std::string_view text, const point& pos);

	/** link variables and functions */
	std::vector<std::pair<rect, std::string>> links_;

	std::function<void(std::string)> link_handler_;

	int get_offset_from_xy(const point& position) const
	{
		return std::get<0>(font::get_text_renderer().xy_to_index(position));
	}

	point get_xy_from_offset(const unsigned offset) const
	{
		return font::get_text_renderer().get_cursor_position(offset);
	}

	point calculate_best_size() const override;

	void request_reduce_width(const unsigned maximum_width) override;

	void place(const point& origin, const point& size) override;

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

		color_t text_color_enabled, text_color_disabled;
		color_t link_color;
		std::string font_family;
		int font_size;
		std::string font_style;
		std::map<std::string, color_t> colors;
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

	PangoAlignment text_alignment;
	bool link_aware;
	unsigned padding;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
