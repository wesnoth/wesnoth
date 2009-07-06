/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/listbox.hpp"

#include "gui/widgets/window.hpp"

namespace gui2 {

namespace {

void callback_list_item_clicked(twidget* caller)
{
	get_parent<tlistbox>(caller)->list_item_clicked(caller);
}

} // namespace

tlistbox::tlistbox(const bool has_minimum, const bool has_maximum,
		const tgenerator_::tplacement placement, const bool select)
	: tscrollbar_container(2) // FIXME magic number
	, generator_(NULL)
	, list_builder_(NULL)
	, callback_value_changed_(NULL)
	, linked_size_initialized_(false)
{
	generator_ = tgenerator_::build(
			has_minimum, has_maximum, placement, select);
}

void tlistbox::add_row(const string_map& item)
{
	assert(generator_);
	generator_->create_item(
			-1, list_builder_, item, callback_list_item_clicked);

	tgrid& grid = generator_->get_item(get_item_count() - 1);
	twindow* window = get_window();
	assert(window);

	if(get_item_count() == 1) {
		init_linked_size_widets(*window, grid.begin(), grid.end());

		/**
		 * @todo add footer and make one function to do this part since the
		 * code is duplicated at two places.
		 */
		tgrid* header_grid = dynamic_cast<tgrid*>(
				content_grid()->find_widget("_header_grid", false));

		if(header_grid) {
			add_linked_size_widgets(*window, header_grid->begin(),
					header_grid->end());
		}
	}
	add_linked_size_widgets(*window, grid.begin(), grid.end());
}

void tlistbox::add_row(
		const std::map<std::string /* widget id */, string_map>& data)
{
	assert(generator_);
	generator_->create_item(
			-1, list_builder_, data, callback_list_item_clicked);

	tgrid& grid = generator_->get_item(get_item_count() - 1);
	twindow* window = get_window();
	assert(window);

	if(!linked_size_initialized_) {
		linked_size_initialized_ = true;

		init_linked_size_widets(*window, grid.begin(), grid.end());

		tgrid* header_grid = dynamic_cast<tgrid*>(
				content_grid()->find_widget("_header_grid", false));

		if(header_grid) {
			add_linked_size_widgets(*window, header_grid->begin(),
					header_grid->end());
		}
	}
	add_linked_size_widgets(*window, grid.begin(), grid.end());
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

	for(; count; --count) {
		twindow* window = get_window();
		assert(window);

		tgrid* grid = get_row_grid(row);
		assert(grid);

		remove_linked_size_widgets(*window, grid->begin(), grid->end());

		generator_->delete_item(row);
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
	generator_->get_item(row).set_active(active);
}

const tgrid* tlistbox::get_row_grid(const unsigned row) const
{
	assert(generator_);
	// rename this function and can we return a reference??
	return &generator_->get_item(row);
}

tgrid* tlistbox::get_row_grid(const unsigned row)
{
	assert(generator_);
	return &generator_->get_item(row);
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

void tlistbox::list_item_clicked(twidget* caller)
{
	assert(caller);
	assert(generator_);

	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	for(size_t i = 0; i < generator_->get_item_count(); ++i) {

		if(generator_->get_item(i).has_widget(caller)) {
			generator_->toggle_item(i);
			if(callback_value_changed_) {
				callback_value_changed_(this);
			}
			return;
		}
	}
	assert(false);
}

void tlistbox::child_populate_dirty_list(twindow& caller,
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
		SDL_Rect rect = generator_->get_item(
				generator_->get_selected_item()).get_rect();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(this);
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
		SDL_Rect rect = generator_->get_item(
				generator_->get_selected_item()).get_rect();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);

		if(callback_value_changed_) {
			callback_value_changed_(this);
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
	if(!handled) {
		tscrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void tlistbox::handle_key_right_arrow(SDLMod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_right_arrow(modifier, handled);

	// Inherited.
	if(!handled) {
		tscrollbar_container::handle_key_right_arrow(modifier, handled);
	}
}

void tlistbox::init_linked_size_widets(twindow& window,
		const tgrid::iterator& begin, const tgrid::iterator& end)
{
	for(tgrid::iterator itor = begin; itor != end; ++itor) {

		assert(*itor);

		// Add to list.
		if(!itor->id().empty()) {
			window.init_linked_size_group(itor->id(), true, false);
		}

		// Recurse though the children.
		tcontainer_* container = dynamic_cast<tcontainer_*>(*itor);
		if(container) {
			init_linked_size_widets(window,
					container->begin(), container->end());
		}
	}
}

void tlistbox::add_linked_size_widgets(twindow& window,
		const tgrid::iterator& begin, const tgrid::iterator& end)
{
	for(tgrid::iterator itor = begin; itor != end; ++itor) {

		assert(*itor);

		// Add to list.
		if(!itor->id().empty()) {
			window.add_linked_widget(itor->id(), *itor);
		}

		// Recurse though the children.
		tcontainer_* container = dynamic_cast<tcontainer_*>(*itor);
		if(container) {
			add_linked_size_widgets(window,
					container->begin(), container->end());
		}
	}
}

void tlistbox::remove_linked_size_widgets(twindow& window,
		const tgrid::iterator& begin, const tgrid::iterator& end)
{
	for(tgrid::iterator itor = begin; itor != end; ++itor) {

		assert(*itor);

		// Add to list.
		if(!itor->id().empty()) {
			window.remove_linked_widget(itor->id(), *itor);
		}

		// Recurse though the children.
		tcontainer_* container = dynamic_cast<tcontainer_*>(*itor);
		if(container) {
			remove_linked_size_widgets(window,
					container->begin(), container->end());
		}
	}
}

namespace {

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
		tgrid* content_grid, twidget* widget, const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = grid->find_widget<tgrid>(id, false, false);
	}
	if(!parent_grid) {
		parent_grid = content_grid->find_widget<tgrid>(id, true, false);
	}
	parent_grid = dynamic_cast<tgrid*>(parent_grid->parent());
	assert(parent_grid);

	// Replace the child.
	widget = parent_grid->swap_child(id, widget, false);
	assert(widget);

	delete widget;
}

} // namespace

void tlistbox::finalize(
		tbuilder_grid_const_ptr header,
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

} // namespace gui2

