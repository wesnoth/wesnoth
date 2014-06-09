/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/listbox.hpp"
#include "gui/auxiliary/window_builder/listbox.hpp"
#include "gui/auxiliary/window_builder/horizontal_listbox.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

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

	if(!count || count > get_item_count()) {
		count = get_item_count();
	}

	int height_reduced = 0;
	for(; count; --count) {
		if(generator_->item(row).get_visible() != tvisible::invisible) {
			height_reduced += generator_->item(row).get_height();
		}
		generator_->delete_item(row);
	}

	if(height_reduced != 0) {
		resize_content(0, -height_reduced);
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
							  const int height_modification)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			  << " width_modification " << width_modification
			  << " height_modification " << height_modification << ".\n";

	if(content_resize_request(width_modification, height_modification)) {

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

	if(footer) {
		swap_grid(&grid(), content_grid(), footer->build(), "_footer_grid");
	}

	generator_->create_items(
			-1, list_builder_, list_data, callback_list_item_clicked);
	swap_grid(NULL, content_grid(), generator_, "_list_grid");
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
} // namespace gui2

#endif
