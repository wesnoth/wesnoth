/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/grid_private.hpp"

#include "gui/auxiliary/iterator/walker_grid.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/widgets/control.hpp"
#include "utils/foreach.tpp"

#include <numeric>

#define LOG_SCOPE_HEADER "tgrid [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'
#define LOG_IMPL_HEADER "tgrid [" + grid.id() + "] " + __func__ + ':'

#define LOG_CHILD_SCOPE_HEADER                                                 \
	"tgrid::tchild [" + (widget_ ? widget_->id() : "-") + "] " + __func__
#define LOG_CHILD_HEADER LOG_CHILD_SCOPE_HEADER + ':'

namespace gui2
{

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
	// Delete the children in this destructor since resizing a vector copies the
	// children and thus frees the child prematurely.
	FOREACH(AUTO & child, children_)
	{
		delete child.widget();
	}
}

unsigned tgrid::add_row(const unsigned count)
{
	assert(count);

	// FIXME the warning in set_rows_cols should be killed.

	unsigned result = rows_;
	set_rows_cols(rows_ + count, cols_);
	return result;
}

void tgrid::set_child(twidget* widget,
					  const unsigned row,
					  const unsigned col,
					  const unsigned flags,
					  const unsigned border_size)
{
	assert(row < rows_ && col < cols_);
	assert(flags & VERTICAL_MASK);
	assert(flags & HORIZONTAL_MASK);

	tchild& cell = child(row, col);

	// clear old child if any
	if(cell.widget()) {
		// free a child when overwriting it
		WRN_GUI_G << LOG_HEADER << " child '" << cell.id() << "' at cell '"
				  << row << ',' << col << "' will be replaced.\n";
		delete cell.widget();
	}

	// copy data
	cell.set_flags(flags);
	cell.set_border_size(border_size);
	cell.set_widget(widget);
	if(cell.widget()) {
		// make sure the new child is valid before deferring
		cell.widget()->set_parent(this);
	}
}

twidget* tgrid::swap_child(const std::string& id,
						   twidget* widget,
						   const bool recurse,
						   twidget* new_parent)
{
	assert(widget);

	FOREACH(AUTO & child, children_)
	{
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
	cell.set_widget(NULL);
}

void tgrid::remove_child(const std::string& id, const bool find_all)
{
	FOREACH(AUTO & child, children_)
	{

		if(child.id() == id) {
			delete child.widget();
			child.set_widget(NULL);

			if(!find_all) {
				break;
			}
		}
	}
}

void tgrid::set_active(const bool active)
{
	FOREACH(AUTO & child, children_)
	{

		twidget* widget = child.widget();
		if(!widget) {
			continue;
		}

		tgrid* grid = dynamic_cast<tgrid*>(widget);
		if(grid) {
			grid->set_active(active);
			continue;
		}

		tcontrol* control = dynamic_cast<tcontrol*>(widget);
		if(control) {
			control->set_active(active);
		}
	}
}

void tgrid::layout_initialise(const bool full_initialisation)
{
	// Inherited.
	twidget::layout_initialise(full_initialisation);

	// Clear child caches.
	FOREACH(AUTO & child, children_)
	{

		child.layout_initialise(full_initialisation);
	}
}

void tgrid::reduce_width(const unsigned maximum_width)
{
	/***** ***** ***** ***** INIT ***** ***** ***** *****/
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " maximum width " << maximum_width << ".\n";

	tpoint size = get_best_size();
	if(size.x <= static_cast<int>(maximum_width)) {
		DBG_GUI_L << LOG_HEADER << " Already fits.\n";
		return;
	}

	/***** ***** ***** ***** Request resize ***** ***** ***** *****/

	request_reduce_width(maximum_width);

	size = get_best_size();
	if(size.x <= static_cast<int>(maximum_width)) {
		DBG_GUI_L << LOG_HEADER << " Resize request honored.\n";
		return;
	}

	/***** ***** ***** ***** Demand resize ***** ***** ***** *****/

	/** @todo Implement. */

	/***** ***** ***** ***** Acknowlegde failure ***** ***** ***** *****/

	DBG_GUI_L << LOG_HEADER << " Resizing failed.\n";

	throw tlayout_exception_width_resize_failed();
}

void tgrid::request_reduce_width(const unsigned maximum_width)
{
	tpoint size = get_best_size();
	if(size.x <= static_cast<int>(maximum_width)) {
		/** @todo this point shouldn't be reached, find out why it does. */
		return;
	}

	const unsigned too_wide = size.x - maximum_width;
	unsigned reduced = 0;
	for(size_t col = 0; col < cols_; ++col) {
		if(too_wide - reduced >= col_width_[col]) {
			DBG_GUI_L << LOG_HEADER << " column " << col
					  << " is too small to be reduced.\n";
			continue;
		}

		const unsigned wanted_width = col_width_[col] - (too_wide - reduced);
		const unsigned width
				= tgrid_implementation::column_request_reduce_width(
						*this, col, wanted_width);

		if(width < col_width_[col]) {
			DBG_GUI_L << LOG_HEADER << " reduced " << col_width_[col] - width
					  << " pixels for column " << col << ".\n";

			size.x -= col_width_[col] - width;
			col_width_[col] = width;
		}

		if(size.x <= static_cast<int>(maximum_width)) {
			break;
		}
	}

	set_layout_size(calculate_best_size());
}

void tgrid::demand_reduce_width(const unsigned /*maximum_width*/)
{
	/** @todo Implement. */
}

void tgrid::reduce_height(const unsigned maximum_height)
{
	/***** ***** ***** ***** INIT ***** ***** ***** *****/
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);
	DBG_GUI_L << LOG_HEADER << " maximum height " << maximum_height << ".\n";

	tpoint size = get_best_size();
	if(size.y <= static_cast<int>(maximum_height)) {
		DBG_GUI_L << LOG_HEADER << " Already fits.\n";
		return;
	}

	/***** ***** ***** ***** Request resize ***** ***** ***** *****/

	request_reduce_height(maximum_height);

	size = get_best_size();
	if(size.y <= static_cast<int>(maximum_height)) {
		DBG_GUI_L << LOG_HEADER << " Resize request honored.\n";
		return;
	}

	/***** ***** ***** ***** Demand resize ***** ***** ***** *****/

	/** @todo Implement. */

	/***** ***** ***** ***** Acknowlegde failure ***** ***** ***** *****/

	DBG_GUI_L << LOG_HEADER << " Resizing failed.\n";

	throw tlayout_exception_height_resize_failed();
}

