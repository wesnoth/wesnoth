/*
   Copyright (C) 2015 - 2016 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef WIDGETS_MULTIMENU_HPP_INCLUDED
#define WIDGETS_MULTIMENU_HPP_INCLUDED

#include "menu.hpp"

namespace gui {

	/**
	 * A widget that additionally to the normal menu contents also displays a
	 * checkbox in each row. An item is considered 'active' if its checkbox is checked,
	 * 'inactive' otherwise.
	 */
	class multimenu : public menu {
	public:
		multimenu(CVideo& video, const std::vector<std::string>& items,
				  bool click_selects=false, int max_height=-1, int max_width=-1,
				  const sorter* sorter_obj=NULL, style *menu_style=NULL, const bool auto_join=true);


		void set_active(size_t index, bool active=true);

		/**
		 * Tells if an item is activated.
		 *
		 * @param index the item's index
		 * @return true if the item is active, false if not
		 */
		bool is_active(size_t index) const { return active_items_[index]; }

		/**
		 * Tell each items status
		 *
		 * @return a vector of bools. The n-th item is active if the vector's n-th item is true
		 */
		const std::vector<bool> &active_items() const { return active_items_; }

		virtual void erase_item(size_t index);
		virtual void set_items(const std::vector<std::string>& items, bool strip_spaces=true,
							   bool keep_viewport=false);

		/**
		 * Returns the item last activated/deactivated. Items changed via set_active
		 * don't count, Invoking this function will set the last changed index to -1.
		 *
		 * @return the item's index, or -1 if no item was changed since the last
		 *         invokation
		 */
		int last_changed();

	protected:
		virtual void draw_row(const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void handle_event(const SDL_Event& event);

		/**
		 * Determine which checkbox was hit by a mouse click.
		 *
		 * @param x the x coordinate of the click
		 * @param y the y coordinate of the click
		 * @return the row whose checkbox was clicked on, or -1 if the click didn't hit a checkbox
		 */
		int hit_checkbox(int x, int y) const;

	private:
		std::vector<bool> active_items_;
		int last_changed_;
	};
}

#endif
