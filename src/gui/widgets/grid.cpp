/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/grid.hpp"

#include "foreach.hpp"
#include "log.hpp"

#include <cassert>
#include <numeric>

// Disable the size caching for now, it gives problems with the new wrapping
// code. Postpone the final faith of this code until we know whether or not the
// caching is needed for speed.
#define DISABLE_CACHE 1

namespace gui2 {

	bool disable_cache = false;

tgrid::tgrid(const unsigned rows, const unsigned cols) :
	rows_(rows),
	cols_(cols),
	best_row_height_(),
	best_col_width_(),
	minimum_row_height_(),
	minimum_col_width_(),
	row_height_(),
	col_width_(),
	row_grow_factor_(rows),
	col_grow_factor_(cols),
	children_(rows * cols)
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
		cell.set_id(cell.widget()->id());
		cell.widget()->set_parent(this);
	} else {
		cell.set_id("");
	}

	clear_cache();
}

twidget* tgrid::swap_child(
		const std::string& id, twidget* widget, const bool recurse)
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
		old->set_parent(NULL);

		widget->set_parent(this);
		child.set_widget(widget);
		child.set_id(widget->id());

		return old;
	}

	return NULL;
}

void tgrid::remove_child(const unsigned row, const unsigned col)
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	cell.set_id("");
	if(cell.widget()) {
		delete cell.widget();
	}
	cell.set_widget(0);
	clear_cache();
}

void tgrid::remove_child(const std::string& id, const bool find_all)
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->id() == id) {
			itor->set_id("");
			if(itor->widget()) {
				delete itor->widget();
			}
			itor->set_widget(0);
			clear_cache();

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

tpoint tgrid::get_minimum_size() const
{
	log_scope2(gui_layout, std::string("tgrid ") + __func__);

	return get_size("minimum", minimum_col_width_, 
		minimum_row_height_, &tchild::get_minimum_size);
}

tpoint tgrid::get_maximum_size() const
{
	log_scope2(gui_layout, std::string("tgrid ") + __func__);

	DBG_G_L << "tgrid " << __func__ << ": Always 0,0.\n";

	// A grid doesn't have a maximum size.
	return tpoint(0,0);
}

tpoint tgrid::get_best_size() const
{
	log_scope2(gui_layout, std::string("tgrid ") + __func__);

	return get_size("best", best_col_width_, 
		best_row_height_, &tchild::get_best_size);
}

tpoint tgrid::get_best_size(const tpoint& maximum_size) const
{
	log_scope2(gui_layout, std::string("tgrid ") + __func__);
	DBG_G_L << "tgrid: maximum size " << maximum_size << ".\n";
	
	tpoint size = get_size("best with maximum", best_col_width_, 
		best_row_height_, NULL, &tchild::get_best_size, maximum_size);

	// If we honoured the size or can't resize return the result.
	if(size.y <= maximum_size.y || !has_vertical_scrollbar()) {
		DBG_G_L << "tgrid: maximum size " 
			<< maximum_size << " returning " << size << ".\n";
		return size;
	}

	// Try to resize.
	
	// The amount we're too high.
	const unsigned too_high = size.y - maximum_size.y;
	// The amount we reduced
	unsigned reduced = 0;
	for(size_t y = 0; y < rows_; ++y) {

		const unsigned wanted_height = (too_high - reduced) >= best_row_height_[y] 
			? 1 : best_row_height_[y] - (too_high - reduced);

		const unsigned height = get_best_row_height(y, wanted_height);

		if(height < best_row_height_[y]) {
			DBG_G_L << "tgrid: reduced " << best_row_height_[y] - height 
				<< " pixels for row " << y << ".\n";

			reduced += best_row_height_[y] - height;
			best_row_height_[y] = height;
		}
		
		if(reduced >= too_high) {
			break;
		}
	}

	/** @todo vertical resizing isn't implemented yet. */

	size.y -= reduced;
	if(reduced >= too_high) {
		DBG_G_L << "tgrid: maximum size " << maximum_size
			<< " need to reduce " << too_high 
			<< " reduced " << reduced 
			<< " resizing succeeded returning " << size.y << ".\n";
	} else if(reduced == 0) {
		DBG_G_L << "tgrid: maximum size " << maximum_size
			<< " need to reduce " << too_high 
			<< " reduced " << reduced 
			<< " resizing completely failed returning " << size.y << ".\n";
	} else {
		DBG_G_L << "tgrid: maximum size " << maximum_size
			<< " need to reduce " << too_high 
			<< " reduced " << reduced 
			<< " resizing partly failed returning " << size.y << ".\n";
	}

	return tpoint(size.x, size.y);
}

bool tgrid::can_wrap() const 
{
	foreach(const tchild& child, children_) {
		if(child.can_wrap()) {
			return true;
		}
	}

	// Inherit
	return twidget::can_wrap();
}

bool tgrid::set_width_constrain(const unsigned maximum_width)
{

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

			widths[col] = (child(row, col)./**widget()->**/get_best_size()).x;
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

				if(chld.get_best_size(tpoint(widget_width, 0)).x <= widget_width
						&& chld.set_width_constrain(widget_width)) {	

					DBG_G_L << "tgrid: column resize succeeded.\n";
					break;
				}

				DBG_G_L << "tgrid: column resize failed.\n";
				widths[col] = width;
				width = 0;
			}

			if(width == 0) {
				DBG_G_L << "tgrid: no solution found.\n";
				return false;
			}
		} else {
			DBG_G_L << "tgrid: row " << row << " in bounds.\n";
		}
	}

	DBG_G_L << "tgrid: found solution.\n";
	return true;
}