void tgrid::request_reduce_height(const unsigned maximum_height)
{
	tpoint size = get_best_size();
	if(size.y <= static_cast<int>(maximum_height)) {
		/** @todo this point shouldn't be reached, find out why it does. */
		return;
	}

	const unsigned too_high = size.y - maximum_height;
	unsigned reduced = 0;
	for(size_t row = 0; row < rows_; ++row) {
		unsigned wanted_height = row_height_[row] - (too_high - reduced);
		/**
		 * @todo Improve this code.
		 *
		 * Now we try every item to be reduced, maybe items need a flag whether
		 * or not to try to reduce and also evaluate whether the force
		 * reduction is still needed.
		 */
		if(too_high - reduced >= row_height_[row]) {
			DBG_GUI_L << LOG_HEADER << " row " << row << " height "
					  << row_height_[row] << " want to reduce " << too_high
					  << " is too small to be reduced fully try 1 pixel.\n";

			wanted_height = 1;
		}

		const unsigned height = tgrid_implementation::row_request_reduce_height(
				*this, row, wanted_height);

		if(height < row_height_[row]) {
			DBG_GUI_L << LOG_HEADER << " row " << row << " height "
					  << row_height_[row] << " want to reduce " << too_high
					  << " reduced " << row_height_[row] - height
					  << " pixels.\n";

			size.y -= row_height_[row] - height;
			row_height_[row] = height;
		}

		if(size.y <= static_cast<int>(maximum_height)) {
			break;
		}
	}

	size = calculate_best_size();

	DBG_GUI_L << LOG_HEADER << " Requested maximum " << maximum_height
			  << " resulting height " << size.y << ".\n";

	set_layout_size(size);
}

void tgrid::demand_reduce_height(const unsigned /*maximum_height*/)
{
	/** @todo Implement. */
}

tpoint tgrid::recalculate_best_size()
{
	tpoint best_size = calculate_best_size();
	set_layout_size(best_size);
	return best_size;
}

