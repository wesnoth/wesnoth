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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/generator_private.hpp"

#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace policy
{

/***** ***** ***** ***** Minimum selection ***** ***** ***** *****/

namespace minimum_selection
{

void tone::set_item_shown(const unsigned index, const bool show)
{
	if(show && get_selected_item_count() == 0) {
		do_select_item(index);
	} else if(!show && is_selected(index)) {
		do_deselect_item(index);

		for(unsigned i = index + 1; i < get_item_count(); ++i) {
			if(get_item_shown(i)) {
				do_select_item(i);
				break;
			}
		}
	}
}

void tone::create_item(const unsigned index)
{
	if(get_selected_item_count() == 0) {
		do_select_item(index);
	}
}

bool tone::deselect_item(const unsigned index)
{
	if(get_selected_item_count() > 1) {
		do_deselect_item(index);
		return true;
	}
	return false;
}

void tone::delete_item(const unsigned index)
{
	/** @todo do_select_item needs to test for shown flag. */

	if(is_selected(index)) {
		do_deselect_item(index);

		if(get_selected_item_count() == 0) {

			// Are there items left?
			const unsigned item_count = get_item_count();
			if(item_count > 1) {
				// Is the last item deselected?
				if(index == item_count - 1) {
					// Select the second last.
					do_select_item(index - 1);
				} else {
					// Select the next item.
					do_select_item(index + 1);
				}
			}
		}
	}
}

void tnone::set_item_shown(const unsigned index, const bool show)
{
	if(!show && is_selected(index)) {
		do_deselect_item(index);
	}
}

} // namespace minimum_selection

/***** ***** ***** ***** Placement ***** ***** ***** *****/

namespace placement
{

thorizontal_list::thorizontal_list() : placed_(false)
{
}

void thorizontal_list::create_item(const unsigned /*index*/)
{
	if(!placed_) {
		return;
	}

	/** @todo implement. */
	assert(false);
}

tpoint thorizontal_list::calculate_best_size() const
{
	// The best size is the sum of the widths and the greatest height.
	tpoint result(0, 0);
	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		const tpoint best_size = grid.get_best_size();

		result.x += best_size.x;

		if(best_size.y > result.y) {
			result.y = best_size.y;
		}
	}

	return result;
}

void thorizontal_list::place(const tpoint& origin, const tpoint& size)
{
	/*
	 * - Set every item to its best size.
	 * - The origin gets increased with the width of the last item.
	 * - No item should be higher as the size.
	 * - In the end the origin should be the sum or the origin and the wanted
	 *   width.
	 */

	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		tpoint best_size = grid.get_best_size();
		assert(best_size.y <= size.y);
		// FIXME should we look at grow factors???
		best_size.y = size.y;

		grid.place(current_origin, best_size);

		current_origin.x += best_size.x;
	}

	assert(current_origin.x == origin.x + size.x);
}

void thorizontal_list::set_origin(const tpoint& origin)
{
	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		grid.set_origin(current_origin);
		current_origin.x += grid.get_width();
	}
}

void thorizontal_list::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Note for most implementations this function could work only for the
	 * tindependent class it probably fails. Evaluate to make a generic
	 * function in the tgenerator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

twidget* thorizontal_list::find_at(const tpoint& coordinate,
								   const bool must_be_active)
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		twidget* widget = grid.find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

const twidget* thorizontal_list::find_at(const tpoint& coordinate,
										 const bool must_be_active) const
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		const twidget* widget = grid.find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

void thorizontal_list::handle_key_left_arrow(SDLMod /*modifier*/, bool& handled)
{
	if(get_selected_item_count() == 0) {
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_selected_item() - 1; i >= 0; --i) {

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		tcontrol* control = dynamic_cast<tcontrol*>(item(i).widget(0, 0));
		if(control && control->get_active()) {
			select_item(i, true);
			return;
		}
	}
}

