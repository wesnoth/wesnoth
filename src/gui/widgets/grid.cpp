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

unsigned tgrid::add_row(const unsigned count)
{
	assert(count);

	//FIXME the warning in set_rows_cols should be killed.
	
	unsigned result = rows_;
	set_rows_cols(rows_ + count, cols_);
	return result;
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
	row_grow_factor_.resize(rows);
	col_grow_factor_.resize(cols);
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

bool tgrid::has_vertical_scrollbar() const 
{
	for(std::vector<tchild>::const_iterator itor = children_.begin();
			itor != children_.end(); ++itor) {
		// FIXME we should check per row and the entire row
		// should have the flag!!!!
		if(itor->widget()) {
			 std::cerr << "Widget type " << typeid(*(itor->widget())).name()
			 	<< "has scrollbar " 
				<< itor->widget()->has_vertical_scrollbar() << ".\n";
		}
		if(itor->widget() && itor->widget()->has_vertical_scrollbar()) {
			return true;
		} 

	}
	
	// Inherit
	return twidget::has_vertical_scrollbar();
}

tpoint tgrid::get_minimum_size() const
{
	return get_size("minimum", minimum_col_width_, 
		minimum_row_height_, &tchild::get_minimum_size);
}

tpoint tgrid::get_maximum_size() const
{
	// A grid doesn't have a maximum size.
	return tpoint(0,0);
}

tpoint tgrid::get_best_size() const
{
	return get_size("best", best_col_width_, 
		best_row_height_, &tchild::get_best_size);
}

//! Helper function to get the best or minimum size.
//!
//! @param id                     Name to use in debug output.
//! @param width                  Reference to the vector width cache for the 
//!                               size function of the caller.
//! @param height                 Reference to the vector height cache for the 
//!                               size function of the caller.
//! @param size_proc              The function to call on the cells in order to
//!                               get their sizes.
//!
//! @return                       The wanted size.
tpoint tgrid::get_size(const std::string& id, std::vector<unsigned>& width, 
		std::vector<unsigned>& height, tpoint (tchild::*size_proc)() const) const
{
	if(height.empty() || width.empty()) {

		DBG_G << "Grid: calculate " << id << " size.\n";

		height.resize(rows_, 0);
		width.resize(cols_, 0);
		
		// First get the sizes for all items.
		for(unsigned row = 0; row < rows_; ++row) {
			for(unsigned col = 0; col < cols_; ++col) {

				const tpoint size = (child(row, col).*size_proc)();

				if(size.x > width[col]) {
					width[col] = size.x;
				}

				if(size.y > height[row]) {
					height[row] = size.y;
				}

			}
		}
	} else {
		DBG_G << "Grid: used cached " << id << " size.\n";
	}

	for(unsigned row = 0; row < rows_; ++row) {
		DBG_G << "Grid: the " << id << " height for row " << row 
			<< " will be " << height[row] << ".\n";
	}

	for(unsigned col = 0; col < cols_; ++col) {
		DBG_G << "Grid: the " << id << " width for col " << col 
			<< " will be " << width[col]  << ".\n";
	}

	return tpoint(
		std::accumulate(width.begin(), width.end(), 0),
		std::accumulate(height.begin(), height.end(), 0));
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
	assert(row_grow_factor_.size() == rows_);
	assert(col_grow_factor_.size() == cols_);
	DBG_G << "Grid: best size " << best_size << " available size " << size << ".\n";

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
			unsigned w_size = 
				std::accumulate(col_grow_factor_.begin(), col_grow_factor_.end(), 0);
			DBG_G << "Grid: extra width " << w << " will be divided amount " 
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
				DBG_G << "Grid: column " << i << " with grow factor " 
					<< col_grow_factor_[i] << " set width to " << col_width_[i] << ".\n";
			}

		}

		if(size.y > best_size.y) {
			const unsigned h = size.y - best_size.y;
			unsigned h_size = 
				std::accumulate(row_grow_factor_.begin(), row_grow_factor_.end(), 0);
			DBG_G << "Grid: extra height " << h << " will be divided amount " 
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
				DBG_G << "Grid: row " << i  << " with grow factor "
					<< row_grow_factor_[i] << " set height to " << row_height_[i] << ".\n";
			}
		}

		layout(orig);
		return;

	}

	if((best_size.x <= size.x /*|| has_horizontal_scrollbar()*/) 
			&& (best_size.y <= size.y || has_vertical_scrollbar())) {

		// FIXME we only do the height atm, the width will be added when needed.
		const unsigned over_shoot = best_size.y - size.y;

		bool set = false;
		row_height_ = best_row_height_;
		col_width_ = best_col_width_;
		// FIXME we assume 1 item per row.
		for(unsigned i = 0; i < rows_; ++i) {
			twidget* row = widget(i, 0);
			if(row && row->has_vertical_scrollbar()) {
				row_height_[i] -= over_shoot; // Assume this row can be resized enough
				set = true;
				break;
			}
			
		}

		assert(set);
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
	for(std::vector<tchild>::iterator itor = children_.begin(); 
			itor != children_.end(); ++itor) {

		twidget* widget = itor->widget();
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
	for(std::vector<tchild>::const_iterator itor = children_.begin(); 
			itor != children_.end(); ++itor) {

		const twidget* widget = itor->widget();
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
		return border_space();
	}

	if(widget_->dirty() || best_size_ == tpoint(0, 0)) {
		best_size_ = widget_->get_best_size() + border_space();
	}

	return best_size_;
}

tpoint tgrid::tchild::get_minimum_size() const
{
	if(!widget_) {
		return border_space();
	}

	if(widget_->dirty() || minimum_size_ == tpoint(0, 0)) {
		minimum_size_ = widget_->get_minimum_size() + border_space();
	}

	return minimum_size_;
}

tpoint tgrid::tchild::get_maximum_size() const
{
	if(!widget_) {
		return tpoint(0, 0);
	}

	if(widget_->dirty() || maximum_size_ == tpoint(0, 0)) {
		maximum_size_ = widget_->get_maximum_size();

		// If the widget has no maximum return that 
		// else we need to add the border.
		if(maximum_size_ != tpoint(0, 0)) {
			maximum_size_ += border_space();
		}
	}

	return maximum_size_;
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
		DBG_G << "Grid cell: horizontal growing from " 
			<< best_size.x << " to " << widget_size.x << ".\n";
	}

	if(flags_ & VERTICAL_GROW_SEND_TO_CLIENT) {
		if(maximum_size.y) {
			widget_size.y = std::min(size.y, maximum_size.y);
		} else {
			widget_size.y = size.y;
		}
		DBG_G << "Grid cell: vertical growing from " 
			<< best_size.y << " to " << widget_size.y << ".\n";
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


/*WIKI
 * @page = GUILayout
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
 * [[Category:WML Reference]]
 * [[Category:Generated]]
 */