tpoint tgrid::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

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
		DBG_GUI_L << LOG_HEADER << " the row_height_ for row " << row
				  << " will be " << row_height_[row] << ".\n";
	}

	for(unsigned col = 0; col < cols_; ++col) {
		DBG_GUI_L << LOG_HEADER << " the col_width_ for column " << col
				  << " will be " << col_width_[col] << ".\n";
	}

	const tpoint result(
			std::accumulate(col_width_.begin(), col_width_.end(), 0),
			std::accumulate(row_height_.begin(), row_height_.end(), 0));

	DBG_GUI_L << LOG_HEADER << " returning " << result << ".\n";
	return result;
}

bool tgrid::can_wrap() const
{
	FOREACH(const AUTO & child, children_)
	{
		if(child.can_wrap()) {
			return true;
		}
	}

	// Inherited.
	return twidget::can_wrap();
}

void tgrid::place(const tpoint& origin, const tpoint& size)
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	/***** INIT *****/

	twidget::place(origin, size);

	if(!rows_ || !cols_) {
		return;
	}

	// call the calculate so the size cache gets updated.
	const tpoint best_size = calculate_best_size();

	assert(row_height_.size() == rows_);
	assert(col_width_.size() == cols_);
	assert(row_grow_factor_.size() == rows_);
	assert(col_grow_factor_.size() == cols_);

	DBG_GUI_L << LOG_HEADER << " best size " << best_size << " available size "
			  << size << ".\n";

	/***** BEST_SIZE *****/

	if(best_size == size) {
		layout(origin);
		return;
	}

	if(best_size.x > size.x || best_size.y > size.y) {
		// The assertion below fails quite often so try to give as much information as possible.
		std::stringstream out;
		out << " Failed to place a grid, we have " << size << " space but we need " << best_size << " space.";
		out << " This happened at a grid with the id '" << id() << "'";
		twidget* pw = parent();
		while(pw != NULL) {
			out << " in a '" << typeid(*pw).name() << "' with the id '" << pw->id() << "'";
			pw = pw->parent();
		}
		ERR_GUI_L << LOG_HEADER << out.str() << ".\n";
		// This shouldn't be possible...
		assert(false);
		return;
	}

	/***** GROW *****/

	// expand it.
	if(size.x > best_size.x) {
		const unsigned w = size.x - best_size.x;
		unsigned w_size = std::accumulate(
			col_grow_factor_.begin(), col_grow_factor_.end(), 0);

		DBG_GUI_L << LOG_HEADER << " extra width " << w
			<< " will be divided amount " << w_size << " units in "
			<< cols_ << " columns.\n";

		if(w_size == 0) {
			// If all sizes are 0 reset them to 1
			FOREACH(AUTO & val, col_grow_factor_)
			{
				val = 1;
			}
			w_size = cols_;
		}
		// We might have a bit 'extra' if the division doesn't fix exactly
		// but we ignore that part for now.
		const unsigned w_normal = w / w_size;
		for(unsigned i = 0; i < cols_; ++i) {
			col_width_[i] += w_normal * col_grow_factor_[i];
			DBG_GUI_L << LOG_HEADER << " column " << i
				<< " with grow factor " << col_grow_factor_[i]
			<< " set width to " << col_width_[i] << ".\n";
		}
	}

	if(size.y > best_size.y) {
		const unsigned h = size.y - best_size.y;
		unsigned h_size = std::accumulate(
			row_grow_factor_.begin(), row_grow_factor_.end(), 0);
		DBG_GUI_L << LOG_HEADER << " extra height " << h
			<< " will be divided amount " << h_size << " units in "
			<< rows_ << " rows.\n";

		if(h_size == 0) {
			// If all sizes are 0 reset them to 1
			FOREACH(AUTO & val, row_grow_factor_)
			{
				val = 1;
			}
			h_size = rows_;
		}
		// We might have a bit 'extra' if the division doesn't fix exactly
		// but we ignore that part for now.
		const unsigned h_normal = h / h_size;
		for(unsigned i = 0; i < rows_; ++i) {
			row_height_[i] += h_normal * row_grow_factor_[i];
			DBG_GUI_L << LOG_HEADER << " row " << i << " with grow factor "
				<< row_grow_factor_[i] << " set height to "
				<< row_height_[i] << ".\n";
		}
	}

	layout(origin);
	return;
}

void tgrid::set_origin(const tpoint& origin)
{
	const tpoint movement = tpoint(origin.x - get_x(), origin.y - get_y());

	// Inherited.
	twidget::set_origin(origin);

	FOREACH(AUTO & child, children_)
	{

		twidget* widget = child.widget();
		assert(widget);

		widget->set_origin(tpoint(widget->get_x() + movement.x,
								  widget->get_y() + movement.y));
	}
}

