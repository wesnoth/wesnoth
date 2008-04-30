/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
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

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)


namespace gui2 {

tgrid::tgrid(const unsigned rows, const unsigned cols, 
		const unsigned default_flags, const unsigned default_border_size) :
	rows_(rows),
	cols_(cols),
	default_flags_(default_flags),
	default_border_size_(default_border_size),
	best_row_height_(),
	best_col_width_(),
	minimum_row_height_(),
	minimum_col_width_(),
	row_height_(),
	col_width_(),
	row_scaling_(),
	col_scaling_(),
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

void tgrid::add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size) 
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	// clear old child if any
	if(cell.widget()) {
		// free a child when overwriting it
		WRN_G << "Grid: child '" << cell.id() 
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
		WRN_G << "Grid: resizing a non-empty grid may give unexpected problems.\n";
	}

	rows_ = rows;
	cols_ = cols;
	row_scaling_.resize(rows);
	col_scaling_.resize(cols);
	children_.resize(rows_ * cols_);
	clear_cache();
}

void tgrid::remove_child(const unsigned row, const unsigned col)
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	cell.set_id("");
	cell.set_widget(0);
	clear_cache();
}

void tgrid::remove_child(const std::string& id, const bool find_all)
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->id() == id) {
			itor->set_id("");
			itor->set_widget(0);
			clear_cache();

			if(!find_all) {
				break;
			}
		}
	}
}

tpoint tgrid::get_best_size() const
{

	if(best_row_height_.empty() || best_col_width_.empty()) {

		DBG_G << "Grid: calculate best size.\n";

		best_row_height_.resize(rows_, 0);
		best_col_width_.resize(cols_, 0);
		
		// First get the best sizes for all items.
		for(unsigned row = 0; row < rows_; ++row) {
			for(unsigned col = 0; col < cols_; ++col) {

				tpoint size = child(row, col).get_best_size();

				if(size.x > best_col_width_[col]) {
					best_col_width_[col] = size.x;
				}

				if(size.y > best_row_height_[row]) {
					best_row_height_[row] = size.y;
				}

			}
		}
	} else {
		DBG_G << "Grid: used cached best size.\n";
	}

	for(unsigned row = 0; row < rows_; ++row) {
		DBG_G << "Grid: the best height for row " << row 
			<< " will be " << best_row_height_[row] << ".\n";
	}

	for(unsigned col = 0; col < cols_; ++col) {
		DBG_G << "Grid: the best width for col " << col 
			<< " will be " << best_col_width_[col]  << ".\n";
	}

	return tpoint(
		std::accumulate(best_col_width_.begin(), best_col_width_.end(), 0),
		std::accumulate(best_row_height_.begin(), best_row_height_.end(), 0));
}

void tgrid::set_size(const SDL_Rect& rect)
{
	log_scope2(gui, "Grid: set size");

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
	assert(row_scaling_.size() == rows_);
	assert(col_scaling_.size() == cols_);
	if(best_size == size) {
		row_height_ = best_row_height_;
		col_width_ = best_col_width_;

		layout(orig);
		return;
	}

	if(best_size < size) {
		row_height_ = best_row_height_;
		col_width_ = best_col_width_;

		// expand it.
		if(size.x > best_size.x) {
			const unsigned w = size.x - best_size.x;
			unsigned w_size = std::accumulate(col_scaling_.begin(), col_scaling_.end(), 0);
			DBG_G << "Grid: extra width " << w << " will be divided amount " << w_size << " units in " << cols_ << " columns.\n";

			if(w_size == 0) {
				// If all sizes are 0 reset them to 1
				foreach(unsigned& val, col_scaling_) {
					val = 1;
				}
				w_size = cols_;
			}
			// We might have a bit 'extra' if the division doesn't fix exactly
			// but we ignore that part for now.
			const unsigned w_normal = w / w_size;
			for(unsigned i = 0; i < cols_; ++i) {
				col_width_[i] += w_normal * col_scaling_[i];
				DBG_G << "Grid: column " << i << " with scale factor " 
					<< col_scaling_[i] << " set width to " << col_width_[i] << ".\n";
			}

		}

		if(size.y > best_size.y) {
			const unsigned h = size.y - best_size.y;
			unsigned h_size = std::accumulate(row_scaling_.begin(), row_scaling_.end(), 0);
			DBG_G << "Grid: extra height " << h << " will be divided amount " << h_size << " units in " << rows_ << " rows.\n";

			if(h_size == 0) {
				// If all sizes are 0 reset them to 1
				foreach(unsigned& val, row_scaling_) {
					val = 1;
				}
				h_size = rows_;
			}
			// We might have a bit 'extra' if the division doesn't fix exactly
			// but we ignore that part for now.
			const unsigned h_normal = h / h_size;
			for(unsigned i = 0; i < rows_; ++i) {
				row_height_[i] += h_normal * row_scaling_[i];
				DBG_G << "Grid: row " << i  << " with scale factor "
					<< row_scaling_[i] << " set height to " << row_height_[i] << ".\n";
			}
		}

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

twidget* tgrid::get_widget(const tpoint& coordinate)
{
	//! FIXME use iterator.
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

			twidget* widget = child(row, col).widget();
			if(!widget) {
				continue;
			}
			
			widget = widget->get_widget(coordinate);
			if(widget) { 
				clear_cache();
				return widget;
			}
		}
	}
	
	return 0;
}

