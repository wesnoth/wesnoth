/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_LIST_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_LIST_HPP_INCLUDED

#include "gettext.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class display;

namespace gui2
{

using unit_ptr_vector = std::vector<unit_const_ptr>;

void show_unit_list(display& gui);

class tunit_list : public tdialog
{
public:
	explicit tunit_list(unit_ptr_vector& unit_list, map_location& scroll_to);

	static bool execute(unit_ptr_vector& unit_list, map_location& scroll_to, CVideo& video)
	{
		if(unit_list.empty()) {
			show_transient_message(video, "", _("No units found."));
			return false;
		}

		return tunit_list(unit_list, scroll_to).show(video);
	}

private:
	unit_ptr_vector& unit_list_;

	map_location& scroll_to_;

	/** Callbacks */
	void list_item_clicked(window& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(window& window);

	/** Inherited from tdialog. */
	void post_show(window& window);
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_LIST_HPP_INCLUDED */