void tgrid::set_visible_rectangle(const SDL_Rect& rectangle)
{
	// Inherited.
	twidget::set_visible_rectangle(rectangle);

	FOREACH(AUTO & child, children_)
	{

		twidget* widget = child.widget();
		assert(widget);

		widget->set_visible_rectangle(rectangle);
	}
}

void tgrid::layout_children()
{
	FOREACH(AUTO & child, children_)
	{
		assert(child.widget());
		child.widget()->layout_children();
	}
}

void tgrid::child_populate_dirty_list(twindow& caller,
									  const std::vector<twidget*>& call_stack)
{
	assert(!call_stack.empty() && call_stack.back() == this);

	FOREACH(AUTO & child, children_)
	{

		assert(child.widget());

		std::vector<twidget*> child_call_stack = call_stack;
		child.widget()->populate_dirty_list(caller, child_call_stack);
	}
}

twidget* tgrid::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return tgrid_implementation::find_at<twidget>(
			*this, coordinate, must_be_active);
}

const twidget* tgrid::find_at(const tpoint& coordinate,
							  const bool must_be_active) const
{
	return tgrid_implementation::find_at<const twidget>(
			*this, coordinate, must_be_active);
}

twidget* tgrid::find(const std::string& id, const bool must_be_active)
{
	return tgrid_implementation::find<twidget>(*this, id, must_be_active);
}

const twidget* tgrid::find(const std::string& id, const bool must_be_active)
		const
{
	return tgrid_implementation::find<const twidget>(*this, id, must_be_active);
}

bool tgrid::has_widget(const twidget& widget) const
{
	if(twidget::has_widget(widget)) {
		return true;
	}

	FOREACH(const AUTO & child, children_)
	{
		if(child.widget()->has_widget(widget)) {
			return true;
		}
	}
	return false;
}

bool tgrid::disable_click_dismiss() const
{
	if(get_visible() != twidget::tvisible::visible) {
		return false;
	}

	FOREACH(const AUTO & child, children_)
	{
		const twidget* widget = child.widget();
		assert(widget);

		if(widget->disable_click_dismiss()) {
			return true;
		}
	}
	return false;
}

iterator::twalker_* tgrid::create_walker()
{
	return new gui2::iterator::tgrid(*this);
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
		WRN_GUI_G << LOG_HEADER << " resizing a non-empty grid "
				  << " may give unexpected problems.\n";
	}

	rows_ = rows;
	cols_ = cols;
	row_grow_factor_.resize(rows);
	col_grow_factor_.resize(cols);
	children_.resize(rows_ * cols_);
}

tpoint tgrid::tchild::get_best_size() const
{
	log_scope2(log_gui_layout, LOG_CHILD_SCOPE_HEADER)

	if(!widget_) {
		DBG_GUI_L << LOG_CHILD_HEADER << " has widget " << false
				  << " returning " << border_space() << ".\n";
		return border_space();
	}

	if(widget_->get_visible() == twidget::tvisible::invisible) {
		DBG_GUI_L << LOG_CHILD_HEADER << " has widget " << true
				  << " widget visible " << false << " returning 0,0"
				  << ".\n";
		return tpoint(0, 0);
	}

	const tpoint best_size = widget_->get_best_size() + border_space();

	DBG_GUI_L << LOG_CHILD_HEADER << " has widget " << true
			  << " widget visible " << true << " returning " << best_size
			  << ".\n";
	return best_size;
}

