/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "wml_exception.hpp"

#include <numeric>

namespace gui2
{
namespace policy
{
/***** ***** ***** ***** Minimum selection ***** ***** ***** *****/

namespace minimum_selection
{
void one_item::set_item_shown(const unsigned index, const bool show)
{
	if(show && get_selected_item_count() == 0) {
		do_select_item(index);
	} else if(!show && is_selected(index)) {
		do_deselect_item(index);

		for(unsigned i = 1; i < get_item_count(); ++i) {
			unsigned new_index = (index + i) % get_item_count();
			if(get_item_shown(new_index)) {
				do_select_item(new_index);
				break;
			}
		}
	}
}

void one_item::create_item(const unsigned index)
{
	if(get_selected_item_count() == 0) {
		do_select_item(index);
	}
}

bool one_item::deselect_item(const unsigned index)
{
	if(get_selected_item_count() > 1) {
		do_deselect_item(index);
		return true;
	}

	return false;
}

void one_item::delete_item(const unsigned index)
{
	/** @todo do_select_item needs to test for shown flag. */

	if(is_selected(index)) {
		do_deselect_item(index);

		if(get_selected_item_count() == 0) {
			// Are there items left?
			const unsigned item_count = get_item_count();
			const unsigned visible_index = get_ordered_index(index);

			if(item_count > 1) {
				// Is the last item deselected?
				if(visible_index == item_count - 1) {
					// Select the second last.
					do_select_item(get_item_at_ordered(visible_index - 1));
				} else {
					// Select the next item.
					do_select_item(get_item_at_ordered(visible_index + 1));
				}
			}
		}
	}
}

void no_item::set_item_shown(const unsigned index, const bool show)
{
	if(!show && is_selected(index)) {
		do_deselect_item(index);
	}
}

} // namespace minimum_selection

/***** ***** ***** ***** Placement ***** ***** ***** *****/

namespace placement
{
horizontal_list::horizontal_list()
	: placed_(false)
{
}

void horizontal_list::create_item(const unsigned /*index*/)
{
	if(!placed_) {
		return;
	}

	/** @todo implement. */
	assert(false);
}

point horizontal_list::calculate_best_size() const
{
	// The best size is the sum of the widths and the greatest height.
	point result(0, 0);

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		const point best_size = item(i).get_best_size();

		result.x += best_size.x;

		if(best_size.y > result.y) {
			result.y = best_size.y;
		}
	}

	return result;
}

void horizontal_list::place(const point& origin, const point& size)
{
	/*
	 * - Set every item to its best size.
	 * - The origin gets increased with the width of the last item.
	 * - No item should be higher as the size.
	 * - In the end the origin should be the sum or the origin and the wanted
	 *   width.
	 */

	point current_origin = origin;

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		point best_size = grid.get_best_size();
		assert(best_size.y <= size.y);

		// FIXME should we look at grow factors???
		best_size.y = size.y;

		grid.place(current_origin, best_size);

		current_origin.x += best_size.x;
	}

	if(current_origin.x != origin.x + size.x) {
		ERR_GUI_L << "Failed to fit horizontal list to requested rect; expected right edge was " << origin.x + size.x;
		ERR_GUI_L << ", actual right edge was " << current_origin.x;
		ERR_GUI_L << " (left edge is " << origin.x << ")\n";
	}
}

void horizontal_list::set_origin(const point& origin)
{
	point current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		grid.set_origin(current_origin);

		current_origin.x += grid.get_width();
	}
}

void horizontal_list::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Note for most implementations this function could work only for the
	 * independent class it probably fails. Evaluate to make a generic
	 * function in the generator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item_ordered(i);
		grid.set_visible_rectangle(rectangle);
	}
}

widget* horizontal_list::find_at(const point& coordinate, const bool must_be_active)
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}

	return nullptr;
}

const widget* horizontal_list::find_at(const point& coordinate, const bool must_be_active) const
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		const widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}

	return nullptr;
}

void horizontal_list::handle_key_left_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(int i = get_ordered_index(get_item_count() - 1); i >= 0; i--) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}

		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_ordered_index(get_selected_item()) - 1; i >= 0; --i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item(get_item_at_ordered(i)).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void horizontal_list::handle_key_right_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(size_t i = get_ordered_index(0); i < get_item_count(); i++) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_ordered_index(get_selected_item()) + 1; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item(get_item_at_ordered(i)).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

vertical_list::vertical_list()
	: placed_(false)
{
}

