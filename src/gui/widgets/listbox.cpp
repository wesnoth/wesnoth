/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/core/layout_exception.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/window.hpp"

#include "gettext.hpp"

#include "utils/functional.hpp"

#include <boost/optional.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(listbox)
REGISTER_WIDGET3(listbox_definition, horizontal_listbox, nullptr)
REGISTER_WIDGET3(listbox_definition, grid_listbox, nullptr)

namespace
{
void callback_list_item_clicked(widget& caller)
{
	get_parent<listbox>(caller).list_item_clicked(caller);
}

} // namespace

listbox::listbox(const implementation::builder_styled_widget& builder,
		const bool has_minimum,
		const bool has_maximum,
		const generator_base::placement placement,
		const bool select)
	: scrollbar_container(builder, get_control_type())
	, generator_(generator_base::build(has_minimum, has_maximum, placement, select))
	, is_horizontal_(placement == generator_base::horizontal_list)
	, list_builder_(nullptr)
	, callback_value_changed_()
	, need_layout_(false)
	, orders_()
{
}

grid& listbox::add_row(const string_map& item, const int index)
{
	assert(generator_);
	grid& row = generator_->create_item(
			index, list_builder_, item, callback_list_item_clicked);

	resize_content(row);

	return row;
}

grid& listbox::add_row(const std::map<std::string /* widget id */, string_map>& data,
				  const int index)
{
	assert(generator_);
	grid& row = generator_->create_item(
			index, list_builder_, data, callback_list_item_clicked);

	resize_content(row);

	return row;
}

void listbox::remove_row(const unsigned row, unsigned count)
{
	assert(generator_);

	if(row >= get_item_count()) {
		return;
	}

	if(!count || count + row > get_item_count()) {
		count = get_item_count() - row;
	}

	int height_reduced = 0;
	int width_reduced = 0;
	//TODO: Fix this for horizontal listboxes
	//Note the we have to use content_grid_ and cannot use "_list_grid" which is what generator_ uses.
	int row_pos_y = is_horizontal_ ? -1 : generator_->item(row).get_y()  - content_grid_->get_y();
	int row_pos_x = is_horizontal_ ? -1 : 0;
	for(; count; --count) {
		if(generator_->item(row).get_visible() != visibility::invisible) {
			if(is_horizontal_) {
				width_reduced += generator_->item(row).get_width();
			}
			else {
				height_reduced += generator_->item(row).get_height();
			}
		}
		generator_->delete_item(row);
	}

	if((height_reduced != 0 || width_reduced != 0) && get_item_count() != 0) {
		resize_content(-width_reduced, -height_reduced, row_pos_x, row_pos_y);
	} else {
		update_content_size();
	}
}

void listbox::clear()
{
	generator_->clear();
	update_content_size();
}

unsigned listbox::get_item_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void listbox::set_row_active(const unsigned row, const bool active)
{
	assert(generator_);
	generator_->item(row).set_active(active);
}