void tgrid::tchild::place(tpoint origin, tpoint size)
{
	assert(widget());
	if(widget()->get_visible() == twidget::tvisible::invisible) {
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
		DBG_GUI_L << LOG_CHILD_HEADER
				  << " in best size range setting widget to " << origin << " x "
				  << size << ".\n";

		widget()->place(origin, size);
		return;
	}

	const tcontrol* control = dynamic_cast<const tcontrol*>(widget());
	const tpoint maximum_size = control ? control->get_config_maximum_size()
										: tpoint(0, 0);

	if((flags_ & (HORIZONTAL_MASK | VERTICAL_MASK))
	   == (HORIZONTAL_GROW_SEND_TO_CLIENT | VERTICAL_GROW_SEND_TO_CLIENT)) {

		if(maximum_size == tpoint(0, 0) || size <= maximum_size) {

			DBG_GUI_L << LOG_CHILD_HEADER
					  << " in maximum size range setting widget to " << origin
					  << " x " << size << ".\n";

			widget()->place(origin, size);
			return;
		}
	}

	tpoint widget_size = tpoint(std::min(size.x, best_size.x),
								std::min(size.y, best_size.y));
	tpoint widget_orig = origin;

	const unsigned v_flag = flags_ & VERTICAL_MASK;

	if(v_flag == VERTICAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.y) {
			widget_size.y = std::min(size.y, maximum_size.y);
		} else {
			widget_size.y = size.y;
		}
		DBG_GUI_L << LOG_CHILD_HEADER << " vertical growing from "
				  << best_size.y << " to " << widget_size.y << ".\n";

	} else if(v_flag == VERTICAL_ALIGN_TOP) {
		// Do nothing.

		DBG_GUI_L << LOG_CHILD_HEADER << " vertically aligned at the top.\n";

	} else if(v_flag == VERTICAL_ALIGN_CENTER) {

		widget_orig.y += (size.y - widget_size.y) / 2;
		DBG_GUI_L << LOG_CHILD_HEADER << " vertically centered.\n";

	} else if(v_flag == VERTICAL_ALIGN_BOTTOM) {

		widget_orig.y += (size.y - widget_size.y);
		DBG_GUI_L << LOG_CHILD_HEADER << " vertically aligned at the bottom.\n";

	} else {
		ERR_GUI_L << LOG_CHILD_HEADER << " Invalid vertical alignment '"
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
		DBG_GUI_L << LOG_CHILD_HEADER << " horizontal growing from "
				  << best_size.x << " to " << widget_size.x << ".\n";

	} else if(h_flag == HORIZONTAL_ALIGN_LEFT) {
		// Do nothing.
		DBG_GUI_L << LOG_CHILD_HEADER << " horizontally aligned at the left.\n";

	} else if(h_flag == HORIZONTAL_ALIGN_CENTER) {

		widget_orig.x += (size.x - widget_size.x) / 2;
		DBG_GUI_L << LOG_CHILD_HEADER << " horizontally centered.\n";

	} else if(h_flag == HORIZONTAL_ALIGN_RIGHT) {

		widget_orig.x += (size.x - widget_size.x);
		DBG_GUI_L << LOG_CHILD_HEADER
				  << " horizontally aligned at the right.\n";

	} else {
		ERR_GUI_L << LOG_CHILD_HEADER << " No horizontal alignment '" << h_flag
				  << "' specified.\n";
		assert(false);
	}

	DBG_GUI_L << LOG_CHILD_HEADER << " resize widget to " << widget_orig
			  << " x " << widget_size << ".\n";

	widget()->place(widget_orig, widget_size);
}

void tgrid::tchild::layout_initialise(const bool full_initialisation)
{
	assert(widget_);

	if(widget_->get_visible() != twidget::tvisible::invisible) {
		widget_->layout_initialise(full_initialisation);
	}
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

		if(flags_ & BORDER_TOP)
			result.y += border_size_;
		if(flags_ & BORDER_BOTTOM)
			result.y += border_size_;

		if(flags_ & BORDER_LEFT)
			result.x += border_size_;
		if(flags_ & BORDER_RIGHT)
			result.x += border_size_;
	}

	return result;
}

void tgrid::layout(const tpoint& origin)
{
	tpoint orig = origin;
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

			const tpoint size(col_width_[col], row_height_[row]);
			DBG_GUI_L << LOG_HEADER << " set widget at " << row << ',' << col
					  << " at origin " << orig << " with size " << size
					  << ".\n";

			if(child(row, col).widget()) {
				child(row, col).place(orig, size);
			}

			orig.x += col_width_[col];
		}
		orig.y += row_height_[row];
		orig.x = origin.x;
	}
}

