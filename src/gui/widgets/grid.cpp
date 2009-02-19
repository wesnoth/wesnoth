/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/grid.hpp"

#include "foreach.hpp"

#include <numeric>

namespace gui2 {

tgrid::tgrid(const unsigned rows, const unsigned cols)
	: rows_(rows)
	, cols_(cols)
	, row_height_()
	, col_width_()
	, row_grow_factor_(rows)
	, col_grow_factor_(cols)
	, children_(rows * cols)
{
}

tgrid::~tgrid()
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->widget()) {
			delete itor->widget();
		}
	}
}

unsigned tgrid::add_row(const unsigned count)
{
	assert(count);

	//FIXME the warning in set_rows_cols should be killed.

	unsigned result = rows_;
	set_rows_cols(rows_ + count, cols_);
	return result;
}

void tgrid::set_child(twidget* widget, const unsigned row,
		const unsigned col, const unsigned flags, const unsigned border_size)
{
	assert(row < rows_ && col < cols_);
	assert(flags & VERTICAL_MASK);
	assert(flags & HORIZONTAL_MASK);

	tchild& cell = child(row, col);

	// clear old child if any
	if(cell.widget()) {
		// free a child when overwriting it
		WRN_GUI << "Grid: child '" << cell.id()
			<< "' at cell '" << row << ',' << col << "' will be replaced.\n";
		delete cell.widget();
	}

	// copy data
	cell.set_flags(flags);
	cell.set_border_size(border_size);
	cell.set_widget(widget);
	if(cell.widget()) {
		// make sure the new child is valid before deferring
		cell.widget()->set_parent(this);

		// Init the easy close state here, normally when put in a grid the grid
		// does have a parent window.
		tcontrol* control = dynamic_cast<tcontrol*>(cell.widget());
		if(control) {
			control->set_block_easy_close(
					control->get_visible()
					&& control->get_active()
					&& control->does_block_easy_close());
		}
	}
}

twidget* tgrid::swap_child(
		const std::string& id, twidget* widget, const bool recurse,
		twidget* new_parent)
{
	assert(widget);

	foreach(tchild& child, children_) {
		if(child.id() != id) {

			if(recurse) {
				// decent in the nested grids.
				tgrid* grid = dynamic_cast<tgrid*>(child.widget());
				if(grid) {

					twidget* old = grid->swap_child(id, widget, true);
					if(old) {
						return old;
					}
				}
			}

			continue;
		}

		// When find the widget there should be a widget.
		twidget* old = child.widget();
		assert(old);
		old->set_parent(new_parent);

		widget->set_parent(this);
		child.set_widget(widget);

		return old;
	}

	return NULL;
}

void tgrid::remove_child(const unsigned row, const unsigned col)
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	if(cell.widget()) {
		delete cell.widget();
	}
	cell.set_widget(0);
}

void tgrid::remove_child(const std::string& id, const bool find_all)
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->id() == id) {
			if(itor->widget()) {
				delete itor->widget();
			}
			itor->set_widget(0);

			if(!find_all) {
				break;
			}
		}
	}
}

void tgrid::set_active(const bool active)
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		twidget* widget = itor->widget();
		if(!widget) {
			continue;
		}

		tgrid* grid = dynamic_cast<tgrid*>(widget);
		if(grid) {
			grid->set_active(active);
			continue;
		}

		tcontrol* control =  dynamic_cast<tcontrol*>(widget);
		if(control) {
			control->set_active(active);
		}
	}
}

void tgrid::layout_init()
{
	// Inherited.
	twidget::layout_init();

	// Clear child caches.
	foreach(tchild& child, children_) {

		twidget* widget = child.widget();
		if(widget) {
			widget->layout_init();
		}
	}
}

