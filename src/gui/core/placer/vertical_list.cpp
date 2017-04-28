/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/placer/vertical_list.hpp"

#include "gui/core/point.hpp"

#include <cassert>
#include <numeric>

namespace gui2
{

namespace implementation
{

placer_vertical_list::placer_vertical_list(const unsigned maximum_columns)
	: maximum_columns_(maximum_columns)
	, rows_(1, std::make_pair(0, 0))
	, columns_(maximum_columns, 0)
	, row_(0)
	, column_(0)
{
	assert(maximum_columns_ > 0);
}

void placer_vertical_list::initialize()
{
	rows_.clear();
	rows_.emplace_back(0, 0);
	std::fill(columns_.begin(), columns_.end(), 0);
	row_ = 0;
	column_ = 0;
}

void placer_vertical_list::add_item(const point& size)
{
	if(size.x > columns_[column_]) {
		columns_[column_] = size.x;
	}

	if(size.y > rows_[row_].second) {
		rows_[row_].second = size.y;
	}

	++column_;
	if(column_ == maximum_columns_) {
		column_ = 0;
		++row_;

		const int origin = rows_.back().first + rows_.back().second;
		rows_.emplace_back(origin, 0);
	}
}

point placer_vertical_list::get_size() const
{
	const int width = std::accumulate(columns_.begin(), columns_.end(), 0);
	const int height = rows_.back().first + rows_.back().second;
	return point(width, height);
}

point placer_vertical_list::get_origin(const unsigned index) const
{
	const unsigned row = index / maximum_columns_;
	const unsigned column = index % maximum_columns_;

	const int width = column == 0 ? 0
								  : std::accumulate(columns_.begin(),
													columns_.begin() + column,
													0);

	return point(width, rows_[row].first);
}

} // namespace implementation

} // namespace gui2
