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

#include "gui/dialogs/dialog.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class display;

namespace gui2
{

class ttext_;

class tunit_list : public tdialog
{
	typedef std::vector<unit_const_ptr> unit_ptr_vector;

public:
	explicit tunit_list(const display& gui);

	map_location get_location_to_scroll_to() const
	{
		return location_of_selected_unit_;
	}

private:
	unit_ptr_vector unit_list_;

	map_location location_of_selected_unit_;

	/** Callbacks */
	void list_item_clicked(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_LIST_HPP_INCLUDED */
