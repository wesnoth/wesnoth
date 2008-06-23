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
#include "gui/widgets/button.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
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

static void callback_scrollbar_button(twidget* caller)
{
	get_listbox(caller)->scrollbar_click(caller);
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
	best_spacer_size_(0, 0),
	rows_()
{
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

void tlistbox::scrollbar_click(twidget* caller)
{
	if(caller->id() == "_begin") {
		scrollbar()->scroll(tscrollbar_::BEGIN);
	} else if(caller->id() == "_line_up") {
		scrollbar()->scroll(tscrollbar_::ITEM_BACKWARDS);
	} else if(caller->id() == "_half_page_up") {
		scrollbar()->scroll(tscrollbar_::HALF_JUMP_BACKWARDS);
	} else if(caller->id() == "_page_up") {
		scrollbar()->scroll(tscrollbar_::JUMP_BACKWARDS);
	} else if(caller->id() == "_end") {
		scrollbar()->scroll(tscrollbar_::END);
	} else if(caller->id() == "_line_down") {
		scrollbar()->scroll(tscrollbar_::ITEM_FORWARD);
	} else if(caller->id() == "_half_page_down") {
		scrollbar()->scroll(tscrollbar_::HALF_JUMP_FORWARD);
	} else if(caller->id() == "_page_down") {
		scrollbar()->scroll(tscrollbar_::JUMP_FORWARD);
	} else {
		assert(false);
	}

	set_scrollbar_button_status();
}

void tlistbox::finalize_setup()
{
	scrollbar()->set_callback_positioner_move(callback_scrollbar);

	static std::vector<std::string> button_names;
	if(button_names.empty()) {
		button_names.push_back("_begin");
		button_names.push_back("_line_up");
		button_names.push_back("_half_page_up");
		button_names.push_back("_page_up");

		button_names.push_back("_end");
		button_names.push_back("_line_down");
		button_names.push_back("_half_page_down");
		button_names.push_back("_page_down");
	}

	foreach(const std::string& name, button_names) {
		tbutton* button = dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));
		if(button) {
			button->set_callback_mouse_left_click(callback_scrollbar_button);
		}
	}
}

void tlistbox::set_scrollbar_button_status()
{
	// Set scroll up button status
	static std::vector<std::string> button_up_names;
	if(button_up_names.empty()) {
		button_up_names.push_back("_begin");
		button_up_names.push_back("_line_up");
		button_up_names.push_back("_half_page_up");
		button_up_names.push_back("_page_up");
	}

	foreach(const std::string& name, button_up_names) {
		tbutton* button = dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));
		if(button) {
			button->set_active(!scrollbar()->at_begin());
		}
	}

	// Set scroll down button status
	static std::vector<std::string> button_down_names;
	if(button_down_names.empty()) {
		button_down_names.push_back("_end");
		button_down_names.push_back("_line_down");
		button_down_names.push_back("_half_page_down");
		button_down_names.push_back("_page_down");
	}

	foreach(const std::string& name, button_down_names) {
		tbutton* button = dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));
		if(button) {
			button->set_active(!scrollbar()->at_end());
		}
	}

	// Set the scrollbar itself
	scrollbar()->set_active(!(scrollbar()->at_begin() && scrollbar()->at_end()));
}

tpoint tlistbox::get_best_size() const 
{

	// Set the size of the spacer to the wanted size for the list.
	unsigned width = 0;
	unsigned height = 0;

	if(best_spacer_size_ != tpoint(0, 0)) {
		// We got a best size set use that instead of calculating it.
		height = best_spacer_size_.y;
		width = best_spacer_size_.x;
	} else {
		// NOTE we should look at the number of visible items etc
		foreach(const trow& row, rows_) {
			assert(row.grid());
			const tpoint best_size = row.grid()->get_best_size();
			width = (static_cast<int>(width) >= best_size.x) ? 
			  width : static_cast<int>(best_size.x);

			height += static_cast<int>(best_size.y);
		}
	}
	
	// Kind of a hack, we edit the spacer in a const function.
	// Of course we could cache the list and make it mutable instead.
	// But since the spacer is a kind of cache the const_cast doesn't 
	// look too ugly.
	const_cast<tspacer*>(list())->set_best_size(tpoint(width, height));

	// Now the container will return the wanted result.
	return tcontainer_::get_best_size();
}

