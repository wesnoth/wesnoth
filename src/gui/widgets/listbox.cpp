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

static void callback_select_list_item(twidget* caller)
{
	twidget* parent = caller;
	do {
		parent = parent->parent();

	} while (parent && !dynamic_cast<tlistbox*>(parent));
	
	tlistbox* listbox = dynamic_cast<tlistbox*>(parent);
	assert(listbox);
	listbox->list_item_selected(caller);
}

void tlistbox::list_item_selected(twidget* caller)
{
	tgrid* scroll = dynamic_cast<tgrid*>(get_widget_by_id("_list"));
	assert(scroll);
	
	const unsigned col_count = scroll->get_cols();
	assert(col_count == 1); // We can't handle multiple columns properly yet
	const unsigned row_count = scroll->get_rows();
	unsigned clicked_row = -1;

	for(unsigned row = 0; row < row_count; ++row) {
		for(unsigned col = 0; col < col_count; ++col) {
			twidget* widget = scroll->widget(row, col);
			if(dynamic_cast<tgrid*>(widget)) {
				widget = dynamic_cast<tgrid*>(widget)->widget(0, 0);
			}
			if(widget == caller) {
				clicked_row = row;
				break;
			} 
		}
		if(clicked_row != -1) {
			break;
		}
	}

	assert(clicked_row != -1);

	// Don't allow deselecting atm.
	if(clicked_row == selected_row_) {
		tselectable_* selectable = dynamic_cast<tselectable_*>(caller);
		assert(selectable);
		selectable->set_selected();	
	} else {
		if(selected_row_ != -1) {
			select_row(selected_row_, false);
		}
		select_row(clicked_row);
	}
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
	}
}

void tlistbox::add_item(const std::string& label)
{
	std::cerr << "Adding '" << label << "'.\n";

	assert(list_builder_);
	tgrid* item = dynamic_cast<tgrid*>(list_builder_->build());
	assert(item);

	for(tgrid::iterator itor = item->begin(); itor != item->end(); ++itor) {
		assert(dynamic_cast<ttoggle_button*>(*itor));
		dynamic_cast<ttoggle_button*>(*itor)->set_label(label);
		dynamic_cast<ttoggle_button*>(*itor)->set_callback_mouse_left_click(callback_select_list_item);
	}

	tgrid* list = dynamic_cast<tgrid*>(get_widget_by_id("_list"));
	unsigned index = 0;
	if(!list) {
		// we should figure out how much columns are wanted but use one now.

		list = new tgrid();
		assert(list);

		list->set_rows_cols(1, 1);
		list->set_id("_list");

		grid().add_child(list, 0, 0, 
			tgrid::VERTICAL_GROW_SEND_TO_CLIENT 
			| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
			| tgrid::VERTICAL_ALIGN_CENTER
			| tgrid::HORIZONTAL_ALIGN_CENTER
			, 0);

		index = 0;
 
	} else {
		index = list->add_row();
	}

	list->add_child(item, index, 0, 
		tgrid::VERTICAL_GROW_SEND_TO_CLIENT 
		| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
		| tgrid::VERTICAL_ALIGN_CENTER
		| tgrid::HORIZONTAL_ALIGN_CENTER
		, 0);
}

unsigned tlistbox::get_item_count() /* const */
{
	/*const*/ tgrid* grid = dynamic_cast</*const*/ tgrid*>(get_widget_by_id("_list"));
	if(!grid) {
		return 0;
	} else {
		return grid->get_rows();
	}
}

void tlistbox::select_row(const unsigned row, const bool select)
{
	std::cerr << "Selecting row " << row << " select " << select << ".\n";

	assert(row != -1);
	assert(row < get_item_count());

	// Find the list area.
	tgrid* grid = dynamic_cast<tgrid*>(get_widget_by_id("_list"));
	assert(grid);

	selected_row_ = select ? row : -1;

	for(unsigned i = 0; i < grid->get_cols(); ++i) {
		select_cell(row, i, select);
	}
}

void tlistbox::select_cell(const unsigned row, const unsigned column, const bool select)
{
	// Find the list area.
	tgrid* grid = dynamic_cast<tgrid*>(get_widget_by_id("_list"));
	assert(grid);

	// Select the cell this should be a grid as well.
	grid = dynamic_cast<tgrid*>(grid->widget(row, column));
	assert(grid);

	// This grid should only contain tselectable items or other grids.
	select_in_grid(grid, select);
}

void tlistbox::select_in_grid(tgrid* grid, const bool select)
{
	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);

			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			tselectable_* selectable = dynamic_cast<tselectable_*>(widget);

			if(selectable) {
				selectable->set_selected(select);
			} else if(grid) {
				select_in_grid(child_grid, select);
			} else {
				std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
				assert(false);
			}
		}
	}
}

} // namespace gui2


