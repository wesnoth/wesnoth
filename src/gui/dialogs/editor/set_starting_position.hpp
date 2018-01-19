/*
   Copyright (C) 2011 - 2018 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

#include <vector>

struct map_location;

namespace gui2
{
namespace dialogs
{

class editor_set_starting_position : public modal_dialog
{
public:
	editor_set_starting_position(
			unsigned current_player,
			unsigned maximum_players,
			const std::vector<map_location>& starting_positions);

	unsigned result() const
	{
		return selection_;
	}

private:
	unsigned selection_;
	std::vector<map_location> starting_positions_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};
} // namespace dialogs
} // namespace gui2
