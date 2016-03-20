/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI2_EXPERIMENTAL_LISTBOX

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/listbox.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/window.hpp"

#include "gettext.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(listbox)

namespace
{
// in separate namespace to avoid name classes
REGISTER_WIDGET3(tlistbox_definition, horizontal_listbox, _4)

void callback_list_item_clicked(twidget& caller)
{
	get_parent<tlistbox>(caller).list_item_clicked(caller);
}

} // namespace

tlistbox::tlistbox(const bool has_minimum,
				   const bool has_maximum,
				   const tgenerator_::tplacement placement,
				   const bool select)
	: tscrollbar_container(2) // FIXME magic number
	, generator_(
			  tgenerator_::build(has_minimum, has_maximum, placement, select))
	, list_builder_(NULL)
	, callback_value_changed_(NULL)
	, need_layout_(false)
	, orders_()
{
}

void tlistbox::add_row(const string_map& item, const int index)
{
	assert(generator_);
	const tgrid& row = generator_->create_item(
			index, list_builder_, item, callback_list_item_clicked);

	resize_content(row);
}

void
tlistbox::add_row(const std::map<std::string /* widget id */, string_map>& data,
				  const int index)
{
	assert(generator_);
	const tgrid& row = generator_->create_item(
			index, list_builder_, data, callback_list_item_clicked);

	resize_content(row);
}

void tlistbox::remove_row(const unsigned row, unsigned count)
{
	assert(generator_);

	if(row >= get_item_count()) {
		return;
	}

	if(!count || count + row > get_item_count()) {
		count = get_item_count() - row;
	}

	int height_reduced = 0;
	//TODO: Fix this for horizinal listboxes
	//Note the we have to use content_grid_ and cannot use "_list_grid" which is what generator_ uses.
	int row_pos = generator_->item(row).get_y()  - content_grid_->get_y();
	for(; count; --count) {
		if(generator_->item(row).get_visible() != tvisible::invisible) {
			height_reduced += generator_->item(row).get_height();
		}
		generator_->delete_item(row);
	}

	if(height_reduced != 0 && get_item_count() != 0) {
		resize_content(0, -height_reduced, 0, row_pos);
	} else {
		update_content_size();
	}
}

void tlistbox::clear()
{
	// Due to the removing from the linked group, don't use
	// generator_->clear() directly.
	remove_row(0, 0);
}

unsigned tlistbox::get_item_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void tlistbox::set_row_active(const unsigned row, const bool active)
{
	assert(generator_);
	generator_->item(row).set_active(active);
}

void tlistbox::set_row_shown(const unsigned row, const bool shown)
{
	assert(generator_);

	twindow* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed;
	{
		twindow::tinvalidate_layout_blocker invalidate_layout_blocker(*window);

		generator_->set_item_shown(row, shown);
		generator_->place(generator_->get_origin(),
						  generator_->calculate_best_size());
		resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		content_grid_->set_visible_rectangle(content_visible_area());
		set_is_dirty(true);
	}

	if(selected_row != get_selected_row() && callback_value_changed_) {
		callback_value_changed_(*this);
	}
}

void tlistbox::set_row_shown(const std::vector<bool>& shown)
{
	assert(generator_);
	assert(shown.size() == get_item_count());

	twindow* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed;
	{
		twindow::tinvalidate_layout_blocker invalidate_layout_blocker(*window);

		for(size_t i = 0; i < shown.size(); ++i) {
			generator_->set_item_shown(i, shown[i]);
		}
		generator_->place(generator_->get_origin(),
						  generator_->calculate_best_size());
		resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		content_grid_->set_visible_rectangle(content_visible_area());
		set_is_dirty(true);
	}

	if(selected_row != get_selected_row() && callback_value_changed_) {
		callback_value_changed_(*this);
	}
}

const tgrid* tlistbox::get_row_grid(const unsigned row) const
{
	assert(generator_);
	// rename this function and can we return a reference??
	return &generator_->item(row);
}

tgrid* tlistbox::get_row_grid(const unsigned row)
{
	assert(generator_);
	return &generator_->item(row);
}

bool tlistbox::select_row(const unsigned row, const bool select)
{
	assert(generator_);

	generator_->select_item(row, select);

	return true; // FIXME test what result should have been!!!
}

int tlistbox::get_selected_row() const
{
	assert(generator_);

	return generator_->get_selected_item();
}

