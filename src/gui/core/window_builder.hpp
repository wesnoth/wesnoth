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

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/linked_group_definition.hpp"
#include "gui/widgets/grid.hpp"
#include "color.hpp"
#include "utils/optional_reference.hpp"



namespace gui2
{

class window;

/** Contains the info needed to instantiate a widget. */
struct builder_widget
{
	/**
	 * The replacements type is used to define replacement types.
	 *
	 * Certain widgets need to build a part of themselves upon instantiation
	 * but at the time of the definition it's not yet known what exactly. By
	 * using and `[instance]' widget this decision can be postponed until
	 * instantiation.
	 */
	using replacements_map = std::map<std::string, std::shared_ptr<builder_widget>>;

	/**
	 * @todo: evaluate whether to combine the two @ref build() functions with this
	 * as the sole parameter.
	 */
	using optional_replacements = utils::optional_reference<const replacements_map>;

	explicit builder_widget(const config& cfg);

	virtual ~builder_widget()
	{
	}

	virtual std::unique_ptr<widget> build() const = 0;

	virtual std::unique_ptr<widget> build(const replacements_map& replacements) const = 0;

	/** Parameters for the widget. */
	std::string id;
	std::string linked_group;

	widget::debug_border debug_border_mode;
	color_t debug_border_color;
};

using builder_widget_ptr = std::shared_ptr<builder_widget>;
using builder_widget_const_ptr = std::shared_ptr<const builder_widget>;

/**
 * Create a widget builder.
 *
 * This object holds the instance builder for a single widget.
 *
 * @param cfg                     The config object holding the information
 *                                regarding the widget instance.
 *
 * @returns                       The builder for the widget instance.
 */
builder_widget_ptr create_widget_builder(const config& cfg);

/**
 * Implementation detail for @ref build_single_widget_instance. Do not use directly!
 *
 * @param type                    String ID of the widget type.
 * @param cfg                     Data config to pass to the widget's builder.
 *
 * @returns                       A shared_ptr of the base widget type containing
 *                                the newly built widget.
 */
std::unique_ptr<widget> build_single_widget_instance_helper(const std::string& type, const config& cfg);

/**
 * Builds a single widget instance of the given type with the specified attributes.
 *
 * This should be used in place of creating a widget object directly, as it
 * allows the widget-specific builder code to be executed.
 *
 * This is equivalent to calling @c build() on the result of @ref create_widget_builder.
 *
 * @tparam T                      The final widget type. The widget pointer will be
 *                                cast to this.
 *
 * @param cfg                     Data config to pass to the widget's builder.
 *
 * @returns                       A shared_ptr of the given type containing the
 *                                newly build widget.
 */
template<typename T>
std::unique_ptr<T> build_single_widget_instance(const config& cfg = {})
{
	static_assert(std::is_base_of_v<widget, T>, "Type is not a widget type");
	return std::unique_ptr<T>{ static_cast<T*>(build_single_widget_instance_helper(T::type(), cfg).release()) };
}

struct builder_grid : public builder_widget
{
	explicit builder_grid(const config& cfg);

	unsigned rows;
	unsigned cols;

	/** The grow factor for the rows / columns. */
	std::vector<unsigned> row_grow_factor;
	std::vector<unsigned> col_grow_factor;

	/** The flags per grid cell. */
	std::vector<unsigned> flags;

	/** The border size per grid cell. */
	std::vector<unsigned> border_size;

	/** The widgets per grid cell. */
	std::vector<builder_widget_ptr> widgets;

	/** Inherited from @ref builder_widget. */
	virtual std::unique_ptr<widget> build() const override;

	/** Inherited from @ref builder_widget. */
	virtual std::unique_ptr<widget> build(const replacements_map& replacements) const override;

	void build(grid& grid, optional_replacements replacements = utils::nullopt) const;
};

using builder_grid_ptr = std::shared_ptr<builder_grid>;
using builder_grid_const_ptr = std::shared_ptr<const builder_grid>;
using builder_grid_map = std::map<std::string, builder_grid_const_ptr>;

class builder_window
{
public:
	explicit builder_window(const config& cfg)
		: resolutions()
		, id_(cfg["id"])
		, description_(cfg["description"])
	{
		read(cfg);
	}

	struct window_resolution
	{
		explicit window_resolution(const config& cfg);

		unsigned window_width;
		unsigned window_height;

		bool automatic_placement;

		typed_formula<unsigned> x;
		typed_formula<unsigned> y;
		typed_formula<unsigned> width;
		typed_formula<unsigned> height;
		typed_formula<bool> reevaluate_best_size;

		wfl::function_symbol_table functions;

		unsigned vertical_placement;
		unsigned horizontal_placement;

		typed_formula<unsigned> maximum_width;
		typed_formula<unsigned> maximum_height;

		bool click_dismiss;

		std::string definition;

		std::vector<linked_group_definition> linked_groups;

		/** Helper struct to store information about the tips. */
		struct tooltip_info
		{
			tooltip_info(const config& cfg, const std::string& tagname);

			std::string id;
		};

		tooltip_info tooltip;
		tooltip_info helptip;

		builder_grid_ptr grid;
	};

	/**
	 * Resolution options for this window instance.
	 *
	 * The window widget handles resolution options differently from other widgets.
	 * Most specify their resolution options in their definitions. However, windows
	 * define different resolution options for each window *instance*. That enables
	 * each dialog to have its own set of options.
	 */
	std::vector<window_resolution> resolutions;

private:
	void read(const config& cfg);

	std::string id_;
	std::string description_;
};

} // namespace gui2