void tgrid::impl_draw_children(surface& frame_buffer)
{
	/*
	 * The call to SDL_PumpEvents seems a bit like black magic.
	 * With the call the resizing doesn't seem to lose resize events.
	 * But when added the queue still only contains one resize event and the
	 * internal SDL queue doesn't seem to overflow (rarely more than 50 pending
	 * events).
	 * Without the call when resizing larger a black area of remains, this is
	 * the area not used for resizing the screen, this call `fixes' that.
	 */
#if !SDL_VERSION_ATLEAST(2,0,0)
	SDL_PumpEvents();
#endif


	assert(get_visible() == twidget::tvisible::visible);
	set_is_dirty(false);

	FOREACH(AUTO & child, children_)
	{

		twidget* widget = child.widget();
		assert(widget);

		if(widget->get_visible() != twidget::tvisible::visible) {
			continue;
		}

		if(widget->get_drawing_action() == twidget::tredraw_action::none) {
			continue;
		}

		widget->draw_background(frame_buffer);
		widget->draw_children(frame_buffer);
		widget->draw_foreground(frame_buffer);
		widget->set_is_dirty(false);
	}
}

void
tgrid::impl_draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	/*
	 * The call to SDL_PumpEvents seems a bit like black magic.
	 * With the call the resizing doesn't seem to lose resize events.
	 * But when added the queue still only contains one resize event and the
	 * internal SDL queue doesn't seem to overflow (rarely more than 50 pending
	 * events).
	 * Without the call when resizing larger a black area of remains, this is
	 * the area not used for resizing the screen, this call `fixes' that.
	 */
#if !SDL_VERSION_ATLEAST(2,0,0)
	SDL_PumpEvents();
#endif

	assert(get_visible() == twidget::tvisible::visible);
	set_is_dirty(false);

	FOREACH(AUTO & child, children_)
	{

		twidget* widget = child.widget();
		assert(widget);

		if(widget->get_visible() != twidget::tvisible::visible) {
			continue;
		}

		if(widget->get_drawing_action() == twidget::tredraw_action::none) {
			continue;
		}

		widget->draw_background(frame_buffer, x_offset, y_offset);
		widget->draw_children(frame_buffer, x_offset, y_offset);
		widget->draw_foreground(frame_buffer, x_offset, y_offset);
		widget->set_is_dirty(false);
	}
}

unsigned tgrid_implementation::row_request_reduce_height(
		tgrid& grid, const unsigned row, const unsigned maximum_height)
{
	// The minimum height required.
	unsigned required_height = 0;

	for(size_t x = 0; x < grid.cols_; ++x) {
		tgrid::tchild& cell = grid.child(row, x);
		cell_request_reduce_height(cell, maximum_height);

		const tpoint size(cell.get_best_size());

		if(required_height == 0 || static_cast<size_t>(size.y)
								   > required_height) {

			required_height = size.y;
		}
	}

	DBG_GUI_L << LOG_IMPL_HEADER << " maximum row height " << maximum_height
			  << " returning " << required_height << ".\n";

	return required_height;
}

unsigned tgrid_implementation::column_request_reduce_width(
		tgrid& grid, const unsigned column, const unsigned maximum_width)
{
	// The minimum width required.
	unsigned required_width = 0;

	for(size_t y = 0; y < grid.rows_; ++y) {
		tgrid::tchild& cell = grid.child(y, column);
		cell_request_reduce_width(cell, maximum_width);

		const tpoint size(cell.get_best_size());

		if(required_width == 0 || static_cast<size_t>(size.x)
								  > required_width) {

			required_width = size.x;
		}
	}

	DBG_GUI_L << LOG_IMPL_HEADER << " maximum column width " << maximum_width
			  << " returning " << required_width << ".\n";

	return required_width;
}

void
tgrid_implementation::cell_request_reduce_height(tgrid::tchild& child,
												 const unsigned maximum_height)
{
	assert(child.widget_);

	if(child.widget_->get_visible() == twidget::tvisible::invisible) {
		return;
	}

	child.widget_->request_reduce_height(maximum_height
										 - child.border_space().y);
}

void
tgrid_implementation::cell_request_reduce_width(tgrid::tchild& child,
												const unsigned maximum_width)
{
	assert(child.widget_);

	if(child.widget_->get_visible() == twidget::tvisible::invisible) {
		return;
	}

	child.widget_->request_reduce_width(maximum_width - child.border_space().x);
}

void set_single_child(tgrid& grid, twidget* widget)
{
	grid.set_rows_cols(1, 1);
	grid.set_child(widget,
				   0,
				   0,
				   tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
				   | tgrid::VERTICAL_GROW_SEND_TO_CLIENT,
				   0);
}

} // namespace gui2


/*WIKI
 * @page = GUILayout
 *
 * {{Autogenerated}}
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
 * = Praxis =
 *
 * This is the code needed to create the skeleton for the structure the extra
 * flags are omitted.
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
 */