void thorizontal_list::handle_key_right_arrow(SDLMod /*modifier*/,
											  bool& handled)
{
	if(get_selected_item_count() == 0) {
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_selected_item() + 1; i < get_item_count(); ++i) {

		if(item(i).get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		tcontrol* control = dynamic_cast<tcontrol*>(item(i).widget(0, 0));
		if(control && control->get_active()) {
			select_item(i, true);
			return;
		}
	}
}

tvertical_list::tvertical_list() : placed_(false)
{
}

void tvertical_list::create_item(const unsigned /*index*/)
{
	if(!placed_) {
		return;
	}

	/** @todo implement. */
	assert(false);
}

tpoint tvertical_list::calculate_best_size() const
{
	// The best size is the sum of the heights and the greatest width.
	tpoint result(0, 0);
	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		const tpoint best_size = grid.get_best_size();

		if(best_size.x > result.x) {
			result.x = best_size.x;
		}

		result.y += best_size.y;
	}

	return result;
}

void tvertical_list::place(const tpoint& origin, const tpoint& size)
{
	/*
	 * - Set every item to its best size.
	 * - The origin gets increased with the height of the last item.
	 * - No item should be wider as the size.
	 * - In the end the origin should be the sum or the origin and the wanted
	 *   height.
	 */

	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		tpoint best_size = grid.get_best_size();
		assert(best_size.x <= size.x);
		// FIXME should we look at grow factors???
		best_size.x = size.x;

		grid.place(current_origin, best_size);

		current_origin.y += best_size.y;
	}

	assert(current_origin.y == origin.y + size.y);
}

void tvertical_list::set_origin(const tpoint& origin)
{
	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		grid.set_origin(current_origin);
		current_origin.y += grid.get_height();
	}
}

void tvertical_list::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Note for most implementations this function could work only for the
	 * tindependent class it probably fails. Evaluate to make a generic
	 * function in the tgenerator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

twidget* tvertical_list::find_at(const tpoint& coordinate,
								 const bool must_be_active)
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}


		twidget* widget = grid.find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

const twidget* tvertical_list::find_at(const tpoint& coordinate,
									   const bool must_be_active) const
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = item(i);
		if(grid.get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		const twidget* widget = grid.find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

void tvertical_list::handle_key_up_arrow(SDLMod /*modifier*/, bool& handled)
{
	if(get_selected_item_count() == 0) {
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_selected_item() - 1; i >= 0; --i) {

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		tcontrol* control = dynamic_cast<tcontrol*>(item(i).widget(0, 0));
		if(control && control->get_active()) {
			select_item(i, true);
			return;
		}
	}
}

void tvertical_list::handle_key_down_arrow(SDLMod /*modifier*/, bool& handled)
{
	if(get_selected_item_count() == 0) {
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_selected_item() + 1; i < get_item_count(); ++i) {

		if(item(i).get_visible() == twidget::tvisible::invisible
		   || !get_item_shown(i)) {

			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		tcontrol* control = dynamic_cast<tcontrol*>(item(i).widget(0, 0));
		if(control && control->get_active()) {
			select_item(i, true);
			return;
		}
	}
}

void tindependent::request_reduce_width(const unsigned maximum_width)
{
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.request_reduce_width(maximum_width);
	}
}

void tindependent::request_reduce_height(const unsigned maximum_height)
{
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.request_reduce_height(maximum_height);
	}
}

tpoint tindependent::calculate_best_size() const
{
	/*
	 * The best size is the combination of the greatest width and greatest
	 * height.
	 */
	tpoint result(0, 0);
	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = item(i);

		const tpoint best_size = grid.get_best_size();

		if(best_size.x > result.x) {
			result.x = best_size.x;
		}

		if(best_size.y > result.y) {
			result.y = best_size.y;
		}
	}

	return result;
}

void tindependent::place(const tpoint& origin, const tpoint& size)
{
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.place(origin, size);
	}
}