void tlistbox::draw(surface& surface, const bool force, 
		const bool invalidate_background)
{
	// Inherit.
	tcontainer_::draw(surface, force, invalidate_background);

	if(invalidate_background) {
		list_background_.assign(NULL);
	}

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
		row.grid()->draw(row.canvas(), force, invalidate_background);
		
		// draw background
		const SDL_Rect rect = {list_rect_.x, offset, list_rect_.w, row.get_height() };

		// draw widget
		blit_surface(row.canvas(), 0, surface, &rect);

		offset += row.get_height();
	}
}

void tlistbox::set_size(const SDL_Rect& rect)
{
	// Since we allow to be resized we need to determine the real size we get.
	assert(best_spacer_size_ == tpoint(0, 0));
	SDL_Rect best_rect = rect;
	if(rows_.size()) {

		const tpoint best_size = get_best_size();

		if(best_size.y > rect.h) {
			best_spacer_size_ = list()->get_best_size();
			best_spacer_size_.y -= (best_size.y - rect.h);
			if(assume_fixed_row_size_) {
				const unsigned row_height = rows_[0].grid()->get_best_size().y;
				const unsigned orig_height = best_spacer_size_.y;
				best_spacer_size_.y = (best_spacer_size_.y / row_height) * row_height;
				best_rect.h -= (orig_height - best_spacer_size_.y);
			
				// This call is required to update the best size.
				get_best_size();
			}
		}
	}

	// Inherit.
	tcontainer_::set_size(best_rect);

	best_spacer_size_ = tpoint(0, 0);

	// Now set the items in the spacer.
	tspacer* spacer = list();
	list_rect_ = spacer->get_rect();

	foreach(trow& row, rows_) {
		assert(row.grid());

		const unsigned height = row.grid()->get_best_size().y;
		row.set_height(height);

		row.grid()->set_size(::create_rect(0, 0, list_rect_.w, height));
		row.canvas().assign(create_neutral_surface(list_rect_.w, height));
	}

	// FIXME we assume fixed row height atm.
	if(rows_.size() > 0) {
		const unsigned row_height = rows_[0].get_height();
		if(row_height) {
			const unsigned rows = list()->get_best_size().y / row_height;
			scrollbar()->set_visible_items(rows);
		} else {
			WRN_G << "Listbox row 0 has no height, making all rows visible.\n";
			scrollbar()->set_visible_items(rows_.size());
		}
	} else {
		scrollbar()->set_visible_items(1);
	}
	set_scrollbar_button_status();
}

twidget* tlistbox::find_widget(const tpoint& coordinate, const bool must_be_active) 
{ 
	// Inherited
	twidget* result = tcontainer_::find_widget(coordinate, must_be_active);

	// if on the panel we need to do special things.
	if(result && result->id() == "_list") {
		int offset = coordinate.y - list_rect_.y;
		assert(offset >= 0);
		for(unsigned i = 0; i < scrollbar()->get_visible_items(); ++i) {

			trow& row = rows_[i + scrollbar()->get_item_position()];
			
			if(offset < static_cast<int>(row.get_height())) {
				assert(row.grid());
				return row.grid()->find_widget(
					tpoint(coordinate.x - list_rect_.x, offset), must_be_active);
			} else {
				offset -= row.get_height();
			}
		}
	}

	return result;
}

const twidget* tlistbox::find_widget(const tpoint& coordinate, const bool must_be_active) const
{
	// Inherited
	const twidget* result = tcontainer_::find_widget(coordinate, must_be_active);

	// if on the panel we need to do special things.
	if(result && result->id() == "_list") {
		int offset = coordinate.y - list_rect_.y;
		assert(offset >= 0);
		for(unsigned i = 0; i < scrollbar()->get_visible_items(); ++i) {

			const trow& row = rows_[i + scrollbar()->get_item_position()];
			
			if(offset < static_cast<int>(row.get_height())) {
				assert(row.grid());
				return row.grid()->find_widget(
					tpoint(coordinate.x - list_rect_.x, offset), must_be_active);
			} else {
				offset -= row.get_height();
			}
		}
	}

	return result;
}

