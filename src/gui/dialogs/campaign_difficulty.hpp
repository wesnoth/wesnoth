/*
   Copyright (C) 2010 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "config.hpp"
#include "gui/dialogs/modal_dialog.hpp"

#include <vector>

namespace gui2
{
namespace dialogs
{

/**
 * Helper function to convert old difficulty markup. Declared outside class to allow other
 * classes to make use of it.
 */
config generate_difficulty_config(const config& source);

class campaign_difficulty : public modal_dialog
{
public:
	/**
	 * @param config of the campaign difficulty is being chosen for
	 */
	campaign_difficulty(const config& campaign);

	/**
	 * Returns the selected difficulty define after displaying.
	 * @return 'CANCEL' if the dialog was canceled.
	 */
	std::string selected_difficulty() const
	{
		return selected_difficulty_;
	}

private:
	config difficulties_;
	std::string campaign_id_;
	std::string selected_difficulty_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};
} // namespace dialogs
} // namespace gui2