void tindependent::set_origin(const tpoint& origin)
{
	/*
	 * Set the origin for every item.
	 *
	 * @todo evaluate whether setting it only for the visible item is better
	 * and what the consequences are.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.set_origin(origin);
	}
}

twidget* tindependent::find_at(const tpoint& coordinate,
							   const bool must_be_active)
{
	assert(get_window());

	const int selected_item = get_selected_item();
	if(selected_item < 0) {
		return NULL;
	}

	tgrid& grid = item(selected_item);
	return grid.find_at(coordinate, must_be_active);
}

const twidget* tindependent::find_at(const tpoint& coordinate,
									 const bool must_be_active) const
{
	assert(get_window());

	const int selected_item = get_selected_item();
	if(selected_item < 0) {
		return NULL;
	}

	const tgrid& grid = item(selected_item);
	return grid.find_at(coordinate, must_be_active);
}

twidget* tindependent::find(const std::string& id, const bool must_be_active)
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(is_selected(i)) {
			if(twidget* widget = item(i).find(id, must_be_active)) {
				return widget;
			}
		}
	}
	return NULL;
}

const twidget* tindependent::find(const std::string& id,
								  const bool must_be_active) const
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(is_selected(i)) {
			if(const twidget* widget = item(i).find(id, must_be_active)) {

				return widget;
			}
		}
	}
	return NULL;
}

void tindependent::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Set the visible rectangle for every item.
	 *
	 * @todo evaluate whether setting it only for the visible item is better
	 * and what the consequences are.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

} // namespace placement

/***** ***** ***** ***** Select action ***** ***** ***** *****/

namespace select_action
{

void tselect::select(tgrid& grid, const bool select)
{
	tselectable_* selectable = dynamic_cast<tselectable_*>(grid.widget(0, 0));
	assert(selectable);

	selectable->set_value(select);
}

void
tselect::init(tgrid* grid,
			  const std::map<std::string /* widget id */, string_map>& data,
			  const boost::function<void(twidget&)>& callback)
{
	for(unsigned row = 0; row < grid->get_rows(); ++row) {
		for(unsigned col = 0; col < grid->get_cols(); ++col) {
			twidget* widget = grid->widget(row, col);
			assert(widget);

			tgrid* child_grid = dynamic_cast<tgrid*>(widget);
			ttoggle_button* btn = dynamic_cast<ttoggle_button*>(widget);
			ttoggle_panel* panel = dynamic_cast<ttoggle_panel*>(widget);

			if(btn) {
				btn->set_callback_state_change(callback);
				std::map<std::string, string_map>::const_iterator itor
						= data.find(btn->id());

				if(itor == data.end()) {
					itor = data.find("");
				}
				if(itor != data.end()) {
					btn->set_members(itor->second);
				}
			} else if(panel) {
				panel->set_callback_state_change(callback);
				panel->set_child_members(data);
			} else if(child_grid) {
				init(child_grid, data, callback);
			} else {
				VALIDATE(false,
						 "Only toggle buttons and panels are allowed as "
						 "the cells of a list definition.");
			}
		}
	}
}

void tshow::init(tgrid* grid,
				 const std::map<std::string /* widget id */, string_map>& data,
				 const boost::function<void(twidget&)>& callback)
{
	assert(!callback);

	FOREACH(const AUTO & item, data)
	{
		if(item.first.empty()) {
			for(unsigned row = 0; row < grid->get_rows(); ++row) {
				for(unsigned col = 0; col < grid->get_cols(); ++col) {
					if(tcontrol* control
					   = dynamic_cast<tcontrol*>(grid->widget(row, col))) {

						control->set_members(item.second);
					}
				}
			}
		} else {
			tcontrol* control
					= dynamic_cast<tcontrol*>(grid->find(item.first, false));
			if(control) {
				control->set_members(item.second);
			}
		}
	}
}

} // namespace select_action

} // namespace policy

/***** ***** ***** ***** Helper macros ***** ***** ***** *****/