void listbox::set_row_shown(const unsigned row, const bool shown)
{
	assert(generator_);

	window* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed = false;
	{
		window::invalidate_layout_blocker invalidate_layout_blocker(*window);

		generator_->set_item_shown(row, shown);
		point best_size = generator_->calculate_best_size();
		generator_->place(generator_->get_origin(), { std::max(best_size.x, content_visible_area().w), best_size.y });
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

void listbox::set_row_shown(const boost::dynamic_bitset<>& shown)
{
	assert(generator_);
	assert(shown.size() == get_item_count());

	if (generator_->get_items_shown() == shown)
	{
		LOG_GUI_G << LOG_HEADER << " returning early" << std::endl;
		return;
	}

	window* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed = false;
	{
		window::invalidate_layout_blocker invalidate_layout_blocker(*window);

		for(size_t i = 0; i < shown.size(); ++i) {
			generator_->set_item_shown(i, shown[i]);
		}
		point best_size = generator_->calculate_best_size();
		generator_->place(generator_->get_origin(), { std::max(best_size.x, content_visible_area().w), best_size.y });
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

boost::dynamic_bitset<> listbox::get_rows_shown() const
{
	return generator_->get_items_shown();
}

bool listbox::any_rows_shown() const
{
	for(size_t i = 0; i < get_item_count(); i++) {
		if(generator_->get_item_shown(i)) {
			return true;
		}
	}
	return false;
}

const grid* listbox::get_row_grid(const unsigned row) const
{
	assert(generator_);
	// rename this function and can we return a reference??
	return &generator_->item(row);
}

grid* listbox::get_row_grid(const unsigned row)
{
	assert(generator_);
	return &generator_->item(row);
}

bool listbox::select_row(const unsigned row, const bool select)
{
	assert(generator_);

	unsigned int before = generator_->get_selected_item_count();
	generator_->select_item(row, select);

	return before != generator_->get_selected_item_count();
}

bool listbox::select_row_at(const unsigned row, const bool select)
{
	assert(generator_);

	return select_row(generator_->get_item_at_ordered(row), select);
}

bool listbox::row_selected(const unsigned row)
{
	assert(generator_);

	return generator_->is_selected(row);
}

int listbox::get_selected_row() const
{
	assert(generator_);

	return generator_->get_selected_item();
}

void listbox::list_item_clicked(widget& caller)
{
	assert(generator_);

	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	for(size_t i = 0; i < generator_->get_item_count(); ++i) {

		if(generator_->item(i).has_widget(caller)) {
			toggle_button* checkbox = dynamic_cast<toggle_button*>(&caller);
			if(checkbox != nullptr) {
				generator_->select_item(i, checkbox->get_value_bool());
			} else {
				generator_->toggle_item(i);
			}

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

void listbox::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool listbox::update_content_size()
{
	if(get_visible() == widget::visibility::invisible) {
		return true;
	}

	if(get_size() == point()) {
		return false;
	}

	if(content_resize_request(true)) {
		content_grid_->set_visible_rectangle(content_visible_area());
		set_is_dirty(true);
		return true;
	}

	return false;
}

/* Suppress -Wmaybe-uninitialized warnings. @GregoryLundberg reported in IRC that GCC 6.2.1 with SCons gives a warning that
 * the value of horizontal_scrollbar_position can be used uninitialized. It's of course not possible (boost::optional
 * conversion to bool returns true only if the value is set), but GCC can't prove that to itself. */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

void listbox::place(const point& origin, const point& size)
{
	boost::optional<unsigned> vertical_scrollbar_position, horizontal_scrollbar_position;
	// Check if this is the first time placing the list box
	if (get_origin() != point{-1, -1})
	{
		vertical_scrollbar_position = get_vertical_scrollbar_item_position();
		horizontal_scrollbar_position = get_horizontal_scrollbar_item_position();
	}

	// Inherited.
	scrollbar_container::place(origin, size);

	const int selected_item = generator_->get_selected_item();
	if (vertical_scrollbar_position && horizontal_scrollbar_position)
	{
		LOG_GUI_L << LOG_HEADER << " restoring scroll position" << std::endl;
		set_vertical_scrollbar_item_position(*vertical_scrollbar_position);
		set_horizontal_scrollbar_item_position(*horizontal_scrollbar_position);
	}
	else if (selected_item != -1)
	{
		LOG_GUI_L << LOG_HEADER << " making the initially selected item visible" << std::endl;
		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(selected_item).get_rectangle();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);
	}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

void listbox::resize_content(const int width_modification,
							  const int height_modification,
							  const int width_modification_pos,
							  const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " width_modification " << width_modification
			  << " height_modification " << height_modification << ".\n";

	if(content_resize_request(width_modification, height_modification, width_modification_pos, height_modification_pos)) {

		// Calculate new size.
		point size = content_grid()->get_size();
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

void listbox::resize_content(const widget& row)
{
	if(row.get_visible() == visibility::invisible) {
		return;
	}

	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " row size " << row.get_best_size() << ".\n";

	const point content = content_grid()->get_size();
	point size = row.get_best_size();
	if(size.x < content.x) {
		size.x = 0;
	} else {
		size.x -= content.x;
	}

	resize_content(size.x, size.y);
}

void listbox::layout_children()
{
	layout_children(false);
}

void
listbox::child_populate_dirty_list(window& caller,
									const std::vector<widget*>& call_stack)
{
	// Inherited.
	scrollbar_container::child_populate_dirty_list(caller, call_stack);

	assert(generator_);
	std::vector<widget*> child_call_stack = call_stack;
	generator_->populate_dirty_list(caller, child_call_stack);
}

void listbox::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
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
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void listbox::handle_key_down_arrow(SDL_Keymod modifier, bool& handled)
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
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void listbox::handle_key_left_arrow(SDL_Keymod modifier, bool& handled)
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
		scrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void listbox::handle_key_right_arrow(SDL_Keymod modifier, bool& handled)
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
		scrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void listbox::finalize(builder_grid_const_ptr header,
						builder_grid_const_ptr footer,
						const std::vector<std::map<std::string, string_map>>& list_data)
{
	// "Inherited."
	scrollbar_container::finalize_setup();

	assert(generator_);

	if(header) {
		swap_grid(&get_grid(), content_grid(), header->build(), "_header_grid");
	}
	grid& p = find_widget<grid>(this, "_header_grid", false);
	for(unsigned i = 0, max = std::max(p.get_cols(), p.get_rows()); i < max; ++i) {
		if(selectable_item* selectable = find_widget<selectable_item>(&p, "sort_" +  std::to_string(i), false, false)) {
			selectable->set_callback_state_change(std::bind(&listbox::order_by_column, this, i, _1));
			if(orders_.size() < max ) {
				orders_.resize(max);
			}
			orders_[i].first = selectable;
		}
	}
	if(footer) {
		swap_grid(&get_grid(), content_grid(), footer->build(), "_footer_grid");
	}

	generator_->create_items(
			-1, list_builder_, list_data, callback_list_item_clicked);
	swap_grid(nullptr, content_grid(), generator_, "_list_grid");
}
namespace {
	bool default_sort(unsigned i1, unsigned i2)
	{
		return i1 < i2;
	}
}

void listbox::order_by_column(unsigned column, widget& widget)
{
	selectable_item& selectable = dynamic_cast<selectable_item&>(widget);
	if(column >= orders_.size()) {
		return;
	}
	for(auto& pair : orders_)
	{
		if(pair.first != nullptr && pair.first != &selectable) {
			pair.first->set_value(SORT_NONE);
		}
	}
	if(selectable.get_value() > orders_[column].second.size()) {
		return;
	}
	if(selectable.get_value() == SORT_NONE) {
		order_by(generator_base::torder_func(&default_sort));
	}
	else {
		order_by(orders_[column].second[selectable.get_value() - 1]);
	}
}

void listbox::order_by(const generator_base::torder_func& func)
{
	generator_->set_order(func);

	set_is_dirty(true);
	need_layout_ = true;
}

void listbox::set_column_order(unsigned col, const generator_sort_array& func)
{
	if(col >= orders_.size()) {
		orders_.resize(col + 1);
	}
	orders_[col].second = func;
}

void listbox::set_active_sorting_option(const order_pair& sort_by, const bool select_first)
{
	// TODO: should this be moved to a public header_grid() getter function?
	grid& header_grid = find_widget<grid>(this, "_header_grid", false);

	selectable_item& wgt = find_widget<selectable_item>(&header_grid, "sort_" +  std::to_string(sort_by.first), false);
	wgt.set_value(static_cast<int>(sort_by.second));

	order_by_column(sort_by.first, dynamic_cast<widget&>(wgt));

	if(select_first && generator_->get_item_count() > 0) {
		select_row_at(0);
	}
}

const listbox::order_pair listbox::get_active_sorting_option()
{
	const auto iter = std::find_if(orders_.begin(), orders_.end(), [](const std::pair<selectable_item*, generator_sort_array>& option) {
		return option.first != nullptr && option.first->get_value() != SORT_NONE;
	});

	if(iter != orders_.end()) {
		return {iter - orders_.begin(), static_cast<SORT_ORDER>(iter->first->get_value())};
	}

	return {-1, SORT_NONE};
}

void listbox::set_content_size(const point& origin, const point& size)
{
	/** @todo This function needs more testing. */
	assert(content_grid());

	const int best_height = content_grid()->get_best_size().y;
	const point s(size.x, size.y < best_height ? size.y : best_height);

	content_grid()->place(origin, s);
}

void listbox::layout_children(const bool force)
{
	assert(content_grid());

	if(need_layout_ || force) {
		const int selected_item = generator_->get_selected_item();

		content_grid()->place(content_grid()->get_origin(),
							  content_grid()->get_size());

		const SDL_Rect& visible = content_visible_area_;

		content_grid()->set_visible_rectangle(visible);

		if(selected_item != -1) {
			SDL_Rect rect = generator_->item(selected_item).get_rectangle();

			rect.x = visible.x;
			rect.w = visible.w;

			show_content_rect(rect);
		}

		need_layout_ = false;
		set_is_dirty(true);
	}
}

const std::string& listbox::get_control_type() const
{
	static const std::string type = "listbox";
	return type;
}

// }---------- DEFINITION ---------{

listbox_definition::listbox_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing listbox " << id << '\n';

	load_resolutions<resolution>(cfg);
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
listbox_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t in listbox.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{listbox_description}
 *
 *        A listbox is a styled_widget that holds several items of the same type.
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

static std::vector<std::map<std::string, string_map>> parse_list_data(const config& data, const unsigned int req_cols)
{
	std::vector<std::map<std::string, string_map>> list_data;
	for(const auto & row : data.child_range("row"))
	{
		auto cols = row.child_range("column");
		VALIDATE(static_cast<unsigned>(cols.size()) == req_cols, _("'list_data' must have the same number of columns as the 'list_definition'."));

		for(const auto & c : cols)
		{
			list_data.emplace_back();
			for(const auto & i : c.attribute_range())
			{
				list_data.back()[""][i.first] = i.second;
			}
			for(const auto& w : c.child_range("widget"))
			{
				VALIDATE(w.has_attribute("id"), missing_mandatory_wml_key("[list_data][row][column][widget]", "id"));
				for(const auto& i : w.attribute_range()) {
					list_data.back()[w["id"]][i.first] = i.second;
				}
			}
		}
	}
	return list_data;
}

builder_listbox::builder_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, header(nullptr)
	, footer(nullptr)
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	if(const config& h = cfg.child("header")) {
		header = std::make_shared<builder_grid>(h);
	}

	if(const config& f = cfg.child("footer")) {
		footer = std::make_shared<builder_grid>(f);
	}

	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = std::make_shared<builder_grid>(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.child("list_data"), list_builder->cols);
	}
}

widget* builder_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	list_view* widget = new list_view(
			true, true, generator_base::vertical_list, true, list_builder);

	//init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	listbox* widget
			= new listbox(*this, has_minimum_, has_maximum_, generator_base::vertical_list, true);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(header, footer, list_data);

	return widget;
#endif
}

/*WIKI_MACRO
 * @begin{macro}{horizontal_listbox_description}
 *
 *        A horizontal listbox is a styled_widget that holds several items of the
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

builder_horizontal_listbox::builder_horizontal_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = std::make_shared<builder_grid>(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.child("list_data"), list_builder->cols);
	}
}

widget* builder_horizontal_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	list_view* widget = new list_view(
			true, true, generator_base::horizontal_list, true, list_builder);

	//init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	listbox* widget
			= new listbox(*this, has_minimum_, has_maximum_, generator_base::horizontal_list, true);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(nullptr, nullptr, list_data);

	return widget;
#endif
}

/*WIKI_MACRO
 * @begin{macro}{grid_listbox_description}
 *
 *        A grid listbox is a styled_widget that holds several items of the
 *        same type.  Normally the items in a listbox are ordered in rows,
 *        this version orders them in a grid instead.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_grid_listbox
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="grid_listbox"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Horizontal listbox ==
 *
 * @macro = grid_listbox_description
 *
 * List with the grid listbox specific variables:
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
 *     has_minimum & bool & true &     If false, less than one cell can be selected. $
 *
 *     has_maximum & bool & true &     If false, more than one cell can be selected. $
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
 * In order to force widgets to be the same size inside a grid listbox,
 * the widgets need to be inside a linked_group.
 *
 * Inside the list section there are only the following widgets allowed
 * * grid (to nest)
 * * selectable widgets which are
 * ** toggle_button
 * ** toggle_panel
 * @end{tag}{name="grid_listbox"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

builder_grid_listbox::builder_grid_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	const config& l = cfg.child("list_definition");

	VALIDATE(l, _("No list defined."));
	list_builder = std::make_shared<builder_grid>(l);
	assert(list_builder);
	VALIDATE(list_builder->rows == 1,
			 _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.child("list_data"), list_builder->cols);
	}
}

widget* builder_grid_listbox::build() const
{
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	list_view* widget = new list_view(
			true, true, generator_base::grid, true, list_builder);

	//init_control(widget);
	if(!list_data.empty()) {
		widget->append_rows(list_data);
	}
	return widget;
#else
	listbox* widget
			= new listbox(*this, has_minimum_, has_maximum_, generator_base::table, true);

	widget->set_list_builder(list_builder); // FIXME in finalize???

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(conf->grid);

	widget->finalize(nullptr, nullptr, list_data);

	return widget;
#endif
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