tpoint tgrid::calculate_best_size() const
{
	log_scope2(gui_layout, std::string("tgrid ") + __func__);

	// Reset the cached values.
	row_height_.clear();
	row_height_.resize(rows_, 0);
	col_width_.clear();
	col_width_.resize(cols_, 0);

	// First get the sizes for all items.
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

			const tpoint size = child(row, col).get_best_size();

			if(size.x > static_cast<int>(col_width_[col])) {
				col_width_[col] = size.x;
			}

			if(size.y > static_cast<int>(row_height_[row])) {
				row_height_[row] = size.y;
			}

		}
	}

	for(unsigned row = 0; row < rows_; ++row) {
		DBG_G_L << "tgrid: the row_height_ for row " << row
			<< " will be " << row_height_[row] << ".\n";
	}

	for(unsigned col = 0; col < cols_; ++col) {
		DBG_G_L << "tgrid: the col_width_ for col " << col
			<< " will be " << col_width_[col]  << ".\n";
	}

	const tpoint result(
		std::accumulate(col_width_.begin(), col_width_.end(), 0),
		std::accumulate(row_height_.begin(), row_height_.end(), 0));

	DBG_G_L << "tgrid: returning " << result << ".\n";
	return result;
}

bool tgrid::can_wrap() const
{
	foreach(const tchild& child, children_) {
		if(child.can_wrap()) {
			return true;
		}
	}

	// Inherited.
	return twidget::can_wrap();
}

void tgrid::layout_wrap(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_wrap(maximum_width);

	/*
	 * 1. Test the width of (every) row.
	 * 2. Row wider as wanted?
	 *    - No goto 3
	 *    - Yes
	 *      2.1 Test every column in the row.
	 *      2.2 Can be resized?
	 *      - No goto 2.1.
	 *      - Yes can be sized small enough?
	 *        - No FAILURE.
	 *        - Yes goto 3.
	 *      2.3 Last column?
	 *        - No goto 2.1
	 *        - Yes FAILURE.
	 * 3. Last row?
	 *    - No goto 1.
	 *    - Yes SUCCESS.
	 */

	log_scope2(gui_layout, std::string("tgrid ") + __func__);
	DBG_G_L << "tgrid:  maximum_width " << maximum_width << ".\n";

	std::vector<int> widths(cols_);
	for(unsigned row = 0; row < rows_; ++row) {

		for(unsigned col = 0; col < cols_; ++col) {

			widths[col] = (child(row, col).get_best_size()).x;
		}

		if(std::accumulate(widths.begin(), widths.end(), 0) >
				static_cast<int>(maximum_width)) {

			DBG_G_L << "tgrid: row " << row << " out of bounds needs "
				<< std::accumulate(widths.begin(), widths.end(), 0)
				<< " available " << maximum_width
				<< ", try to resize.\n";
			log_scope2(gui_layout, "tgrid: testing all columns");

			int width = 0;
			for(unsigned col = 0; col < cols_; ++col) {

				log_scope2(gui_layout, "tgrid: column "
					+ lexical_cast<std::string>(col));

				tchild& chld = child(row, col);

				if(!chld.can_wrap()) {
					DBG_G_L << "tgrid: column can't wrap, skip.\n";
					continue;
				}

				if(widths[col] == 0) {
					DBG_G_L << "tgrid: column has zero width, skip.\n";
				}

				width = widths[col];
				widths[col] = 0;

				const int widget_width = maximum_width
					- std::accumulate(widths.begin(), widths.end(), 0);

				if(widget_width <=0) {

					DBG_G_L << "tgrid: column is too small to resize, skip.\n";
					widths[col] = width;
					width = 0;
					continue;
				}

				chld.layout_wrap(widget_width);

				if(chld.get_best_size().x <= widget_width) {

					DBG_G_L << "tgrid: column resize succeeded.\n";
					break;
				}

				DBG_G_L << "tgrid: column resize failed.\n";
				widths[col] = width;
				width = 0;
			}

			if(width == 0) {
				DBG_G_L << "tgrid: no solution found.\n";
				return;
			}
		} else {
			DBG_G_L << "tgrid: row " << row << " in bounds.\n";
		}
	}

	DBG_G_L << "tgrid: found solution.\n";
}

bool tgrid::has_vertical_scrollbar() const
{
	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {
		// FIXME we should check per row and the entire row
		// should have the flag!!!!
		if(itor->widget() && itor->widget()->has_vertical_scrollbar()) {
			return true;
		}
	}

	// Inherit
	return twidget::has_vertical_scrollbar();
}