void tgrid::clear_width_constrain()
{
	foreach(tchild& cell, children_) {
		
		if(cell.widget() && cell.widget()->can_wrap()) {
			cell.widget()->clear_width_constrain();
		} 
	}
	
	// Inherit
	twidget::clear_width_constrain();
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

void tgrid::draw(surface& surface, const bool force, 
		const bool invalidate_background)
{
	for(iterator itor = begin(); itor != end(); ++itor) {

		log_scope2(gui_draw, "Grid: draw child.");

		assert(*itor);
		itor->draw(surface, force, invalidate_background);
	}

	set_dirty(false);
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
			clear_cache();
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
			clear_cache();
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

void tgrid::set_size(const SDL_Rect& rect)
{
	log_scope2(gui_layout, "tgrid: set size");

	/***** INIT *****/

	twidget::set_size(rect);

	if(!rows_ || !cols_) {
		return;
	}

	const tpoint orig(rect.x, rect.y);
	const tpoint size(rect.w, rect.h);

	const tpoint best_size = get_best_size();
	row_height_ = best_row_height_;
	col_width_ = best_col_width_;

	assert(row_height_.size() == rows_);
	assert(col_width_.size() == cols_);
	assert(row_grow_factor_.size() == rows_);
	assert(col_grow_factor_.size() == cols_);
	DBG_G_L << "tgrid: best size " << best_size << " available size " << size << ".\n";

	/***** BEST_SIZE *****/

	if(best_size == size) {
		row_height_ = best_row_height_;
		col_width_ = best_col_width_;

		layout(orig);
		return;
	}

	/***** GROW *****/

	if(best_size < size) {
		row_height_ = best_row_height_;
		col_width_ = best_col_width_;

		// expand it.
		if(size.x > best_size.x) {
			const unsigned w = size.x - best_size.x;
			unsigned w_size = 
				std::accumulate(col_grow_factor_.begin(), col_grow_factor_.end(), 0);
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
				DBG_G_L << "tgrid: column " << i << " with grow factor " 
					<< col_grow_factor_[i] << " set width to " << col_width_[i] << ".\n";
			}

		}

		if(size.y > best_size.y) {
			const unsigned h = size.y - best_size.y;
			unsigned h_size = 
				std::accumulate(row_grow_factor_.begin(), row_grow_factor_.end(), 0);
			DBG_G_L << "tgrid: extra height " << h << " will be divided amount " 
				<< h_size << " units in " << rows_ << " rows.\n";

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
				DBG_G_L << "tgrid: row " << i  << " with grow factor "
					<< row_grow_factor_[i] << " set height to " << row_height_[i] << ".\n";
			}
		}

		layout(orig);
		return;

	}

	/***** SHRINK SCROLLBAR *****/

	if((best_size.x <= size.x /*|| has_horizontal_scrollbar()*/) 
			&& (best_size.y <= size.y || has_vertical_scrollbar())) {

		tpoint new_size = get_best_size(size);
		assert(new_size <= size);

		row_height_ = best_row_height_;
		col_width_ = best_col_width_;
		layout(orig);
		return;
	}




	// FIXME make other cases work as well
	assert(false);
/*
	const tpoint minimum_size = get_minimum_size();
	if(minimum_size == size) {
*/		
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
	clear_cache();
}

