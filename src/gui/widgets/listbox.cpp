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

#include "gui/widgets/listbox.hpp"

#include "foreach.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "log.hpp"

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

static tlistbox* get_listbox(twidget* widget)
{
	do {
		widget = widget->parent();

	} while (widget && !dynamic_cast<tlistbox*>(widget));
	
	tlistbox* listbox = dynamic_cast<tlistbox*>(widget);
	assert(listbox);
	return listbox;
}

static void callback_select_list_item(twidget* caller)
{
	get_listbox(caller)->list_item_selected(caller);
}

static void callback_scrollbar(twidget* caller)
{
	get_listbox(caller)->scrollbar_moved(caller);
}

tlistbox::tlistbox() :
	tcontainer_(COUNT),
	state_(ENABLED),
	list_builder_(0),
	assume_fixed_row_size_(true),
	selected_row_(-1),
	selection_count_(0),
	row_select_(true),
	must_select_(true),
	multi_select_(false),
	list_rect_(),
	list_background_(),
	rows_()
{
	load_config();
}

void tlistbox::list_item_selected(twidget* caller)
{
	for(unsigned i = 0; i < scrollbar()->get_visible_items(); ++i) {

		const unsigned row = i + scrollbar()->get_item_position();

		assert(rows_[row].grid());
		if(rows_[row].grid()->has_widget(caller)) {

			if(!select_row(row, !rows_[row].get_selected())) {
				// if not allowed to deselect reselect.
				tselectable_* selectable = dynamic_cast<tselectable_*>(caller);
				assert(selectable);
				selectable->set_selected();	
			}

			return;
		}
	}

	// we aren't supposed to get here.
	assert(false);
}

void tlistbox::finalize_setup()
{
	// If we have a list already set up wire in the callback routine.
	tgrid* list = dynamic_cast<tgrid*>(get_widget_by_id("_list"));
	if(list) {
		const unsigned col_count = list->get_cols();
		const unsigned row_count = list->get_rows();

		// We need to validate the stuff put inside us, we expect
		// * panels or grids with more nested items.
		// * selectable items.
		for(unsigned row = 0; row < row_count; ++row) {
			for(unsigned col = 0; col < col_count; ++col) {
				twidget* widget = list->widget(row, col);
				assert(widget);


				tgrid* grid = dynamic_cast<tgrid*>(widget);
				tselectable_* selectable = dynamic_cast<tselectable_*>(widget);

				if(selectable) {
					
					// FIXME move the callback also in the selectable class
					ttoggle_button* btn = dynamic_cast<ttoggle_button*>(widget);
					assert(btn);

					btn->set_callback_mouse_left_click(callback_select_list_item);

				} else if(grid) {
					// Grid not allowed atm.
					assert(false);
				} else {
					std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
					assert(false);
				}
			}
		}
	} else {
		tspacer* spacer = dynamic_cast<tspacer*>(get_widget_by_id("_list"));
		assert(spacer);
	}

	scrollbar()->set_callback_positioner_move(callback_scrollbar);
}

/**
 * Helper function to avoid a const problem.
 *
 * get_best_size is const but we use a spacer so we can use the generic routine.
 * So we drop the const and set the size of the spacer, a bit and a cleaner 
 * solution might be needed. We could make set_best_size() const but 
 * get_widget_by_id is also not available in a const version.
 *
 * @param size                    The new size for the spacer.
 * @param const_grid              The grid containing a spacer with id "_list".
 */
static void set_spacer_size(const tpoint& size, const tgrid& const_grid)
{
	tgrid& grid = const_cast<tgrid&>(const_grid);

	tspacer* spacer = dynamic_cast<tspacer*>(grid.get_widget_by_id("_list"));
	assert(spacer);
	spacer->set_best_size(size);
}

tpoint tlistbox::get_best_size() const
{
	// Set the size of the spacer to the wanted size for the list.
	unsigned width = 0;
	unsigned height = 0;

	// NOTE we should look at the number of visible items etc
	foreach(const trow& row, rows_) {
		assert(row.grid());
		const tpoint best_size = row.grid()->get_best_size();
		width = width >= best_size.x ? width : best_size.x;

		height += best_size.y;
	}

	// Hack to limit us to 15 items.
	if(height) { // if we have a height there are items a height there are items.
		height = 15 * height / get_item_count();
	}

	set_spacer_size(tpoint(width, height), grid());

	// Now the container will return the wanted result.
	return tcontainer_::get_best_size();
}

