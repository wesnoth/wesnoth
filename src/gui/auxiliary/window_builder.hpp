/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_HPP_INCLUDED

#include "gui/auxiliary/formula.hpp"
#include "reference_counted_object.hpp"

#include <string>
#include <vector>

class config;
class CVideo;

namespace gui2 {

class twidget;
class tgrid;
class twindow;

twindow* build(CVideo& video, const std::string& type);

/** Contains the info needed to instantiate a widget. */
struct tbuilder_widget : public reference_counted_object
{
private:
	tbuilder_widget();

public:
	tbuilder_widget(const config& /*cfg*/) {}


	virtual twidget* build() const = 0;
	virtual ~tbuilder_widget() {}
};

typedef boost::intrusive_ptr<tbuilder_widget> tbuilder_widget_ptr;
typedef boost::intrusive_ptr<const tbuilder_widget> const_tbuilder_widget_ptr;

struct tbuilder_grid : public tbuilder_widget
{
private:
	tbuilder_grid();

public:
	tbuilder_grid(const config& cfg);

	std::string id;
	std::string linked_group;
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

	twidget* build() const;
	twidget* build(tgrid* grid) const;
};

typedef boost::intrusive_ptr<tbuilder_grid> tbuilder_grid_ptr;
typedef boost::intrusive_ptr<const tbuilder_grid> tbuilder_grid_const_ptr;

class twindow_builder
{
public:
	const std::string& read(const config& cfg);

	struct tresolution
	{
	private:
		tresolution();

	public:
		tresolution(const config& cfg);

		unsigned window_width;
		unsigned window_height;

		bool automatic_placement;

		tformula<unsigned> x;
		tformula<unsigned> y;
		tformula<unsigned> width;
		tformula<unsigned> height;

		unsigned vertical_placement;
		unsigned horizontal_placement;

		unsigned maximum_width;
		unsigned maximum_height;

		bool click_dismiss;

		std::string definition;

		struct tlinked_group
		{
			tlinked_group()
				: id()
				, fixed_width(false)
				, fixed_height(false)
			{
			}

			std::string id;
			bool fixed_width;
			bool fixed_height;
		};

		std::vector<tlinked_group> linked_groups;

		tbuilder_grid_ptr grid;
	};

	std::vector<tresolution> resolutions;

private:
	std::string id_;
	std::string description_;

};

} // namespace gui2


#endif