void vertical_list::create_item(const unsigned /*index*/)
{
	if(!placed_) {
		return;
	}

	/** @todo implement. */
	assert(false);
}

point vertical_list::calculate_best_size() const
{
	// The best size is the sum of the heights and the greatest width.
	point result(0, 0);
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		const point best_size = item(i).get_best_size();

		if(best_size.x > result.x) {
			result.x = best_size.x;
		}

		result.y += best_size.y;
	}

	return result;
}

void vertical_list::place(const point& origin, const point& size)
{
	/*
	 * - Set every item to its best size.
	 * - The origin gets increased with the height of the last item.
	 * - No item should be wider as the size.
	 * - In the end the origin should be the sum or the origin and the wanted
	 *   height.
	 */

	point current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		point best_size = grid.get_best_size();
		assert(best_size.x <= size.x);

		// FIXME should we look at grow factors???
		best_size.x = size.x;

		grid.place(current_origin, best_size);

		current_origin.y += best_size.y;
	}

	if(current_origin.y != origin.y + size.y) {
		ERR_GUI_L << "Failed to fit vertical list to requested rect; expected bottom edge was " << origin.y + size.y;
		ERR_GUI_L << ", actual bottom edge was " << current_origin.y;
		ERR_GUI_L << " (top edge is " << origin.y << ")\n";
	}
}

void vertical_list::set_origin(const point& origin)
{
	point current_origin = origin;
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		grid.set_origin(current_origin);

		current_origin.y += grid.get_height();
	}
}

void vertical_list::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Note for most implementations this function could work only for the
	 * independent class it probably fails. Evaluate to make a generic
	 * function in the generator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

widget* vertical_list::find_at(const point& coordinate, const bool must_be_active)
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return nullptr;
}

const widget* vertical_list::find_at(const point& coordinate, const bool must_be_active) const
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		const widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return nullptr;
}

void vertical_list::handle_key_up_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(int i = get_ordered_index(get_item_count() - 1); i >= 0; i--) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_ordered_index(get_selected_item()) - 1; i >= 0; --i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item_ordered(i).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void vertical_list::handle_key_down_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(size_t i = get_ordered_index(0); i < get_item_count(); i++) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}
		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_ordered_index(get_selected_item()) + 1; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item_ordered(i).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

table::table()
	: placed_(false) //, n_cols_(2)
{
}

void table::create_item(const unsigned /*index*/)
{
	if(!placed_) {
		return;
	}

	/** @todo implement. */
	assert(false);
}

point table::calculate_best_size() const
{
	/* The best size is that which minimizes the aspect ratio of the enclosing rect.
	 * We first calculate the best size of each item, then find the number of rows
	 * that minimizes the aspect ratio. We try a number of columns from 1 up to
	 * sqrt(visible_items) + 2.
	 *
	 * @todo these calculations need rethinking since the grid layout doesn't work
	 * properly as of now.
	 *
	 * - vultraz, 2017-08-25
	 */

	size_t n_items = get_item_count();
	size_t max_cols = sqrt(n_items) + 2;

	std::vector<point> item_sizes;
	for(size_t i = 0; i < n_items; i++) {
		if(get_item_shown(i)) {
			item_sizes.push_back(item(i).get_best_size());
		}
	}

	if(item_sizes.empty()) {
		return point();
	}

	std::vector<point> best_sizes(1);

	best_sizes[0] = std::accumulate(item_sizes.begin(), item_sizes.end(), point(),
		[](point a, point b) { return point(std::max(a.x, b.x), a.y + b.y); }
	);

	int max_xtra = std::min_element(item_sizes.begin(), item_sizes.end(),
		[](point a, point b) { return a.x < b.x; }
	)->x / 2;

	for(size_t cells_in_1st_row = 2; cells_in_1st_row <= max_cols; cells_in_1st_row++) {
		int row_min_width = std::accumulate(item_sizes.begin(), item_sizes.begin() + cells_in_1st_row, 0,
			[](int a, point b) { return a + b.x; }
		);

		int row_max_width = row_min_width + max_xtra;
		int row = 0;

		point row_size, total_size;

		for(size_t n = 0; n < item_sizes.size(); n++) {
			if(row_size.x + item_sizes[n].x > row_max_width) {
				// Start new row
				row++;

				total_size.y += row_size.y;

				if(total_size.x < row_size.x) {
					total_size.x = row_size.x;
				}

				row_size = point();
			}

			row_size.x += item_sizes[n].x;

			if(row_size.y < item_sizes[n].y) {
				row_size.y = item_sizes[n].y;
			}
		}

		total_size.y += row_size.y;

		if(total_size.x < row_size.x) {
			total_size.x = row_size.x;
		}

		best_sizes.push_back(total_size);
	}

	return *std::min_element(best_sizes.begin(), best_sizes.end(), [](point p1, point p2) {
		return
			std::max<double>(p1.x, p1.y) / std::min<double>(p1.x, p1.y) <
			std::max<double>(p2.x, p2.y) / std::min<double>(p2.x, p2.y);
	});
}