void tlistbox::add_row(const std::map<
		std::string /* member id */, t_string /* member value */>& item)
{
	std::map<std::string /* widget id */, std::map<
		std::string /* member id */, t_string /* member value */> > data;

	data.insert(std::make_pair("", item));
	add_row(data);
}

void tlistbox::add_row(const std::map<std::string /* widget id */, std::map<
		std::string /* member id */, t_string /* member value */> >& data)
{
	assert(list_builder_);

	trow row(*list_builder_, data);
	assert(row.grid());

	row.grid()->set_parent(this);
	rows_.push_back(row);

	if(must_select_ && !selection_count_) {
		select_row(get_item_count() - 1);
	}

	scrollbar()->set_item_count(get_item_count());
	set_scrollbar_button_status();
}

void tlistbox::add_rows(const std::vector< std::map<std::string, t_string> >& data)
{
	// foreach(const std::map<std::string, t_string>& cell, data) {
	// doesn't compile it sees 3 paramters instead of 2 so use a typedef.
	typedef std::map<std::string, t_string> hack ;
	foreach(const hack& cell, data) {
		add_row(cell);
	}
}

tscrollbar_* tlistbox::scrollbar()
{
	// Note we don't cache the result, we might want change things later.	
	tscrollbar_* result = 
		dynamic_cast<tscrollbar_*>(tcontainer_::find_widget("_scrollbar", false));
	assert(result);
	return result;
}

const tscrollbar_* tlistbox::scrollbar() const
{
	// Note we don't cache the result, we might want change things later.	
	const tscrollbar_* result = 
		dynamic_cast<const tscrollbar_*>(tcontainer_::find_widget("_scrollbar", false));
	assert(result);
	return result;
}

tspacer* tlistbox::list()
{
	tspacer* list = 
		dynamic_cast<tspacer*>(tcontainer_::find_widget("_list", false));
	assert(list);
	return list;
}

const tspacer* tlistbox::list() const
{
	const tspacer* list = 
		dynamic_cast<const tspacer*>(tcontainer_::find_widget("_list", false));
	assert(list);
	return list;
}

bool tlistbox::select_row(const unsigned row, const bool select)
{
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

void tlistbox::set_row_active(const unsigned row, const bool active)
{
	assert(row < get_item_count());
	assert(rows_[row].grid());
	rows_[row].grid()->set_active(active);
}

const tgrid* tlistbox::get_row_grid(const unsigned row) const
{
	assert(row < rows_.size());
	return rows_[row].grid();
}

tlistbox::trow::trow(const tbuilder_grid& list_builder_, 
		const std::map<std::string /* widget id */, std::map<
		std::string /* member id */, t_string /* member value */> >& data) :
	grid_(dynamic_cast<tgrid*>(list_builder_.build())),
	height_(0),
	selected_(false)
{
	assert(grid_);
	init_in_grid(grid_, data);
}

void tlistbox::trow::init_in_grid(tgrid* grid, 
		const std::map<std::string /* widget id */, std::map<
		std::string /* member id */, t_string /* member value */> >& data)
{		
	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);


			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			ttoggle_button* btn = dynamic_cast<ttoggle_button*>(widget);
			ttoggle_panel* panel = dynamic_cast<ttoggle_panel*>(widget);

			if(btn) {
				btn->set_callback_mouse_left_click(callback_select_list_item);
				std::map<std::string /* widget id */, std::map<
					std::string /* member id */, t_string /* member value */> >
					::const_iterator itor = data.find(btn->id());

				if(itor == data.end()) {
					itor = data.find("");
				}
				if(itor != data.end()) {
					btn->set_members(itor->second);
				}
			} else if(panel) {
				panel->set_callback_mouse_left_click(callback_select_list_item);
				panel->set_members(data);
			} else if(child_grid) {
				init_in_grid(child_grid, data);
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