void tlistbox::list_item_clicked(twidget& caller)
{
	assert(generator_);

	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	for(size_t i = 0; i < generator_->get_item_count(); ++i) {

		if(generator_->item(i).has_widget(caller)) {
			generator_->toggle_item(i);
			if(callback_item_changed_) {
				callback_item_changed_(i);
			}
			if(callback_value_changed_) {
				callback_value_changed_(*this);
			}
			return;
		}
	}
	assert(false);
}

void tlistbox::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tlistbox::update_content_size()
{
	if(get_visible() == twidget::tvisible::invisible) {
		return true;
	}

	if(get_size() == tpoint(0, 0)) {
		return false;
	}

	if(content_resize_request(true)) {
		content_grid_->set_visible_rectangle(content_visible_area());
		set_is_dirty(true);
		return true;
	}

	return false;
}

void tlistbox::place(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	tscrollbar_container::place(origin, size);

	/**
	 * @todo Work-around to set the selected item visible again.
	 *
	 * At the moment the listboxes and dialogs in general are resized a lot as
	 * work-around for sizing. So this function makes the selected item in view
	 * again. It doesn't work great in all cases but the proper fix is to avoid
	 * resizing dialogs a lot. Need more work later on.
	 */
	const int selected_item = generator_->get_selected_item();
	if(selected_item != -1) {
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(selected_item).get_rectangle();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);
	}
}

void tlistbox::resize_content(const int width_modification,
							  const int height_modification,
							  const int width_modification_pos,
							  const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " width_modification " << width_modification
			  << " height_modification " << height_modification << ".\n";

	if(content_resize_request(width_modification, height_modification, width_modification_pos, height_modification_pos)) {

		// Calculate new size.
		tpoint size = content_grid()->get_size();
		size.x += width_modification;
		size.y += height_modification;

		// Set new size.
		content_grid()->set_size(size);

		// Set status.
		need_layout_ = true;
		// If the content grows assume it "overwrites" the old content.
		if(width_modification < 0 || height_modification < 0) {
			set_is_dirty(true);
		}
		DBG_GUI_L << LOG_HEADER << " succeeded.\n";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.\n";
	}
}

void tlistbox::resize_content(const twidget& row)
{
	if(row.get_visible() == tvisible::invisible) {
		return;
	}

	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " row size " << row.get_best_size() << ".\n";

	const tpoint content = content_grid()->get_size();
	tpoint size = row.get_best_size();
	if(size.x < content.x) {
		size.x = 0;
	} else {
		size.x -= content.x;
	}

	resize_content(size.x, size.y);
}

void tlistbox::layout_children()
{
	layout_children(false);
}

void
tlistbox::child_populate_dirty_list(twindow& caller,
									const std::vector<twidget*>& call_stack)
{
	// Inherited.
	tscrollbar_container::child_populate_dirty_list(caller, call_stack);

	assert(generator_);
	std::vector<twidget*> child_call_stack = call_stack;
	generator_->populate_dirty_list(caller, child_call_stack);
}

