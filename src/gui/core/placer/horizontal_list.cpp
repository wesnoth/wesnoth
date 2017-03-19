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

#include "gui/core/placer/horizontal_list.hpp"

#include "gui/core/point.hpp"

#include <cassert>
#include <numeric>

namespace gui2
{

namespace implementation
{

placer_horizontal_list::placer_horizontal_list(const unsigned maximum_rows)
	: maximum_rows_(maximum_rows)
	, rows_(maximum_rows, 0)
	, columns_(1, std::make_pair(0, 0))
	, row_(0)
	, column_(0)
{
	assert(maximum_rows_ > 0);
}

void placer_horizontal_list::initialise()
{
	std::fill(rows_.begin(), rows_.end(), 0);
	columns_.clear();
	columns_.push_back(std::make_pair(0, 0));
	row_ = 0;
	column_ = 0;
}

void placer_horizontal_list::add_item(const point& size)
{
	if(size.x > columns_[column_].second) {
		columns_[column_].second = size.x;
	}

	if(size.y > rows_[row_]) {
		rows_[row_] = size.y;
	}

	++row_;
	if(row_ == maximum_rows_) {
		row_ = 0;
		++column_;

		const int origin = columns_.back().first + columns_.back().second;
		columns_.push_back(std::make_pair(origin, 0));
	}
}

point placer_horizontal_list::get_size() const
{
	const int width = columns_.back().first + columns_.back().second;
	const int height = std::accumulate(rows_.begin(), rows_.end(), 0);
	return point(width, height);
}

point placer_horizontal_list::get_origin(const unsigned index) const
{
	const unsigned row = index % maximum_rows_;
	const unsigned column = index / maximum_rows_;

	const int height
			= row == 0 ? 0
					   : std::accumulate(rows_.begin(), rows_.begin() + row, 0);

	return point(columns_[column].first, height);
}

} // namespace implementation

} // namespace gui2