void tlistbox::draw(surface& surface)
{
	// Inherit.
	tcontainer_::draw(surface);

	// Handle our full redraw for the spacer area.
	if(!list_background_) {
		list_background_.assign(gui2::save_background(surface, list_rect_));
	} else {
		gui2::restore_background(list_background_, surface, list_rect_);
	}
	
	// Now paint the list over the spacer.
	unsigned offset = list_rect_.y;
	for(unsigned i = 0; i < scrollbar()->get_visible_items(); ++i) {
		trow& row = rows_[i + scrollbar()->get_item_position()];

		assert(row.grid());
		if(row.grid()->dirty()) {
			row.grid()->draw(row.canvas());
		}
		
		// draw background
		const SDL_Rect rect = {list_rect_.x, offset, list_rect_.w, row.get_height() };

		// draw widget
		blit_surface(row.canvas(), 0, surface, &rect);

		offset += row.get_height();
	}
}

void tlistbox::set_size(const SDL_Rect& rect)
{
	// Inherit.
	tcontainer_::set_size(rect);

	// Now set the items in the spacer.
	tspacer* spacer = dynamic_cast<tspacer*>(get_widget_by_id("_list"));
	assert(spacer);
	list_rect_ = spacer->get_rect();

	foreach(trow& row, rows_) {
		assert(row.grid());

		const unsigned height = row.grid()->get_best_size().y;
		row.set_height(height);

		row.grid()->set_size(::create_rect(0, 0, list_rect_.w, height));
		row.canvas().assign(SDL_CreateRGBSurface(SDL_SWSURFACE, 
			list_rect_.w, height, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000));
	}
}

twidget* tlistbox::get_widget(const tpoint& coordinate)
{ 
	// Inherited
	twidget* result = tcontainer_::get_widget(coordinate);

	// if on the panel we need to do special things.
	if(result && result->id() == "_list") {

		int offset = coordinate.y - list_rect_.y;
		assert(offset >= 0);
		for(unsigned i = 0; i < scrollbar()->get_visible_items(); ++i) {

			trow& row = rows_[i + scrollbar()->get_item_position()];
			
			if(offset < row.get_height()) {
				assert(row.grid());
				return row.grid()->get_widget(
					tpoint(coordinate.x - list_rect_.x, offset));
			} else {
				offset -= row.get_height();
			}
		}
	}

	return result;
}

void tlistbox::add_item(const t_string& label)
{
	std::cerr << "Adding '" << label << "'.\n";

	assert(list_builder_);

	trow row(*list_builder_, label);
	assert(row.grid());

	row.grid()->set_parent(this);
	rows_.push_back(row);

	if(must_select_ && !selection_count_) {
		select_row(get_item_count() - 1);
	}

	scrollbar()->set_item_count(get_item_count());

	// Hack to limit the number of visible items.
	if(get_item_count() <= 15) {
		scrollbar()->set_visible_items(get_item_count());
	}
}

tscrollbar_* tlistbox::scrollbar()
{
	// Note we don't cache the result, we might want change things later.	
	tscrollbar_* result = dynamic_cast<tscrollbar_*>(get_widget_by_id("_scrollbar"));
	assert(result);
	return result;
}

bool tlistbox::select_row(const unsigned row, const bool select)
{
/*
	std::cerr << "Selecting row " << row << " select " << select 
		<< " must_select " << must_select_
		<< " multi_select " << multi_select_
		<< " selection_count " << selection_count_ << ".\n";
*/
	if(!select && must_select_ && selection_count_ < 2) {
		return false;
	}

	if((select && rows_[row].get_selected()) 
		|| (!select && !rows_[row].get_selected())) {
		return true;
	}

	if(select && !multi_select_ && selection_count_ == 1) {
		assert(selected_row_ < get_item_count());
		rows_[selected_row_].select(false);
		--selection_count_;
	}

	if(select) {
		++selection_count_;
	} else {
		--selection_count_;
	}

	assert(row < get_item_count());
	selected_row_ = row;
	rows_[row].select();

	return true;
}

tlistbox::trow::trow(const tbuilder_grid& list_builder_,const t_string& label) :
	grid_(dynamic_cast<tgrid*>(list_builder_.build())),
	height_(0),
	selected_(false)
{
	assert(grid_);
	init_in_grid(grid_, label);
}

void tlistbox::trow::init_in_grid(tgrid* grid, const t_string& label) 
{
	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);


			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			ttoggle_button* btn = dynamic_cast<ttoggle_button*>(widget);

			if(btn) {
				btn->set_callback_mouse_left_click(callback_select_list_item);
				btn->set_label(label);
			} else if(grid) {
				init_in_grid(child_grid, label);
			} else {
				std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
				assert(false);
			}
		}
	}
}

void tlistbox::trow::select(const bool sel) 
{
	selected_ = sel;
	assert(grid_);
	select_in_grid(grid_, sel);
}

void tlistbox::trow::select_in_grid(tgrid* grid, const bool sel)
{
	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);

			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			tselectable_* selectable = dynamic_cast<tselectable_*>(widget);

			if(selectable) {
				selectable->set_selected(sel);
			} else if(grid) {
				select_in_grid(child_grid, sel);
			} else {
				std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
				assert(false);
			}
		}
	}
}

} // namespace gui2