void tlistbox::handle_key_up_arrow(SDLMod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_up_arrow(modifier, handled);

	if(handled) {
		// When scrolling make sure the new items is visible but leave the
		// horizontal scrollbar position.
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(generator_->get_selected_item())
								.get_rectangle();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(*this);
		}
	} else {
		// Inherited.
		tscrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void tlistbox::handle_key_down_arrow(SDLMod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_down_arrow(modifier, handled);

	if(handled) {
		// When scrolling make sure the new items is visible but leave the
		// horizontal scrollbar position.
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(generator_->get_selected_item())
								.get_rectangle();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(*this);
		}
	} else {
		// Inherited.
		tscrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void tlistbox::handle_key_left_arrow(SDLMod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_left_arrow(modifier, handled);

	// Inherited.
	if(handled) {
		// When scrolling make sure the new items is visible but leave the
		// vertical scrollbar position.
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(generator_->get_selected_item())
								.get_rectangle();

		rect.y = visible.y;
		rect.h = visible.h;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(*this);
		}
	} else {
		tscrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void tlistbox::handle_key_right_arrow(SDLMod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_right_arrow(modifier, handled);

	// Inherited.
	if(handled) {
		// When scrolling make sure the new items is visible but leave the
		// vertical scrollbar position.
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(generator_->get_selected_item())
								.get_rectangle();

		rect.y = visible.y;
		rect.h = visible.h;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(*this);
		}
	} else {
		tscrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

namespace
{

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
			   tgrid* content_grid,
			   twidget* widget,
			   const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = find_widget<tgrid>(grid, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<tgrid>(content_grid, id, true, false);
	}
	parent_grid = dynamic_cast<tgrid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void tlistbox::finalize(tbuilder_grid_const_ptr header,
						tbuilder_grid_const_ptr footer,
						const std::vector<string_map>& list_data)
{
	// "Inherited."
	tscrollbar_container::finalize_setup();

	assert(generator_);

	if(header) {
		swap_grid(&grid(), content_grid(), header->build(), "_header_grid");
	}
	tgrid& p = find_widget<tgrid>(this, "_header_grid", false);
	for(unsigned i = 0, max = std::max(p.get_cols(), p.get_rows()); i < max; ++i) {
		if(tselectable_* selectable = find_widget<tselectable_>(&p, "sort_" +  lexical_cast<std::string>(i), false, false)) {
			selectable->set_callback_state_change(boost::bind(&tlistbox::order_by_column, this, i, _1));
			if(orders_.size() < max ) {
				orders_.resize(max);
			}
			orders_[i].first = selectable;
		}
	}
	if(footer) {
		swap_grid(&grid(), content_grid(), footer->build(), "_footer_grid");
	}

	generator_->create_items(
			-1, list_builder_, list_data, callback_list_item_clicked);
	swap_grid(NULL, content_grid(), generator_, "_list_grid");
}
namespace {
	bool default_sort(unsigned i1, unsigned i2)
	{
		return i1 < i2;
	}
}

void tlistbox::order_by_column(unsigned column, twidget& widget)
{
	tselectable_& selectable = dynamic_cast<tselectable_&>(widget);
	if(column >= orders_.size()) {
		return;
	}
	FOREACH(AUTO& pair, orders_)
	{
		if(pair.first != NULL && pair.first != &selectable) {
			pair.first->set_value(0);
		}
	}
	if(selectable.get_value() > orders_[column].second.size()) {
		return;
	}
	if(selectable.get_value() == 0) {
		order_by(tgenerator_::torder_func(&default_sort));
	}
	else {
		order_by(orders_[column].second[selectable.get_value() - 1]);
	}
}

void tlistbox::order_by(const tgenerator_::torder_func& func)
{
	generator_->set_order(func);

	set_is_dirty(true);
	need_layout_ = true;
}

void tlistbox::set_column_order(unsigned col, const std::vector<tgenerator_::torder_func>& func)
{
	if(col >= orders_.size()) {
		orders_.resize(col + 1);
	}
	orders_[col].second = func;
}

void tlistbox::set_content_size(const tpoint& origin, const tpoint& size)
{
	/** @todo This function needs more testing. */
	assert(content_grid());

	const int best_height = content_grid()->get_best_size().y;
	const tpoint s(size.x, size.y < best_height ? size.y : best_height);

	content_grid()->place(origin, s);
}

void tlistbox::layout_children(const bool force)
{
	assert(content_grid());

	if(need_layout_ || force) {
		content_grid()->place(content_grid()->get_origin(),
							  content_grid()->get_size());

		content_grid()->set_visible_rectangle(content_visible_area_);

		need_layout_ = false;
		set_is_dirty(true);
	}
}

const std::string& tlistbox::get_control_type() const
{
	static const std::string type = "listbox";
	return type;
}

// }---------- DEFINITION ---------{

tlistbox_definition::tlistbox_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing listbox " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_listbox
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="listbox_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * == Listbox ==
 *
 * @macro = listbox_description
 *
 * The definition of a listbox contains the definition of its scrollbar.
 *
 * The resolution for a listbox also contains the following keys:
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 * @begin{table}{config}
 *     scrollbar & section & &         A grid containing the widgets for the
 *                                     scrollbar. The scrollbar has some special
 *                                     widgets so it can make default behavior
 *                                     for certain widgets. $
 * @end{table}
 * @begin{table}{dialog_widgets}
 *     _begin & & clickable & o &      Moves the position to the beginning
 *                                     of the list. $
 *     _line_up & & clickable & o &    Move the position one item up. (NOTE
 *                                     if too many items to move per item it
 *                                     might be more items.) $
 *     _half_page_up & & clickable & o &
 *                                     Move the position half the number of the
 *                                     visible items up. (See note at
 *                                     _line_up.) $
 *     _page_up & & clickable & o &    Move the position the number of
 *                                     visible items up. (See note at
 *                                     _line_up.) $
 *
 *     _end & & clickable & o &        Moves the position to the end of the
 *                                     list. $
 *     _line_down & & clickable & o &  Move the position one item down.(See
 *                                     note at _line_up.) $
 *     _half_page_down & & clickable & o &
 *                                     Move the position half the number of the
 *                                     visible items down. (See note at
 *                                     _line_up.) $
 *     _page_down & & clickable & o &  Move the position the number of
 *                                     visible items down. (See note at
 *                                     _line_up.) $
 *
 *     _scrollbar & & vertical_scrollbar & m &
 *                                     This is the scrollbar so the user can
 *                                     scroll through the list. $
 * @end{table}
 * A clickable is one of:
 * * button
 * * repeating_button
 * @{allow}{link}{name="gui/window/resolution/grid/row/column/button"}
 * @{allow}{link}{name="gui/window/resolution/grid/row/column/repeating_button"}
 * The following states exist:
 * * state_enabled, the listbox is enabled.
 * * state_disabled, the listbox is disabled.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="listbox_definition"}
 * @end{parent}{name="gui/"}
 */

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_horizonal_listbox
 *
 * == Horizontal listbox ==
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="horizontal_listbox_definition"}{min=0}{max=-1}{super="gui/listbox_definition"}
 * @end{tag}{name="horizontal_listbox_definition"}
 * @end{parent}{name="gui/"}
 * @macro = horizontal_listbox_description
 * The definition of a horizontal listbox is the same as for a normal listbox.
 */
tlistbox_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg), grid(NULL)
{
	// Note the order should be the same as the enum tstate in listbox.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{listbox_description}
 *
 *        A listbox is a control that holds several items of the same type.
 *        Normally the items in a listbox are ordered in rows, this version
 *        might allow more options for ordering the items in the future.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_listbox
 *
 * == Listbox ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="listbox"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @macro = listbox_description
 *
 * List with the listbox specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *
 *     header & grid & [] &            Defines the grid for the optional
 *                                     header. (This grid will automatically
 *                                     get the id _header_grid.) $
 *     footer & grid & [] &            Defines the grid for the optional
 *                                     footer. (This grid will automatically
 *                                     get the id _footer_grid.) $
 *
 *     list_definition & section & &   This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 row of the list. $
 *
 *     list_data & section & [] &      A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'. $
 *
 *     has_minimum & bool & true &     If false, less than one row can be selected. $
 *
 *     has_maximum & bool & true &     If false, more than one row can be selected. $
 *
 * @end{table}
 * @begin{tag}{name="header"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="header"}
 * @begin{tag}{name="footer"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="footer"}
 * @begin{tag}{name="list_definition"}{min=0}{max=1}
 * @begin{tag}{name="row"}{min=1}{max=1}{super="generic/listbox_grid/row"}
 * @end{tag}{name="row"}
 * @end{tag}{name="list_definition"}x
 * @begin{tag}{name="list_data"}{min=0}{max=1}{super="generic/listbox_grid"}
 * @end{tag}{name="list_data"}
 *
 * In order to force widgets to be the same size inside a listbox, the widgets
 * need to be inside a linked_group.
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 * @end{tag}{name="listbox"}
 *
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

/*WIKI
 * @begin{parent}{name="generic/"}
 * @begin{tag}{name="listbox_grid"}{min="0"}{max="-1"}
 * @begin{tag}{name="row"}{min="0"}{max="-1"}
 * @begin{table}{config}
 *     grow_factor & unsigned & 0 &      The grow factor for a row. $
 * @end{table}
 * @begin{tag}{name="column"}{min="0"}{max="-1"}{super="gui/window/resolution/grid/row/column"}
 * @begin{table}{config}
 *     label & t_string & "" &  $
 *     tooltip & t_string & "" &  $
 *     icon & t_string & "" &  $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid/row/column/toggle_button"}
 * @allow{link}{name="gui/window/resolution/grid/row/column/toggle_panel"}
 * @end{tag}{name="column"}
 * @end{tag}{name="row"}
 * @end{tag}{name="listbox_grid"}
 * @end{parent}{name="generic/"}
 */

namespace implementation
{

tbuilder_listbox::tbuilder_listbox(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, header(NULL)
	, footer(NULL)
	, list_builder(NULL)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	if(const config& h = cfg.child("header")) {
		header = new tbuilder_grid(h);
	}

	if(const config& f = cfg.child("footer")) {
		footer = new tbuilder_grid(f);
	}

	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = new tbuilder_grid(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	const config& data = cfg.child("list_data");
	if(!data) {
		return;
	}

	FOREACH(const AUTO & row, data.child_range("row"))
	{
		unsigned col = 0;

		FOREACH(const AUTO & c, row.child_range("column"))
		{
			list_data.push_back(string_map());
			FOREACH(const AUTO & i, c.attribute_range())
			{
				list_data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == list_builder->cols,
				 _("'list_data' must have the same number of "
				   "columns as the 'list_definition'."));
	}
}

twidget* tbuilder_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	tlist* widget = new tlist(
			true, true, tgenerator_::vertical_list, true, list_builder);

	init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	if(new_widgets) {

		tpane* pane = new tpane(list_builder);
		pane->set_id(id);


		tgrid* grid = new tgrid();
		grid->set_rows_cols(1, 1);
#if 0
		grid->set_child(
				  pane
				, 0
				, 0
				, tgrid::VERTICAL_GROW_SEND_TO_CLIENT
					| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
				, tgrid::BORDER_ALL);
#else
		tviewport* viewport = new tviewport(*pane);
		grid->set_child(viewport,
						0,
						0,
						tgrid::VERTICAL_GROW_SEND_TO_CLIENT
						| tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT,
						tgrid::BORDER_ALL);
#endif
		return grid;
	}

	tlistbox* widget
			= new tlistbox(has_minimum_, has_maximum_, tgenerator_::vertical_list, true);

	init_control(widget);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const tlistbox_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tlistbox_definition::tresolution>(
			widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(header, footer, list_data);

	return widget;
#endif
}

/*WIKI_MACRO
 * @begin{macro}{horizontal_listbox_description}
 *
 *        A horizontal listbox is a control that holds several items of the
 *        same type.  Normally the items in a listbox are ordered in rows,
 *        this version orders them in columns instead.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_horizontal_listbox
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="horizontal_listbox"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Horizontal listbox ==
 *
 * @macro = horizontal_listbox_description
 *
 * List with the horizontal listbox specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *
 *     list_definition & section & &   This defines how a listbox item
 *                                     looks. It must contain the grid
 *                                     definition for 1 column of the list. $
 *
 *     list_data & section & [] &      A grid alike section which stores the
 *                                     initial data for the listbox. Every row
 *                                     must have the same number of columns as
 *                                     the 'list_definition'. $
 *
 * @end{table}
 * @begin{tag}{name="header"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="header"}
 * @begin{tag}{name="footer"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="footer"}
 * @begin{tag}{name="list_definition"}{min=0}{max=1}
 * @begin{tag}{name="row"}{min=1}{max=1}{super="generic/listbox_grid/row"}
 * @end{tag}{name="row"}
 * @end{tag}{name="list_definition"}
 * @begin{tag}{name="list_data"}{min=0}{max=1}{super="generic/listbox_grid"}
 * @end{tag}{name="list_data"}
 * In order to force widgets to be the same size inside a horizontal listbox,
 * the widgets need to be inside a linked_group.
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 * @end{tag}{name="horizontal_listbox"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

tbuilder_horizontal_listbox::tbuilder_horizontal_listbox(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(NULL)
	, list_data()
{
	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = new tbuilder_grid(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	const config& data = cfg.child("list_data");
	if(!data)
		return;

	FOREACH(const AUTO & row, data.child_range("row"))
	{
		unsigned col = 0;

		FOREACH(const AUTO & c, row.child_range("column"))
		{
			list_data.push_back(string_map());
			FOREACH(const AUTO & i, c.attribute_range())
			{
				list_data.back()[i.first] = i.second;
			}
			++col;
		}

		VALIDATE(col == list_builder->cols,
				 _("'list_data' must have "
				   "the same number of columns as the 'list_definition'."));
	}
}

twidget* tbuilder_horizontal_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	tlist* widget = new tlist(
			true, true, tgenerator_::horizontal_list, true, list_builder);

	init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	tlistbox* widget
			= new tlistbox(true, true, tgenerator_::horizontal_list, true);

	init_control(widget);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const tlistbox_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tlistbox_definition::tresolution>(
			widget->config());
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(NULL, NULL, list_data);

	return widget;
#endif
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