//! Gets a widget with the wanted id.
//! Override base class.
twidget* tgrid::get_widget_by_id(const std::string& id)
{
	//! FIXME use iterator.
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

			twidget* widget = child(row, col).widget();
			if(!widget) {
				continue;
			}
			
			widget = widget->get_widget_by_id(id);
			if(widget) { 
				clear_cache();
				return widget;
			}
		}
	}
	
	return twidget::get_widget_by_id(id);
}

void tgrid::draw(surface& surface)
{
	for(iterator itor = begin(); itor != end(); ++itor) {
		if(! *itor || !itor->dirty()) {
			continue;
		}

		log_scope2(gui_draw, "Grid: draw child.");

		itor->draw(surface);
	}

	set_dirty(false);
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
			DBG_G << "Grid: set widget at " << row << ',' << col 
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

tpoint tgrid::tchild::get_best_size() const
{
	if(!widget_) {
		return tpoint(0, 0);
	}

	if(widget_->dirty() || best_size_ == tpoint(0, 0)) {

		best_size_ = widget_->get_best_size();

		if(border_size_) {

			if(flags_ & BORDER_TOP) best_size_.y += border_size_;
			if(flags_ & BORDER_BOTTOM) best_size_.y += border_size_;
					
			if(flags_ & BORDER_LEFT) best_size_.x += border_size_;
			if(flags_ & BORDER_RIGHT) best_size_.x += border_size_;
		}
	}

	return best_size_;
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
		DBG_G << "Grid cell: in best size range setting widget to " 
			<< orig << " x " << size << ".\n";

		widget()->set_size(create_rect(orig, size));
		return;
	}

	const tpoint maximum_size = widget()->get_maximum_size();
	if(flags_ & (HORIZONTAL_GROW_SEND_TO_CLIENT | HORIZONTAL_GROW_SEND_TO_CLIENT)) {
		if(maximum_size == tpoint(0,0) || size <= maximum_size) {
	
			DBG_G << "Grid cell: in maximum size range setting widget to " 
				<< orig << " x " << size << ".\n";

			widget()->set_size(create_rect(orig, size));
			return;
	
		}
	}

	tpoint widget_size = best_size;
	tpoint widget_orig = orig;

	if(flags_ & HORIZONTAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.x) {
			widget_size.x = std::min(size.x, maximum_size.x);
		} else {
			widget_size.x = size.x;
		}
		DBG_G << "Grid cell: horizontal growing from " << best_size.x << " to " << widget_size.x << ".\n";
	}

	if(flags_ & VERTICAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.y) {
			widget_size.y = std::min(size.y, maximum_size.y);
		} else {
			widget_size.y = size.y;
		}
		DBG_G << "Grid cell: vertical growing from " << best_size.y << " to " << widget_size.y << ".\n";
	}

	if((flags_ & VERTICAL_ALIGN_TOP) == VERTICAL_ALIGN_TOP) {
		// Do nothing.
		
		DBG_G << "Grid cell: vertically aligned at the top.\n";

	} else if((flags_ & VERTICAL_ALIGN_CENTER) == VERTICAL_ALIGN_CENTER) {
		
		widget_orig.y += (size.y - widget_size.y) / 2;
		DBG_G << "Grid cell: vertically centred.\n";

	} else if((flags_ & VERTICAL_ALIGN_BOTTOM) == VERTICAL_ALIGN_BOTTOM) {

		widget_orig.y += (size.y - widget_size.y);
		DBG_G << "Grid cell: vertically aligned at the bottom.\n";

	} else {
		assert(false);
	}
	
	if((flags_ & HORIZONTAL_ALIGN_LEFT) == HORIZONTAL_ALIGN_LEFT) {
		// Do nothing.
		DBG_G << "Grid cell: horizontally aligned at the left.\n";

	} else if((flags_ & HORIZONTAL_ALIGN_CENTER) == HORIZONTAL_ALIGN_CENTER) {
		
		widget_orig.x += (size.x - widget_size.x) / 2;
		DBG_G << "Grid cell: horizontally centred.\n";

	} else if((flags_ & HORIZONTAL_ALIGN_RIGHT) == HORIZONTAL_ALIGN_RIGHT) {

		widget_orig.x += (size.x - widget_size.x);
		DBG_G << "Grid cell: horizontally aligned at the right.\n";

	} else {
		assert(false);
	}

	DBG_G << "Grid cell: resize widget to " 
		<< widget_orig << " x " << widget_size << ".\n";


	widget()->set_size(create_rect(widget_orig, widget_size));
}

} // namespace gui2


