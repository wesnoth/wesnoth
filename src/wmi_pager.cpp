/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
#include "gettext.hpp"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
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
	std::string desc = _("Earlier Items");
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

typedef game_events::wmi_container::const_iterator wmi_it;

void wmi_pager::get_items(const map_location& hex,
               std::vector<boost::shared_ptr<const game_events::wml_menu_item> > & items,
               std::vector<std::string> & descriptions)
{
	if (!foo_) {
		return;
	}

	assert(page_size_ > 2u); //if we dont have at least 3 items, we can't display anything...

	if (foo_->size() <= page_size_) { //In this case the first page is sufficient and we don't have to do anything.
		foo_->get_items(hex, items, descriptions);
		page_num_ = 0; //reset page num in case there are more items later.
		return;
	}

	if (page_num_ < 0) //should never happen but maybe some wierd gui thing happens idk
	{
		page_num_ = 0;
	}

	if (page_num_ == 0) { //we are on the first page, so show page_size_-1 items and a next button
		wmi_it end_first_page = foo_->begin();
		std::advance(end_first_page, page_size_ - 1);
	
		foo_->get_items(hex, items, descriptions, foo_->begin(), end_first_page);
		add_next_page_item(items, descriptions);
		return;
	}

	add_prev_page_item(items, descriptions); //this will be necessary since we aren't on the first page

	// first page has page_size_ - 1.
	// last page has page_size_ - 1.
	// all other pages have page_size_ - 2;

	size_t first_displayed_index = (page_size_ - 2) * page_num_ + 1; //this is the 0-based index of the first item displayed on this page.
									//alternatively, the number of items displayed on earlier pages

	while (first_displayed_index >= foo_->size())
	{
		page_num_--; //The list must have gotten shorter and our page counter is now off the end, so decrement
		first_displayed_index = (page_size_ - 2) * page_num_ + 1; //recalculate
	}
	// ^ This loop terminates with first_displayed_index > 0, because foo_->size() > page_size_ or else we exited earlier, and we only decrease by (page_size_-2) each time.

	wmi_it start_range = foo_->begin();
	std::advance(start_range, first_displayed_index); // <-- get an iterator to the start of our range. begin() + n doesn't work because map is not random access
	//^  = foo_->begin() + first_displayed_index

	if (first_displayed_index + page_size_-1 >= foo_->size()) //if this can be the last page, then we won't put next page at the bottom.
	{
		foo_->get_items(hex, items, descriptions, start_range, foo_->end()); // display all of the remaining items
		return;
	} else { //we are in a middle page
		wmi_it end_range = start_range;
		std::advance(end_range, page_size_-2);

		foo_->get_items(hex, items, descriptions, start_range, end_range);
		add_next_page_item(items, descriptions);
		return;
	}
}
