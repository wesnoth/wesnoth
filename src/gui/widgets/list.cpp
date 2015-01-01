/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef GUI2_EXPERIMENTAL_LISTBOX

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/list.hpp"

#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/listbox.hpp"
#include "gui/auxiliary/window_builder/listbox.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

#ifdef GUI2_EXPERIMENTAL_LISTBOX
REGISTER_WIDGET(listbox)
#endif

tlist::tlist(const bool has_minimum,
			 const bool has_maximum,
			 const tgenerator_::tplacement placement,
			 const bool select,
			 const tbuilder_grid_const_ptr list_builder)
	: tcontainer_(2) // FIXME magic number
	, state_(ENABLED)
	, generator_(NULL)
	, list_builder_(list_builder)
	, need_layout_(false)
{
	assert(list_builder);

	generator_
			= tgenerator_::build(has_minimum, has_maximum, placement, select);
	assert(generator_);

	connect_signal<event::LEFT_BUTTON_DOWN>(
			boost::bind(&tlist::signal_handler_left_button_down, this, _2),
			event::tdispatcher::back_pre_child);

	connect_signal<event::SDL_KEY_DOWN>(boost::bind(
			&tlist::signal_handler_sdl_key_down, this, _2, _3, _5, _6));

	connect_signal<event::SDL_KEY_DOWN>(
			boost::bind(
					&tlist::signal_handler_sdl_key_down, this, _2, _3, _5, _6),
			event::tdispatcher::back_pre_child);
}

void tlist::add_row(const string_map& item, const int index)
{
	std::map<std::string, string_map> data;

	data.insert(std::make_pair("", item));
	add_row(data, index);
}

void
tlist::add_row(const std::map<std::string /* widget id */, string_map>& data,
			   const int index)
{
	assert(generator_);
	tgrid& grid = generator_->create_item(index, list_builder_, data, NULL);

	tselectable_* selectable
			= find_widget<tselectable_>(&grid, "_toggle", false, false);

	if(selectable) {
		dynamic_cast<twidget&>(*selectable)
				.connect_signal<event::LEFT_BUTTON_CLICK>(
						 boost::bind(
								 &tlist::signal_handler_pre_child_left_button_click,
								 this,
								 &grid,
								 _2,
								 _3,
								 _4),
						 event::tdispatcher::back_pre_child);

		// Post widget for panel.
		dynamic_cast<twidget&>(*selectable)
				.connect_signal<event::LEFT_BUTTON_CLICK>(
						 boost::bind(&tlist::signal_handler_left_button_click,
									 this,
									 &grid,
									 _2),
						 event::tdispatcher::back_post_child);

		// Post widget for button and widgets on the panel.
		dynamic_cast<twidget&>(*selectable)
				.connect_signal<event::LEFT_BUTTON_CLICK>(
						 boost::bind(&tlist::signal_handler_left_button_click,
									 this,
									 &grid,
									 _2),
						 event::tdispatcher::back_child);
	}
}

void tlist::append_rows(const std::vector<string_map>& items)
{
	foreach(const string_map & item, items)
	{
		add_row(item);
	}
}

void tlist::remove_row(const unsigned row, unsigned count)
{
	assert(generator_);

	if(row >= get_item_count()) {
		return;
	}

	if(!count || count > get_item_count()) {
		count = get_item_count();
	}

	unsigned height_reduced = 0;
	for(; count; --count) {
		if(generator_->item(row).get_visible() != tvisible::invisible) {
			height_reduced += generator_->item(row).get_height();
		}
		generator_->delete_item(row);
	}

	if(height_reduced != 0) {
		// resize_content(0, -height_reduced);
	}
}

void tlist::clear()
{
	// Due to the removing from the linked group, don't use
	// generator_->clear() directly.
	remove_row(0, 0);
}

unsigned tlist::get_item_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void tlist::set_row_active(const unsigned row, const bool active)
{
	assert(generator_);
	generator_->item(row).set_active(active);
}

void tlist::set_row_shown(const unsigned row, const bool shown)
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
		// resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		// grid().set_visible_rectangle(content_visible_rectangle());
		set_dirty(true);
	}

	if(selected_row != get_selected_row()) {
		fire(event::NOTIFY_MODIFIED, *this, NULL);
	}
}

void tlist::set_row_shown(const std::vector<bool>& shown)
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
		// resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		// content_grid_->set_visible_rectangle(content_visible_rectangle());
		set_dirty(true);
	}

	if(selected_row != get_selected_row()) {
		fire(event::NOTIFY_MODIFIED, *this, NULL);
	}
}

const tgrid* tlist::get_row_grid(const unsigned row) const
{
	assert(generator_);
	// rename this function and can we return a reference??
	return &generator_->item(row);
}

tgrid* tlist::get_row_grid(const unsigned row)
{
	assert(generator_);
	return &generator_->item(row);
}