void tgrid::layout_use_vertical_scrollbar(const unsigned maximum_height)
{
	// Inherited.
	twidget::layout_use_vertical_scrollbar(maximum_height);

	log_scope2(gui_layout, std::string("tgrid ") + __func__);
	DBG_G_L << "tgrid: maximum height " << maximum_height << ".\n";

	tpoint size = get_best_size();

	// If we honoured the size or can't resize return the result.
	if(size.y <= static_cast<int>(maximum_height) || !has_vertical_scrollbar()) {
		DBG_G_L << "tgrid: maximum height "
			<< maximum_height << " returning " << size << ".\n";
		return;
	}

	// Try to resize.

	// The amount we're too high.
	const unsigned too_high = size.y - maximum_height;
	// The amount we reduced
	unsigned reduced = 0;
	for(size_t y = 0; y < rows_; ++y) {

		if(too_high - reduced >=  row_height_[y]) {
			DBG_G_L << "tgrid: row " << y << " is too small to be reduced.\n";
			continue;
		}

		const unsigned wanted_height = row_height_[y] - (too_high - reduced);

		const unsigned height = row_use_vertical_scrollbar(y, wanted_height);

		if(height < row_height_[y]) {
			DBG_G_L << "tgrid: reduced " << row_height_[y] - height
				<< " pixels for row " << y << ".\n";

			reduced += row_height_[y] - height;
			row_height_[y] = height;
		}

		if(reduced >= too_high) {
			break;
		}
	}

	size.y -= reduced;
	if(reduced >= too_high) {
		DBG_G_L << "tgrid: maximum height " << maximum_height
			<< " need to reduce " << too_high
			<< " reduced " << reduced
			<< " resizing succeeded returning " << size.y << ".\n";
	} else if(reduced == 0) {
		DBG_G_L << "tgrid: maximum height " << maximum_height
			<< " need to reduce " << too_high
			<< " reduced " << reduced
			<< " resizing completely failed returning " << size.y << ".\n";
	} else {
		DBG_G_L << "tgrid: maximum height " << maximum_height
			<< " need to reduce " << too_high
			<< " reduced " << reduced
			<< " resizing partly failed returning " << size.y << ".\n";
	}

	set_layout_size(calculate_best_size());
}

bool tgrid::has_horizontal_scrollbar() const
{
	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {
		// FIXME we should check per column and the entire column
		// should have the flag!!!!
		if(itor->widget() && itor->widget()->has_horizontal_scrollbar()) {
			return true;
		}
	}

	// Inherit
	return twidget::has_horizontal_scrollbar();
}

void tgrid::layout_use_horizontal_scrollbar(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_use_horizontal_scrollbar(maximum_width);

	log_scope2(gui_layout, std::string("tgrid ") + __func__);
	DBG_G_L << "tgrid: maximum width " << maximum_width << ".\n";

	tpoint size = get_best_size();

	// If we honoured the size or can't resize return the result.
	if(size.x <= static_cast<int>(maximum_width) || !has_horizontal_scrollbar()) {
		DBG_G_L << "tgrid: maximum width "
			<< maximum_width << " returning " << size << ".\n";
		return;
	}

	// Try to resize.

	// The amount we're too wide.
	const unsigned too_wide = size.x - maximum_width;
	// The amount we reduced
	unsigned reduced = 0;
	for(size_t x = 0; x < cols_; ++x) {

		if(too_wide - reduced >= col_width_[x]) {
			DBG_G_L << "tgrid: col " << x << " is too small to be reduced.\n";
			continue;
		}
		const unsigned wanted_width = col_width_[x] - (too_wide - reduced);

		const unsigned width = column_use_horizontal_scrollbar(x, wanted_width);

		if(width < col_width_[x]) {
			DBG_G_L << "tgrid: reduced " << col_width_[x] - width
				<< " pixels for column " << x << ".\n";

			reduced += col_width_[x] - width;
			col_width_[x] = width;
		}

		if(reduced >= too_wide) {
			break;
		}
	}

	size.x -= reduced;
	if(reduced >= too_wide) {
		DBG_G_L << "tgrid: maximum width " << maximum_width
			<< " need to reduce " << too_wide
			<< " reduced " << reduced
			<< " resizing succeeded returning " << size.x << ".\n";
	} else if(reduced == 0) {
		DBG_G_L << "tgrid: maximum width " << maximum_width
			<< " need to reduce " << too_wide
			<< " reduced " << reduced
			<< " resizing completely failed returning " << size.x << ".\n";
	} else {
		DBG_G_L << "tgrid: maximum width " << maximum_width
			<< " need to reduce " << too_wide
			<< " reduced " << reduced
			<< " resizing partly failed returning " << size.x << ".\n";
	}

	set_layout_size(calculate_best_size());
}