#ifdef GENERATE_PLACEMENT
char compile_assert[0];
#else
#define GENERATE_PLACEMENT                                                     \
	switch(placement) {                                                        \
		case tgenerator_::horizontal_list:                                     \
			result = new tgenerator<minimum,                                   \
									maximum,                                   \
									policy::placement::thorizontal_list,       \
									select>;                                   \
			break;                                                             \
		case tgenerator_::vertical_list:                                       \
			result = new tgenerator<minimum,                                   \
									maximum,                                   \
									policy::placement::tvertical_list,         \
									select>;                                   \
			break;                                                             \
		case tgenerator_::grid:                                                \
			result = new tgenerator<minimum,                                   \
									maximum,                                   \
									policy::placement::tmatrix,                \
									select>;                                   \
			break;                                                             \
		case tgenerator_::independent:                                         \
			result = new tgenerator<minimum,                                   \
									maximum,                                   \
									policy::placement::tindependent,           \
									select>;                                   \
			break;                                                             \
		default:                                                               \
			assert(false);                                                     \
	}
#endif

#ifdef GENERATE_SELECT
char compile_assert[0];
#else
#define GENERATE_SELECT                                                        \
	if(select) {                                                               \
		typedef policy::select_action::tselect select;                         \
		GENERATE_PLACEMENT                                                     \
	} else {                                                                   \
		typedef policy::select_action::tshow select;                           \
		GENERATE_PLACEMENT                                                     \
	}
#endif

#ifdef GENERATE_MAXIMUM
char compile_assert[0];
#else
#define GENERATE_MAXIMUM                                                       \
	if(has_maximum) {                                                          \
		typedef policy::maximum_selection::tone maximum;                       \
		GENERATE_SELECT                                                        \
	} else {                                                                   \
		typedef policy::maximum_selection::tinfinite maximum;                  \
		GENERATE_SELECT                                                        \
	}
#endif

#ifdef GENERATE_BODY
char compile_assert[0];
#else
#define GENERATE_BODY                                                          \
	if(has_minimum) {                                                          \
		typedef policy::minimum_selection::tone minimum;                       \
		GENERATE_MAXIMUM                                                       \
	} else {                                                                   \
		typedef policy::minimum_selection::tnone minimum;                      \
		GENERATE_MAXIMUM                                                       \
	}
#endif

tgenerator_* tgenerator_::build(const bool has_minimum,
								const bool has_maximum,
								const tplacement placement,
								const bool select)
{
	tgenerator_* result = NULL;
	GENERATE_BODY;
	return result;
}

/***** ***** ***** ***** Test code ***** ***** ***** *****/
#if 0
namespace {

void pointer_test()
{

	tgenerator_ *a = tgenerator_::build(
			true, true, tgenerator_::horizontal_list, true);

	tgenerator_ *b = tgenerator_::build(
			true, false, tgenerator_::horizontal_list, true);

	tgenerator_ *c = tgenerator_::build(
			false, true, tgenerator_::horizontal_list, true);

	tgenerator_ *d = tgenerator_::build(
			false, false, tgenerator_::horizontal_list, true);

	a->clear();
	b->clear();
	c->clear();
	d->clear();

	delete a;
	delete b;
	delete c;
	delete d;
}

void direct_test()
{
	tgenerator
		< policy::minimum_selection::tone
		, policy::maximum_selection::tone
		, policy::placement::tvertical_list
		, policy::select_action::tselect
		> a;

	tgenerator
		< policy::minimum_selection::tone
		, policy::maximum_selection::tinfinite
		, policy::placement::tvertical_list
		, policy::select_action::tselect
		> b;

	tgenerator
		< policy::minimum_selection::tnone
		, policy::maximum_selection::tone
		, policy::placement::tvertical_list
		, policy::select_action::tselect
		> c;

	tgenerator
		< policy::minimum_selection::tnone
		, policy::maximum_selection::tinfinite
		, policy::placement::tvertical_list
		, policy::select_action::tselect
		> d;

	a.clear();
	b.clear();
	c.clear();
	d.clear();

}

} // namespace
#endif

} // namespace gui2