void table::place(const point& origin, const point& size)
{
	/*
	 * - Set every item to its best size.
	 * - The origin gets increased with the height of the last item.
	 * - No item should be wider as the size.
	 * - In the end the origin should be the sum of the origin and the wanted
	 *   height.
	 */

	// TODO: Make sure all cells in a row are the same height
	point current_origin = origin;
	int row_height = 0;
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		point best_size = grid.get_best_size();
		// FIXME should we look at grow factors???

		if(current_origin.x + best_size.x > origin.x + size.x) {
			current_origin.x = origin.x;
			current_origin.y += row_height;
			row_height = 0;
		}

		grid.place(current_origin, best_size);

		current_origin.x += best_size.x;
		if(best_size.y > row_height) {
			row_height = best_size.y;
		}
	}

	// TODO: If size is wider than best_size, the matrix will take too much vertical space.
	// This block is supposed to correct for that, but doesn't work properly.
	// To be more specific, it requires invalidating the layout to take effect.
	if(current_origin.y + row_height != origin.y + size.y) {
		point better_size = size;
		better_size.y -= current_origin.y + row_height - origin.y;
		set_layout_size(better_size);
	}
}

void table::set_origin(const point& origin)
{
	point current_origin = origin;
	size_t row_height = 0;
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		grid& grid = item_ordered(i);
		if(current_origin.x + grid.get_width() > origin.x + get_width()) {
			current_origin.x = origin.x;
			current_origin.y += row_height;
			row_height = 0;
		}

		grid.set_origin(current_origin);

		current_origin.x += grid.get_width();
		if(grid.get_height() > row_height) {
			row_height = grid.get_height();
		}
	}
}

void table::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Note for most implementations this function could work only for the
	 * independent class it probably fails. Evaluate to make a generic
	 * function in the generator template class and call it from the wanted
	 * placement functions.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

widget* table::find_at(const point& coordinate, const bool must_be_active)
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}
	return nullptr;
}

const widget* table::find_at(const point& coordinate, const bool must_be_active) const
{
	assert(get_window());

	for(size_t i = 0; i < get_item_count(); ++i) {
		if(!get_item_shown(i)) {
			continue;
		}

		const widget* widget = item(i).find_at(coordinate, must_be_active);

		if(widget) {
			return widget;
		}
	}

	return nullptr;
}

void table::handle_key_up_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(int i = get_ordered_index(get_item_count() - 1); i >= 0; i--) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}

		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_ordered_index(get_selected_item()) - 1; i >= 0; --i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item_ordered(i).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void table::handle_key_down_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(size_t i = get_ordered_index(0); i < get_item_count(); i++) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}

		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_ordered_index(get_selected_item()) + 1; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item_ordered(i).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void table::handle_key_left_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(int i = get_ordered_index(get_item_count() - 1); i >= 0; i--) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}

		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(int i = get_ordered_index(get_selected_item()) - 1; i >= 0; --i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item(get_item_at_ordered(i)).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void table::handle_key_right_arrow(SDL_Keymod /*modifier*/, bool& handled)
{
	if(get_item_count() == 0) {
		return;
	}

	if(get_selected_item_count() == 0) {
		for(size_t i = get_ordered_index(0); i < get_item_count(); i++) {
			if(get_item_shown(get_item_at_ordered(i))) {
				// TODO: Check if active?
				handled = true;
				select_item(get_item_at_ordered(i), true);
				break;
			}
		}

		return;
	}

	// NOTE maybe this should only work if we can select only one item...
	handled = true;

	for(size_t i = get_ordered_index(get_selected_item()) + 1; i < get_item_count(); ++i) {
		if(!get_item_shown(get_item_at_ordered(i))) {
			continue;
		}

		// NOTE we check the first widget to be active since grids have no
		// active flag. This method might not be entirely reliable.
		styled_widget* control = dynamic_cast<styled_widget*>(item(get_item_at_ordered(i)).get_widget(0, 0));
		if(control && control->get_active()) {
			select_item(get_item_at_ordered(i), true);
			return;
		}
	}
}

void independent::request_reduce_width(const unsigned maximum_width)
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.request_reduce_width(maximum_width);
	}
}