void tgrid::set_size(const tpoint& origin, const tpoint& size)
{
	log_scope2(gui_layout, "tgrid: set size");

	/***** INIT *****/

	twidget::set_size(origin, size);

	if(!rows_ || !cols_) {
		return;
	}

	// call the calculate so the size cache gets updated.
	const tpoint best_size = calculate_best_size();

	assert(row_height_.size() == rows_);
	assert(col_width_.size() == cols_);
	assert(row_grow_factor_.size() == rows_);
	assert(col_grow_factor_.size() == cols_);

	DBG_G_L << "tgrid: best size " << best_size
		<< " available size " << size << ".\n";

	/***** BEST_SIZE *****/

	if(best_size == size) {
		layout(origin);
		return;
	}

	/***** GROW *****/
	if(best_size < size) {

		// expand it.
		if(size.x > best_size.x) {
			const unsigned w = size.x - best_size.x;
			unsigned w_size = std::accumulate(
					col_grow_factor_.begin(), col_grow_factor_.end(), 0);

			DBG_G_L << "tgrid: extra width " << w << " will be divided amount "
				<< w_size << " units in " << cols_ << " columns.\n";

			if(w_size == 0) {
				// If all sizes are 0 reset them to 1
				foreach(unsigned& val, col_grow_factor_) {
					val = 1;
				}
				w_size = cols_;
			}
			// We might have a bit 'extra' if the division doesn't fix exactly
			// but we ignore that part for now.
			const unsigned w_normal = w / w_size;
			for(unsigned i = 0; i < cols_; ++i) {
				col_width_[i] += w_normal * col_grow_factor_[i];
				DBG_G_L << "tgrid: column " << i
					<< " with grow factor " << col_grow_factor_[i]
					<< " set width to " << col_width_[i] << ".\n";
			}

		}

		if(size.y > best_size.y) {
			const unsigned h = size.y - best_size.y;
			unsigned h_size = std::accumulate(
					row_grow_factor_.begin(), row_grow_factor_.end(), 0);
			DBG_G_L << "tgrid: extra height " << h
				<< " will be divided amount " << h_size
				<< " units in " << rows_ << " rows.\n";

			if(h_size == 0) {
				// If all sizes are 0 reset them to 1
				foreach(unsigned& val, row_grow_factor_) {
					val = 1;
				}
				h_size = rows_;
			}
			// We might have a bit 'extra' if the division doesn't fix exactly
			// but we ignore that part for now.
			const unsigned h_normal = h / h_size;
			for(unsigned i = 0; i < rows_; ++i) {
				row_height_[i] += h_normal * row_grow_factor_[i];
				DBG_G_L << "tgrid: row " << i
					<< " with grow factor " << row_grow_factor_[i]
					<< " set height to " << row_height_[i] << ".\n";
			}
		}

		layout(origin);
		return;
	}

	// This shouldn't be possible...
	assert(false);
}

void tgrid::set_origin(const tpoint& origin)
{
	const tpoint movement = tpoint(
			origin.x - get_x(),
			origin.y - get_y());

	// Inherited.
	twidget::set_origin(origin);

	foreach(tchild& child, children_) {

		twidget* widget = child.widget();
		assert(widget);

		widget->set_origin(tpoint(
				widget->get_x() + movement.x,
				widget->get_y() + movement.y));
	}
}

void tgrid::set_visible_area(const SDL_Rect& area)
{
	// Inherited.
	twidget::set_visible_area(area);

	foreach(tchild& child, children_) {

		twidget* widget = child.widget();
		assert(widget);

		widget->set_visible_area(area);
	}
}

void tgrid::child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack)
{
	foreach(tchild& child, children_) {

		assert(child.widget());

		std::vector<twidget*> child_call_stack = call_stack;

		child.widget()->populate_dirty_list(caller, child_call_stack);
	}

}

