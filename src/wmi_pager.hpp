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


/** This class manages the paging of WML menu items results, from a
 * container. It is an adapter, managing the production of items lists
 * from the container, and screening the "fire" signals coming back
 * in to intercept the paging signals.
 *
 * TODO: Implement this as a helper class for menu perhaps, so that it
 * can interact with the gui layout algorithm.
 */

class filter_context;
class game_data;
class game_state;
struct map_location;
class unit_map;
namespace game_events { class wml_menu_item; }
namespace game_events { class wmi_container; }

#include "global.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

class wmi_pager {
private:
	int page_num_; //!< Current page number
	const game_events::wmi_container * wmi_container_; //!< Internal pointer to the collection of wml menu items

public:
	wmi_pager() : page_num_(0), wmi_container_(NULL) {}

	void update_ref(game_events::wmi_container * ptr) { wmi_container_ = ptr; } //!< Updates the internal wmi_container pointer

	/** Adds the currently paged range of menu items to the given lists */
	void get_items(const map_location& hex, //!< Game hex related to this context menu
		game_data & gamedata, filter_context & fc, unit_map & units, //!< Data needed to create scoped objects when evaluating wml filters
               std::vector<boost::shared_ptr<const game_events::wml_menu_item> > & items, //!< List of accumulated menu items so far.
               std::vector<std::string> & descriptions); //!< List of menu item descriptions

	bool capture(const game_events::wml_menu_item & item); //!< Captures a page up / page down event in the case that it is fired.
};
