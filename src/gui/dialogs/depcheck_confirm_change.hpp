/*
   Copyright (C) 2012 - 2017 by Boldizsár Lipka <lipkab@zoho.com>
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
#include <string>

namespace gui2
{
namespace dialogs
{

class depcheck_confirm_change : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param action 		true if the listed modifications are to be enabled,
	 * 						false if they're to be disabled
	 * @param mods 			the names of the affected modifications
	 * @param requester 	the name of the component which requests the change
	 *
	 */
	depcheck_confirm_change(bool action,
								const std::vector<std::string>& mods,
								const std::string& requester);

protected:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};

} // namespace dialogs
} // namespace gui2