twidget* tgrid::find_widget(const tpoint& coordinate, const bool must_be_active)
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		twidget* widget = itor->widget();
		if(!widget) {
			continue;
		}

		widget = widget->find_widget(coordinate, must_be_active);
		if(widget) {
			return widget;
		}

	}

	return 0;
}

const twidget* tgrid::find_widget(const tpoint& coordinate,
		const bool must_be_active) const
{
	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		const twidget* widget = itor->widget();
		if(!widget) {
			continue;
		}

		widget = widget->find_widget(coordinate, must_be_active);
		if(widget) {
			return widget;
		}

	}

	return 0;
}

twidget* tgrid::find_widget(const std::string& id, const bool must_be_active)
{
	// Inherited.
	twidget* widget = twidget::find_widget(id, must_be_active);
	if(widget) {
		return widget;
	}

	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		widget = itor->widget();
		if(!widget) {
			continue;
		}

		widget = widget->find_widget(id, must_be_active);
		if(widget) {
			return widget;
		}

	}

	return 0;
}

const twidget* tgrid::find_widget(const std::string& id,
		const bool must_be_active) const
{
	// Inherited.
	const twidget* widget = twidget::find_widget(id, must_be_active);
	if(widget) {
		return widget;
	}

	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		widget = itor->widget();
		if(!widget) {
			continue;
		}

		widget = widget->find_widget(id, must_be_active);
		if(widget) {
			return widget;
		}

	}

	return 0;
}

bool tgrid::has_widget(const twidget* widget) const
{
	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->widget() == widget) {
			return true;
		}
	}
	return false;
}

void tgrid::set_rows(const unsigned rows)
{
	if(rows == rows_) {
		return;
	}

	set_rows_cols(rows, cols_);
}

void tgrid::set_cols(const unsigned cols)
{
	if(cols == cols_) {
		return;
	}

	set_rows_cols(rows_, cols);
}

void tgrid::set_rows_cols(const unsigned rows, const unsigned cols)
{
	if(rows == rows_ && cols == cols_) {
		return;
	}

	if(!children_.empty()) {
		WRN_GUI << "Grid: resizing a non-empty grid may give unexpected problems.\n";
	}

	rows_ = rows;
	cols_ = cols;
	row_grow_factor_.resize(rows);
	col_grow_factor_.resize(cols);
	children_.resize(rows_ * cols_);
}

tpoint tgrid::tchild::get_best_size() const
{
	log_scope2(gui_layout, std::string("tgrid::tchild ") + __func__);

	if(!widget_) {
		DBG_G_L << "tgrid::tchild:"
			<< " has widget " << false
			<< " returning " << border_space()
			<< ".\n";
		return border_space();
	}

	if(widget_->is_invisible()) {
		DBG_G_L << "tgrid::tchild:"
			<< " has widget " << true
			<< " widget invisible " << true
			<< " returning 0,0"
			<< ".\n";
		return tpoint(0, 0);
	}

	const tpoint best_size = widget_->get_best_size() + border_space();

	DBG_G_L << "tgrid::tchild:"
		<< " has widget " << true
		<< " widget invisible " << false
		<< " returning " << best_size
		<< ".\n";
	return best_size;
}

