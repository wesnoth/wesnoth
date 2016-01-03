/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_HPP_INCLUDED

#include "gui/auxiliary/formula.hpp"
#include "gui/widgets/grid.hpp"
#include "reference_counted_object.hpp"

#include "utils/boost_function_guarded.hpp"

class config;
class CVideo;

namespace gui2
{

class twindow;

/**
 * Builds a window.
 *
 * @param video                   The frame buffer to draw upon.
 * @param type                    The type id string of the window, this window
 *                                must be registered at startup.
 */
twindow* build(CVideo& video, const std::string& type);

/** Contains the info needed to instantiate a widget. */
struct tbuilder_widget : public reference_counted_object
{
public:
	/**
	 * The replacements type is used to define replacement types.
	 *
	 * Certain widgets need to build a part of themselves upon instantiation
	 * but at the time of the definition it's not yet known what exactly. By
	 * using and `[instance]' widget this decision can be postponed until
	 * instantiation.
	 */
	typedef std::map<std::string, boost::intrusive_ptr<tbuilder_widget> >
	treplacements;

	explicit tbuilder_widget(const config& cfg);

	virtual ~tbuilder_widget()
	{
	}

	virtual twidget* build() const = 0;

	virtual twidget* build(const treplacements& replacements) const = 0;

	/** Parameters for the widget. */
	std::string id;
	std::string linked_group;

#ifndef LOW_MEM
	int debug_border_mode;
	unsigned debug_border_color;
#endif
};

typedef boost::intrusive_ptr<tbuilder_widget> tbuilder_widget_ptr;
typedef boost::intrusive_ptr<const tbuilder_widget> const_tbuilder_widget_ptr;

/**
 * Registers a widget to be build.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @param id                      The id of the widget as used in WML.
 * @param functor                 The functor to create the widget.
 */
void
register_builder_widget(const std::string& id,
						boost::function<tbuilder_widget_ptr(config)> functor);


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
tbuilder_widget_ptr create_builder_widget(const config& cfg);

/**
 * Helper to generate a widget from a WML widget instance.
 *
 * Mainly used as functor for @ref register_builder_widget.
 *
 * @param cfg                     The config with the information to
 *                                Instantiate the widget.
 *
 * @returns                       A generic widget builder pointer.
 */
template <class T>
tbuilder_widget_ptr build_widget(const config& cfg)
{
	return new T(cfg);
}

struct tbuilder_grid : public tbuilder_widget
{
public:
	explicit tbuilder_grid(const config& cfg);

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
	std::vector<tbuilder_widget_ptr> widgets;

	tgrid* build() const;
	twidget* build(const treplacements& replacements) const;


	tgrid* build(tgrid* grid) const;
	void build(tgrid& grid, const treplacements& replacements) const;
};

typedef boost::intrusive_ptr<tbuilder_grid> tbuilder_grid_ptr;
typedef boost::intrusive_ptr<const tbuilder_grid> tbuilder_grid_const_ptr;

class twindow_builder
{
public:
	twindow_builder() : resolutions(), id_(), description_()
	{
	}

	const std::string& read(const config& cfg);

	struct tresolution
	{
	public:
		explicit tresolution(const config& cfg);

		unsigned window_width;
		unsigned window_height;

		bool automatic_placement;

		tformula<unsigned> x;
		tformula<unsigned> y;
		tformula<unsigned> width;
		tformula<unsigned> height;
		tformula<bool> reevaluate_best_size;

		game_logic::function_symbol_table functions;

		unsigned vertical_placement;
		unsigned horizontal_placement;

		unsigned maximum_width;
		unsigned maximum_height;

		bool click_dismiss;

		std::string definition;

		struct tlinked_group
		{
			tlinked_group() : id(), fixed_width(false), fixed_height(false)
			{
			}

			std::string id;
			bool fixed_width;
			bool fixed_height;
		};

		std::vector<tlinked_group> linked_groups;

		/** Helper struct to store information about the tips. */
		struct ttip
		{
			ttip(const config& cfg);

			std::string id;
		};

		ttip tooltip;
		ttip helptip;

		tbuilder_grid_ptr grid;
	};

	std::vector<tresolution> resolutions;

private:
	std::string id_;
	std::string description_;
};

/**
 * Builds a window.
 */
twindow* build(CVideo& video, const twindow_builder::tresolution* res);

} // namespace gui2

#endif