void tgrid::set_dirty(const bool dirty)
{
	// Inherited.
	twidget::set_dirty(dirty);

	if(!dirty) {
		for(std::vector<tchild>::iterator itor = children_.begin();
				itor != children_.end(); ++itor) {

			if(itor->widget()) {
				itor->widget()->set_dirty(dirty);
			}
		}
	}
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
#if 0 // HACK
#if disable_cache
	if(true) {
#else	
	if(widget_->is_dirty() || best_size_ == tpoint(0, 0)) {
#endif	
		best_size_ = widget_->get_best_size() + border_space();
	}
#else
if(disable_cache || widget_->is_dirty() || best_size_ == tpoint(0, 0)) {
	best_size_ = widget_->get_best_size() + border_space();
}
#endif // HACK

	DBG_G_L << "tgrid::tchild:"
		<< " has widget " << true
		<< " returning " << best_size_ 
		<< ".\n";
	return best_size_;
}

tpoint tgrid::tchild::get_best_size(const tpoint& maximum) const
{
	log_scope2(gui_layout, std::string("tgrid::tchild ") + __func__);
	
	if(!widget_) {
		DBG_G_L << "tgrid::tchild:"
			<< " has widget " << false
			<< " maximum size " << maximum 
			<< " returning " << border_space()
			<< ".\n";
		return border_space();
	}

	best_size_ =  widget_->get_best_size(maximum - border_space()) + border_space();

	DBG_G_L << "tgrid::tchild:"
		<< " has widget " << true
		<< " maximum size " << maximum 
		<< " returning " << best_size_ 
		<< ".\n";
	return best_size_;
}

tpoint tgrid::tchild::get_minimum_size() const
{
	if(!widget_) {
		return border_space();
	}

	if(widget_->is_dirty() || minimum_size_ == tpoint(0, 0)) {
		minimum_size_ = widget_->get_minimum_size() + border_space();
	}

	return minimum_size_;
}

tpoint tgrid::tchild::get_maximum_size() const
{
	if(!widget_) {
		return tpoint(0, 0);
	}

	if(widget_->is_dirty() || maximum_size_ == tpoint(0, 0)) {
		maximum_size_ = widget_->get_maximum_size();

		// If the widget has no maximum return that 
		// else we need to add the border.
		if(maximum_size_ != tpoint(0, 0)) {
			maximum_size_ += border_space();
		}
	}

	return maximum_size_;
}

bool tgrid::tchild::set_width_constrain(const unsigned width)
{
	assert(widget_ && widget_->can_wrap());

	return widget_->set_width_constrain(width - border_space().x); 
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

void tgrid::clear_cache()
{
	best_row_height_.clear();
	best_col_width_.clear();

	minimum_row_height_.clear();
	minimum_col_width_.clear();
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
void tgrid::tchild::set_size(tpoint orig, tpoint size)
{
	assert(widget());

	if(border_size_) {
		if(flags_ & BORDER_TOP) {
			orig.y += border_size_;
			size.y -= border_size_;
		}
		if(flags_ & BORDER_BOTTOM) {
			size.y -= border_size_;
		}
				
		if(flags_ & BORDER_LEFT) {
			orig.x += border_size_;
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
			<< orig << " x " << size << ".\n";

		widget()->set_size(create_rect(orig, size));
		return;
	}

	const tpoint maximum_size = widget()->get_maximum_size();
	if((flags_ & (HORIZONTAL_MASK | VERTICAL_MASK))
			== (HORIZONTAL_GROW_SEND_TO_CLIENT | VERTICAL_GROW_SEND_TO_CLIENT)) {

		if(maximum_size == tpoint(0,0) || size <= maximum_size) {
	
			DBG_G_L << "tgrid::tchild: in maximum size range setting widget to " 
				<< orig << " x " << size << ".\n";

			widget()->set_size(create_rect(orig, size));
			return;
	
		}
	}

	tpoint widget_size = tpoint(
		std::min(size.x, best_size.x), 
		std::min(size.y, best_size.y));
	tpoint widget_orig = orig;

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


	widget()->set_size(create_rect(widget_orig, widget_size));
}

tpoint tgrid::get_size(const std::string& id, std::vector<unsigned>& width, 
		std::vector<unsigned>& height, 
		tpoint (tchild::*size_proc)() const,
		tpoint (tchild::*size_proc_max)(const tpoint&) const,
		const tpoint& maximum_size) const
{
#if 0 // HACK
#if DISABLE_CACHE
	height.clear();
	width.clear();
#endif	
#else 
if(disable_cache) {
	height.clear();
	width.clear();
}
#endif // HACK

	if(height.empty() || width.empty() || maximum_size != tpoint(0, 0)) {

		DBG_G_L << "tgrid: calculate " << id << " size.\n";

		height.resize(rows_, 0);
		width.resize(cols_, 0);
		
		// First get the sizes for all items.
		for(unsigned row = 0; row < rows_; ++row) {
			for(unsigned col = 0; col < cols_; ++col) {

				assert(size_proc || size_proc_max);

				const tpoint size = size_proc
					? (child(row, col).*size_proc)()
					: (child(row, col).*size_proc_max)(maximum_size);

				if(size.x > static_cast<int>(width[col])) {
					width[col] = size.x;
				}

				if(size.y > static_cast<int>(height[row])) {
					height[row] = size.y;
				}

			}
		}
	} else {
		DBG_G_L << "tgrid: used cached " << id << " size.\n";
	}

	for(unsigned row = 0; row < rows_; ++row) {
		DBG_G_L << "tgrid: the " << id << " height for row " << row 
			<< " will be " << height[row] << ".\n";
	}

	for(unsigned col = 0; col < cols_; ++col) {
		DBG_G_L << "tgrid: the " << id << " width for col " << col 
			<< " will be " << width[col]  << ".\n";
	}

	const tpoint result(
		std::accumulate(width.begin(), width.end(), 0),
		std::accumulate(height.begin(), height.end(), 0));
	
	DBG_G_L << "tgrid: returning " << result << ".\n";
	return result;
}

unsigned tgrid::get_best_row_height(const unsigned row, const unsigned maximum_height) const
{
	// The minimum height required.
	unsigned required_height = 0;

	for(size_t x = 0; x < cols_; ++x) {

		const tchild& cell = child(row, x);

		const tpoint size = cell.get_best_size(tpoint(0, maximum_height));
		if(required_height == 0 || static_cast<size_t>(size.y) > required_height) {
			required_height = size.y;
		}
	}

	DBG_G_L << "tgrid: maximum row height " << maximum_height << " returning " 
		<< required_height << ".\n";

	return required_height;
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