void tgrid::tchild::set_size(tpoint origin, tpoint size)
{
	assert(widget());
	if(widget()->is_invisible()) {
		return;
	}

	if(border_size_) {
		if(flags_ & BORDER_TOP) {
			origin.y += border_size_;
			size.y -= border_size_;
		}
		if(flags_ & BORDER_BOTTOM) {
			size.y -= border_size_;
		}

		if(flags_ & BORDER_LEFT) {
			origin.x += border_size_;
			size.x -= border_size_;
		}
		if(flags_ & BORDER_RIGHT) {
			size.x -= border_size_;
		}
	}

	// If size smaller or equal to best size set that size.
	// No need to check > min size since this is what we got.
	const tpoint best_size = widget()->get_best_size();
	if(size <= best_size) {
		DBG_G_L << "tgrid::tchild: in best size range setting widget to "
			<< origin << " x " << size << ".\n";

		widget()->set_size(origin, size);
		return;
	}

	const tcontrol* control = dynamic_cast<const tcontrol*>(widget());
	const tpoint maximum_size = control
		? control->get_config_maximum_size()
		: tpoint(0, 0);

	if((flags_ & (HORIZONTAL_MASK | VERTICAL_MASK))
			== (HORIZONTAL_GROW_SEND_TO_CLIENT | VERTICAL_GROW_SEND_TO_CLIENT)) {

		if(maximum_size == tpoint(0,0) || size <= maximum_size) {

			DBG_G_L << "tgrid::tchild: in maximum size range setting widget to "
				<< origin << " x " << size << ".\n";

			widget()->set_size(origin, size);
			return;

		}
	}

	tpoint widget_size = tpoint(
		std::min(size.x, best_size.x),
		std::min(size.y, best_size.y));
	tpoint widget_orig = origin;

	const unsigned v_flag = flags_ & VERTICAL_MASK;

	if(v_flag == VERTICAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.y) {
			widget_size.y = std::min(size.y, maximum_size.y);
		} else {
			widget_size.y = size.y;
		}
		DBG_G_L << "tgrid::tchild: vertical growing from "
			<< best_size.y << " to " << widget_size.y << ".\n";

	} else if(v_flag == VERTICAL_ALIGN_TOP) {
		// Do nothing.

		DBG_G_L << "tgrid::tchild: vertically aligned at the top.\n";

	} else if(v_flag == VERTICAL_ALIGN_CENTER) {

		widget_orig.y += (size.y - widget_size.y) / 2;
		DBG_G_L << "tgrid::tchild: vertically centred.\n";

	} else if(v_flag == VERTICAL_ALIGN_BOTTOM) {

		widget_orig.y += (size.y - widget_size.y);
		DBG_G_L << "tgrid::tchild: vertically aligned at the bottom.\n";

	} else {
		ERR_G_L << "tgrid::tchild: Invalid vertical alignment '"
			<< v_flag << "' specified.\n";
		assert(false);
	}

	const unsigned h_flag = flags_ & HORIZONTAL_MASK;

	if(h_flag == HORIZONTAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.x) {
			widget_size.x = std::min(size.x, maximum_size.x);
		} else {
			widget_size.x = size.x;
		}
		DBG_G_L << "tgrid::tchild: horizontal growing from "
			<< best_size.x << " to " << widget_size.x << ".\n";

	} else if(h_flag == HORIZONTAL_ALIGN_LEFT) {
		// Do nothing.
		DBG_G_L << "tgrid::tchild: horizontally aligned at the left.\n";

	} else if(h_flag == HORIZONTAL_ALIGN_CENTER) {

		widget_orig.x += (size.x - widget_size.x) / 2;
		DBG_G_L << "tgrid::tchild: horizontally centred.\n";

	} else if(h_flag == HORIZONTAL_ALIGN_RIGHT) {

		widget_orig.x += (size.x - widget_size.x);
		DBG_G_L << "tgrid::tchild: horizontally aligned at the right.\n";

	} else {
		ERR_G_L << "tgrid::tchild: No horizontal alignment '"
			<< h_flag << "' specified.\n";
		assert(false);
	}

	DBG_G_L << "tgrid::tchild: resize widget to "
		<< widget_orig << " x " << widget_size << ".\n";

	widget()->set_size(widget_orig, widget_size);
}

void tgrid::tchild::layout_wrap(const unsigned maximum_width)
{
	assert(widget_);

	if(! widget_->can_wrap()) {
		return;
	}

	widget_->layout_wrap(maximum_width - border_space().x);
}


void tgrid::tchild::layout_use_vertical_scrollbar(const unsigned maximum_height)
{

	assert(widget_);

	if(! widget_->has_vertical_scrollbar()) {
		return;
	}

	widget_->layout_use_vertical_scrollbar(maximum_height - border_space().y);
}

void tgrid::tchild::layout_use_horizontal_scrollbar(
		const unsigned maximum_width)
{

	assert(widget_);

	if(! widget_->has_horizontal_scrollbar()) {
		return;
	}

	widget_->layout_use_horizontal_scrollbar(
			maximum_width - border_space().x);
}

const std::string& tgrid::tchild::id() const
{
	assert(widget_);
	return widget_->id();
}