void independent::request_reduce_height(const unsigned maximum_height)
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.request_reduce_height(maximum_height);
	}
}

point independent::calculate_best_size() const
{
	/*
	 * The best size is the combination of the greatest width and greatest
	 * height.
	 */
	point result(0, 0);

	for(size_t i = 0; i < get_item_count(); ++i) {
		const grid& grid = item(i);

		const point best_size = grid.get_best_size();

		if(best_size.x > result.x) {
			result.x = best_size.x;
		}

		if(best_size.y > result.y) {
			result.y = best_size.y;
		}
	}

	return result;
}

void independent::place(const point& origin, const point& size)
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.place(origin, size);
	}
}

void independent::set_origin(const point& origin)
{
	/*
	 * Set the origin for every item.
	 *
	 * @todo evaluate whether setting it only for the visible item is better
	 * and what the consequences are.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.set_origin(origin);
	}
}

widget* independent::find_at(const point& coordinate, const bool must_be_active)
{
	assert(get_window());

	const int selected_item = get_selected_item();
	if(selected_item < 0) {
		return nullptr;
	}

	grid& grid = item(selected_item);
	return grid.find_at(coordinate, must_be_active);
}

const widget* independent::find_at(const point& coordinate, const bool must_be_active) const
{
	assert(get_window());

	const int selected_item = get_selected_item();
	if(selected_item < 0) {
		return nullptr;
	}

	const grid& grid = item(selected_item);
	return grid.find_at(coordinate, must_be_active);
}

widget* independent::find(const std::string& id, const bool must_be_active)
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(is_selected(i)) {
			if(widget* widget = item(i).find(id, must_be_active)) {
				return widget;
			}
		}
	}

	return nullptr;
}

const widget* independent::find(const std::string& id, const bool must_be_active) const
{
	for(size_t i = 0; i < get_item_count(); ++i) {
		if(is_selected(i)) {
			if(const widget* widget = item(i).find(id, must_be_active)) {
				return widget;
			}
		}
	}

	return nullptr;
}

void independent::set_visible_rectangle(const SDL_Rect& rectangle)
{
	/*
	 * Set the visible rectangle for every item.
	 *
	 * @todo evaluate whether setting it only for the visible item is better
	 * and what the consequences are.
	 */
	for(size_t i = 0; i < get_item_count(); ++i) {
		grid& grid = item(i);
		grid.set_visible_rectangle(rectangle);
	}
}

} // namespace placement

/***** ***** ***** ***** Select action ***** ***** ***** *****/

