/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

namespace gui2::dialogs
{
void show_unit_list(display& gui);

class unit_list : public modal_dialog
{
public:
	explicit unit_list(std::vector<unit_const_ptr>& unit_list, map_location& scroll_to);

	static bool execute(std::vector<unit_const_ptr>& units, map_location& scroll_to)
	{
		if(units.empty()) {
			show_transient_message("", _("No units found."));
			return false;
		}

		return unit_list(units, scroll_to).show();
	}

private:
	std::vector<unit_const_ptr>& unit_list_;

	map_location& scroll_to_;

	/** Callbacks */
	void list_item_clicked();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