tpoint tgrid::tchild::border_space() const
{
	tpoint result(0, 0);

	if(border_size_) {

		if(flags_ & BORDER_TOP) result.y += border_size_;
		if(flags_ & BORDER_BOTTOM) result.y += border_size_;

		if(flags_ & BORDER_LEFT) result.x += border_size_;
		if(flags_ & BORDER_RIGHT) result.x += border_size_;
	}

	return result;
}

void tgrid::layout(const tpoint& origin)
{
	tpoint orig = origin;
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

			const tpoint size(col_width_[col], row_height_[row]);
			DBG_G_L << "tgrid: set widget at " << row << ',' << col
				<< " at origin " << orig << " with size " << size << ".\n";

			if(child(row, col).widget()) {
				child(row, col).set_size(orig, size);
			}

			orig.x += col_width_[col];
		}
		orig.y += row_height_[row];
		orig.x = origin.x;
	}
}

void tgrid::impl_draw_children(surface& frame_buffer)
{
	foreach(tchild& child, children_) {

		twidget* widget = child.widget();
		assert(widget);

		if(!widget->is_visible()) {
			continue;
		}

		if(widget->get_drawing_action() == twidget::NOT_DRAWN) {
			continue;
		}

		widget->draw_background(frame_buffer);
		widget->draw_children(frame_buffer);
		widget->draw_foreground(frame_buffer);
		widget->set_dirty(false);
	}
}

unsigned tgrid::row_use_vertical_scrollbar(
		const unsigned row, const unsigned maximum_height)
{
	// The minimum height required.
	unsigned required_height = 0;

	for(size_t x = 0; x < cols_; ++x) {
		tchild& cell = child(row, x);
		cell.layout_use_vertical_scrollbar(maximum_height);

		const tpoint size(cell.get_best_size());

		if(required_height == 0
				|| static_cast<size_t>(size.y) > required_height) {

			required_height = size.y;
		}
	}

	DBG_G_L << "tgrid: maximum row height " << maximum_height
		<< " returning " << required_height << ".\n";

	return required_height;
}

unsigned tgrid::column_use_horizontal_scrollbar(
		const unsigned column, const unsigned maximum_width)
{
	// The minimum width required.
	unsigned required_width = 0;

	for(size_t y = 0; y < rows_; ++y) {
		tchild& cell = child(y, column);
		cell.layout_use_horizontal_scrollbar(maximum_width);

		const tpoint size(cell.get_best_size());

		if(required_width == 0
				|| static_cast<size_t>(size.x) > required_width) {

			required_width = size.x;
		}
	}

	DBG_G_L << "tgrid: maximum column width " << maximum_width
		<< " returning " << required_width << ".\n";

	return required_width;
}

} // namespace gui2


