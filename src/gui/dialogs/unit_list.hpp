/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class display;

namespace gui2
{
namespace dialogs
{

using unit_ptr_vector = std::vector<unit_const_ptr>;

void show_unit_list(display& gui);

class unit_list : public modal_dialog
{
public:
	explicit unit_list(unit_ptr_vector& unit_list, map_location& scroll_to);

	static bool execute(unit_ptr_vector& units, map_location& scroll_to)
	{
		if(units.empty()) {
			show_transient_message("", _("No units found."));
			return false;
		}

		return unit_list(units, scroll_to).show();
	}

private:
	unit_ptr_vector& unit_list_;

	map_location& scroll_to_;

	/** Callbacks */
	void list_item_clicked(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
