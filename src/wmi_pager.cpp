/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#include "wmi_pager.hpp"
#include "global.hpp"

#include "config.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/wmi_container.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"

#include <algorithm> //std::transform
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <iterator> //std::advance
#include <string>
#include <vector>

struct map_location;

static const char * next_id = "__wml_items_next_page";
static const char * prev_id = "__wml_items_prev_page";

static void add_next_page_item( std::vector<boost::shared_ptr<const game_events::wml_menu_item> > & items,
               std::vector<std::string> & descriptions)
{
	std::string desc = _("More Items");
	config temp;
	temp["description"] = desc;
	items.push_back(boost::make_shared<const game_events::wml_menu_item>(next_id, temp));
	descriptions.push_back(desc);
}

static void add_prev_page_item( std::vector<boost::shared_ptr<const game_events::wml_menu_item> > & items,
               std::vector<std::string> & descriptions)
{
	std::string desc = _("Previous Items");
	config temp;
	temp["description"] = desc;
	items.push_back(boost::make_shared<const game_events::wml_menu_item>(prev_id, temp));
	descriptions.push_back(desc);
}

bool wmi_pager::capture ( const game_events::wml_menu_item & item )
{
	if (item.id() == next_id) {
		page_num_++;
		return true;
	} else if (item.id() == prev_id) {
		page_num_--;
		return true;
	}
	return false;
}

typedef boost::shared_ptr<const game_events::wml_menu_item> wmi_ptr;
typedef std::pair<wmi_ptr, std::string> wmi_pair;
typedef std::vector<wmi_pair>::iterator wmi_it;

static wmi_ptr select_first(const wmi_pair & p)
{
	return p.first;
}

static std::string select_second(const wmi_pair & p)
{
	return p.second;
}

void wmi_pager::get_items(const map_location& hex,
		game_data & gamedata, filter_context & fc, unit_map & units,
               std::vector<wmi_ptr > & items,
               std::vector<std::string> & descriptions)
{
	if (!wmi_container_) {
		return;
	}

	const int page_size_int = preferences::max_wml_menu_items();

	assert(page_size_int >= 0 && "max wml menu items cannot be negative, this indicates preferences corruption");

	const size_t page_size = page_size_int;

	assert(page_size > 2u && "if we dont have at least 3 items, we can't display anything on a middle page...");

	std::vector<wmi_pair > new_items = wmi_container_->get_items(hex, gamedata, fc, units);

	if (new_items.size() <= page_size) { //In this case the first page is sufficient and we don't have to do anything.
		std::transform(new_items.begin(), new_items.end(), back_inserter(items), select_first);
		std::transform(new_items.begin(), new_items.end(), back_inserter(descriptions), select_second);

		page_num_ = 0; //reset page num in case there are more items later.
		return;
	}

	if (page_num_ < 0) //should never happen but maybe some wierd gui thing happens idk
	{
		page_num_ = 0;
	}

	if (page_num_ == 0) { //we are on the first page, so show page_size-1 items and a next button
		wmi_it end_first_page = new_items.begin();
		std::advance(end_first_page, page_size - 1);

		std::transform(new_items.begin(), end_first_page, back_inserter(items), select_first);
		std::transform(new_items.begin(), end_first_page, back_inserter(descriptions), select_second);

		add_next_page_item(items, descriptions);
		return;
	}

	add_prev_page_item(items, descriptions); //this will be necessary since we aren't on the first page

	// first page has page_size - 1.
	// last page has page_size - 1.
	// all other pages have page_size - 2;

	size_t first_displayed_index = (page_size - 2) * page_num_ + 1; //this is the 0-based index of the first item displayed on this page.
									//alternatively, the number of items displayed on earlier pages

	while (first_displayed_index >= new_items.size())
	{
		page_num_--; //The list must have gotten shorter and our page counter is now off the end, so decrement
		first_displayed_index = (page_size - 2) * page_num_ + 1; //recalculate
	}
	// ^ This loop terminates with first_displayed_index > 0, because new_items.size() > page_size or else we exited earlier, and we only decrease by (page_size-2) each time.

	if (first_displayed_index + page_size-1 >= new_items.size()) //if this can be the last page, then we won't put next page at the bottom.
	{
		//The last page we treat differently -- we always want to display (page_size) entries, to prevent resizing the context menu, so count back from end.
		wmi_it end_range = new_items.end(); // It doesn't really matter if we display some entries that appeared on the previous page by doing this.
		wmi_it start_range = end_range;
		std::advance(start_range, -static_cast<signed int>(page_size-1));

		std::transform(start_range, end_range, back_inserter(items), select_first);
		std::transform(start_range, end_range, back_inserter(descriptions), select_second);
		return;
	} else { //we are in a middle page
		wmi_it start_range = new_items.begin();
		std::advance(start_range, first_displayed_index); // <-- get an iterator to the start of our range. begin() + n doesn't work because map is not random access

		wmi_it end_range = start_range;
		std::advance(end_range, page_size-2);

		std::transform(start_range, end_range, back_inserter(items), select_first);
		std::transform(start_range, end_range, back_inserter(descriptions), select_second);

		add_next_page_item(items, descriptions);
		return;
	}
}