/*WIKI
 * @page = GUILayout
 *
 * THIS PAGE IS AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY !!!
 *
 * = Abstract =
 *
 * In the widget library the placement and sizes of elements is determined by
 * a grid. Therefore most widgets have no fixed size.
 *
 *
 * = Theory =
 *
 * We have two examples for the addon dialog, the first example the lower
 * buttons are in one grid, that means if the remove button gets wider
 * (due to translations) the connect button (4.1 - 2.2) will be aligned
 * to the left of the remove button. In the second example the connect
 * button will be partial underneath the remove button.
 *
 * A grid exists of x rows and y columns for all rows the number of columns
 * needs to be the same, there is no column (nor row) span. If spanning is
 * required place a nested grid to do so. In the examples every row has 1 column
 * but rows 3, 4 (and in the second 5) have a nested grid to add more elements
 * per row.
 *
 * In the grid every cell needs to have a widget, if no widget is wanted place
 * the special widget ''spacer''. This is a non-visible item which normally
 * shouldn't have a size. It is possible to give a spacer a size as well but
 * that is discussed elsewhere.
 *
 * Every row and column has a ''grow_factor'', since all columns in a grid are
 * aligned only the columns in the first row need to define their grow factor.
 * The grow factor is used to determine with the extra size available in a
 * dialog. The algorithm determines the extra size work like this:
 *
 * * determine the extra size
 * * determine the sum of the grow factors
 * * if this sum is 0 set the grow factor for every item to 1 and sum to sum of items.
 * * divide the extra size with the sum of grow factors
 * * for every item multiply the grow factor with the division value
 *
 * eg
 *  extra size 100
 *  grow factors 1, 1, 2, 1
 *  sum 5
 *  division 100 / 5 = 20
 *  extra sizes 20, 20, 40, 20
 *
 * Since we force the factors to 1 if all zero it's not possible to have non
 * growing cells. This can be solved by adding an extra cell with a spacer and a
 * grow factor of 1. This is used for the buttons in the examples.
 *
 * Every cell has a ''border_size'' and ''border'' the ''border_size'' is the
 * number of pixels in the cell which aren't available for the widget. This is
 * used to make sure the items in different cells aren't put side to side. With
 * ''border'' it can be determined which sides get the border. So a border is
 * either 0 or ''border_size''.
 *
 * If the widget doesn't grow when there's more space available the alignment
 * determines where in the cell the widget is placed.
 *
 * == Examples ==
 *
 *  |---------------------------------------|
 *  | 1.1                                   |
 *  |---------------------------------------|
 *  | 2.1                                   |
 *  |---------------------------------------|
 *  | |-----------------------------------| |
 *  | | 3.1 - 1.1          | 3.1 - 1.2    | |
 *  | |-----------------------------------| |
 *  |---------------------------------------|
 *  | |-----------------------------------| |
 *  | | 4.1 - 1.1 | 4.1 - 1.2 | 4.1 - 1.3 | |
 *  | |-----------------------------------| |
 *  | | 4.1 - 2.1 | 4.1 - 2.2 | 4.1 - 2.3 | |
 *  | |-----------------------------------| |
 *  |---------------------------------------|
 *
 *
 *  1.1       label : title
 *  2.1       label : description
 *  3.1 - 1.1 label : server
 *  3.1 - 1.2 text box : server to connect to
 *  4.1 - 1.1 spacer
 *  4.1 - 1.2 spacer
 *  4.1 - 1.3 button : remove addon
 *  4.2 - 2.1 spacer
 *  4.2 - 2.2 button : connect
 *  4.2 - 2.3 button : cancel
 *
 *
 *  |---------------------------------------|
 *  | 1.1                                   |
 *  |---------------------------------------|
 *  | 2.1                                   |
 *  |---------------------------------------|
 *  | |-----------------------------------| |
 *  | | 3.1 - 1.1          | 3.1 - 1.2    | |
 *  | |-----------------------------------| |
 *  |---------------------------------------|
 *  | |-----------------------------------| |
 *  | | 4.1 - 1.1         | 4.1 - 1.2     | |
 *  | |-----------------------------------| |
 *  |---------------------------------------|
 *  | |-----------------------------------| |
 *  | | 5.1 - 1.1 | 5.1 - 1.2 | 5.1 - 2.3 | |
 *  | |-----------------------------------| |
 *  |---------------------------------------|
 *
 *
 *  1.1       label : title
 *  2.1       label : description
 *  3.1 - 1.1 label : server
 *  3.1 - 1.2 text box : server to connect to
 *  4.1 - 1.1 spacer
 *  4.1 - 1.2 button : remove addon
 *  5.2 - 1.1 spacer
 *  5.2 - 1.2 button : connect
 *  5.2 - 1.3 button : cancel
 *
 *  = Praxis =
 *
 * This is the code needed to create the skeleton for the structure the extra
 * flags are ommitted.
 *
 *  	[grid]
 *  		[row]
 *  			[column]
 *  				[label]
 *  					# 1.1
 *  				[/label]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[label]
 *  					# 2.1
 *  				[/label]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[grid]
 *  					[row]
 *  						[column]
 *  							[label]
 *  								# 3.1 - 1.1
 *  							[/label]
 *  						[/column]
 *  						[column]
 *  							[text_box]
 *  								# 3.1 - 1.2
 *  							[/text_box]
 *  						[/column]
 *  					[/row]
 *  				[/grid]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[grid]
 *  					[row]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 1.1
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 1.2
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 1.3
 *  							[/button]
 *  						[/column]
 *  					[/row]
 *  					[row]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 2.1
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 2.2
 *  							[/button]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 2.3
 *  							[/button]
 *  						[/column]
 *  					[/row]
 *  				[/grid]
 *  			[/column]
 *  		[/row]
 *  	[/grid]
 *
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 * [[Category: Generated]]
 */