namespace select_action
{
void selection::select(grid& grid, const bool select)
{
	selectable_item* selectable = dynamic_cast<selectable_item*>(grid.get_widget(0, 0));
	assert(selectable);

	selectable->set_value(select);
}

void selection::init(grid* g,
		const std::map<std::string /* widget id */, string_map>& data,
		const std::function<void(widget&)>& callback)
{
	for(unsigned row = 0; row < g->get_rows(); ++row) {
		for(unsigned col = 0; col < g->get_cols(); ++col) {
			widget* widget = g->get_widget(row, col);
			assert(widget);

			grid* child_grid = dynamic_cast<grid*>(widget);
			toggle_button* btn = dynamic_cast<toggle_button*>(widget);
			toggle_panel* panel = dynamic_cast<toggle_panel*>(widget);

			if(btn) {
				connect_signal_notify_modified(*btn, std::bind(callback, _1));

				std::map<std::string, string_map>::const_iterator itor = data.find(btn->id());

				if(itor == data.end()) {
					itor = data.find("");
				}
				if(itor != data.end()) {
					btn->set_members(itor->second);
				}
			} else if(panel) {
				connect_signal_notify_modified(*panel, std::bind(callback, _1));

				panel->set_child_members(data);
			} else if(child_grid) {
				init(child_grid, data, callback);
			} else {
				FAIL("Only toggle buttons and panels are allowed as the cells of a list definition.");
			}
		}
	}
}

void show::init(grid* grid,
		const std::map<std::string /* widget id */, string_map>& data,
		const std::function<void(widget&)>& callback)
{
	assert(!callback);

	for(const auto& item : data) {
		if(item.first.empty()) {
			for(unsigned row = 0; row < grid->get_rows(); ++row) {
				for(unsigned col = 0; col < grid->get_cols(); ++col) {
					if(styled_widget* control = dynamic_cast<styled_widget*>(grid->get_widget(row, col))) {
						control->set_members(item.second);
					}
				}
			}
		} else {
			styled_widget* control = dynamic_cast<styled_widget*>(grid->find(item.first, false));
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
static_assert(false, "GUI2/Generator: GENERATE_PLACEMENT already defined!");
#else
#define GENERATE_PLACEMENT                                                                                             \
	switch(placement) {                                                                                                \
	case generator_base::horizontal_list:                                                                              \
		result = new generator<minimum, maximum, policy::placement::horizontal_list, select_action>;                   \
		break;                                                                                                         \
	case generator_base::vertical_list:                                                                                \
		result = new generator<minimum, maximum, policy::placement::vertical_list, select_action>;                     \
		break;                                                                                                         \
	case generator_base::table:                                                                                        \
		result = new generator<minimum, maximum, policy::placement::table, select_action>;                             \
		break;                                                                                                         \
	case generator_base::independent:                                                                                  \
		result = new generator<minimum, maximum, policy::placement::independent, select_action>;                       \
		break;                                                                                                         \
	default:                                                                                                           \
		assert(false);                                                                                                 \
	}
#endif

#ifdef GENERATE_SELECT
static_assert(false, "GUI2/Generator: GENERATE_SELECT already defined!");
#else
#define GENERATE_SELECT                                                                                                \
	if(select) {                                                                                                       \
		typedef policy::select_action::selection select_action;                                                        \
		GENERATE_PLACEMENT                                                                                             \
	} else {                                                                                                           \
		typedef policy::select_action::show select_action;                                                             \
		GENERATE_PLACEMENT                                                                                             \
	}
#endif

#ifdef GENERATE_MAXIMUM
static_assert(false, "GUI2/Generator: GENERATE_MAXIMUM already defined!");
#else
#define GENERATE_MAXIMUM                                                                                               \
	if(has_maximum) {                                                                                                  \
		typedef policy::maximum_selection::one_item maximum;                                                           \
		GENERATE_SELECT                                                                                                \
	} else {                                                                                                           \
		typedef policy::maximum_selection::many_items maximum;                                                         \
		GENERATE_SELECT                                                                                                \
	}
#endif

#ifdef GENERATE_BODY
static_assert(false, "GUI2/Generator: GENERATE_BODY already defined!");
#else
#define GENERATE_BODY                                                                                                  \
	if(has_minimum) {                                                                                                  \
		typedef policy::minimum_selection::one_item minimum;                                                           \
		GENERATE_MAXIMUM                                                                                               \
	} else {                                                                                                           \
		typedef policy::minimum_selection::no_item minimum;                                                            \
		GENERATE_MAXIMUM                                                                                               \
	}
#endif

generator_base* generator_base::build(
		const bool has_minimum, const bool has_maximum, const placement placement, const bool select)
{
	generator_base* result = nullptr;
	GENERATE_BODY;
	return result;
}

/***** ***** ***** ***** Test code ***** ***** ***** *****/
#if 0
namespace {

void pointer_test()
{
	generator_base *a = generator_base::build(
			true, true, generator_base::horizontal_list, true);

	generator_base *b = generator_base::build(
			true, false, generator_base::horizontal_list, true);

	generator_base *c = generator_base::build(
			false, true, generator_base::horizontal_list, true);

	generator_base *d = generator_base::build(
			false, false, generator_base::horizontal_list, true);

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
	generator
		< policy::minimum_selection::one_item
		, policy::maximum_selection::one_item
		, policy::placement::vertical_list
		, policy::select_action::selection
		> a;

	generator
		< policy::minimum_selection::one_item
		, policy::maximum_selection::many_items
		, policy::placement::vertical_list
		, policy::select_action::selection
		> b;

	generator
		< policy::minimum_selection::no_item
		, policy::maximum_selection::one_item
		, policy::placement::vertical_list
		, policy::select_action::selection
		> c;

	generator
		< policy::minimum_selection::no_item
		, policy::maximum_selection::many_items
		, policy::placement::vertical_list
		, policy::select_action::selection
		> d;

	a.clear();
	b.clear();
	c.clear();
	d.clear();
}

} // namespace
#endif

} // namespace gui2
