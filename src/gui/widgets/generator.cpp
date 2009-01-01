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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/generator_private.hpp"

#include "gui/widgets/window.hpp"

namespace gui2 {

namespace policy {

/***** ***** ***** ***** Minimum selection ***** ***** ***** *****/

namespace minimum_selection {

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
	if(is_selected(index)) {
		do_deselect_item(index);

		if(get_selected_item_count() == 0) {

			// Are there items left?
			const unsigned item_count = get_item_count();
			if(item_count > 1) {
				// Is the last item deselected?
				if(index == item_count - 1) {
					// Select the second last.
					do_select_item(index - 2);
				} else {
					// Select the next item.
					do_select_item(index + 1);
				}
			}
		}
	}
}

} // namespace minimum_selection

/***** ***** ***** ***** Placement ***** ***** ***** *****/

namespace placement {

tvertical_list::tvertical_list()
	: placed_(false)
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

		const tgrid& grid = get_item(i);

		const tpoint best_size = grid.get_best_size();

		if(best_size.x > result.x) {
			result.x = best_size.x;
		}

		result.y += best_size.y;
	}

	return result;
}

void tvertical_list::set_size(const tpoint& origin, const tpoint& size)
{
	/*
	 * - Set every item to it's best size.
	 * - The origin gets increased with the height of the last item.
	 * - No item should be wider as the size.
	 * - In the end the origin should be the sum or the origin and the wanted
	 *   height.
	 */

	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = get_item(i);

		tpoint best_size = grid.get_best_size();
		assert(best_size.x <= size.x);
		// FIXME should we look at grow factors???
		best_size.x = size.x;

		/*
		 * For set_size to work properly, we need to disable the parent
		 * temporary. Without a parent the screen coordinates won't be
		 * remapped, which is wanted in this case. For event handling the
		 * parent is needed.
		 */
		twidget* parent = grid.parent();
		grid.set_parent(NULL);
		grid.set_size(current_origin, best_size);
		grid.set_parent(parent);

		current_origin.y += best_size.y;
	}

	assert(current_origin.y == origin.y + size.y);
}

void tvertical_list::set_origin(const tpoint& origin)
{
	tpoint current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = get_item(i);

		grid.set_origin(current_origin);
		current_origin.y += grid.get_height();
	}
}

void tvertical_list::set_visible_area(const SDL_Rect& area)
{
	/*
	 * Note for most implementations this function could work only for the
	 * tindependant class it probably fails. Evalute to make a generic
	 * function in the tgenerator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = get_item(i);
		grid.set_visible_area(area);
	}
}

twidget* tvertical_list::find_widget(
		const tpoint& coordinate, const bool must_be_active)
{
	twindow* window = get_window();
	assert(window);

	const tpoint screen_coordinate =
	   		window->screen_position(coordinate);

	for(size_t i = 0; i < get_item_count(); ++i) {

		tgrid& grid = get_item(i);

		twidget* widget =
				grid.find_widget2(screen_coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

const twidget* tvertical_list::find_widget(const tpoint& coordinate,
		const bool must_be_active) const
{
	const twindow* window = get_window();
	assert(window);

	const tpoint screen_coordinate =
	   		window->screen_position(coordinate);

	for(size_t i = 0; i < get_item_count(); ++i) {

		const tgrid& grid = get_item(i);

		const twidget* widget =
				grid.find_widget2(screen_coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return NULL;
}

} // namespace placement

/***** ***** ***** ***** Select action ***** ***** ***** *****/

namespace select_action {

void tselect::select(tgrid& grid, const bool select)
{
	tselectable_* selectable =
			dynamic_cast<tselectable_*>(grid.widget(0, 0));
	assert(selectable);

	selectable->set_value(select);
}

} // namespace select_action

} // namespace policy

/***** ***** ***** ***** Helper macros ***** ***** ***** *****/

#ifdef GENERATE_PLACEMENT
char compile_assert[0];
#else
#define GENERATE_PLACEMENT                                 \
switch(placement) {                                        \
	case tgenerator_::horizontal_list :                    \
		result = new tgenerator                            \
				< minimum                                  \
				, maximum                                  \
				, policy::placement::thorizontal_list      \
				, select                                   \
				>;                                         \
		break;                                             \
	case tgenerator_::vertical_list :                      \
		result = new tgenerator                            \
				< minimum                                  \
				, maximum                                  \
				, policy::placement::tvertical_list        \
				, select                                   \
				>;                                         \
		break;                                             \
	case tgenerator_::grid :                               \
		result = new tgenerator                            \
				< minimum                                  \
				, maximum                                  \
				, policy::placement::tmatrix               \
				, select                                   \
				>;                                         \
		break;                                             \
	case tgenerator_::independant :                        \
		result = new tgenerator                            \
				< minimum                                  \
				, maximum                                  \
				, policy::placement::tindependant          \
				, select                                   \
				>;                                         \
		break;                                             \
	default:                                               \
		assert(false);                                     \
}
#endif

#ifdef GENERATE_SELECT
char compile_assert[0];
#else
#define GENERATE_SELECT                                    \
if(select) {                                               \
	typedef policy::select_action::tselect select;         \
	GENERATE_PLACEMENT                                     \
} else {                                                   \
	typedef policy::select_action::tshow select;           \
	GENERATE_PLACEMENT                                     \
}
#endif

#ifdef GENERATE_MAXIMUM
char compile_assert[0];
#else
#define GENERATE_MAXIMUM                                   \
if(has_maximum) {                                          \
	typedef policy::maximum_selection::tone maximum;       \
	GENERATE_SELECT                                        \
} else {                                                   \
	typedef policy::maximum_selection::tinfinite maximum;  \
	GENERATE_SELECT                                        \
}
#endif

#ifdef GENERATE_BODY
char compile_assert[0];
#else
#define GENERATE_BODY                                     \
if(has_minimum) {                                         \
	typedef policy::minimum_selection::tone minimum;      \
	GENERATE_MAXIMUM                                      \
} else {                                                  \
	typedef policy::minimum_selection::tnone minimum;     \
	GENERATE_MAXIMUM                                      \
}
#endif

tgenerator_* tgenerator_::build(
		const bool has_minimum, const bool has_maximum,
		const tplacement placement, const bool select)
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