bool tlist::select_row(const unsigned row, const bool select)
{
	assert(generator_);

	generator_->select_item(row, select);

	return true; // FIXME test what result should have been!!!
}

int tlist::get_selected_row() const
{
	assert(generator_);

	return generator_->get_selected_item();
}

void tlist::place(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	tcontainer_::place(origin, size);

	/**
	 * @todo Work-around to set the selected item visible again.
	 *
	 * At the moment the lists and dialogs in general are resized a lot as
	 * work-around for sizing. So this function makes the selected item in view
	 * again. It doesn't work great in all cases but the proper fix is to avoid
	 * resizing dialogs a lot. Need more work later on.
	 */
	const int selected_item = generator_->get_selected_item();
	if(selected_item != -1) {
		/*
				const SDL_Rect& visible = content_visible_area();
				SDL_Rect rect = generator_->item(selected_item).get_rectangle();

				rect.x = visible.x;
				rect.w = visible.w;

				show_content_rect(rect);
		*/
	}
}
#if 0
void tlist::resize_content(
		  const int width_modification
		, const int height_modification)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size()
			<< " width_modification " << width_modification
			<< " height_modification " << height_modification
			<< ".\n";

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
			set_dirty(true);
		}
		DBG_GUI_L << LOG_HEADER << " succeeded.\n";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.\n";
	}
}
#endif

void tlist::init()
{
	init_grid(cast<tlistbox_definition::tresolution>(config()).grid);

	set_single_child(find_widget<tgrid>(&grid(), "_list_grid", false),
					 generator_);

	/*
	 * These items should be managed by the new listbox class.
	 * So make them invisible for now.
	 */
	tgrid* g = find_widget<tgrid>(&grid(), "_header_grid", false, false);
	if(g)
		g->set_visible(twidget::tvisible::invisible);

	g = find_widget<tgrid>(&grid(), "_footer_grid", false, false);
	if(g)
		g->set_visible(twidget::tvisible::invisible);

	g = find_widget<tgrid>(&grid(), "_vertical_scrollbar_grid", false, false);
	if(g)
		g->set_visible(twidget::tvisible::invisible);

	g = find_widget<tgrid>(&grid(), "_horizontal_scrollbar_grid", false, false);
	if(g)
		g->set_visible(twidget::tvisible::invisible);
}

bool tlist::get_active() const
{
	return state_ != DISABLED;
}

unsigned tlist::get_state() const
{
	return state_;
}

void tlist::layout_children(const bool force)
{
	if(need_layout_ || force) {
		grid().place(grid().get_origin(), grid().get_size());

		/*
				grid().set_visible_rectangle(content_visible_area_);
		*/
		need_layout_ = false;
		set_dirty(true);
	}
}

void tlist::set_self_active(const bool active)
{
	/* DO NOTHING */
}

const std::string& tlist::get_control_type() const
{
	static const std::string type = "list";
	return type;
}

void tlist::signal_handler_left_button_down(const event::tevent event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	assert(get_window());
	get_window()->keyboard_capture(this);
}

void tlist::signal_handler_pre_child_left_button_click(
		tgrid* grid, const event::tevent event, bool& handled, bool& halt)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	assert(grid);
	assert(generator_);

	for(size_t i = 0; i < generator_->get_item_count(); ++i) {
		if(&generator_->item(i) == grid) {

			/**
			 * @todo Here we should check whether the panel can be toggled.
			 *
			 * NO set halt + handled
			 * YES do nothing
			 *
			 * Then a post to the widget, which if done sets the proper state
			 * in the list.
			 *
			 * For now we simply assume an item can only be selected and not
			 * deselected (which is true at the moment).
			 */
			if(generator_->is_selected(i)) {
				halt = true;
				handled = true;
			}
			return;
		}
	}
	assert(false);
}

void tlist::signal_handler_left_button_click(tgrid* grid,
											 const event::tevent event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";
	assert(grid);
	assert(generator_);

	/** @todo Test the proper state to set. */
	for(size_t i = 0; i < generator_->get_item_count(); ++i) {
		if(&generator_->item(i) == grid) {
			generator_->select_item(i);
			fire(event::NOTIFY_MODIFIED, *this, NULL);
		}
	}
}

void tlist::signal_handler_sdl_key_down(const event::tevent event,
										bool& handled,
										const SDLKey key,
										SDLMod modifier)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(handled) {
		return;
	}

	switch(key) {
		case SDLK_UP:
			generator_->handle_key_up_arrow(modifier, handled);
			break;
		case SDLK_DOWN:
			generator_->handle_key_down_arrow(modifier, handled);
			break;
		case SDLK_LEFT:
			generator_->handle_key_left_arrow(modifier, handled);
			break;
		case SDLK_RIGHT:
			generator_->handle_key_right_arrow(modifier, handled);
			break;
		default:
			;
			/* Do nothing. */
	}
	if(handled) {
		fire(event::NOTIFY_MODIFIED, *this, NULL);
	}
}

} // namespace gui2

#endif
